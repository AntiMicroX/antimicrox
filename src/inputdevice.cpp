/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "inputdevice.h"

#include "antimicrosettings.h"
#include "common.h"
#include "globalvariables.h"
#include "joybuttontypes/joycontrolstickbutton.h"
#include "joybuttontypes/joydpadbutton.h"
#include "joybuttontypes/joysensorbutton.h"
#include "joycontrolstick.h"
#include "joydpad.h"
#include "joysensor.h"
#include "vdpad.h"

#include <typeinfo>

#include <QDebug>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

InputDevice::InputDevice(SDL_Joystick *joystick, int deviceIndex, AntiMicroSettings *settings, QObject *parent)
    : QObject(parent)
    , m_calibrations(this)
{
    buttonDownCount = 0;
    joyNumber = deviceIndex;
    active_set = 0;
    joystickID = 0;
    keyPressTime = 0;
    m_joyhandle = joystick;
    deviceEdited = false;
    keyRepeatEnabled = false;
    keyRepeatDelay = 0;
    keyRepeatRate = 0;
    rawAxisDeadZone = GlobalVariables::InputDevice::RAISEDDEADZONE;
    m_settings = settings;
}

InputDevice::~InputDevice() {}

int InputDevice::getJoyNumber() { return joyNumber; }

int InputDevice::getRealJoyNumber()
{
    int joynumber = getJoyNumber();
    return joynumber + 1;
}

void InputDevice::reset()
{
    resetButtonDownCount();
    deviceEdited = false;
    profileName = "";

    for (int i = 0; i < GlobalVariables::InputDevice::NUMBER_JOYSETS; i++)
    {
        SetJoystick *set = getJoystick_sets().value(i);
        set->reset();
    }
}

/**
 * @brief Obtain current joystick element values, create new SetJoystick objects,
 *     and then transfer most recent joystick element values to new
 *     current set.
 */
void InputDevice::transferReset()
{ // Grab current states for all elements in old set
    SetJoystick *current_set = getJoystick_sets().value(active_set);
    for (int i = 0; i < current_set->getNumberButtons(); i++)
    {
        JoyButton *button = current_set->getJoyButton(i);
        getButtonstatesLocal().append(button->getButtonState());
    }

    for (int i = 0; i < current_set->getNumberAxes(); i++)
    {
        JoyAxis *axis = current_set->getJoyAxis(i);
        getAxesstatesLocal().append(axis->getCurrentRawValue());
    }

    for (int i = 0; i < current_set->getNumberHats(); i++)
    {
        JoyDPad *dpad = current_set->getJoyDPad(i);
        getDpadstatesLocal().append(dpad->getCurrentDirection());
    }

    reset();
}

void InputDevice::reInitButtons()
{
    SetJoystick *current_set = getJoystick_sets().value(active_set);

    for (int i = 0; i < current_set->getNumberButtons(); i++)
    {
        bool value = getButtonstatesLocal().at(i);
        JoyButton *button = current_set->getJoyButton(i);
        button->queuePendingEvent(value);
    }

    for (int i = 0; i < current_set->getNumberAxes(); i++)
    {
        int value = getAxesstatesLocal().at(i);
        JoyAxis *axis = current_set->getJoyAxis(i);
        axis->queuePendingEvent(value);
    }

    for (int i = 0; i < current_set->getNumberHats(); i++)
    {
        int value = getDpadstatesLocal().at(i);
        JoyDPad *dpad = current_set->getJoyDPad(i);
        dpad->queuePendingEvent(value);
    }

    activatePossibleControlStickEvents();
    activatePossibleAxisEvents();
    activatePossibleDPadEvents();
    activatePossibleVDPadEvents();
    activatePossibleButtonEvents();

    buttonstates.clear();
    axesstates.clear();
    dpadstates.clear();
}

