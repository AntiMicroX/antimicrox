/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
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

#include "globalvariables.h"
#include "messagehandler.h"
#include "common.h"
#include "antimicrosettings.h"
#include "joydpad.h"
#include "joycontrolstick.h"
#include "joybuttontypes/joydpadbutton.h"
#include "vdpad.h"
#include "joybuttontypes/joycontrolstickbutton.h"

#include <typeinfo>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>


InputDevice::InputDevice(int deviceIndex, AntiMicroSettings *settings, QObject *parent) :
    QObject(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    buttonDownCount = 0;
    joyNumber = deviceIndex;
    active_set = 0;
    joystickID = 0;
    keyPressTime = 0;
    deviceEdited = false;
#ifdef Q_OS_WIN
    keyRepeatEnabled = true;
#elif defined(Q_OS_UNIX)
    keyRepeatEnabled = false;
#endif

    keyRepeatDelay = 0;
    keyRepeatRate = 0;
    rawAxisDeadZone = GlobalVariables::InputDevice::RAISEDDEADZONE;
    m_settings = settings;
}

InputDevice::~InputDevice()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<int, SetJoystick*> iter(getJoystick_sets());
    while (iter.hasNext())
    {
        SetJoystick *setjoystick = iter.next().value();
        if (setjoystick != nullptr)
        {
            delete setjoystick;
            setjoystick = nullptr;
        }
    }

    getJoystick_sets().clear();
}

int InputDevice::getJoyNumber()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return joyNumber;
}

int InputDevice::getRealJoyNumber()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int joynumber = getJoyNumber();
    return joynumber + 1;
}

void InputDevice::reset()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    resetButtonDownCount();
    deviceEdited = false;
    profileName = "";

    for (int i = 0; i < GlobalVariables::InputDevice::NUMBER_JOYSETS; i++)
    {
        SetJoystick* set = getJoystick_sets().value(i);
        set->reset();
    }
}

/**
 * @brief Obtain current joystick element values, create new SetJoystick objects,
 *     and then transfer most recent joystick element values to new
 *     current set.
 */