void InputDevice::setActiveSetNumber(int index)
{
    VERBOSE() << "Change active set number from: " << active_set << " to: " << index
              << " for device with Index: " << getRealJoyNumber();
    if (((index >= 0) && (index < GlobalVariables::InputDevice::NUMBER_JOYSETS)) && (index != active_set))
    {
        QList<bool> buttonstates;
        QList<int> axesstates;
        QList<int> dpadstates;
        QList<JoyControlStick::JoyStickDirections> stickstates;
        QList<int> vdpadstates;

        // Grab current states for all elements in old set
        SetJoystick *current_set = getJoystick_sets().value(active_set);
        SetJoystick *old_set = current_set;
        SetJoystick *tempSet = getJoystick_sets().value(index);

        for (int i = 0; i < current_set->getNumberButtons(); i++)
        {
            JoyButton *button = current_set->getJoyButton(i);
            buttonstates.append(button->getButtonState());
            tempSet->getJoyButton(i)->copyLastMouseDistanceFromDeadZone(button);
            tempSet->getJoyButton(i)->copyLastAccelerationDistance(button);
            tempSet->getJoyButton(i)->setUpdateInitAccel(false);
        }

        for (int i = 0; i < current_set->getNumberAxes(); i++)
        {
            JoyAxis *axis = current_set->getJoyAxis(i);
            axesstates.append(axis->getCurrentRawValue());
            tempSet->getJoyAxis(i)->copyRawValues(axis);
            tempSet->getJoyAxis(i)->copyThrottledValues(axis);
            JoyAxisButton *button = tempSet->getJoyAxis(i)->getAxisButtonByValue(axis->getCurrentRawValue());

            if (button != nullptr)
            {
                button->setUpdateInitAccel(false);
            }
        }

        for (int i = 0; i < current_set->getNumberHats(); i++)
        {
            JoyDPad *dpad = current_set->getJoyDPad(i);
            dpadstates.append(dpad->getCurrentDirection());
            JoyDPadButton::JoyDPadDirections tempDir =
                static_cast<JoyDPadButton::JoyDPadDirections>(dpad->getCurrentDirection());
            tempSet->getJoyDPad(i)->setDirButtonsUpdateInitAccel(tempDir, false);
            tempSet->getJoyDPad(i)->copyLastDistanceValues(dpad);
        }

        for (int i = 0; i < current_set->getNumberSticks(); i++)
        {
            // Last distances for elements are taken from associated axes.
            // Copying is not required here.
            JoyControlStick *stick = current_set->getJoyStick(i);
            stickstates.append(stick->getCurrentDirection());
            tempSet->getJoyStick(i)->setDirButtonsUpdateInitAccel(stick->getCurrentDirection(), false);
        }

        for (int i = 0; i < current_set->getNumberVDPads(); i++)
        {
            JoyDPad *dpad = current_set->getVDPad(i);
            vdpadstates.append(dpad->getCurrentDirection());
            JoyDPadButton::JoyDPadDirections tempDir =
                static_cast<JoyDPadButton::JoyDPadDirections>(dpad->getCurrentDirection());
            tempSet->getVDPad(i)->setDirButtonsUpdateInitAccel(tempDir, false);
            tempSet->getVDPad(i)->copyLastDistanceValues(dpad);
        }

        // Release all current pressed elements and change set number
        getJoystick_sets().value(active_set)->release();
        active_set = index;

        // Activate all buttons in the switched set
        current_set = getJoystick_sets().value(active_set);

        for (int i = 0; i < current_set->getNumberSticks(); i++)
        {
            JoyControlStick::JoyStickDirections value = stickstates.at(i);
            QList<JoyControlStickButton *> buttonList;
            QList<JoyControlStickButton *> oldButtonList;
            JoyControlStick *stick = current_set->getJoyStick(i);
            JoyControlStick *oldStick = old_set->getJoyStick(i);

            if ((stick->getJoyMode() == JoyControlStick::StandardMode) && value)
            {
                switch (value)
                {
                case JoyControlStick::StickRightUp: {
                    buttonList.append(stick->getDirectionButton(JoyControlStick::StickUp));
                    buttonList.append(stick->getDirectionButton(JoyControlStick::StickRight));
                    oldButtonList.append(oldStick->getDirectionButton(JoyControlStick::StickUp));
                    oldButtonList.append(oldStick->getDirectionButton(JoyControlStick::StickRight));
                    break;
                }
                case JoyControlStick::StickRightDown: {
                    buttonList.append(stick->getDirectionButton(JoyControlStick::StickRight));
                    buttonList.append(stick->getDirectionButton(JoyControlStick::StickDown));
                    oldButtonList.append(oldStick->getDirectionButton(JoyControlStick::StickRight));
                    oldButtonList.append(oldStick->getDirectionButton(JoyControlStick::StickDown));
                    break;
                }
                case JoyControlStick::StickLeftDown: {
                    buttonList.append(stick->getDirectionButton(JoyControlStick::StickDown));
                    buttonList.append(stick->getDirectionButton(JoyControlStick::StickLeft));
                    oldButtonList.append(oldStick->getDirectionButton(JoyControlStick::StickDown));
                    oldButtonList.append(oldStick->getDirectionButton(JoyControlStick::StickLeft));
                    break;
                }
                case JoyControlStick::StickLeftUp: {
                    buttonList.append(stick->getDirectionButton(JoyControlStick::StickLeft));
                    buttonList.append(stick->getDirectionButton(JoyControlStick::StickUp));
                    oldButtonList.append(oldStick->getDirectionButton(JoyControlStick::StickLeft));
                    oldButtonList.append(oldStick->getDirectionButton(JoyControlStick::StickUp));
                    break;
                }
                default: {
                    buttonList.append(stick->getDirectionButton(value));
                    oldButtonList.append(oldStick->getDirectionButton(value));
                }
                }
            } else if (value)
            {
                buttonList.append(stick->getDirectionButton(value));
                oldButtonList.append(oldStick->getDirectionButton(value));
            }

            QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton *> iter(*stick->getButtons());
            while (iter.hasNext())
            {
                JoyControlStickButton *tempButton = iter.next().value();
                if (!buttonList.contains(tempButton))
                {
                    tempButton->setWhileHeldStatus(false);
                }
            }

            for (int j = 0; j < buttonList.size(); j++)
            {
                JoyControlStickButton *button = buttonList.at(j);
                JoyControlStickButton *oldButton = oldButtonList.at(j);

                if ((button != nullptr) && (oldButton != nullptr))
                {
                    if (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld)
                    {
                        if ((oldButton->getChangeSetCondition() == JoyButton::SetChangeWhileHeld) &&
                            oldButton->getWhileHeldStatus())
                        {
                            // Button from old set involved in a while held set
                            // change. Carry over to new set button to ensure
                            // set changes are done in the proper order.
                            button->setWhileHeldStatus(true);
                        } else if (!button->getWhileHeldStatus())
                        {
                            // Ensure that set change events are performed if needed.
                            // tempignore = false;
                        }
                    }
                }
            }
        }

        // Activate all dpad buttons in the switched set
        for (int i = 0; i < current_set->getNumberVDPads(); i++)
        {
            int value = vdpadstates.at(i);
            JoyDPad *dpad = current_set->getVDPad(i);
            QList<JoyDPadButton *> buttonList;
            QList<JoyDPadButton *> oldButtonList;
            bool valueTrue = (value != 0);

            if ((dpad->getJoyMode() == JoyDPad::StandardMode) && valueTrue)
            {
                switch (value)
                {
                case JoyDPadButton::DpadRightUp: {
                    buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadUp));
                    buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadRight));
                    oldButtonList.append(old_set->getVDPad(i)->getJoyButton(JoyDPadButton::DpadUp));
                    oldButtonList.append(old_set->getVDPad(i)->getJoyButton(JoyDPadButton::DpadRight));
                    break;
                }
                case JoyDPadButton::DpadRightDown: {
                    buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadRight));
                    buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadDown));
                    oldButtonList.append(old_set->getVDPad(i)->getJoyButton(JoyDPadButton::DpadRight));
                    oldButtonList.append(old_set->getVDPad(i)->getJoyButton(JoyDPadButton::DpadDown));
                    break;
                }
                case JoyDPadButton::DpadLeftDown: {
                    buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadDown));
                    buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadLeft));
                    oldButtonList.append(old_set->getVDPad(i)->getJoyButton(JoyDPadButton::DpadDown));
                    oldButtonList.append(old_set->getVDPad(i)->getJoyButton(JoyDPadButton::DpadLeft));
                    break;
                }
                case JoyDPadButton::DpadLeftUp: {
                    buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadLeft));
                    buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadUp));
                    oldButtonList.append(old_set->getVDPad(i)->getJoyButton(JoyDPadButton::DpadLeft));
                    oldButtonList.append(old_set->getVDPad(i)->getJoyButton(JoyDPadButton::DpadUp));
                    break;
                }
                default: {
                    buttonList.append(dpad->getJoyButton(value));
                    oldButtonList.append(old_set->getVDPad(i)->getJoyButton(value));
                }
                }
            } else if (valueTrue)
            {
                buttonList.append(dpad->getJoyButton(value));
                oldButtonList.append(old_set->getVDPad(i)->getJoyButton(value));
            }

            QHashIterator<int, JoyDPadButton *> iter(*dpad->getJoyButtons());

            while (iter.hasNext())
            {
                // Ensure that set change events are performed if needed.
                JoyDPadButton *button = iter.next().value();

                if (!buttonList.contains(button))
                {
                    button->setWhileHeldStatus(false);
                }
            }

            for (int j = 0; j < buttonList.size(); j++)
            {
                JoyDPadButton *button = buttonList.at(j);
                JoyDPadButton *oldButton = oldButtonList.at(j);

                if ((button != nullptr) && (oldButton != nullptr) &&
                    (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld) && valueTrue &&
                    (oldButton->getChangeSetCondition() == JoyButton::SetChangeWhileHeld) && oldButton->getWhileHeldStatus())
                {
                    // Button from old set involved in a while held set
                    // change. Carry over to new set button to ensure
                    // set changes are done in the proper order.

                    button->setWhileHeldStatus(true);
                } else if ((button != nullptr) && (oldButton != nullptr) &&
                           (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld) && !valueTrue)
                {
                    button->setWhileHeldStatus(false);
                }
            }
        }

        for (int i = 0; i < current_set->getNumberButtons(); i++)
        {
            bool value = buttonstates.at(i);
            bool tempignore = false;
            JoyButton *button = current_set->getJoyButton(i);
            JoyButton *oldButton = old_set->getJoyButton(i);

            if (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld && value &&
                (oldButton->getChangeSetCondition() == JoyButton::SetChangeWhileHeld) && oldButton->getWhileHeldStatus())
            {
                // Button from old set involved in a while held set
                // change. Carry over to new set button to ensure
                // set changes are done in the proper order.

                button->setWhileHeldStatus(true);
            } else if (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld && value &&
                       !button->getWhileHeldStatus())
            {
                // Ensure that set change events are performed if needed.
                tempignore = false;
            } else if (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld && !value)
            {
                // Ensure that set change events are performed if needed.
                button->setWhileHeldStatus(false);
            }

            button->queuePendingEvent(value, tempignore);
        }

        // Activate all axis buttons in the switched set
        for (int i = 0; i < current_set->getNumberAxes(); i++)
        {
            int value = axesstates.at(i);
            bool tempignore = false;
            JoyAxis *axis = current_set->getJoyAxis(i);
            JoyAxisButton *oldButton = old_set->getJoyAxis(i)->getAxisButtonByValue(value);
            JoyAxisButton *button = axis->getAxisButtonByValue(value);

            if ((button != nullptr) && (oldButton != nullptr) &&
                (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld) &&
                (oldButton->getChangeSetCondition() == JoyButton::SetChangeWhileHeld) && oldButton->getWhileHeldStatus())
            {
                // Button from old set involved in a while held set
                // change. Carry over to new set button to ensure
                // set changes are done in the proper order.

                button->setWhileHeldStatus(true);
            } else if ((button != nullptr) && (oldButton != nullptr) &&
                       (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld) && !button->getWhileHeldStatus())
            {
                // Ensure that set change events are performed if needed.
                tempignore = false;
            } else if (button == nullptr)
            {
                // Ensure that set change events are performed if needed.
                axis->getPAxisButton()->setWhileHeldStatus(false);
                axis->getNAxisButton()->setWhileHeldStatus(false);
            }

            axis->queuePendingEvent(value, tempignore, false);
        }

        // Activate all dpad buttons in the switched set
        for (int i = 0; i < current_set->getNumberHats(); i++)
        {
            int value = dpadstates.at(i);
            bool tempignore = false;
            JoyDPad *dpad = current_set->getJoyDPad(i);
            QList<JoyDPadButton *> buttonList;
            QList<JoyDPadButton *> oldButtonList;
            bool valueTrue = (value != 0);

            if ((dpad->getJoyMode() == JoyDPad::StandardMode) && valueTrue)
            {
                switch (value)
                {
                case JoyDPadButton::DpadRightUp: {
                    buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadUp));
                    buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadRight));
                    oldButtonList.append(old_set->getJoyDPad(i)->getJoyButton(JoyDPadButton::DpadUp));
                    oldButtonList.append(old_set->getJoyDPad(i)->getJoyButton(JoyDPadButton::DpadRight));
                    break;
                }
                case JoyDPadButton::DpadRightDown: {
                    buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadRight));
                    buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadDown));
                    oldButtonList.append(old_set->getJoyDPad(i)->getJoyButton(JoyDPadButton::DpadRight));
                    oldButtonList.append(old_set->getJoyDPad(i)->getJoyButton(JoyDPadButton::DpadDown));
                    break;
                }
                case JoyDPadButton::DpadLeftDown: {
                    buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadDown));
                    buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadLeft));
                    oldButtonList.append(old_set->getJoyDPad(i)->getJoyButton(JoyDPadButton::DpadDown));
                    oldButtonList.append(old_set->getJoyDPad(i)->getJoyButton(JoyDPadButton::DpadLeft));
                    break;
                }
                case JoyDPadButton::DpadLeftUp: {
                    buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadLeft));
                    buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadUp));
                    oldButtonList.append(old_set->getJoyDPad(i)->getJoyButton(JoyDPadButton::DpadLeft));
                    oldButtonList.append(old_set->getJoyDPad(i)->getJoyButton(JoyDPadButton::DpadUp));
                    break;
                }
                default: {
                    buttonList.append(dpad->getJoyButton(value));
                    oldButtonList.append(old_set->getJoyDPad(i)->getJoyButton(value));
                }
                }
            } else if (valueTrue)
            {
                buttonList.append(dpad->getJoyButton(value));
                oldButtonList.append(old_set->getJoyDPad(i)->getJoyButton(value));
            }

            QHashIterator<int, JoyDPadButton *> iter(*dpad->getJoyButtons());

            while (iter.hasNext())
            {
                // Ensure that set change events are performed if needed.
                JoyDPadButton *button = iter.next().value();

                if (!buttonList.contains(button))
                    button->setWhileHeldStatus(false);
            }

            for (int j = 0; j < buttonList.size(); j++)
            {
                JoyDPadButton *button = buttonList.at(j);
                JoyDPadButton *oldButton = oldButtonList.at(j);

                if ((button != nullptr) && (oldButton != nullptr) &&
                    (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld) && valueTrue &&
                    (oldButton->getChangeSetCondition() == JoyButton::SetChangeWhileHeld) && oldButton->getWhileHeldStatus())
                {
                    // Button from old set involved in a while held set
                    // change. Carry over to new set button to ensure
                    // set changes are done in the proper order.

                    button->setWhileHeldStatus(true);
                } else if ((button != nullptr) && (oldButton != nullptr) &&
                           (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld) && valueTrue &&
                           !button->getWhileHeldStatus())
                {
                    // Ensure that set change events are performed if needed.
                    tempignore = false;
                } else if ((button != nullptr) && (oldButton != nullptr) &&
                           (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld) && !valueTrue)
                {
                    button->setWhileHeldStatus(false);
                }
            }

            dpad->queuePendingEvent(value, tempignore);
        }

        activatePossibleControlStickEvents();
        activatePossibleAxisEvents();
        activatePossibleDPadEvents();
        activatePossibleVDPadEvents();
        activatePossibleButtonEvents();
    } else
    {
        DEBUG() << "Set is not changed";
    }
}

int InputDevice::getActiveSetNumber() { return active_set; }

SetJoystick *InputDevice::getActiveSetJoystick() { return getJoystick_sets().value(active_set); }

int InputDevice::getNumberButtons() { return getActiveSetJoystick()->getNumberButtons(); }

int InputDevice::getNumberAxes() { return getActiveSetJoystick()->getNumberAxes(); }

int InputDevice::getNumberHats() { return getActiveSetJoystick()->getNumberHats(); }

int InputDevice::getNumberSticks() { return getActiveSetJoystick()->getNumberSticks(); }

/**
 * @brief Checks if this input device has a sensor of given type
 * @returns True if sensor is present, false otherwise
 */
bool InputDevice::hasSensor(JoySensorType type) { return getActiveSetJoystick()->hasSensor(type); }

int InputDevice::getNumberVDPads() { return getActiveSetJoystick()->getNumberVDPads(); }

SetJoystick *InputDevice::getSetJoystick(int index) { return getJoystick_sets().value(index); }

void InputDevice::propogateSetChange(int index) { emit setChangeActivated(index); }

void InputDevice::changeSetButtonAssociation(int button_index, int originset, int newset, int mode)
{
    JoyButton *button = getJoystick_sets().value(newset)->getJoyButton(button_index);
    JoyButton::SetChangeCondition tempmode = static_cast<JoyButton::SetChangeCondition>(mode);
    button->setChangeSetSelection(originset);
    button->setChangeSetCondition(tempmode, true);
}

void InputDevice::changeSetAxisButtonAssociation(int button_index, int axis_index, int originset, int newset, int mode)
{
    JoyAxisButton *button = nullptr;

    if (button_index == 0)
    {
        button = getJoystick_sets().value(newset)->getJoyAxis(axis_index)->getNAxisButton();
    } else if (button_index == 1)
    {
        button = getJoystick_sets().value(newset)->getJoyAxis(axis_index)->getPAxisButton();
    } else
    {
        WARN() << "Invalid button_index value: " << button_index;
        return;
    }
    if (button != nullptr)
    {
        JoyButton::SetChangeCondition tempmode = static_cast<JoyButton::SetChangeCondition>(mode);
        button->setChangeSetSelection(originset);
        button->setChangeSetCondition(tempmode, true);
    }
}

void InputDevice::changeSetStickButtonAssociation(int button_index, int stick_index, int originset, int newset, int mode)
{
    JoyControlStickButton *button = getJoystick_sets()
                                        .value(newset)
                                        ->getJoyStick(stick_index)
                                        ->getDirectionButton(static_cast<JoyControlStick::JoyStickDirections>(button_index));

    JoyButton::SetChangeCondition tempmode = static_cast<JoyButton::SetChangeCondition>(mode);
    button->setChangeSetSelection(originset);
    button->setChangeSetCondition(tempmode, true);
}

/**
 * @brief Creates reverse set change button mapping for toggle and while-hold set
 *  change mappings.
 */
void InputDevice::changeSetSensorButtonAssociation(JoySensorDirection direction, JoySensorType type, int originset,
                                                   int newset, int mode)
{
    JoySensorButton *button = getJoystick_sets().value(newset)->getSensor(type)->getDirectionButton(direction);

    JoyButton::SetChangeCondition tempmode = static_cast<JoyButton::SetChangeCondition>(mode);
    button->setChangeSetSelection(originset);
    button->setChangeSetCondition(tempmode, true);
}

void InputDevice::changeSetDPadButtonAssociation(int button_index, int dpad_index, int originset, int newset, int mode)
{
    JoyDPadButton *button = getJoystick_sets().value(newset)->getJoyDPad(dpad_index)->getJoyButton(button_index);

    JoyButton::SetChangeCondition tempmode = static_cast<JoyButton::SetChangeCondition>(mode);
    button->setChangeSetSelection(originset);
    button->setChangeSetCondition(tempmode, true);
}