void InputDevice::transferReset()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    // Grab current states for all elements in old set
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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

        for (int i=0; i < current_set->getNumberSticks(); i++)
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

        for (int i=0; i < current_set->getNumberSticks(); i++)
        {
            JoyControlStick::JoyStickDirections value = stickstates.at(i);
            QList<JoyControlStickButton*> buttonList;
            QList<JoyControlStickButton*> oldButtonList;
            JoyControlStick *stick = current_set->getJoyStick(i);
            JoyControlStick *oldStick = old_set->getJoyStick(i);

            if ((stick->getJoyMode() == JoyControlStick::StandardMode) && value)
            {
                switch (value)
                {
                    case JoyControlStick::StickRightUp:
                    {
                        buttonList.append(stick->getDirectionButton(JoyControlStick::StickUp));
                        buttonList.append(stick->getDirectionButton(JoyControlStick::StickRight));
                        oldButtonList.append(oldStick->getDirectionButton(JoyControlStick::StickUp));
                        oldButtonList.append(oldStick->getDirectionButton(JoyControlStick::StickRight));
                        break;
                    }
                    case JoyControlStick::StickRightDown:
                    {
                        buttonList.append(stick->getDirectionButton(JoyControlStick::StickRight));
                        buttonList.append(stick->getDirectionButton(JoyControlStick::StickDown));
                        oldButtonList.append(oldStick->getDirectionButton(JoyControlStick::StickRight));
                        oldButtonList.append(oldStick->getDirectionButton(JoyControlStick::StickDown));
                        break;
                    }
                    case JoyControlStick::StickLeftDown:
                    {
                        buttonList.append(stick->getDirectionButton(JoyControlStick::StickDown));
                        buttonList.append(stick->getDirectionButton(JoyControlStick::StickLeft));
                        oldButtonList.append(oldStick->getDirectionButton(JoyControlStick::StickDown));
                        oldButtonList.append(oldStick->getDirectionButton(JoyControlStick::StickLeft));
                        break;
                    }
                    case JoyControlStick::StickLeftUp:
                    {
                        buttonList.append(stick->getDirectionButton(JoyControlStick::StickLeft));
                        buttonList.append(stick->getDirectionButton(JoyControlStick::StickUp));
                        oldButtonList.append(oldStick->getDirectionButton(JoyControlStick::StickLeft));
                        oldButtonList.append(oldStick->getDirectionButton(JoyControlStick::StickUp));
                        break;
                    }
                    default:
                    {
                        buttonList.append(stick->getDirectionButton(value));
                        oldButtonList.append(oldStick->getDirectionButton(value));
                    }
                }
            }
            else if (value)
            {
                buttonList.append(stick->getDirectionButton(value));
                oldButtonList.append(oldStick->getDirectionButton(value));
            }

            QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> iter(*stick->getButtons());
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
                        if ((oldButton->getChangeSetCondition() == JoyButton::SetChangeWhileHeld) && oldButton->getWhileHeldStatus())
                        {
                            // Button from old set involved in a while held set
                            // change. Carry over to new set button to ensure
                            // set changes are done in the proper order.
                            button->setWhileHeldStatus(true);
                        }
                        else if (!button->getWhileHeldStatus())
                        {
                            // Ensure that set change events are performed if needed.
                            //tempignore = false;
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
            QList<JoyDPadButton*> buttonList;
            QList<JoyDPadButton*> oldButtonList;
            bool valueTrue = (value != 0);

            if ((dpad->getJoyMode() == JoyDPad::StandardMode) && valueTrue)
            {
                switch (value)
                {
                    case JoyDPadButton::DpadRightUp:
                    {
                        buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadUp));
                        buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadRight));
                        oldButtonList.append(old_set->getVDPad(i)->getJoyButton(JoyDPadButton::DpadUp));
                        oldButtonList.append(old_set->getVDPad(i)->getJoyButton(JoyDPadButton::DpadRight));
                        break;
                    }

                    case JoyDPadButton::DpadRightDown:
                    {
                        buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadRight));
                        buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadDown));
                        oldButtonList.append(old_set->getVDPad(i)->getJoyButton(JoyDPadButton::DpadRight));
                        oldButtonList.append(old_set->getVDPad(i)->getJoyButton(JoyDPadButton::DpadDown));
                        break;
                    }
                    case JoyDPadButton::DpadLeftDown:
                    {
                        buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadDown));
                        buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadLeft));
                        oldButtonList.append(old_set->getVDPad(i)->getJoyButton(JoyDPadButton::DpadDown));
                        oldButtonList.append(old_set->getVDPad(i)->getJoyButton(JoyDPadButton::DpadLeft));
                        break;
                    }
                    case JoyDPadButton::DpadLeftUp:
                    {
                        buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadLeft));
                        buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadUp));
                        oldButtonList.append(old_set->getVDPad(i)->getJoyButton(JoyDPadButton::DpadLeft));
                        oldButtonList.append(old_set->getVDPad(i)->getJoyButton(JoyDPadButton::DpadUp));
                        break;
                    }
                    default:
                    {
                        buttonList.append(dpad->getJoyButton(value));
                        oldButtonList.append(old_set->getVDPad(i)->getJoyButton(value));
                    }
                }
            }
            else if (valueTrue)
            {
                buttonList.append(dpad->getJoyButton(value));
                oldButtonList.append(old_set->getVDPad(i)->getJoyButton(value));
            }

            QHashIterator<int, JoyDPadButton*> iter(*dpad->getJoyButtons());
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

                if ((button != nullptr) && (oldButton != nullptr))
                {
                    if (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld)
                    {
                        if (valueTrue)
                        {
                            if ((oldButton->getChangeSetCondition() == JoyButton::SetChangeWhileHeld) && oldButton->getWhileHeldStatus())
                            {
                                // Button from old set involved in a while held set
                                // change. Carry over to new set button to ensure
                                // set changes are done in the proper order.
                                button->setWhileHeldStatus(true);
                            }
                            else if (!button->getWhileHeldStatus())
                            {
                                // Ensure that set change events are performed if needed.
                                //tempignore = false;
                            }
                        }
                        else
                        {
                            button->setWhileHeldStatus(false);
                        }
                    }
                }
            }
        }

        for (int i = 0; i < current_set->getNumberButtons(); i++)
        {
            bool value = buttonstates.at(i);
            bool tempignore = false;
            JoyButton *button = current_set->getJoyButton(i);
            JoyButton *oldButton = old_set->getJoyButton(i);
            if (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld)
            {
                if (value)
                {
                    if ((oldButton->getChangeSetCondition() == JoyButton::SetChangeWhileHeld) && oldButton->getWhileHeldStatus())
                    {
                        // Button from old set involved in a while held set
                        // change. Carry over to new set button to ensure
                        // set changes are done in the proper order.
                        button->setWhileHeldStatus(true);
                    }
                    else if (!button->getWhileHeldStatus())
                    {
                        // Ensure that set change events are performed if needed.
                        tempignore = false;
                    }
                }
                else
                {
                    // Ensure that set change events are performed if needed.
                    button->setWhileHeldStatus(false);
                }
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

            if ((button != nullptr) && (oldButton != nullptr))
            {
                if (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld)
                {
                    if ((oldButton->getChangeSetCondition() == JoyButton::SetChangeWhileHeld) && oldButton->getWhileHeldStatus())
                    {
                        // Button from old set involved in a while held set
                        // change. Carry over to new set button to ensure
                        // set changes are done in the proper order.
                        button->setWhileHeldStatus(true);
                    }
                    else if (!button->getWhileHeldStatus())
                    {
                        // Ensure that set change events are performed if needed.
                        tempignore = false;
                    }
                }
            }
            else if (button == nullptr)
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
            QList<JoyDPadButton*> buttonList;
            QList<JoyDPadButton*> oldButtonList;
            bool valueTrue = (value != 0);

            if ((dpad->getJoyMode() == JoyDPad::StandardMode) && valueTrue)
            {
                switch (value)
                {
                    case JoyDPadButton::DpadRightUp:
                    {
                        buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadUp));
                        buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadRight));
                        oldButtonList.append(old_set->getJoyDPad(i)->getJoyButton(JoyDPadButton::DpadUp));
                        oldButtonList.append(old_set->getJoyDPad(i)->getJoyButton(JoyDPadButton::DpadRight));
                        break;
                    }

                    case JoyDPadButton::DpadRightDown:
                    {
                        buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadRight));
                        buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadDown));
                        oldButtonList.append(old_set->getJoyDPad(i)->getJoyButton(JoyDPadButton::DpadRight));
                        oldButtonList.append(old_set->getJoyDPad(i)->getJoyButton(JoyDPadButton::DpadDown));
                        break;
                    }
                    case JoyDPadButton::DpadLeftDown:
                    {
                        buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadDown));
                        buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadLeft));
                        oldButtonList.append(old_set->getJoyDPad(i)->getJoyButton(JoyDPadButton::DpadDown));
                        oldButtonList.append(old_set->getJoyDPad(i)->getJoyButton(JoyDPadButton::DpadLeft));
                        break;
                    }
                    case JoyDPadButton::DpadLeftUp:
                    {
                        buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadLeft));
                        buttonList.append(dpad->getJoyButton(JoyDPadButton::DpadUp));
                        oldButtonList.append(old_set->getJoyDPad(i)->getJoyButton(JoyDPadButton::DpadLeft));
                        oldButtonList.append(old_set->getJoyDPad(i)->getJoyButton(JoyDPadButton::DpadUp));
                        break;
                    }
                    default:
                    {
                        buttonList.append(dpad->getJoyButton(value));
                        oldButtonList.append(old_set->getJoyDPad(i)->getJoyButton(value));
                    }
                }
            }
            else if (valueTrue)
            {
                buttonList.append(dpad->getJoyButton(value));
                oldButtonList.append(old_set->getJoyDPad(i)->getJoyButton(value));
            }

            QHashIterator<int, JoyDPadButton*> iter(*dpad->getJoyButtons());
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

                if ((button != nullptr) && (oldButton != nullptr))
                {
                    if (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld)
                    {
                        if (valueTrue)
                        {
                            if ((oldButton->getChangeSetCondition() == JoyButton::SetChangeWhileHeld) && oldButton->getWhileHeldStatus())
                            {
                                // Button from old set involved in a while held set
                                // change. Carry over to new set button to ensure
                                // set changes are done in the proper order.
                                button->setWhileHeldStatus(true);
                            }
                            else if (!button->getWhileHeldStatus())
                            {
                                // Ensure that set change events are performed if needed.
                                tempignore = false;
                            }
                        }
                        else
                        {
                            button->setWhileHeldStatus(false);
                        }

                    }
                }
            }

            dpad->queuePendingEvent(value, tempignore);
        }

        activatePossibleControlStickEvents();
        activatePossibleAxisEvents();
        activatePossibleDPadEvents();
        activatePossibleVDPadEvents();
        activatePossibleButtonEvents();
    }
}

int InputDevice::getActiveSetNumber()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return active_set;
}

SetJoystick* InputDevice::getActiveSetJoystick()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getJoystick_sets().value(active_set);
}

int InputDevice::getNumberButtons()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getActiveSetJoystick()->getNumberButtons();
}

int InputDevice::getNumberAxes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getActiveSetJoystick()->getNumberAxes();
}

int InputDevice::getNumberHats()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getActiveSetJoystick()->getNumberHats();
}

int InputDevice::getNumberSticks()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getActiveSetJoystick()->getNumberSticks();
}

int InputDevice::getNumberVDPads()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getActiveSetJoystick()->getNumberVDPads();
}

SetJoystick* InputDevice::getSetJoystick(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getJoystick_sets().value(index);
}

void InputDevice::propogateSetChange(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    emit setChangeActivated(index);
}

void InputDevice::changeSetButtonAssociation(int button_index, int originset, int newset, int mode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButton *button = getJoystick_sets().value(newset)->getJoyButton(button_index);
    JoyButton::SetChangeCondition tempmode = static_cast<JoyButton::SetChangeCondition>(mode);
    button->setChangeSetSelection(originset);
    button->setChangeSetCondition(tempmode, true);
}