void InputDevice::changeSetVDPadButtonAssociation(int button_index, int dpad_index, int originset, int newset, int mode)
{
    JoyDPadButton *button = getJoystick_sets().value(newset)->getVDPad(dpad_index)->getJoyButton(button_index);

    JoyButton::SetChangeCondition tempmode = static_cast<JoyButton::SetChangeCondition>(mode);
    button->setChangeSetSelection(originset);
    button->setChangeSetCondition(tempmode, true);
}

void InputDevice::propogateSetAxisThrottleChange(int index, int originset)
{
    SetJoystick *currentSet = getJoystick_sets().value(originset);

    if (currentSet != nullptr)
    {
        JoyAxis *axis = currentSet->getJoyAxis(index);

        if (axis != nullptr)
        {
            int throttleSetting = axis->getThrottle();

            for (auto &temp : getJoystick_sets())
            {
                // Ignore change for set axis that initiated the change
                if (temp != currentSet)
                    temp->getJoyAxis(index)->setThrottle(throttleSetting);
            }
        }
    }
}

void InputDevice::removeControlStick(int index)
{
    for (int i = 0; i < GlobalVariables::InputDevice::NUMBER_JOYSETS; i++)
    {
        SetJoystick *currentset = getSetJoystick(i);

        if (currentset->getJoyStick(index))
            currentset->removeControlStick(index);
    }
}

bool InputDevice::isActive() { return buttonDownCount > 0; }

void InputDevice::buttonDownEvent(int setindex, int buttonindex)
{
    Q_UNUSED(setindex);
    Q_UNUSED(buttonindex);

    bool old = isActive();
    buttonDownCount += 1;

    if (isActive() != old)
        emit clicked(joyNumber);
}

void InputDevice::buttonUpEvent(int setindex, int buttonindex)
{
    Q_UNUSED(setindex);
    Q_UNUSED(buttonindex);

    bool old = isActive();
    buttonDownCount -= 1;

    if (buttonDownCount < 0)
        buttonDownCount = 0;

    if (isActive() != old)
        emit released(joyNumber);
}

void InputDevice::buttonClickEvent(int buttonindex) { emit rawButtonClick(buttonindex); }

void InputDevice::buttonReleaseEvent(int buttonindex) { emit rawButtonRelease(buttonindex); }

void InputDevice::axisButtonDownEvent(int setindex, int axisindex, int buttonindex)
{
    Q_UNUSED(axisindex);

    buttonDownEvent(setindex, buttonindex);
}

void InputDevice::axisButtonUpEvent(int setindex, int axisindex, int buttonindex)
{
    Q_UNUSED(axisindex);

    buttonUpEvent(setindex, buttonindex);
}

void InputDevice::dpadButtonClickEvent(int buttonindex)
{
    JoyDPadButton *dpadbutton = qobject_cast<JoyDPadButton *>(sender());

    if (dpadbutton != nullptr)
        emit rawDPadButtonClick(dpadbutton->getDPad()->getIndex(), buttonindex);
}

void InputDevice::dpadButtonReleaseEvent(int buttonindex)
{
    JoyDPadButton *dpadbutton = qobject_cast<JoyDPadButton *>(sender());

    if (dpadbutton != nullptr)
        emit rawDPadButtonRelease(dpadbutton->getDPad()->getIndex(), buttonindex);
}

void InputDevice::dpadButtonDownEvent(int setindex, int dpadindex, int buttonindex)
{
    Q_UNUSED(dpadindex);

    buttonDownEvent(setindex, buttonindex);
}

void InputDevice::dpadButtonUpEvent(int setindex, int dpadindex, int buttonindex)
{
    Q_UNUSED(dpadindex);

    buttonUpEvent(setindex, buttonindex);
}

void InputDevice::stickButtonDownEvent(int setindex, int stickindex, int buttonindex)
{
    Q_UNUSED(stickindex);

    buttonDownEvent(setindex, buttonindex);
}

void InputDevice::stickButtonUpEvent(int setindex, int stickindex, int buttonindex)
{
    Q_UNUSED(stickindex);

    buttonUpEvent(setindex, buttonindex);
}

void InputDevice::sensorButtonDownEvent(int setindex, JoySensorType type, JoySensorDirection direction)
{
    Q_UNUSED(type);

    buttonDownEvent(setindex, direction);
}

void InputDevice::sensorButtonUpEvent(int setindex, JoySensorType type, JoySensorDirection direction)
{
    Q_UNUSED(type);

    buttonUpEvent(setindex, direction);
}

void InputDevice::setButtonName(int index, QString tempName)
{
    for (auto &tempSet : getJoystick_sets())
    {
        disconnect(tempSet, &SetJoystick::setButtonNameChange, this, &InputDevice::updateSetButtonNames);
        JoyButton *button = tempSet->getJoyButton(index);

        if (button != nullptr)
            button->setButtonName(tempName);

        connect(tempSet, &SetJoystick::setButtonNameChange, this, &InputDevice::updateSetButtonNames);
    }
}

void InputDevice::setAxisButtonName(int axisIndex, int buttonIndex, QString tempName)
{
    for (auto &tempSet : getJoystick_sets())
    {
        disconnect(tempSet, &SetJoystick::setAxisButtonNameChange, this, &InputDevice::updateSetAxisButtonNames);
        JoyAxis *axis = tempSet->getJoyAxis(axisIndex);

        if (axis != nullptr)
        {
            JoyAxisButton *button = nullptr;

            if (buttonIndex == 0)
                button = axis->getNAxisButton();
            else if (buttonIndex == 1)
                button = axis->getPAxisButton();

            if (button != nullptr)
                button->setButtonName(tempName);
        }

        connect(tempSet, &SetJoystick::setAxisButtonNameChange, this, &InputDevice::updateSetAxisButtonNames);
    }
}

void InputDevice::setStickButtonName(int stickIndex, int buttonIndex, QString tempName)
{
    for (auto &tempSet : getJoystick_sets())
    {
        disconnect(tempSet, &SetJoystick::setStickButtonNameChange, this, &InputDevice::updateSetStickButtonNames);
        JoyControlStick *stick = tempSet->getJoyStick(stickIndex);

        if (stick != nullptr)
        {
            JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::JoyStickDirections(buttonIndex));

            if (button != nullptr)
                button->setButtonName(tempName);
        }

        connect(tempSet, &SetJoystick::setStickButtonNameChange, this, &InputDevice::updateSetStickButtonNames);
    }
}

/**
 * @brief Sets the name of a mapped sensor button in all sets
 *  Used during XML loading.
 * @param type The sensor type which has the to be renamed button
 * @param direction The direction of the to be renamed button
 * @param tempName The new name
 */
void InputDevice::setSensorButtonName(JoySensorType type, JoySensorDirection direction, QString tempName)
{
    auto sets = getJoystick_sets();
    for (auto &tempSet : sets)
    {
        disconnect(tempSet, &SetJoystick::setStickButtonNameChange, this, &InputDevice::updateSetStickButtonNames);
        JoySensor *sensor = tempSet->getSensor(type);

        if (sensor != nullptr)
        {
            JoySensorButton *button = sensor->getDirectionButton(direction);

            if (button != nullptr)
                button->setButtonName(tempName);
        }

        connect(tempSet, &SetJoystick::setSensorButtonNameChange, this, &InputDevice::updateSetSensorButtonNames);
    }
}
void InputDevice::setDPadButtonName(int dpadIndex, int buttonIndex, QString tempName)
{
    for (auto &tempSet : getJoystick_sets())
    {
        disconnect(tempSet, &SetJoystick::setDPadButtonNameChange, this, &InputDevice::updateSetDPadButtonNames);
        JoyDPad *dpad = tempSet->getJoyDPad(dpadIndex);

        if (dpad != nullptr)
        {
            JoyDPadButton *button = dpad->getJoyButton(buttonIndex);

            if (button != nullptr)
                button->setButtonName(tempName);
        }

        connect(tempSet, &SetJoystick::setDPadButtonNameChange, this, &InputDevice::updateSetDPadButtonNames);
    }
}

void InputDevice::setVDPadButtonName(int vdpadIndex, int buttonIndex, QString tempName)
{
    for (auto &tempSet : getJoystick_sets())
    {
        disconnect(tempSet, &SetJoystick::setVDPadButtonNameChange, this, &InputDevice::updateSetVDPadButtonNames);
        VDPad *vdpad = tempSet->getVDPad(vdpadIndex);

        if (vdpad != nullptr)
        {
            JoyDPadButton *button = vdpad->getJoyButton(buttonIndex);

            if (button != nullptr)
                button->setButtonName(tempName);
        }

        connect(tempSet, &SetJoystick::setVDPadButtonNameChange, this, &InputDevice::updateSetVDPadButtonNames);
    }
}

void InputDevice::setAxisName(int axisIndex, QString tempName)
{
    for (auto &tempSet : getJoystick_sets())
    {
        disconnect(tempSet, &SetJoystick::setAxisNameChange, this, &InputDevice::updateSetAxisNames);
        JoyAxis *axis = tempSet->getJoyAxis(axisIndex);

        if (axis != nullptr)
            axis->setAxisName(tempName);

        connect(tempSet, &SetJoystick::setAxisNameChange, this, &InputDevice::updateSetAxisNames);
    }
}

void InputDevice::setStickName(int stickIndex, QString tempName)
{
    for (auto &tempSet : getJoystick_sets())
    {
        disconnect(tempSet, &SetJoystick::setStickNameChange, this, &InputDevice::updateSetStickNames);
        JoyControlStick *stick = tempSet->getJoyStick(stickIndex);

        if (stick != nullptr)
            stick->setStickName(tempName);

        connect(tempSet, &SetJoystick::setStickNameChange, this, &InputDevice::updateSetStickNames);
    }
}

/**
 * @brief Sets the name of a sensor in all sets
 *  Used during XML loading.
 * @param type The sensor type to be renamed
 * @param tempName The new name
 */
void InputDevice::setSensorName(JoySensorType type, QString tempName)
{
    auto sets = getJoystick_sets();
    for (auto &tempSet : sets)
    {
        disconnect(tempSet, &SetJoystick::setSensorNameChange, this, &InputDevice::updateSetSensorNames);
        JoySensor *sensor = tempSet->getSensor(type);

        if (sensor != nullptr)
            sensor->setSensorName(tempName);

        connect(tempSet, &SetJoystick::setSensorNameChange, this, &InputDevice::updateSetSensorNames);
    }
}

void InputDevice::setDPadName(int dpadIndex, QString tempName)
{
    for (auto &tempSet : getJoystick_sets())
    {
        disconnect(tempSet, &SetJoystick::setDPadNameChange, this, &InputDevice::updateSetDPadNames);
        JoyDPad *dpad = tempSet->getJoyDPad(dpadIndex);

        if (dpad != nullptr)
            dpad->setDPadName(tempName);

        connect(tempSet, &SetJoystick::setDPadNameChange, this, &InputDevice::updateSetDPadNames);
    }
}

void InputDevice::setVDPadName(int vdpadIndex, QString tempName)
{
    for (auto &tempSet : getJoystick_sets())
    {
        disconnect(tempSet, &SetJoystick::setVDPadNameChange, this, &InputDevice::updateSetVDPadNames);
        VDPad *vdpad = tempSet->getVDPad(vdpadIndex);

        if (vdpad != nullptr)
            vdpad->setDPadName(tempName);

        connect(tempSet, &SetJoystick::setVDPadNameChange, this, &InputDevice::updateSetVDPadNames);
    }
}

void InputDevice::updateSetButtonNames(int index)
{
    JoyButton *button = getActiveSetJoystick()->getJoyButton(index);

    if (button != nullptr)
        setButtonName(index, button->getButtonName());
}

void InputDevice::updateSetAxisButtonNames(int axisIndex, int buttonIndex)
{
    JoyAxis *axis = getActiveSetJoystick()->getJoyAxis(axisIndex);

    if (axis != nullptr)
    {
        JoyAxisButton *button = nullptr;

        if (buttonIndex == 0)
            button = axis->getNAxisButton();
        else if (buttonIndex == 1)
            button = axis->getPAxisButton();

        if (button != nullptr)
            setAxisButtonName(axisIndex, buttonIndex, button->getButtonName());
    }
}

void InputDevice::updateSetStickButtonNames(int stickIndex, int buttonIndex)
{
    JoyControlStick *stick = getActiveSetJoystick()->getJoyStick(stickIndex);

    if (stick != nullptr)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::JoyStickDirections(buttonIndex));

        if (button != nullptr)
            setStickButtonName(stickIndex, buttonIndex, button->getButtonName());
    }
}

/**
 * @brief Rename mapped sensor button in all sets to the name in the active set
 * @param type The sensor type which has the to be renamed button
 * @param direction The direction of the to be renamed button
 */
void InputDevice::updateSetSensorButtonNames(JoySensorType type, JoySensorDirection direction)
{
    JoySensor *sensor = getActiveSetJoystick()->getSensor(type);

    if (sensor != nullptr)
    {
        JoySensorButton *button = sensor->getDirectionButton(direction);

        if (button != nullptr)
            setSensorButtonName(type, direction, button->getButtonName());
    }
}

void InputDevice::updateSetDPadButtonNames(int dpadIndex, int buttonIndex)
{
    JoyDPad *dpad = getActiveSetJoystick()->getJoyDPad(dpadIndex);

    if (dpad != nullptr)
    {
        JoyDPadButton *button = dpad->getJoyButton(buttonIndex);

        if (button != nullptr)
            setDPadButtonName(dpadIndex, buttonIndex, button->getButtonName());
    }
}

void InputDevice::updateSetVDPadButtonNames(int vdpadIndex, int buttonIndex)
{
    VDPad *vdpad = getActiveSetJoystick()->getVDPad(vdpadIndex);

    if (vdpad != nullptr)
    {
        JoyDPadButton *button = vdpad->getJoyButton(buttonIndex);

        if (button != nullptr)
            setVDPadButtonName(vdpadIndex, buttonIndex, button->getButtonName());
    }
}

void InputDevice::updateSetAxisNames(int axisIndex)
{
    JoyAxis *axis = getActiveSetJoystick()->getJoyAxis(axisIndex);

    if (axis != nullptr)
        setAxisName(axisIndex, axis->getAxisName());
}

void InputDevice::updateSetStickNames(int stickIndex)
{
    JoyControlStick *stick = getActiveSetJoystick()->getJoyStick(stickIndex);

    if (stick != nullptr)
        setStickName(stickIndex, stick->getStickName());
}

/**
 * @brief Rename sensor in all sets to the name in the current set
 * @param type The sensor to rename
 */
void InputDevice::updateSetSensorNames(JoySensorType type)
{
    JoySensor *sensor = getActiveSetJoystick()->getSensor(type);

    if (sensor != nullptr)
        setSensorName(type, sensor->getSensorName());
}

void InputDevice::updateSetDPadNames(int dpadIndex)
{
    JoyDPad *dpad = getActiveSetJoystick()->getJoyDPad(dpadIndex);

    if (dpad != nullptr)
        setDPadName(dpadIndex, dpad->getDpadName());
}

void InputDevice::updateSetVDPadNames(int vdpadIndex)
{
    VDPad *vdpad = getActiveSetJoystick()->getVDPad(vdpadIndex);

    if (vdpad != nullptr)
        setVDPadName(vdpadIndex, vdpad->getDpadName());
}

void InputDevice::resetButtonDownCount()
{
    buttonDownCount = 0;
    emit released(joyNumber);
}

/**
 * @brief Establishes necessary connections for set change slots
 */