void InputDevice::readConfig(QXmlStreamReader *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (xml->isStartElement() && (xml->name() == getXmlName()))
    {
        transferReset();

        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != getXmlName())))
        {
            if ((xml->name() == "sets") && xml->isStartElement())
            {
                xml->readNextStartElement();

                while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != "sets")))
                {
                    if ((xml->name() == "set") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        index = index - 1;
                        if ((index >= 0) && (index < getJoystick_sets().size()))
                        {
                            getJoystick_sets().value(index)->readConfig(xml);
                        }
                    }
                    else
                    {
                        // If none of the above, skip the element
                        xml->skipCurrentElement();
                    }

                    xml->readNextStartElement();
                }
            }
            else if ((xml->name() == "stickAxisAssociation") && xml->isStartElement())
            {
                int stickIndex = xml->attributes().value("index").toString().toInt();
                int xAxis = xml->attributes().value("xAxis").toString().toInt();
                int yAxis = xml->attributes().value("yAxis").toString().toInt();

                if ((stickIndex > 0) && (xAxis > 0) && (yAxis > 0))
                {
                    xAxis -= 1;
                    yAxis -= 1;
                    stickIndex -= 1;

                    for (int i = 0; i < getJoystick_sets().size(); i++)
                    {
                        SetJoystick *currentset = getJoystick_sets().value(i);
                        JoyAxis *axis1 = currentset->getJoyAxis(xAxis);
                        JoyAxis *axis2 = currentset->getJoyAxis(yAxis);
                        if ((axis1 != nullptr) && (axis2 != nullptr))
                        {
                            JoyControlStick *stick = new JoyControlStick(axis1, axis2, stickIndex, i, this);
                            currentset->addControlStick(stickIndex, stick);
                        }
                    }

                    xml->readNext();
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if ((xml->name() == "vdpadButtonAssociations") && xml->isStartElement())
            {
                int vdpadIndex = xml->attributes().value("index").toString().toInt();
                if (vdpadIndex > 0)
                {
                    for (int i = 0; i < getJoystick_sets().size(); i++)
                    {
                        SetJoystick *currentset = getJoystick_sets().value(i);
                        VDPad *vdpad = currentset->getVDPad(vdpadIndex-1);
                        if (vdpad == nullptr)
                        {
                            vdpad = new VDPad(vdpadIndex-1, i, currentset, currentset);
                            currentset->addVDPad(vdpadIndex-1, vdpad);
                        }
                    }

                    xml->readNextStartElement();
                    while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != "vdpadButtonAssociations")))
                    {
                        if ((xml->name() == "vdpadButtonAssociation") && xml->isStartElement())
                        {
                            int vdpadAxisIndex = xml->attributes().value("axis").toString().toInt();
                            int vdpadButtonIndex = xml->attributes().value("button").toString().toInt();
                            int vdpadDirection = xml->attributes().value("direction").toString().toInt();

                            if ((vdpadAxisIndex > 0) && (vdpadDirection > 0))
                            {
                                vdpadAxisIndex -= 1;
                                for (int i=0; i < getJoystick_sets().size(); i++)
                                {
                                    SetJoystick *currentset = getJoystick_sets().value(i);
                                    VDPad *vdpad = currentset->getVDPad(vdpadIndex-1);
                                    if (vdpad != nullptr)
                                    {
                                        JoyAxis *axis = currentset->getJoyAxis(vdpadAxisIndex);
                                        if (axis != nullptr)
                                        {
                                            JoyButton *button = nullptr;
                                            if (vdpadButtonIndex == 0)
                                            {
                                                button = axis->getNAxisButton();
                                            }
                                            else if (vdpadButtonIndex == 1)
                                            {
                                                button = axis->getPAxisButton();
                                            }

                                            if (button != nullptr)
                                            {
                                                vdpad->addVButton(static_cast<JoyDPadButton::JoyDPadDirections>(vdpadDirection), button);
                                            }
                                        }
                                    }
                                }
                            }
                            else if ((vdpadButtonIndex > 0) && (vdpadDirection > 0))
                            {
                                vdpadButtonIndex -= 1;

                                for (int i = 0; i < getJoystick_sets().size(); i++)
                                {
                                    SetJoystick *currentset = getJoystick_sets().value(i);
                                    VDPad *vdpad = currentset->getVDPad(vdpadIndex-1);
                                    if (vdpad != nullptr)
                                    {
                                        JoyButton *button = currentset->getJoyButton(vdpadButtonIndex);
                                        if (button != nullptr)
                                        {
                                            vdpad->addVButton(static_cast<JoyDPadButton::JoyDPadDirections>(vdpadDirection), button);
                                        }
                                    }
                                }
                            }
                            xml->readNext();
                        }
                        else
                        {
                            xml->skipCurrentElement();
                        }

                        xml->readNextStartElement();
                    }
                }

                for (int i=0; i < getJoystick_sets().size(); i++)
                {
                    SetJoystick *currentset = getJoystick_sets().value(i);
                    for (int j=0; j < currentset->getNumberVDPads(); j++)
                    {
                        VDPad *vdpad = currentset->getVDPad(j);
                        if ((vdpad != nullptr) && vdpad->isEmpty())
                        {
                            currentset->removeVDPad(j);
                        }
                    }
                }
            }
            else if ((xml->name() == "names") && xml->isStartElement())
            {
                xml->readNextStartElement();
                while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != "names")))
                {
                    if ((xml->name() == "buttonname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if ((index >= 0) && !temp.isEmpty())
                        {
                            setButtonName(index, temp);
                        }
                    }
                    else if ((xml->name() == "axisbuttonname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        int buttonIndex = xml->attributes().value("button").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        buttonIndex = buttonIndex - 1;
                        if ((index >= 0) && !temp.isEmpty())
                        {
                            setAxisButtonName(index, buttonIndex, temp);
                        }
                    }
                    else if ((xml->name() == "controlstickbuttonname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        int buttonIndex = xml->attributes().value("button").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if ((index >= 0) && !temp.isEmpty())
                        {
                            setStickButtonName(index, buttonIndex, temp);
                        }
                    }
                    else if ((xml->name() == "dpadbuttonname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        int buttonIndex = xml->attributes().value("button").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if ((index >= 0) && !temp.isEmpty())
                        {
                            setDPadButtonName(index, buttonIndex, temp);
                        }
                    }
                    else if ((xml->name() == "vdpadbuttonname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        int buttonIndex = xml->attributes().value("button").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if ((index >= 0) && !temp.isEmpty())
                        {
                            setVDPadButtonName(index, buttonIndex, temp);
                        }
                    }
                    else if ((xml->name() == "axisname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if ((index >= 0) && !temp.isEmpty())
                        {
                            setAxisName(index, temp);
                        }
                    }
                    else if ((xml->name() == "controlstickname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if ((index >= 0) && !temp.isEmpty())
                        {
                            setStickName(index, temp);
                        }
                    }
                    else if ((xml->name() == "dpadname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if ((index >= 0) && !temp.isEmpty())
                        {
                            setDPadName(index, temp);
                        }
                    }
                    else if ((xml->name() == "vdpadname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if ((index >= 0) && !temp.isEmpty())
                        {
                            setVDPadName(index, temp);
                        }
                    }
                    else
                    {
                        // If none of the above, skip the element
                        xml->skipCurrentElement();
                    }

                    xml->readNextStartElement();
                }
            }
            else if ((xml->name() == "keyPressTime") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                if (tempchoice >= 10)
                {
                    this->setDeviceKeyPressTime(tempchoice);
                }
            }
            else if ((xml->name() == "profilename") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                this->setProfileName(temptext);
            }
            else
            {
                // If none of the above, skip the element
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }

        reInitButtons();
    }
}

void InputDevice::writeConfig(QXmlStreamWriter *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    xml->writeStartElement(getXmlName());
    xml->writeAttribute("configversion", QString::number(PadderCommon::LATESTCONFIGFILEVERSION));
    xml->writeAttribute("appversion", PadderCommon::programVersion);

    xml->writeComment("The SDL name for a joystick is included for informational purposes only.");
    xml->writeTextElement("sdlname", getSDLName());
    xml->writeComment("The GUID for a joystick is included for informational purposes only.");
    xml->writeTextElement("guid", getGUIDString());

    if (!profileName.isEmpty())
    {
        xml->writeTextElement("profilename", profileName);
    }

    for (int i=0; i < getNumberSticks(); i++)
    {
        JoyControlStick *stick = getActiveSetJoystick()->getJoyStick(i);
        xml->writeStartElement("stickAxisAssociation");
        xml->writeAttribute("index", QString::number(stick->getRealJoyIndex()));
        xml->writeAttribute("xAxis", QString::number(stick->getAxisX()->getRealJoyIndex()));
        xml->writeAttribute("yAxis", QString::number(stick->getAxisY()->getRealJoyIndex()));
        xml->writeEndElement();
    }

    for (int i=0; i < getNumberVDPads(); i++)
    {
        VDPad *vdpad = getActiveSetJoystick()->getVDPad(i);
        xml->writeStartElement("vdpadButtonAssociations");
        xml->writeAttribute("index", QString::number(vdpad->getRealJoyNumber()));

        JoyButton *button = vdpad->getVButton(JoyDPadButton::DpadUp);
        if (button != nullptr)
        {
            xml->writeStartElement("vdpadButtonAssociation");

            if (typeid(*button) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = qobject_cast<JoyAxisButton*>(button); // static_cast
                xml->writeAttribute("axis", QString::number(axisbutton->getAxis()->getRealJoyIndex()));
                xml->writeAttribute("button", QString::number(button->getJoyNumber()));
            }
            else
            {
                xml->writeAttribute("axis", QString::number(0));
                xml->writeAttribute("button", QString::number(button->getRealJoyNumber()));
            }

            xml->writeAttribute("direction", QString::number(JoyDPadButton::DpadUp));
            xml->writeEndElement();
        }

        button = vdpad->getVButton(JoyDPadButton::DpadDown);
        if (button != nullptr)
        {
            xml->writeStartElement("vdpadButtonAssociation");

            if (typeid(*button) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = qobject_cast<JoyAxisButton*>(button); // static_cast
                xml->writeAttribute("axis", QString::number(axisbutton->getAxis()->getRealJoyIndex()));
                xml->writeAttribute("button", QString::number(button->getJoyNumber()));
            }
            else
            {
                xml->writeAttribute("axis", QString::number(0));
                xml->writeAttribute("button", QString::number(button->getRealJoyNumber()));
            }

            xml->writeAttribute("direction", QString::number(JoyDPadButton::DpadDown));
            xml->writeEndElement();
        }

        button = vdpad->getVButton(JoyDPadButton::DpadLeft);
        if (button != nullptr)
        {
            xml->writeStartElement("vdpadButtonAssociation");

            if (typeid(*button) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = qobject_cast<JoyAxisButton*>(button); // static_cast
                xml->writeAttribute("axis", QString::number(axisbutton->getAxis()->getRealJoyIndex()));
                xml->writeAttribute("button", QString::number(button->getJoyNumber()));
            }
            else
            {
                xml->writeAttribute("axis", QString::number(0));
                xml->writeAttribute("button", QString::number(button->getRealJoyNumber()));
            }

            xml->writeAttribute("direction", QString::number(JoyDPadButton::DpadLeft));
            xml->writeEndElement();
        }

        button = vdpad->getVButton(JoyDPadButton::DpadRight);
        if (button != nullptr)
        {
            xml->writeStartElement("vdpadButtonAssociation");

            if (typeid(*button) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = qobject_cast<JoyAxisButton*>(button); // static_cast
                xml->writeAttribute("axis", QString::number(axisbutton->getAxis()->getRealJoyIndex()));
                xml->writeAttribute("button", QString::number(button->getJoyNumber()));
            }
            else
            {
                xml->writeAttribute("axis", QString::number(0));
                xml->writeAttribute("button", QString::number(button->getRealJoyNumber()));
            }

            xml->writeAttribute("direction", QString::number(JoyDPadButton::DpadRight));
            xml->writeEndElement();
        }

        xml->writeEndElement();
    }

    bool tempHasNames = elementsHaveNames();
    if (tempHasNames)
    {
        xml->writeStartElement("names"); // <name>

        SetJoystick *tempSet = getActiveSetJoystick();
        for (int i=0; i < getNumberButtons(); i++)
        {
            JoyButton *button = tempSet->getJoyButton(i);
            if ((button != nullptr) && !button->getButtonName().isEmpty())
            {
                xml->writeStartElement("buttonname");
                xml->writeAttribute("index", QString::number(button->getRealJoyNumber()));
                xml->writeCharacters(button->getButtonName());
                xml->writeEndElement();
            }
        }

        for (int i=0; i < getNumberAxes(); i++)
        {
            JoyAxis *axis = tempSet->getJoyAxis(i);
            if (axis != nullptr)
            {
                if (!axis->getAxisName().isEmpty())
                {
                    xml->writeStartElement("axisname");
                    xml->writeAttribute("index", QString::number(axis->getRealJoyIndex()));
                    xml->writeCharacters(axis->getAxisName());
                    xml->writeEndElement();
                }

                JoyAxisButton *naxisbutton = axis->getNAxisButton();
                if (!naxisbutton->getButtonName().isEmpty())
                {
                    xml->writeStartElement("axisbuttonname");
                    xml->writeAttribute("index", QString::number(axis->getRealJoyIndex()));
                    xml->writeAttribute("button", QString::number(naxisbutton->getRealJoyNumber()));
                    xml->writeCharacters(naxisbutton->getButtonName());
                    xml->writeEndElement();
                }

                JoyAxisButton *paxisbutton = axis->getPAxisButton();
                if (!paxisbutton->getButtonName().isEmpty())
                {
                    xml->writeStartElement("axisbuttonname");
                    xml->writeAttribute("index", QString::number(axis->getRealJoyIndex()));
                    xml->writeAttribute("button", QString::number(paxisbutton->getRealJoyNumber()));
                    xml->writeCharacters(paxisbutton->getButtonName());
                    xml->writeEndElement();
                }
            }
        }

        for (int i=0; i < getNumberSticks(); i++)
        {
            JoyControlStick *stick = tempSet->getJoyStick(i);
            if (stick != nullptr)
            {
                if (!stick->getStickName().isEmpty())
                {
                    xml->writeStartElement("controlstickname");
                    xml->writeAttribute("index", QString::number(stick->getRealJoyIndex()));
                    xml->writeCharacters(stick->getStickName());
                    xml->writeEndElement();
                }

                QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton*> *buttons = stick->getButtons();
                QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> iter(*buttons);
                while (iter.hasNext())
                {
                    JoyControlStickButton *button = iter.next().value();
                    if (button && !button->getButtonName().isEmpty())
                    {
                        xml->writeStartElement("controlstickbuttonname");
                        xml->writeAttribute("index", QString::number(stick->getRealJoyIndex()));
                        xml->writeAttribute("button", QString::number(button->getRealJoyNumber()));
                        xml->writeCharacters(button->getButtonName());
                        xml->writeEndElement();
                    }
                }
            }
        }

        for (int i=0; i < getNumberHats(); i++)
        {
            JoyDPad *dpad = tempSet->getJoyDPad(i);
            if (dpad != nullptr)
            {
                if (!dpad->getDpadName().isEmpty())
                {
                    xml->writeStartElement("dpadname");
                    xml->writeAttribute("index", QString::number(dpad->getRealJoyNumber()));
                    xml->writeCharacters(dpad->getDpadName());
                    xml->writeEndElement();
                }

                QHash<int, JoyDPadButton*> *temp = dpad->getButtons();
                QHashIterator<int, JoyDPadButton*> iter(*temp);
                while (iter.hasNext())
                {
                    JoyDPadButton *button = iter.next().value();
                    if (button && !button->getButtonName().isEmpty())
                    {
                        xml->writeStartElement("dpadbuttonname");
                        xml->writeAttribute("index", QString::number(dpad->getRealJoyNumber()));
                        xml->writeAttribute("button", QString::number(button->getRealJoyNumber()));
                        xml->writeCharacters(button->getButtonName());
                        xml->writeEndElement();
                    }
                }
            }
        }

        for (int i=0; i < getNumberVDPads(); i++)
        {
            VDPad *vdpad = getActiveSetJoystick()->getVDPad(i);
            if (vdpad != nullptr)
            {
                if (!vdpad->getDpadName().isEmpty())
                {
                    xml->writeStartElement("vdpadname");
                    xml->writeAttribute("index", QString::number(vdpad->getRealJoyNumber()));
                    xml->writeCharacters(vdpad->getDpadName());
                    xml->writeEndElement();
                }

                QHash<int, JoyDPadButton*> *temp = vdpad->getButtons();
                QHashIterator<int, JoyDPadButton*> iter(*temp);
                while (iter.hasNext())
                {
                    JoyDPadButton *button = iter.next().value();
                    if ((button != nullptr) && !button->getButtonName().isEmpty())
                    {
                        xml->writeStartElement("vdpadbutton");
                        xml->writeAttribute("index", QString::number(vdpad->getRealJoyNumber()));
                        xml->writeAttribute("button", QString::number(button->getRealJoyNumber()));
                        xml->writeCharacters(button->getButtonName());
                        xml->writeEndElement();
                    }
                }
            }
        }

        xml->writeEndElement(); // </names>
    }


    if ((keyPressTime > 0) && (keyPressTime != GlobalVariables::InputDevice::DEFAULTKEYPRESSTIME))
    {
        xml->writeTextElement("keyPressTime", QString::number(keyPressTime));
    }

    xml->writeStartElement("sets");
    for (int i = 0; i < getJoystick_sets().size(); i++)
    {
        getJoystick_sets().value(i)->writeConfig(xml);
    }
    xml->writeEndElement();

    xml->writeEndElement();
}

void InputDevice::changeSetAxisButtonAssociation(int button_index, int axis_index, int originset, int newset, int mode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyAxisButton *button = nullptr;
    if (button_index == 0)
    {
        button = getJoystick_sets().value(newset)->getJoyAxis(axis_index)->getNAxisButton();
    }
    else if (button_index == 1)
    {
        button = getJoystick_sets().value(newset)->getJoyAxis(axis_index)->getPAxisButton();
    }

    JoyButton::SetChangeCondition tempmode = static_cast<JoyButton::SetChangeCondition>(mode);
    button->setChangeSetSelection(originset);
    button->setChangeSetCondition(tempmode, true);
}

void InputDevice::changeSetStickButtonAssociation(int button_index, int stick_index, int originset, int newset, int mode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyControlStickButton *button = getJoystick_sets().value(newset)->getJoyStick(stick_index)->getDirectionButton(static_cast<JoyControlStick::JoyStickDirections>(button_index));

    JoyButton::SetChangeCondition tempmode = static_cast<JoyButton::SetChangeCondition>(mode);
    button->setChangeSetSelection(originset);
    button->setChangeSetCondition(tempmode, true);
}

void InputDevice::changeSetDPadButtonAssociation(int button_index, int dpad_index, int originset, int newset, int mode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyDPadButton *button = getJoystick_sets().value(newset)->getJoyDPad(dpad_index)->getJoyButton(button_index);

    JoyButton::SetChangeCondition tempmode = static_cast<JoyButton::SetChangeCondition>(mode);
    button->setChangeSetSelection(originset);
    button->setChangeSetCondition(tempmode, true);
}

void InputDevice::changeSetVDPadButtonAssociation(int button_index, int dpad_index, int originset, int newset, int mode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyDPadButton *button = getJoystick_sets().value(newset)->getVDPad(dpad_index)->getJoyButton(button_index);

    JoyButton::SetChangeCondition tempmode = static_cast<JoyButton::SetChangeCondition>(mode);
    button->setChangeSetSelection(originset);
    button->setChangeSetCondition(tempmode, true);
}

void InputDevice::propogateSetAxisThrottleChange(int index, int originset)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    SetJoystick *currentSet = getJoystick_sets().value(originset);
    if (currentSet != nullptr)
    {
        JoyAxis *axis = currentSet->getJoyAxis(index);
        if (axis != nullptr)
        {
            int throttleSetting = axis->getThrottle();

            QHashIterator<int, SetJoystick*> iter(getJoystick_sets());
            while (iter.hasNext())
            {
                iter.next();
                SetJoystick *temp = iter.value();
                // Ignore change for set axis that initiated the change
                if (temp != currentSet)
                {
                    temp->getJoyAxis(index)->setThrottle(throttleSetting);
                }
            }
        }
    }
}

void InputDevice::removeControlStick(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    for (int i=0; i < GlobalVariables::InputDevice::NUMBER_JOYSETS; i++)
    {
        SetJoystick *currentset = getSetJoystick(i);
        if (currentset->getJoyStick(index))
        {
            currentset->removeControlStick(index);
        }
    }
}

bool InputDevice::isActive()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return buttonDownCount > 0;
}

void InputDevice::buttonDownEvent(int setindex, int buttonindex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_UNUSED(setindex);
    Q_UNUSED(buttonindex);

    bool old = isActive();
    buttonDownCount += 1;
    if (isActive() != old)
    {
        emit clicked(joyNumber);
    }
}

void InputDevice::buttonUpEvent(int setindex, int buttonindex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_UNUSED(setindex);
    Q_UNUSED(buttonindex);

    bool old = isActive();
    buttonDownCount -= 1;
    if (buttonDownCount < 0)
    {
        buttonDownCount = 0;
    }

    if (isActive() != old)
    {
        emit released(joyNumber);
    }
}

void InputDevice::buttonClickEvent(int buttonindex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    emit rawButtonClick(buttonindex);
}

void InputDevice::buttonReleaseEvent(int buttonindex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    emit rawButtonRelease(buttonindex);
}

void InputDevice::axisButtonDownEvent(int setindex, int axisindex, int buttonindex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_UNUSED(axisindex);

    buttonDownEvent(setindex, buttonindex);
}

void InputDevice::axisButtonUpEvent(int setindex, int axisindex, int buttonindex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_UNUSED(axisindex);

    buttonUpEvent(setindex, buttonindex);
}

void InputDevice::dpadButtonClickEvent(int buttonindex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyDPadButton *dpadbutton = qobject_cast<JoyDPadButton*>(sender()); // static_cast
    if (dpadbutton != nullptr)
    {
        emit rawDPadButtonClick(dpadbutton->getDPad()->getIndex(), buttonindex);
    }
}

void InputDevice::dpadButtonReleaseEvent(int buttonindex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyDPadButton *dpadbutton = qobject_cast<JoyDPadButton*>(sender()); // static_cast
    if (dpadbutton != nullptr)
    {
        emit rawDPadButtonRelease(dpadbutton->getDPad()->getIndex(), buttonindex);
    }
}

void InputDevice::dpadButtonDownEvent(int setindex, int dpadindex, int buttonindex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_UNUSED(dpadindex);

    buttonDownEvent(setindex, buttonindex);
}

void InputDevice::dpadButtonUpEvent(int setindex, int dpadindex, int buttonindex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_UNUSED(dpadindex);

    buttonUpEvent(setindex, buttonindex);
}

void InputDevice::stickButtonDownEvent(int setindex, int stickindex, int buttonindex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_UNUSED(stickindex);

    buttonDownEvent(setindex, buttonindex);
}

void InputDevice::stickButtonUpEvent(int setindex, int stickindex, int buttonindex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_UNUSED(stickindex);

    buttonUpEvent(setindex, buttonindex);
}

void InputDevice::setButtonName(int index, QString tempName)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<int, SetJoystick*> iter(getJoystick_sets());
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, &SetJoystick::setButtonNameChange, this, &InputDevice::updateSetButtonNames);
        JoyButton *button = tempSet->getJoyButton(index);
        if (button != nullptr)
        {
            button->setButtonName(tempName);
        }
        connect(tempSet, &SetJoystick::setButtonNameChange, this, &InputDevice::updateSetButtonNames);
    }
}

void InputDevice::setAxisButtonName(int axisIndex, int buttonIndex, QString tempName)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<int, SetJoystick*> iter(getJoystick_sets());
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, &SetJoystick::setAxisButtonNameChange, this, &InputDevice::updateSetAxisButtonNames);
        JoyAxis *axis = tempSet->getJoyAxis(axisIndex);
        if (axis != nullptr)
        {
            JoyAxisButton *button = nullptr;
            if (buttonIndex == 0)
            {
                button = axis->getNAxisButton();
            }
            else if (buttonIndex == 1)
            {
                button = axis->getPAxisButton();
            }

            if (button != nullptr)
            {
                button->setButtonName(tempName);
            }
        }
        connect(tempSet, &SetJoystick::setAxisButtonNameChange, this, &InputDevice::updateSetAxisButtonNames);
    }
}