void InputDevice::enableSetConnections(SetJoystick *setstick)
{
    connect(setstick, &SetJoystick::setChangeActivated, this, &InputDevice::resetButtonDownCount);
    connect(setstick, &SetJoystick::setChangeActivated, this, &InputDevice::setActiveSetNumber);
    connect(setstick, &SetJoystick::setChangeActivated, this, &InputDevice::propogateSetChange);
    connect(setstick, &SetJoystick::setAssignmentButtonChanged, this, &InputDevice::changeSetButtonAssociation);

    connect(setstick, &SetJoystick::setAssignmentAxisChanged, this, &InputDevice::changeSetAxisButtonAssociation);
    connect(setstick, &SetJoystick::setAssignmentDPadChanged, this, &InputDevice::changeSetDPadButtonAssociation);
    connect(setstick, &SetJoystick::setAssignmentVDPadChanged, this, &InputDevice::changeSetVDPadButtonAssociation);
    connect(setstick, &SetJoystick::setAssignmentStickChanged, this, &InputDevice::changeSetStickButtonAssociation);
    connect(setstick, &SetJoystick::setAssignmentSensorChanged, this, &InputDevice::changeSetSensorButtonAssociation);
    connect(setstick, &SetJoystick::setAssignmentAxisThrottleChanged, this, &InputDevice::propogateSetAxisThrottleChange);

    connect(setstick, &SetJoystick::setButtonClick, this, &InputDevice::buttonDownEvent);

    connect(setstick, &SetJoystick::setButtonRelease, this, &InputDevice::buttonUpEvent);

    connect(setstick, &SetJoystick::setAxisButtonClick, this, &InputDevice::axisButtonDownEvent);
    connect(setstick, &SetJoystick::setAxisButtonRelease, this, &InputDevice::axisButtonUpEvent);
    connect(setstick, &SetJoystick::setAxisActivated, this, &InputDevice::axisActivatedEvent);
    connect(setstick, &SetJoystick::setAxisReleased, this, &InputDevice::axisReleasedEvent);

    connect(setstick, &SetJoystick::setDPadButtonClick, this, &InputDevice::dpadButtonDownEvent);
    connect(setstick, &SetJoystick::setDPadButtonRelease, this, &InputDevice::dpadButtonUpEvent);

    connect(setstick, &SetJoystick::setStickButtonClick, this, &InputDevice::stickButtonDownEvent);
    connect(setstick, &SetJoystick::setStickButtonRelease, this, &InputDevice::stickButtonUpEvent);
    connect(setstick, &SetJoystick::setSensorButtonClick, this, &InputDevice::sensorButtonDownEvent);
    connect(setstick, &SetJoystick::setSensorButtonRelease, this, &InputDevice::sensorButtonUpEvent);

    connect(setstick, &SetJoystick::setButtonNameChange, this, &InputDevice::updateSetButtonNames);
    connect(setstick, &SetJoystick::setAxisButtonNameChange, this, &InputDevice::updateSetAxisButtonNames);
    connect(setstick, &SetJoystick::setStickButtonNameChange, this, &InputDevice::updateSetStickButtonNames);
    connect(setstick, &SetJoystick::setSensorButtonNameChange, this, &InputDevice::updateSetSensorButtonNames);
    connect(setstick, &SetJoystick::setDPadButtonNameChange, this, &InputDevice::updateSetDPadButtonNames);
    connect(setstick, &SetJoystick::setVDPadButtonNameChange, this, &InputDevice::updateSetVDPadButtonNames);

    connect(setstick, &SetJoystick::setAxisNameChange, this, &InputDevice::updateSetAxisNames);
    connect(setstick, &SetJoystick::setStickNameChange, this, &InputDevice::updateSetStickNames);
    connect(setstick, &SetJoystick::setSensorNameChange, this, &InputDevice::updateSetSensorNames);
    connect(setstick, &SetJoystick::setDPadNameChange, this, &InputDevice::updateSetDPadNames);
    connect(setstick, &SetJoystick::setVDPadNameChange, this, &InputDevice::updateSetVDPadNames);
}

void InputDevice::axisActivatedEvent(int setindex, int axisindex, int value)
{
    Q_UNUSED(setindex);

    emit rawAxisActivated(axisindex, value);
}

void InputDevice::axisReleasedEvent(int setindex, int axisindex, int value)
{
    Q_UNUSED(setindex);

    emit rawAxisReleased(axisindex, value);
}

void InputDevice::setIndex(int index)
{
    if (index >= 0)
        joyNumber = index;
    else
        joyNumber = 0;
}

void InputDevice::setDeviceKeyPressTime(int newPressTime)
{
    keyPressTime = newPressTime;
    emit propertyUpdated();
}

int InputDevice::getDeviceKeyPressTime() { return keyPressTime; }

void InputDevice::profileEdited()
{
    if (!deviceEdited)
    {
        deviceEdited = true;
        emit profileUpdated();
    }
}

bool InputDevice::isDeviceEdited() { return deviceEdited; }

void InputDevice::revertProfileEdited() { deviceEdited = false; }

QString InputDevice::getStringIdentifier()
{
    QString identifier = QString();
    // QString tempGUID = getGUIDString();
    QString tempUniqueID = getUniqueIDString();
    QString tempName = getSDLName();

    if (!tempUniqueID.isEmpty())
        identifier = tempUniqueID;
    // else if (!tempGUID.isEmpty()) identifier = tempGUID;
    else if (!tempName.isEmpty())
        identifier = tempName;

    return identifier;
}

void InputDevice::establishPropertyUpdatedConnection()
{
    connect(this, &InputDevice::propertyUpdated, this, &InputDevice::profileEdited);
}

void InputDevice::disconnectPropertyUpdatedConnection()
{
    disconnect(this, &InputDevice::propertyUpdated, this, &InputDevice::profileEdited);
}

void InputDevice::setKeyRepeatStatus(bool enabled) { keyRepeatEnabled = enabled; }

void InputDevice::setKeyRepeatDelay(int delay)
{
    if ((delay >= 250) && (delay <= 1000))
    {
        keyRepeatDelay = delay;
    }
}

void InputDevice::setKeyRepeatRate(int rate)
{
    if ((rate >= 20) && (rate <= 200))
    {
        keyRepeatRate = rate;
    }
}

bool InputDevice::isKeyRepeatEnabled() { return keyRepeatEnabled; }

int InputDevice::getKeyRepeatDelay()
{
    int tempKeyRepeatDelay = GlobalVariables::InputDevice::DEFAULTKEYREPEATDELAY;

    if (keyRepeatDelay != 0)
    {
        tempKeyRepeatDelay = keyRepeatDelay;
    }

    return tempKeyRepeatDelay;
}

int InputDevice::getKeyRepeatRate()
{
    int tempKeyRepeatRate = GlobalVariables::InputDevice::DEFAULTKEYREPEATRATE;

    if (keyRepeatRate != 0)
    {
        tempKeyRepeatRate = keyRepeatRate;
    }

    return tempKeyRepeatRate;
}

void InputDevice::setProfileName(QString value)
{
    if (profileName != value)
    {
        if (value.size() > 50)
        {
            value.truncate(47);
            value.append("...");
        }

        profileName = value;
        emit propertyUpdated();
        emit profileNameEdited(value);
    }
}

QString InputDevice::getProfileName() { return profileName; }

int InputDevice::getButtonDownCount() { return buttonDownCount; }

QString InputDevice::getDescription()
{
    QString full_desc = QString("Index:            %1").arg(getRealJoyNumber()) + "\n  " +
                        QString("UniqueID:         %1").arg(getUniqueIDString()) + "\n  " +
                        QString("GUID:             %1").arg(getGUIDString()) + "\n  " +
                        QString("VendorID:         %1").arg(getVendorString()) + "\n  " +
                        QString("ProductID:        %1").arg(getProductIDString()) + "\n  " +
                        QString("Serial:           %1").arg(getSerialString()) + "\n  " +
                        QString("Product Version:  %1").arg(getProductVersion()) + "\n  " +
                        QString("Name:             %1").arg(getSDLName()) + "\n";
    QString gameControllerStatus = isGameController() ? QString("Yes") : QString("No");

    full_desc = full_desc + "  " + QString("Game Controller: %1").arg(gameControllerStatus) + "\n  " +
                QString("# of RawAxes:    %1").arg(getNumberRawAxes()) + "\n  " +
                QString("# of Axes:       %1").arg(getNumberAxes()) + "\n  " +
                QString("# of RawButtons: %1").arg(getNumberRawButtons()) + "\n  " +
                QString("# of Buttons:    %1").arg(getNumberButtons()) + "\n  " +
                QString("# of Hats:       %1").arg(getNumberHats()) + "\n  " +
                QString("Accelerometer:   %1").arg(hasSensor(ACCELEROMETER)) + "\n  " +
                QString("Gyroscope:       %1").arg(hasSensor(GYROSCOPE)) + "\n";
    return full_desc;
}