void InputDevice::setStickButtonName(int stickIndex, int buttonIndex, QString tempName)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<int, SetJoystick*> iter(getJoystick_sets());
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, &SetJoystick::setStickButtonNameChange, this, &InputDevice::updateSetStickButtonNames);
        JoyControlStick *stick = tempSet->getJoyStick(stickIndex);
        if (stick != nullptr)
        {
            JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::JoyStickDirections(buttonIndex));
            if (button != nullptr)
            {
                button->setButtonName(tempName);
            }
        }
        connect(tempSet, &SetJoystick::setStickButtonNameChange, this, &InputDevice::updateSetStickButtonNames);
    }
}

void InputDevice::setDPadButtonName(int dpadIndex, int buttonIndex, QString tempName)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<int, SetJoystick*> iter(getJoystick_sets());
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, &SetJoystick::setDPadButtonNameChange, this, &InputDevice::updateSetDPadButtonNames);
        JoyDPad *dpad = tempSet->getJoyDPad(dpadIndex);
        if (dpad != nullptr)
        {
            JoyDPadButton *button = dpad->getJoyButton(buttonIndex);
            if (button != nullptr)
            {
                button->setButtonName(tempName);
            }
        }
        connect(tempSet, &SetJoystick::setDPadButtonNameChange, this, &InputDevice::updateSetDPadButtonNames);
    }
}

void InputDevice::setVDPadButtonName(int vdpadIndex, int buttonIndex, QString tempName)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<int, SetJoystick*> iter(getJoystick_sets());
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, &SetJoystick::setVDPadButtonNameChange, this, &InputDevice::updateSetVDPadButtonNames);
        VDPad *vdpad = tempSet->getVDPad(vdpadIndex);
        if (vdpad != nullptr)
        {
            JoyDPadButton *button = vdpad->getJoyButton(buttonIndex);
            if (button != nullptr)
            {
                button->setButtonName(tempName);
            }
        }
        connect(tempSet, &SetJoystick::setVDPadButtonNameChange, this, &InputDevice::updateSetVDPadButtonNames);
    }
}

void InputDevice::setAxisName(int axisIndex, QString tempName)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<int, SetJoystick*> iter(getJoystick_sets());
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, &SetJoystick::setAxisNameChange, this, &InputDevice::updateSetAxisNames);
        JoyAxis *axis = tempSet->getJoyAxis(axisIndex);
        if (axis != nullptr)
        {
            axis->setAxisName(tempName);
        }
        connect(tempSet, &SetJoystick::setAxisNameChange, this, &InputDevice::updateSetAxisNames);
    }
}

void InputDevice::setStickName(int stickIndex, QString tempName)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<int, SetJoystick*> iter(getJoystick_sets());
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, &SetJoystick::setStickNameChange, this, &InputDevice::updateSetStickNames);
        JoyControlStick *stick = tempSet->getJoyStick(stickIndex);
        if (stick != nullptr)
        {
            stick->setStickName(tempName);
        }
        connect(tempSet, &SetJoystick::setStickNameChange, this, &InputDevice::updateSetStickNames);
    }
}