QString InputDevice::getSDLPlatform()
{
    QString temp = SDL_GetPlatform();
    return temp;
}

/**
 * @brief Check if device is using the SDL Game Controller API
 * @return Status showing if device is using the Game Controller API
 */
bool InputDevice::isGameController() { return false; }

bool InputDevice::hasCalibrationThrottle(int axisNum)
{
    bool result = false;

    if (getCali().contains(axisNum))
        result = true;

    return result;
}

JoyAxis::ThrottleTypes InputDevice::getCalibrationThrottle(int axisNum) { return getCali().value(axisNum); }

void InputDevice::setCalibrationThrottle(int axisNum, JoyAxis::ThrottleTypes throttle)
{
    if (!getCali().contains(axisNum))
    {
        for (int i = 0; i < GlobalVariables::InputDevice::NUMBER_JOYSETS; i++)
        {
            getJoystick_sets().value(i)->setAxisThrottle(axisNum, throttle);
        }

        getCali().insert(axisNum, throttle);
    }
}

void InputDevice::setCalibrationStatus(int axisNum, JoyAxis::ThrottleTypes throttle)
{
    if (!getCali().contains(axisNum))
        getCali().insert(axisNum, throttle);
}

void InputDevice::removeCalibrationStatus(int axisNum)
{
    if (getCali().contains(axisNum))
        getCali().remove(axisNum);
}

void InputDevice::sendLoadProfileRequest(QString location)
{
    if (!location.isEmpty())
        emit requestProfileLoad(location);
}

AntiMicroSettings *InputDevice::getSettings() { return m_settings; }

bool InputDevice::isKnownController()
{
    bool result = false;

    if (isGameController())
    {
        result = true;
    } else
    {
        m_settings->beginGroup("Mappings");

        //        if (m_settings->contains(getGUIDString())) result = true;
        //        else if (m_settings->contains(QString("%1%2").arg(getGUIDString()).arg("Disabled"))) result = true;

        convertToUniqueMappSett(m_settings, getGUIDString(), getUniqueIDString());
        convertToUniqueMappSett(m_settings, (QString("%1%2").arg(getGUIDString()).arg("Disabled")),
                                (QString("%1%2").arg(getUniqueIDString()).arg("Disabled")));

        if (m_settings->contains(getUniqueIDString()))
            result = true;
        else if (m_settings->contains(QString("%1%2").arg(getUniqueIDString()).arg("Disabled")))
            result = true;

        m_settings->endGroup();
    }

    return result;
}

void InputDevice::activatePossiblePendingEvents()
{
    activatePossibleControlStickEvents();
    activatePossibleAxisEvents();
    activatePossibleDPadEvents();
    activatePossibleVDPadEvents();
    activatePossibleButtonEvents();
}

void InputDevice::activatePossibleControlStickEvents()
{
    SetJoystick *currentSet = getActiveSetJoystick();

    for (int i = 0; i < currentSet->getNumberSticks(); i++)
    {
        JoyControlStick *tempStick = currentSet->getJoyStick(i);

        if ((tempStick != nullptr) && tempStick->hasPendingEvent())
        {
            tempStick->activatePendingEvent();
        }
    }
}

void InputDevice::activatePossibleAxisEvents()
{
    SetJoystick *currentSet = getActiveSetJoystick();

    for (int i = 0; i < currentSet->getNumberAxes(); i++)
    {
        JoyAxis *tempAxis = currentSet->getJoyAxis(i);

        if ((tempAxis != nullptr) && tempAxis->hasPendingEvent())
        {
            tempAxis->activatePendingEvent();
        }
    }
}

void InputDevice::activatePossibleSensorEvents()
{
    SetJoystick *currentSet = getActiveSetJoystick();
    JoySensor *sensor = nullptr;

    for (size_t i = 0; i < SENSOR_COUNT; ++i)
    {
        JoySensorType type = static_cast<JoySensorType>(i);
        sensor = currentSet->getSensor(type);
        if ((sensor != nullptr) && sensor->hasPendingEvent())
            sensor->activatePendingEvent();
    }
}

void InputDevice::activatePossibleDPadEvents()
{
    SetJoystick *currentSet = getActiveSetJoystick();

    for (int i = 0; i < currentSet->getNumberHats(); i++)
    {
        JoyDPad *tempDPad = currentSet->getJoyDPad(i);

        if ((tempDPad != nullptr) && tempDPad->hasPendingEvent())
            tempDPad->activatePendingEvent();
    }
}

void InputDevice::activatePossibleVDPadEvents()
{
    SetJoystick *currentSet = getActiveSetJoystick();

    for (int i = 0; i < currentSet->getNumberVDPads(); i++)
    {
        VDPad *tempVDPad = currentSet->getVDPad(i);

        if ((tempVDPad != nullptr) && tempVDPad->hasPendingEvent())
            tempVDPad->activatePendingEvent();
    }
}

void InputDevice::activatePossibleButtonEvents()
{
    SetJoystick *currentSet = getActiveSetJoystick();

    for (int i = 0; i < currentSet->getNumberButtons(); i++)
    {
        JoyButton *tempButton = currentSet->getJoyButton(i);

        if ((tempButton != nullptr) && tempButton->hasPendingEvent())
            tempButton->activatePendingEvent();
    }
}

bool InputDevice::elementsHaveNames()
{
    bool result = false;

    SetJoystick *tempSet = getActiveSetJoystick();

    for (int i = 0; i < getNumberButtons() && !result; i++)
    {
        JoyButton *button = tempSet->getJoyButton(i);

        if ((button != nullptr) && !button->getButtonName().isEmpty())
            result = true;
    }

    for (int i = 0; (i < getNumberAxes()) && !result; i++)
    {
        JoyAxis *axis = tempSet->getJoyAxis(i);

        if (axis != nullptr)
        {
            if (!axis->getAxisName().isEmpty())
                result = true;

            JoyAxisButton *naxisbutton = axis->getNAxisButton();

            if (!naxisbutton->getButtonName().isEmpty())
                result = true;

            JoyAxisButton *paxisbutton = axis->getPAxisButton();

            if (!paxisbutton->getButtonName().isEmpty())
                result = true;
        }
    }

    for (int i = 0; (i < getNumberSticks()) && !result; i++)
    {
        JoyControlStick *stick = tempSet->getJoyStick(i);

        if (stick != nullptr)
        {
            if (!stick->getStickName().isEmpty())
                result = true;

            QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton *> *buttons = stick->getButtons();
            QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton *> iter(*buttons);

            while (iter.hasNext() && !result)
            {
                JoyControlStickButton *button = iter.next().value();

                if (button && !button->getButtonName().isEmpty())
                    result = true;
            }
        }
    }

    for (int i = 0; (i < getNumberHats()) && !result; i++)
    {
        JoyDPad *dpad = tempSet->getJoyDPad(i);

        if (dpad != nullptr)
        {
            if (!dpad->getDpadName().isEmpty())
                result = true;

            QHash<int, JoyDPadButton *> *temp = dpad->getButtons();
            QHashIterator<int, JoyDPadButton *> iter(*temp);

            while (iter.hasNext() && !result)
            {
                JoyDPadButton *button = iter.next().value();

                if (button && !button->getButtonName().isEmpty())
                    result = true;
            }
        }
    }

    for (int i = 0; (i < getNumberVDPads()) && !result; i++)
    {
        VDPad *vdpad = getActiveSetJoystick()->getVDPad(i);

        if (vdpad != nullptr)
        {
            if (!vdpad->getDpadName().isEmpty())
                result = true;

            QHash<int, JoyDPadButton *> *temp = vdpad->getButtons();
            QHashIterator<int, JoyDPadButton *> iter(*temp);

            while (iter.hasNext() && !result)
            {
                JoyDPadButton *button = iter.next().value();

                if ((button != nullptr) && !button->getButtonName().isEmpty())
                    result = true;
            }
        }
    }

    return result;
}