void InputDevice::setDPadName(int dpadIndex, QString tempName)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<int, SetJoystick*> iter(getJoystick_sets());
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, &SetJoystick::setDPadNameChange, this, &InputDevice::updateSetDPadNames);
        JoyDPad *dpad = tempSet->getJoyDPad(dpadIndex);
        if (dpad != nullptr)
        {
            dpad->setDPadName(tempName);
        }
        connect(tempSet, &SetJoystick::setDPadNameChange, this, &InputDevice::updateSetDPadNames);
    }
}

void InputDevice::setVDPadName(int vdpadIndex, QString tempName)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<int, SetJoystick*> iter(getJoystick_sets());
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, &SetJoystick::setVDPadNameChange, this, &InputDevice::updateSetVDPadNames);
        VDPad *vdpad = tempSet->getVDPad(vdpadIndex);
        if (vdpad != nullptr)
        {
            vdpad->setDPadName(tempName);
        }
        connect(tempSet, &SetJoystick::setVDPadNameChange, this, &InputDevice::updateSetVDPadNames);
    }
}


void InputDevice::updateSetButtonNames(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButton *button = getActiveSetJoystick()->getJoyButton(index);
    if (button != nullptr)
    {
        setButtonName(index, button->getButtonName());
    }
}

void InputDevice::updateSetAxisButtonNames(int axisIndex, int buttonIndex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyAxis *axis = getActiveSetJoystick()->getJoyAxis(axisIndex);
    if (axis != nullptr)
    {
        JoyAxisButton *button = nullptr;
        if (buttonIndex == 0)
        {
            button = axis->getNAxisButton();
        }
        else if (buttonIndex == 1)
        {
            button = axis->getPAxisButton();
        }

        if (button != nullptr)
        {
            setAxisButtonName(axisIndex, buttonIndex, button->getButtonName());
        }
    }
}

void InputDevice::updateSetStickButtonNames(int stickIndex, int buttonIndex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyControlStick *stick = getActiveSetJoystick()->getJoyStick(stickIndex);
    if (stick != nullptr)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::JoyStickDirections(buttonIndex));
        if (button != nullptr)
        {
            setStickButtonName(stickIndex, buttonIndex, button->getButtonName());
        }
    }
}

void InputDevice::updateSetDPadButtonNames(int dpadIndex, int buttonIndex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyDPad *dpad = getActiveSetJoystick()->getJoyDPad(dpadIndex);
    if (dpad != nullptr)
    {
        JoyDPadButton *button = dpad->getJoyButton(buttonIndex);
        if (button != nullptr)
        {
            setDPadButtonName(dpadIndex, buttonIndex, button->getButtonName());
        }
    }
}

void InputDevice::updateSetVDPadButtonNames(int vdpadIndex, int buttonIndex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    VDPad *vdpad = getActiveSetJoystick()->getVDPad(vdpadIndex);
    if (vdpad != nullptr)
    {
        JoyDPadButton *button = vdpad->getJoyButton(buttonIndex);
        if (button != nullptr)
        {
            setVDPadButtonName(vdpadIndex, buttonIndex, button->getButtonName());
        }
    }
}

void InputDevice::updateSetAxisNames(int axisIndex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyAxis *axis = getActiveSetJoystick()->getJoyAxis(axisIndex);
    if (axis != nullptr)
    {
        setAxisName(axisIndex, axis->getAxisName());
    }
}

void InputDevice::updateSetStickNames(int stickIndex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyControlStick *stick = getActiveSetJoystick()->getJoyStick(stickIndex);
    if (stick != nullptr)
    {
        setStickName(stickIndex, stick->getStickName());
    }
}

void InputDevice::updateSetDPadNames(int dpadIndex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyDPad *dpad = getActiveSetJoystick()->getJoyDPad(dpadIndex);
    if (dpad != nullptr)
    {
        setDPadName(dpadIndex, dpad->getDpadName());
    }
}

void InputDevice::updateSetVDPadNames(int vdpadIndex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    VDPad *vdpad = getActiveSetJoystick()->getVDPad(vdpadIndex);
    if (vdpad != nullptr)
    {
        setVDPadName(vdpadIndex, vdpad->getDpadName());
    }
}

void InputDevice::resetButtonDownCount()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    buttonDownCount = 0;
    emit released(joyNumber);
}

void InputDevice::enableSetConnections(SetJoystick *setstick)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    connect(setstick, &SetJoystick::setChangeActivated, this, &InputDevice::resetButtonDownCount);
    connect(setstick, &SetJoystick::setChangeActivated, this, &InputDevice::setActiveSetNumber);
    connect(setstick, &SetJoystick::setChangeActivated, this, &InputDevice::propogateSetChange);
    connect(setstick, &SetJoystick::setAssignmentButtonChanged, this, &InputDevice::changeSetButtonAssociation);

    connect(setstick, &SetJoystick::setAssignmentAxisChanged, this, &InputDevice::changeSetAxisButtonAssociation);
    connect(setstick, &SetJoystick::setAssignmentDPadChanged, this, &InputDevice::changeSetDPadButtonAssociation);
    connect(setstick, &SetJoystick::setAssignmentVDPadChanged, this, &InputDevice::changeSetVDPadButtonAssociation);
    connect(setstick, &SetJoystick::setAssignmentStickChanged, this, &InputDevice::changeSetStickButtonAssociation);
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

    connect(setstick, &SetJoystick::setButtonNameChange, this, &InputDevice::updateSetButtonNames);
    connect(setstick, &SetJoystick::setAxisButtonNameChange, this, &InputDevice::updateSetAxisButtonNames);
    connect(setstick, &SetJoystick::setStickButtonNameChange, this, &InputDevice::updateSetStickButtonNames);
    connect(setstick, &SetJoystick::setDPadButtonNameChange, this, &InputDevice::updateSetDPadButtonNames);
    connect(setstick, &SetJoystick::setVDPadButtonNameChange, this, &InputDevice::updateSetVDPadButtonNames);

    connect(setstick, &SetJoystick::setAxisNameChange, this, &InputDevice::updateSetAxisNames);
    connect(setstick, &SetJoystick::setStickNameChange, this, &InputDevice::updateSetStickNames);
    connect(setstick, &SetJoystick::setDPadNameChange, this, &InputDevice::updateSetDPadNames);
    connect(setstick, &SetJoystick::setVDPadNameChange, this, &InputDevice::updateSetVDPadNames);
}

void InputDevice::axisActivatedEvent(int setindex, int axisindex, int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_UNUSED(setindex);

    emit rawAxisActivated(axisindex, value);
}

void InputDevice::axisReleasedEvent(int setindex, int axisindex, int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_UNUSED(setindex);

    emit rawAxisReleased(axisindex, value);
}

void InputDevice::setIndex(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (index >= 0)
    {
        joyNumber = index;
    }
    else
    {
        joyNumber = 0;
    }
}

void InputDevice::setDeviceKeyPressTime(int newPressTime)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    keyPressTime = newPressTime;
    emit propertyUpdated();
}

int InputDevice::getDeviceKeyPressTime()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return keyPressTime;
}

void InputDevice::profileEdited()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (!deviceEdited)
    {
        deviceEdited = true;
        emit profileUpdated();
    }
}

bool InputDevice::isDeviceEdited()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return deviceEdited;
}

void InputDevice::revertProfileEdited()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    deviceEdited = false;
}

QString InputDevice::getStringIdentifier()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString identifier = QString();
    QString tempGUID = getGUIDString();
    QString tempName = getSDLName();
    if (!tempGUID.isEmpty())
    {
        identifier = tempGUID;
    }
    else if (!tempName.isEmpty())
    {
        identifier = tempName;
    }

    return identifier;
}

void InputDevice::establishPropertyUpdatedConnection()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    connect(this, &InputDevice::propertyUpdated, this, &InputDevice::profileEdited);
}

void InputDevice::disconnectPropertyUpdatedConnection()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    disconnect(this, &InputDevice::propertyUpdated, this, &InputDevice::profileEdited);
}

void InputDevice::setKeyRepeatStatus(bool enabled)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    keyRepeatEnabled = enabled;
}

void InputDevice::setKeyRepeatDelay(int delay)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((delay >= 250) && (delay <= 1000))
    {
        keyRepeatDelay = delay;
    }
}

void InputDevice::setKeyRepeatRate(int rate)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((rate >= 20) && (rate <= 200))
    {
        keyRepeatRate = rate;
    }
}

bool InputDevice::isKeyRepeatEnabled()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return keyRepeatEnabled;
}

int InputDevice::getKeyRepeatDelay()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int tempKeyRepeatDelay = GlobalVariables::InputDevice::DEFAULTKEYREPEATDELAY;
    if (keyRepeatDelay != 0)
    {
        tempKeyRepeatDelay = keyRepeatDelay;
    }

    return tempKeyRepeatDelay;
}

int InputDevice::getKeyRepeatRate()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int tempKeyRepeatRate = GlobalVariables::InputDevice::DEFAULTKEYREPEATRATE;
    if (keyRepeatRate != 0)
    {
        tempKeyRepeatRate = keyRepeatRate;
    }

    return tempKeyRepeatRate;
}

void InputDevice::setProfileName(QString value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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

QString InputDevice::getProfileName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return profileName;
}

int InputDevice::getButtonDownCount()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return buttonDownCount;
}

QString InputDevice::getSDLPlatform()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = SDL_GetPlatform();
    return temp;
}


/**
 * @brief Check if device is using the SDL Game Controller API
 * @return Status showing if device is using the Game Controller API
 */
bool InputDevice::isGameController()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return false;
}

bool InputDevice::hasCalibrationThrottle(int axisNum)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = false;
    if (getCali().contains(axisNum))
    {
        result = true;
    }

    return result;
}

JoyAxis::ThrottleTypes InputDevice::getCalibrationThrottle(int axisNum)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getCali().value(axisNum);
}

void InputDevice::setCalibrationThrottle(int axisNum, JoyAxis::ThrottleTypes throttle)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (!getCali().contains(axisNum))
    {
        getCali().insert(axisNum, throttle);
    }
}

void InputDevice::removeCalibrationStatus(int axisNum)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (getCali().contains(axisNum))
    {
        getCali().remove(axisNum);
    }
}