/**
 * @brief Check if the GUID passed is considered empty.
 * @param GUID string
 * @return if GUID is considered empty.
 */
// bool InputDevice::isEmptyGUID(QString tempGUID)
//{
//    bool result = false;

//    if (tempGUID.contains(GlobalVariables::InputDevice::emptyGUID)) result = true;

//    return result;
//}

bool InputDevice::isEmptyUniqueID(QString tempUniqueID)
{
    bool result = false;

    if (tempUniqueID.contains(GlobalVariables::InputDevice::emptyUniqueID))
        result = true;

    return result;
}

/**
 * @brief Check if GUID passed matches the expected GUID for a device.
 *     Needed for xinput GUID abstraction.
 * @param GUID string
 * @return if GUID is considered a match.
 */
// bool InputDevice::isRelevantGUID(QString tempGUID)
//{
//    bool result = false;

//    if (tempGUID == getGUIDString()) result = true;

//    return result;
//}

bool InputDevice::isRelevantUniqueID(QString tempUniqueID)
{
    bool result = false;

    if (tempUniqueID == getUniqueIDString())
        result = true;

    return result;
}

QString InputDevice::getRawGUIDString() const { return getGUIDString(); }

QString InputDevice::getRawVendorString() const { return getVendorString(); }

QString InputDevice::getRawProductIDString() const { return getProductIDString(); }

QString InputDevice::getRawProductVersion() const { return getProductVersion(); }

QString InputDevice::getRawUniqueIDString() const { return getUniqueIDString(); }

void InputDevice::haltServices() { emit requestWait(); }

void InputDevice::finalRemoval()
{
    this->closeSDLDevice();
    this->deleteLater();
}

void InputDevice::setRawAxisDeadZone(int deadZone)
{
    if ((deadZone > 0) && (deadZone <= GlobalVariables::JoyAxis::AXISMAX))
    {
        this->rawAxisDeadZone = deadZone;
    } else
    {
        this->rawAxisDeadZone = GlobalVariables::InputDevice::RAISEDDEADZONE;
    }
}

int InputDevice::getRawAxisDeadZone() { return rawAxisDeadZone; }

void InputDevice::rawAxisEvent(int index, int value) { emit rawAxisMoved(index, value); }

void InputDevice::convertToUniqueMappSett(QSettings *sett, QString gUIDmappGroupSett, QString uniqueIDGroupSett)
{
    if (sett->contains(gUIDmappGroupSett))
    {
        if (sett->contains(gUIDmappGroupSett) &&
            (sett->value(gUIDmappGroupSett).toString().split(",").first() == getGUIDString()))
        {
            QStringList gg = sett->value(gUIDmappGroupSett).toString().split(",");
            gg.removeFirst();
            gg.prepend(uniqueIDGroupSett);
            sett->setValue(uniqueIDGroupSett, sett->value(gg.join(",")));
            sett->remove(gUIDmappGroupSett);
        } else
        {
            sett->setValue(uniqueIDGroupSett, sett->value(gUIDmappGroupSett));
            sett->remove(gUIDmappGroupSett);
        }
    }
}

QMap<int, SetJoystick *> &InputDevice::getJoystick_sets() { return joystick_sets; }

QHash<int, JoyAxis::ThrottleTypes> &InputDevice::getCali() { return cali; }

SDL_JoystickID *InputDevice::getJoystickID() { return &joystickID; }

QList<bool> &InputDevice::getButtonstatesLocal() { return buttonstates; }

QList<int> &InputDevice::getAxesstatesLocal() { return axesstates; }

QList<int> &InputDevice::getDpadstatesLocal() { return dpadstates; }

SDL_Joystick *InputDevice::getJoyHandle() const { return m_joyhandle; }

/**
 * @brief Returns the current controller model.
 *   For non gamecontroller devices it always returns UNKNOWN.
 */
SDL_GameControllerType InputDevice::getControllerType() const { return SDL_CONTROLLER_TYPE_UNKNOWN; }

/**
 * @brief Returns a pointer to the internal calibration storage backend.
 */
InputDeviceCalibration *InputDevice::getCalibrationBackend() { return &m_calibrations; }

/**
 * @brief Updates stored calibration for this controller and applies
 *   calibration to the specified stick in all sets
 *   See JoyControlStick::setCalibration
 * @param[in] index Stick index
 * @param[in] offsetX Offset value for X axis
 * @param[in] gainX Gain value for X axis
 * @param[in] offsetY Offset value for Y axis
 * @param[in] gainY Gain value for Y axis
 */
void InputDevice::updateStickCalibration(int index, double offsetX, double gainX, double offsetY, double gainY)
{
    m_calibrations.setStickCalibration(index, offsetX, gainX, offsetY, gainY);
    applyStickCalibration(index, offsetX, gainX, offsetY, gainY);
}

/**
 * @brief Applies calibration to the specified stick in all sets
 *  See JoyControlStick::setCalibration
 * @param[in] index Stick index
 * @param[in] offsetX Offset value for X axis
 * @param[in] gainX Gain value for X axis
 * @param[in] offsetY Offset value for Y axis
 * @param[in] gainY Gain value for Y axis
 */
void InputDevice::applyStickCalibration(int index, double offsetX, double gainX, double offsetY, double gainY)
{
    for (auto &set : joystick_sets)
    {
        JoyControlStick *stick = set->getSticks().value(index);
        if (stick != nullptr)
            stick->setCalibration(offsetX, gainX, offsetY, gainY);
    }
}

/**
 * @brief Applies calibration to the specified accelerometer in all sets
 *  See JoySensor::setCalibration
 * @param[in] offsetX Offset angle around the X axis
 * @param[in] offsetY Offset angle around the Y axis
 * @param[in] offsetZ Offset angle around the Z axis
 */
void InputDevice::updateAccelerometerCalibration(double offsetX, double offsetY, double offsetZ)
{
    m_calibrations.setAccelerometerCalibration(offsetX, offsetY, offsetZ);
    applyAccelerometerCalibration(offsetX, offsetY, offsetZ);
}

/**
 * @brief Applies calibration to the specified accelerometer in all sets
 *  See JoySensor::setCalibration
 * @param[in] offsetX Offset angle around the X axis
 * @param[in] offsetY Offset angle around the Y axis
 * @param[in] offsetZ Offset angle around the Z axis
 */
void InputDevice::applyAccelerometerCalibration(double offsetX, double offsetY, double offsetZ)
{
    for (auto &set : joystick_sets)
    {
        JoySensor *accelerometer = set->getSensor(ACCELEROMETER);
        if (accelerometer != nullptr)
            accelerometer->setCalibration(offsetX, offsetY, offsetZ);
    }
}

/**
 * @brief Updates stored calibration for this controller and applies
 *   calibration to the specified gyroscope in all sets
 *   See JoySensor::setCalibration
 * @param[in] offsetX Offset value for X axis
 * @param[in] offsetY Offset value for Y axis
 * @param[in] offsetZ Offset value for Z axis
 */
void InputDevice::updateGyroscopeCalibration(double offsetX, double offsetY, double offsetZ)
{
    m_calibrations.setGyroscopeCalibration(offsetX, offsetY, offsetZ);
    applyGyroscopeCalibration(offsetX, offsetY, offsetZ);
}

/**
 * @brief Applies calibration to the specified gyroscope in all sets
 *  See JoySensor::setCalibration
 * @param[in] offsetX Offset value for X axis
 * @param[in] offsetY Offset value for Y axis
 * @param[in] offsetZ Offset value for Z axis
 */
void InputDevice::applyGyroscopeCalibration(double offsetX, double offsetY, double offsetZ)
{
    for (auto &set : joystick_sets)
    {
        JoySensor *gyroscope = set->getSensor(GYROSCOPE);
        if (gyroscope != nullptr)
            gyroscope->setCalibration(offsetX, offsetY, offsetZ);
    }
}