void InputDevice::sendLoadProfileRequest(QString location)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (!location.isEmpty())
    {
        emit requestProfileLoad(location);
    }
}

AntiMicroSettings* InputDevice::getSettings()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_settings;
}

bool InputDevice::isKnownController()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = false;
    if (isGameController())
    {
        result = true;
    }
    else
    {
        m_settings->beginGroup("Mappings");
        if (m_settings->contains(getGUIDString()))
        {
            result = true;
        }
        else if (m_settings->contains(QString("%1%2").arg(getGUIDString()).arg("Disabled")))
        {
            result = true;
        }

        m_settings->endGroup();
    }

    return result;
}

void InputDevice::activatePossiblePendingEvents()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    activatePossibleControlStickEvents();
    activatePossibleAxisEvents();
    activatePossibleDPadEvents();
    activatePossibleVDPadEvents();
    activatePossibleButtonEvents();
}

void InputDevice::activatePossibleControlStickEvents()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    SetJoystick *currentSet = getActiveSetJoystick();
    for (int i=0; i < currentSet->getNumberSticks(); i++)
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    SetJoystick *currentSet = getActiveSetJoystick();
    for (int i=0; i < currentSet->getNumberAxes(); i++)
    {
        JoyAxis *tempAxis = currentSet->getJoyAxis(i);
        if ((tempAxis != nullptr) && tempAxis->hasPendingEvent())
        {
            tempAxis->activatePendingEvent();
        }
    }
}

void InputDevice::activatePossibleDPadEvents()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    SetJoystick *currentSet = getActiveSetJoystick();
    for (int i=0; i < currentSet->getNumberHats(); i++)
    {
        JoyDPad *tempDPad = currentSet->getJoyDPad(i);
        if ((tempDPad != nullptr) && tempDPad->hasPendingEvent())
        {
            tempDPad->activatePendingEvent();
        }
    }
}

void InputDevice::activatePossibleVDPadEvents()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    SetJoystick *currentSet = getActiveSetJoystick();
    for (int i=0; i < currentSet->getNumberVDPads(); i++)
    {
        VDPad *tempVDPad = currentSet->getVDPad(i);
        if ((tempVDPad != nullptr) && tempVDPad->hasPendingEvent())
        {
            tempVDPad->activatePendingEvent();
        }
    }
}

void InputDevice::activatePossibleButtonEvents()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    SetJoystick *currentSet = getActiveSetJoystick();
    for (int i=0; i < currentSet->getNumberButtons(); i++)
    {
        JoyButton *tempButton = currentSet->getJoyButton(i);
        if ((tempButton != nullptr) && tempButton->hasPendingEvent())
        {
            tempButton->activatePendingEvent();
        }
    }
}

bool InputDevice::elementsHaveNames()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = false;

    SetJoystick *tempSet = getActiveSetJoystick();
    for (int i=0; i < getNumberButtons() && !result; i++)
    {
        JoyButton *button = tempSet->getJoyButton(i);
        if ((button != nullptr) && !button->getButtonName().isEmpty())
        {
            result = true;
        }
    }

    for (int i=0; (i < getNumberAxes()) && !result; i++)
    {
        JoyAxis *axis = tempSet->getJoyAxis(i);
        if (axis != nullptr)
        {
            if (!axis->getAxisName().isEmpty())
            {
                result = true;
            }

            JoyAxisButton *naxisbutton = axis->getNAxisButton();
            if (!naxisbutton->getButtonName().isEmpty())
            {
                result = true;
            }

            JoyAxisButton *paxisbutton = axis->getPAxisButton();
            if (!paxisbutton->getButtonName().isEmpty())
            {
                result = true;
            }
        }
    }

    for (int i=0; (i < getNumberSticks()) && !result; i++)
    {
        JoyControlStick *stick = tempSet->getJoyStick(i);
        if (stick != nullptr)
        {
            if (!stick->getStickName().isEmpty())
            {
                result = true;
            }

            QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton*> *buttons = stick->getButtons();
            QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> iter(*buttons);
            while (iter.hasNext() && !result)
            {
                JoyControlStickButton *button = iter.next().value();
                if (button && !button->getButtonName().isEmpty())
                {
                    result = true;
                }
            }
        }
    }

    for (int i=0; (i < getNumberHats()) && !result; i++)
    {
        JoyDPad *dpad = tempSet->getJoyDPad(i);
        if (dpad != nullptr)
        {
            if (!dpad->getDpadName().isEmpty())
            {
                result = true;
            }

            QHash<int, JoyDPadButton*> *temp = dpad->getButtons();
            QHashIterator<int, JoyDPadButton*> iter(*temp);
            while (iter.hasNext() && !result)
            {
                JoyDPadButton *button = iter.next().value();
                if (button && !button->getButtonName().isEmpty())
                {
                    result = true;
                }
            }
        }
    }

    for (int i=0; (i < getNumberVDPads()) && !result; i++)
    {
        VDPad *vdpad = getActiveSetJoystick()->getVDPad(i);
        if (vdpad != nullptr)
        {
            if (!vdpad->getDpadName().isEmpty())
            {
                result = true;
            }

            QHash<int, JoyDPadButton*> *temp = vdpad->getButtons();
            QHashIterator<int, JoyDPadButton*> iter(*temp);
            while (iter.hasNext() && !result)
            {
                JoyDPadButton *button = iter.next().value();
                if ((button != nullptr) && !button->getButtonName().isEmpty())
                {
                    result = true;
                }
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
bool InputDevice::isEmptyGUID(QString tempGUID)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = false;

    if (tempGUID.contains(GlobalVariables::InputDevice::emptyGUID))
    {
        result = true;
    }

    return result;
}

/**
 * @brief Check if GUID passed matches the expected GUID for a device.
 *     Needed for xinput GUID abstraction.
 * @param GUID string
 * @return if GUID is considered a match.
 */
bool InputDevice::isRelevantGUID(QString tempGUID)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = false;
    if (tempGUID == getGUIDString())
    {
        result = true;
    }

    return result;
}

QString InputDevice::getRawGUIDString()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = getGUIDString();
    return temp;
}

void InputDevice::haltServices()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    emit requestWait();
}

void InputDevice::finalRemoval()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->closeSDLDevice();
    this->deleteLater();
}

void InputDevice::setRawAxisDeadZone(int deadZone)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((deadZone > 0) && (deadZone <= GlobalVariables::JoyAxis::AXISMAX))
    {
        this->rawAxisDeadZone = deadZone;
    }
    else
    {
        this->rawAxisDeadZone = GlobalVariables::InputDevice::RAISEDDEADZONE;
    }
}

int InputDevice::getRawAxisDeadZone()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return rawAxisDeadZone;
}

void InputDevice::rawAxisEvent(int index, int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    emit rawAxisMoved(index, value);
}

QHash<int, SetJoystick*>& InputDevice::getJoystick_sets() {

    return joystick_sets;
}

QHash<int, JoyAxis::ThrottleTypes>& InputDevice::getCali() {

    return cali;
}

SDL_JoystickID* InputDevice::getJoystickID() {

    return &joystickID;
}

QList<bool>& InputDevice::getButtonstatesLocal() {

    return buttonstates;
}

QList<int>& InputDevice::getAxesstatesLocal() {

    return axesstates;
}

QList<int>& InputDevice::getDpadstatesLocal() {

    return dpadstates;
}

SDL_Joystick* InputDevice::getJoyHandle() const
{
    return m_joyhandle;
}
