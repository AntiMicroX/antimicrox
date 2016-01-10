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

//#include <QDebug>
#include <typeinfo>

#include "inputdevice.h"

const int InputDevice::NUMBER_JOYSETS = 8;
const int InputDevice::DEFAULTKEYPRESSTIME = 100;
const unsigned int InputDevice::DEFAULTKEYREPEATDELAY = 660; // 660 ms
const unsigned int InputDevice::DEFAULTKEYREPEATRATE = 40; // 40 ms. 25 times per second
const int InputDevice::RAISEDDEADZONE = 20000;

QRegExp InputDevice::emptyGUID("^[0]+$");

InputDevice::InputDevice(int deviceIndex, AntiMicroSettings *settings, QObject *parent) :
    QObject(parent)
{
    buttonDownCount = 0;
    joyNumber = deviceIndex;
    active_set = 0;
    joystickID = 0;
    keyPressTime = 0;
    deviceEdited = false;
#ifdef Q_OS_WIN
    keyRepeatEnabled = true;
#else
    keyRepeatEnabled = false;
#endif

    keyRepeatDelay = 0;
    keyRepeatRate = 0;
    rawAxisDeadZone = RAISEDDEADZONE;
    this->settings = settings;
}

InputDevice::~InputDevice()
{
    QHashIterator<int, SetJoystick*> iter(joystick_sets);
    while (iter.hasNext())
    {
        SetJoystick *setjoystick = iter.next().value();
        if (setjoystick)
        {
            delete setjoystick;
            setjoystick = 0;
        }
    }

    joystick_sets.clear();
}

int InputDevice::getJoyNumber()
{
    return joyNumber;
}

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
    //cali.clear();
    //buttonstates.clear();
    //axesstates.clear();
    //dpadstates.clear();

    for (int i=0; i < NUMBER_JOYSETS; i++)
    {
        SetJoystick* set = joystick_sets.value(i);
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
    // Grab current states for all elements in old set
    SetJoystick *current_set = joystick_sets.value(active_set);
    for (int i = 0; i < current_set->getNumberButtons(); i++)
    {
        JoyButton *button = current_set->getJoyButton(i);
        buttonstates.append(button->getButtonState());
    }

    for (int i = 0; i < current_set->getNumberAxes(); i++)
    {
        JoyAxis *axis = current_set->getJoyAxis(i);
        axesstates.append(axis->getCurrentRawValue());
    }

    for (int i = 0; i < current_set->getNumberHats(); i++)
    {
        JoyDPad *dpad = current_set->getJoyDPad(i);
        dpadstates.append(dpad->getCurrentDirection());
    }

    reset();
}

void InputDevice::reInitButtons()
{
    SetJoystick *current_set = joystick_sets.value(active_set);
    for (int i = 0; i < current_set->getNumberButtons(); i++)
    {
        bool value = buttonstates.at(i);
        JoyButton *button = current_set->getJoyButton(i);
        //button->joyEvent(value);
        button->queuePendingEvent(value);
    }

    for (int i = 0; i < current_set->getNumberAxes(); i++)
    {
        int value = axesstates.at(i);
        JoyAxis *axis = current_set->getJoyAxis(i);
        //axis->joyEvent(value);
        axis->queuePendingEvent(value);
    }

    for (int i = 0; i < current_set->getNumberHats(); i++)
    {
        int value = dpadstates.at(i);
        JoyDPad *dpad = current_set->getJoyDPad(i);
        //dpad->joyEvent(value);
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
    if ((index >= 0 && index < NUMBER_JOYSETS) && (index != active_set))
    {
        QList<bool> buttonstates;
        QList<int> axesstates;
        QList<int> dpadstates;
        QList<JoyControlStick::JoyStickDirections> stickstates;
        QList<int> vdpadstates;

        // Grab current states for all elements in old set
        SetJoystick *current_set = joystick_sets.value(active_set);
        SetJoystick *old_set = current_set;
        SetJoystick *tempSet = joystick_sets.value(index);

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
            if (button)
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
        joystick_sets.value(active_set)->release();
        active_set = index;

        // Activate all buttons in the switched set
        current_set = joystick_sets.value(active_set);

        for (int i=0; i < current_set->getNumberSticks(); i++)
        {
            JoyControlStick::JoyStickDirections value = stickstates.at(i);
            //bool tempignore = true;
            bool tempignore = false;
            QList<JoyControlStickButton*> buttonList;
            QList<JoyControlStickButton*> oldButtonList;
            JoyControlStick *stick = current_set->getJoyStick(i);
            JoyControlStick *oldStick = old_set->getJoyStick(i);

            if (stick->getJoyMode() == JoyControlStick::StandardMode && value)
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

            for (int j=0; j < buttonList.size(); j++)
            {
                JoyControlStickButton *button = buttonList.at(j);
                JoyControlStickButton *oldButton = oldButtonList.at(j);

                if (button && oldButton)
                {
                    if (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld)
                    {
                        if (oldButton->getChangeSetCondition() == JoyButton::SetChangeWhileHeld && oldButton->getWhileHeldStatus())
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
            }
        }

        // Activate all dpad buttons in the switched set
        for (int i = 0; i < current_set->getNumberVDPads(); i++)
        {
            int value = vdpadstates.at(i);
            //bool tempignore = true;
            bool tempignore = false;
            JoyDPad *dpad = current_set->getVDPad(i);
            QList<JoyDPadButton*> buttonList;
            QList<JoyDPadButton*> oldButtonList;

            if (dpad->getJoyMode() == JoyDPad::StandardMode && value)
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
            else if (value)
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

            for (int j=0; j < buttonList.size(); j++)
            {
                JoyDPadButton *button = buttonList.at(j);
                JoyDPadButton *oldButton = oldButtonList.at(j);

                if (button && oldButton)
                {
                    if (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld)
                    {
                        if (value)
                        {
                            if (oldButton->getChangeSetCondition() == JoyButton::SetChangeWhileHeld && oldButton->getWhileHeldStatus())
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
        }

        for (int i = 0; i < current_set->getNumberButtons(); i++)
        {
            bool value = buttonstates.at(i);
            //bool tempignore = true;
            bool tempignore = false;
            JoyButton *button = current_set->getJoyButton(i);
            JoyButton *oldButton = old_set->getJoyButton(i);
            if (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld)
            {
                if (value)
                {
                    if (oldButton->getChangeSetCondition() == JoyButton::SetChangeWhileHeld && oldButton->getWhileHeldStatus())
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
                    //tempignore = false;
                }
            }

            //button->joyEvent(value, tempignore);
            button->queuePendingEvent(value, tempignore);
        }

        // Activate all axis buttons in the switched set
        for (int i = 0; i < current_set->getNumberAxes(); i++)
        {
            int value = axesstates.at(i);
            //bool tempignore = true;
            bool tempignore = false;
            JoyAxis *axis = current_set->getJoyAxis(i);
            JoyAxisButton *oldButton = old_set->getJoyAxis(i)->getAxisButtonByValue(value);
            JoyAxisButton *button = axis->getAxisButtonByValue(value);

            if (button && oldButton)
            {
                if (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld)
                {
                    if (oldButton->getChangeSetCondition() == JoyButton::SetChangeWhileHeld && oldButton->getWhileHeldStatus())
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
            else if (!button)
            {
                // Ensure that set change events are performed if needed.
                axis->getPAxisButton()->setWhileHeldStatus(false);
                axis->getNAxisButton()->setWhileHeldStatus(false);
            }

            //axis->joyEvent(value, tempignore);
            axis->queuePendingEvent(value, tempignore, false);
        }

        // Activate all dpad buttons in the switched set
        for (int i = 0; i < current_set->getNumberHats(); i++)
        {
            int value = dpadstates.at(i);
            //bool tempignore = true;
            bool tempignore = false;
            JoyDPad *dpad = current_set->getJoyDPad(i);
            QList<JoyDPadButton*> buttonList;
            QList<JoyDPadButton*> oldButtonList;

            if (dpad->getJoyMode() == JoyDPad::StandardMode && value)
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
            else if (value)
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

            for (int j=0; j < buttonList.size(); j++)
            {
                JoyDPadButton *button = buttonList.at(j);
                JoyDPadButton *oldButton = oldButtonList.at(j);

                if (button && oldButton)
                {
                    if (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld)
                    {
                        if (value)
                        {
                            if (oldButton->getChangeSetCondition() == JoyButton::SetChangeWhileHeld && oldButton->getWhileHeldStatus())
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

            //dpad->joyEvent(value, tempignore);
            dpad->queuePendingEvent(value, tempignore);
        }

        activatePossibleControlStickEvents();
        activatePossibleAxisEvents();
        activatePossibleDPadEvents();
        activatePossibleVDPadEvents();
        activatePossibleButtonEvents();
        /*if (JoyButton::shouldInvokeMouseEvents())
        {
            // Run mouse events early if needed.
            JoyButton::invokeMouseEvents();
        }
        */
    }
}

int InputDevice::getActiveSetNumber()
{
    return active_set;
}

SetJoystick* InputDevice::getActiveSetJoystick()
{
    return joystick_sets.value(active_set);
}

int InputDevice::getNumberButtons()
{
    return getActiveSetJoystick()->getNumberButtons();
}

int InputDevice::getNumberAxes()
{
    return getActiveSetJoystick()->getNumberAxes();
}

int InputDevice::getNumberHats()
{
    return getActiveSetJoystick()->getNumberHats();
}

int InputDevice::getNumberSticks()
{
    return getActiveSetJoystick()->getNumberSticks();
}

int InputDevice::getNumberVDPads()
{
    return getActiveSetJoystick()->getNumberVDPads();
}

SetJoystick* InputDevice::getSetJoystick(int index)
{
    return joystick_sets.value(index);
}

void InputDevice::propogateSetChange(int index)
{
    emit setChangeActivated(index);
}

void InputDevice::changeSetButtonAssociation(int button_index, int originset, int newset, int mode)
{
    JoyButton *button = joystick_sets.value(newset)->getJoyButton(button_index);
    JoyButton::SetChangeCondition tempmode = (JoyButton::SetChangeCondition)mode;
    button->setChangeSetSelection(originset);
    button->setChangeSetCondition(tempmode, true);
}

void InputDevice::readConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && xml->name() == getXmlName())
    {
        //reset();
        transferReset();

        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != getXmlName()))
        {
            if (xml->name() == "sets" && xml->isStartElement())
            {
                xml->readNextStartElement();

                while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != "sets"))
                {
                    if (xml->name() == "set" && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        index = index - 1;
                        if (index >= 0 && index < joystick_sets.size())
                        {
                            joystick_sets.value(index)->readConfig(xml);
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
            else if (xml->name() == "stickAxisAssociation" && xml->isStartElement())
            {
                int stickIndex = xml->attributes().value("index").toString().toInt();
                int xAxis = xml->attributes().value("xAxis").toString().toInt();
                int yAxis = xml->attributes().value("yAxis").toString().toInt();

                if (stickIndex > 0 && xAxis > 0 && yAxis > 0)
                {
                    xAxis -= 1;
                    yAxis -= 1;
                    stickIndex -= 1;

                    for (int i=0; i <joystick_sets.size(); i++)
                    {
                        SetJoystick *currentset = joystick_sets.value(i);
                        JoyAxis *axis1 = currentset->getJoyAxis(xAxis);
                        JoyAxis *axis2 = currentset->getJoyAxis(yAxis);
                        if (axis1 && axis2)
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
            else if (xml->name() == "vdpadButtonAssociations" && xml->isStartElement())
            {
                int vdpadIndex = xml->attributes().value("index").toString().toInt();
                if (vdpadIndex > 0)
                {
                    for (int i=0; i <joystick_sets.size(); i++)
                    {
                        SetJoystick *currentset = joystick_sets.value(i);
                        VDPad *vdpad = currentset->getVDPad(vdpadIndex-1);
                        if (!vdpad)
                        {
                            vdpad = new VDPad(vdpadIndex-1, i, currentset, currentset);
                            currentset->addVDPad(vdpadIndex-1, vdpad);
                        }
                    }

                    xml->readNextStartElement();
                    while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != "vdpadButtonAssociations"))
                    {
                        if (xml->name() == "vdpadButtonAssociation" && xml->isStartElement())
                        {
                            int vdpadAxisIndex = xml->attributes().value("axis").toString().toInt();
                            int vdpadButtonIndex = xml->attributes().value("button").toString().toInt();
                            int vdpadDirection = xml->attributes().value("direction").toString().toInt();

                            if (vdpadAxisIndex > 0 && vdpadDirection > 0)
                            {
                                vdpadAxisIndex -= 1;
                                for (int i=0; i < joystick_sets.size(); i++)
                                {
                                    SetJoystick *currentset = joystick_sets.value(i);
                                    VDPad *vdpad = currentset->getVDPad(vdpadIndex-1);
                                    if (vdpad)
                                    {
                                        JoyAxis *axis = currentset->getJoyAxis(vdpadAxisIndex);
                                        if (axis)
                                        {
                                            JoyButton *button = 0;
                                            if (vdpadButtonIndex == 0)
                                            {
                                                button = axis->getNAxisButton();
                                            }
                                            else if (vdpadButtonIndex == 1)
                                            {
                                                button = axis->getPAxisButton();
                                            }

                                            if (button)
                                            {
                                                vdpad->addVButton((JoyDPadButton::JoyDPadDirections)vdpadDirection, button);
                                            }
                                        }
                                    }
                                }
                            }
                            else if (vdpadButtonIndex > 0 && vdpadDirection > 0)
                            {
                                vdpadButtonIndex -= 1;

                                for (int i=0; i < joystick_sets.size(); i++)
                                {
                                    SetJoystick *currentset = joystick_sets.value(i);
                                    VDPad *vdpad = currentset->getVDPad(vdpadIndex-1);
                                    if (vdpad)
                                    {
                                        JoyButton *button = currentset->getJoyButton(vdpadButtonIndex);
                                        if (button)
                                        {
                                            vdpad->addVButton((JoyDPadButton::JoyDPadDirections)vdpadDirection, button);
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

                for (int i=0; i < joystick_sets.size(); i++)
                {
                    SetJoystick *currentset = joystick_sets.value(i);
                    for (int j=0; j < currentset->getNumberVDPads(); j++)
                    {
                        VDPad *vdpad = currentset->getVDPad(j);
                        if (vdpad && vdpad->isEmpty())
                        {
                            currentset->removeVDPad(j);
                        }
                    }
                }
            }
            else if (xml->name() == "names" && xml->isStartElement())
            {
                xml->readNextStartElement();
                while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != "names"))
                {
                    if (xml->name() == "buttonname" && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if (index >= 0 && !temp.isEmpty())
                        {
                            setButtonName(index, temp);
                        }
                    }
                    else if (xml->name() == "axisbuttonname" && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        int buttonIndex = xml->attributes().value("button").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        buttonIndex = buttonIndex - 1;
                        if (index >= 0 && !temp.isEmpty())
                        {
                            setAxisButtonName(index, buttonIndex, temp);
                        }
                    }
                    else if (xml->name() == "controlstickbuttonname" && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        int buttonIndex = xml->attributes().value("button").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if (index >= 0 && !temp.isEmpty())
                        {
                            setStickButtonName(index, buttonIndex, temp);
                        }
                    }
                    else if (xml->name() == "dpadbuttonname" && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        int buttonIndex = xml->attributes().value("button").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if (index >= 0 && !temp.isEmpty())
                        {
                            setDPadButtonName(index, buttonIndex, temp);
                        }
                    }
                    else if (xml->name() == "vdpadbuttonname" && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        int buttonIndex = xml->attributes().value("button").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if (index >= 0 && !temp.isEmpty())
                        {
                            setVDPadButtonName(index, buttonIndex, temp);
                        }
                    }
                    else if (xml->name() == "axisname" && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if (index >= 0 && !temp.isEmpty())
                        {
                            setAxisName(index, temp);
                        }
                    }
                    else if (xml->name() == "controlstickname" && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if (index >= 0 && !temp.isEmpty())
                        {
                            setStickName(index, temp);
                        }
                    }
                    else if (xml->name() == "dpadname" && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if (index >= 0 && !temp.isEmpty())
                        {
                            setDPadName(index, temp);
                        }
                    }
                    else if (xml->name() == "vdpadname" && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if (index >= 0 && !temp.isEmpty())
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
            else if (xml->name() == "keyPressTime" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                if (tempchoice >= 10)
                {
                    this->setDeviceKeyPressTime(tempchoice);
                }
            }
            else if (xml->name() == "profilename" && xml->isStartElement())
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
    xml->writeStartElement(getXmlName());
    xml->writeAttribute("configversion", QString::number(PadderCommon::LATESTCONFIGFILEVERSION));
    xml->writeAttribute("appversion", PadderCommon::programVersion);

    xml->writeComment("The SDL name for a joystick is included for informational purposes only.");
    xml->writeTextElement("sdlname", getSDLName());
#ifdef USE_SDL_2
    xml->writeComment("The GUID for a joystick is included for informational purposes only.");
    xml->writeTextElement("guid", getGUIDString());
#endif

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
        if (button)
        {
            xml->writeStartElement("vdpadButtonAssociation");

            if (typeid(*button) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = static_cast<JoyAxisButton*>(button);
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
        if (button)
        {
            xml->writeStartElement("vdpadButtonAssociation");

            if (typeid(*button) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = static_cast<JoyAxisButton*>(button);
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
        if (button)
        {
            xml->writeStartElement("vdpadButtonAssociation");

            if (typeid(*button) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = static_cast<JoyAxisButton*>(button);
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
        if (button)
        {
            xml->writeStartElement("vdpadButtonAssociation");

            if (typeid(*button) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = static_cast<JoyAxisButton*>(button);
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
            if (button && !button->getButtonName().isEmpty())
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
            if (axis)
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
            if (stick)
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
            if (dpad)
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
            if (vdpad)
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
                    if (button && !button->getButtonName().isEmpty())
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


    if (keyPressTime > 0 && keyPressTime != DEFAULTKEYPRESSTIME)
    {
        xml->writeTextElement("keyPressTime", QString::number(keyPressTime));
    }

    xml->writeStartElement("sets");
    for (int i=0; i < joystick_sets.size(); i++)
    {
        joystick_sets.value(i)->writeConfig(xml);
    }
    xml->writeEndElement();

    xml->writeEndElement();
}

void InputDevice::changeSetAxisButtonAssociation(int button_index, int axis_index, int originset, int newset, int mode)
{
    JoyAxisButton *button = 0;
    if (button_index == 0)
    {
        button = joystick_sets.value(newset)->getJoyAxis(axis_index)->getNAxisButton();
    }
    else if (button_index == 1)
    {
        button = joystick_sets.value(newset)->getJoyAxis(axis_index)->getPAxisButton();
    }

    JoyButton::SetChangeCondition tempmode = (JoyButton::SetChangeCondition)mode;
    button->setChangeSetSelection(originset);
    button->setChangeSetCondition(tempmode, true);
}

void InputDevice::changeSetStickButtonAssociation(int button_index, int stick_index, int originset, int newset, int mode)
{
    JoyControlStickButton *button = joystick_sets.value(newset)->getJoyStick(stick_index)->getDirectionButton((JoyControlStick::JoyStickDirections)button_index);

    JoyButton::SetChangeCondition tempmode = (JoyButton::SetChangeCondition)mode;
    button->setChangeSetSelection(originset);
    button->setChangeSetCondition(tempmode, true);
}

void InputDevice::changeSetDPadButtonAssociation(int button_index, int dpad_index, int originset, int newset, int mode)
{
    JoyDPadButton *button = joystick_sets.value(newset)->getJoyDPad(dpad_index)->getJoyButton(button_index);

    JoyButton::SetChangeCondition tempmode = (JoyButton::SetChangeCondition)mode;
    button->setChangeSetSelection(originset);
    button->setChangeSetCondition(tempmode, true);
}

void InputDevice::changeSetVDPadButtonAssociation(int button_index, int dpad_index, int originset, int newset, int mode)
{
    JoyDPadButton *button = joystick_sets.value(newset)->getVDPad(dpad_index)->getJoyButton(button_index);

    JoyButton::SetChangeCondition tempmode = (JoyButton::SetChangeCondition)mode;
    button->setChangeSetSelection(originset);
    button->setChangeSetCondition(tempmode, true);
}

void InputDevice::propogateSetAxisThrottleChange(int index, int originset)
{
    SetJoystick *currentSet = joystick_sets.value(originset);
    if (currentSet)
    {
        JoyAxis *axis = currentSet->getJoyAxis(index);
        if (axis)
        {
            int throttleSetting = axis->getThrottle();

            QHashIterator<int, SetJoystick*> iter(joystick_sets);
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
    for (int i=0; i < NUMBER_JOYSETS; i++)
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
    return buttonDownCount > 0;
}

void InputDevice::buttonDownEvent(int setindex, int buttonindex)
{
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
    emit rawButtonClick(buttonindex);
}

void InputDevice::buttonReleaseEvent(int buttonindex)
{
    emit rawButtonRelease(buttonindex);
}

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
    JoyDPadButton *dpadbutton = static_cast<JoyDPadButton*>(sender());
    if (dpadbutton)
    {
        emit rawDPadButtonClick(dpadbutton->getDPad()->getIndex(), buttonindex);
    }
}

void InputDevice::dpadButtonReleaseEvent(int buttonindex)
{
    JoyDPadButton *dpadbutton = static_cast<JoyDPadButton*>(sender());
    if (dpadbutton)
    {
        emit rawDPadButtonRelease(dpadbutton->getDPad()->getIndex(), buttonindex);
    }
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

void InputDevice::setButtonName(int index, QString tempName)
{
    QHashIterator<int, SetJoystick*> iter(joystick_sets);
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, SIGNAL(setButtonNameChange(int)), this, SLOT(updateSetButtonNames(int)));
        JoyButton *button = tempSet->getJoyButton(index);
        if (button)
        {
            button->setButtonName(tempName);
        }
        connect(tempSet, SIGNAL(setButtonNameChange(int)), this, SLOT(updateSetButtonNames(int)));
    }
}

void InputDevice::setAxisButtonName(int axisIndex, int buttonIndex, QString tempName)
{
    QHashIterator<int, SetJoystick*> iter(joystick_sets);
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, SIGNAL(setAxisButtonNameChange(int,int)), this, SLOT(updateSetAxisButtonNames(int,int)));
        JoyAxis *axis = tempSet->getJoyAxis(axisIndex);
        if (axis)
        {
            JoyAxisButton *button = 0;
            if (buttonIndex == 0)
            {
                button = axis->getNAxisButton();
            }
            else if (buttonIndex == 1)
            {
                button = axis->getPAxisButton();
            }

            if (button)
            {
                button->setButtonName(tempName);
            }
        }
        connect(tempSet, SIGNAL(setAxisButtonNameChange(int,int)), this, SLOT(updateSetAxisButtonNames(int,int)));
    }
}

void InputDevice::setStickButtonName(int stickIndex, int buttonIndex, QString tempName)
{
    QHashIterator<int, SetJoystick*> iter(joystick_sets);
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, SIGNAL(setStickButtonNameChange(int,int)), this, SLOT(updateSetStickButtonNames(int,int)));
        JoyControlStick *stick = tempSet->getJoyStick(stickIndex);
        if (stick)
        {
            JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::JoyStickDirections(buttonIndex));
            if (button)
            {
                button->setButtonName(tempName);
            }
        }
        connect(tempSet, SIGNAL(setStickButtonNameChange(int,int)), this, SLOT(updateSetStickButtonNames(int,int)));
    }
}

void InputDevice::setDPadButtonName(int dpadIndex, int buttonIndex, QString tempName)
{
    QHashIterator<int, SetJoystick*> iter(joystick_sets);
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, SIGNAL(setDPadButtonNameChange(int,int)), this, SLOT(updateSetDPadButtonNames(int,int)));
        JoyDPad *dpad = tempSet->getJoyDPad(dpadIndex);
        if (dpad)
        {
            JoyDPadButton *button = dpad->getJoyButton(buttonIndex);
            if (button)
            {
                button->setButtonName(tempName);
            }
        }
        connect(tempSet, SIGNAL(setDPadButtonNameChange(int,int)), this, SLOT(updateSetDPadButtonNames(int,int)));
    }
}

void InputDevice::setVDPadButtonName(int vdpadIndex, int buttonIndex, QString tempName)
{
    QHashIterator<int, SetJoystick*> iter(joystick_sets);
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, SIGNAL(setVDPadButtonNameChange(int,int)), this, SLOT(updateSetVDPadButtonNames(int,int)));
        VDPad *vdpad = tempSet->getVDPad(vdpadIndex);
        if (vdpad)
        {
            JoyDPadButton *button = vdpad->getJoyButton(buttonIndex);
            if (button)
            {
                button->setButtonName(tempName);
            }
        }
        connect(tempSet, SIGNAL(setVDPadButtonNameChange(int,int)), this, SLOT(updateSetVDPadButtonNames(int,int)));
    }
}

void InputDevice::setAxisName(int axisIndex, QString tempName)
{
    QHashIterator<int, SetJoystick*> iter(joystick_sets);
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, SIGNAL(setAxisNameChange(int)), this, SLOT(updateSetAxisNames(int)));
        JoyAxis *axis = tempSet->getJoyAxis(axisIndex);
        if (axis)
        {
            axis->setAxisName(tempName);
        }
        connect(tempSet, SIGNAL(setAxisNameChange(int)), this, SLOT(updateSetAxisNames(int)));
    }
}

void InputDevice::setStickName(int stickIndex, QString tempName)
{
    QHashIterator<int, SetJoystick*> iter(joystick_sets);
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, SIGNAL(setStickNameChange(int)), this, SLOT(updateSetStickNames(int)));
        JoyControlStick *stick = tempSet->getJoyStick(stickIndex);
        if (stick)
        {
            stick->setStickName(tempName);
        }
        connect(tempSet, SIGNAL(setStickNameChange(int)), this, SLOT(updateSetStickNames(int)));
    }
}

void InputDevice::setDPadName(int dpadIndex, QString tempName)
{
    QHashIterator<int, SetJoystick*> iter(joystick_sets);
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, SIGNAL(setDPadNameChange(int)), this, SLOT(updateSetDPadNames(int)));
        JoyDPad *dpad = tempSet->getJoyDPad(dpadIndex);
        if (dpad)
        {
            dpad->setDPadName(tempName);
        }
        connect(tempSet, SIGNAL(setDPadNameChange(int)), this, SLOT(updateSetDPadNames(int)));
    }
}

void InputDevice::setVDPadName(int vdpadIndex, QString tempName)
{
    QHashIterator<int, SetJoystick*> iter(joystick_sets);
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, SIGNAL(setVDPadNameChange(int)), this, SLOT(updateSetVDPadNames(int)));
        VDPad *vdpad = tempSet->getVDPad(vdpadIndex);
        if (vdpad)
        {
            vdpad->setDPadName(tempName);
        }
        connect(tempSet, SIGNAL(setVDPadNameChange(int)), this, SLOT(updateSetVDPadNames(int)));
    }
}


void InputDevice::updateSetButtonNames(int index)
{
    JoyButton *button = getActiveSetJoystick()->getJoyButton(index);
    if (button)
    {
        setButtonName(index, button->getButtonName());
    }
}

void InputDevice::updateSetAxisButtonNames(int axisIndex, int buttonIndex)
{
    JoyAxis *axis = getActiveSetJoystick()->getJoyAxis(axisIndex);
    if (axis)
    {
        JoyAxisButton *button = 0;
        if (buttonIndex == 0)
        {
            button = axis->getNAxisButton();
        }
        else if (buttonIndex == 1)
        {
            button = axis->getPAxisButton();
        }

        if (button)
        {
            setAxisButtonName(axisIndex, buttonIndex, button->getButtonName());
        }
    }
}

void InputDevice::updateSetStickButtonNames(int stickIndex, int buttonIndex)
{
    JoyControlStick *stick = getActiveSetJoystick()->getJoyStick(stickIndex);
    if (stick)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::JoyStickDirections(buttonIndex));
        if (button)
        {
            setStickButtonName(stickIndex, buttonIndex, button->getButtonName());
        }
    }
}

void InputDevice::updateSetDPadButtonNames(int dpadIndex, int buttonIndex)
{
    JoyDPad *dpad = getActiveSetJoystick()->getJoyDPad(dpadIndex);
    if (dpad)
    {
        JoyDPadButton *button = dpad->getJoyButton(buttonIndex);
        if (button)
        {
            setDPadButtonName(dpadIndex, buttonIndex, button->getButtonName());
        }
    }
}

void InputDevice::updateSetVDPadButtonNames(int vdpadIndex, int buttonIndex)
{
    VDPad *vdpad = getActiveSetJoystick()->getVDPad(vdpadIndex);
    if (vdpad)
    {
        JoyDPadButton *button = vdpad->getJoyButton(buttonIndex);
        if (button)
        {
            setVDPadButtonName(vdpadIndex, buttonIndex, button->getButtonName());
        }
    }
}

void InputDevice::updateSetAxisNames(int axisIndex)
{
    JoyAxis *axis = getActiveSetJoystick()->getJoyAxis(axisIndex);
    if (axis)
    {
        setAxisName(axisIndex, axis->getAxisName());
    }
}

void InputDevice::updateSetStickNames(int stickIndex)
{
    JoyControlStick *stick = getActiveSetJoystick()->getJoyStick(stickIndex);
    if (stick)
    {
        setStickName(stickIndex, stick->getStickName());
    }
}

void InputDevice::updateSetDPadNames(int dpadIndex)
{
    JoyDPad *dpad = getActiveSetJoystick()->getJoyDPad(dpadIndex);
    if (dpad)
    {
        setDPadName(dpadIndex, dpad->getDpadName());
    }
}

void InputDevice::updateSetVDPadNames(int vdpadIndex)
{
    VDPad *vdpad = getActiveSetJoystick()->getVDPad(vdpadIndex);
    if (vdpad)
    {
        setVDPadName(vdpadIndex, vdpad->getDpadName());
    }
}

void InputDevice::resetButtonDownCount()
{
    buttonDownCount = 0;
    emit released(joyNumber);
}

void InputDevice::enableSetConnections(SetJoystick *setstick)
{
    connect(setstick, SIGNAL(setChangeActivated(int)), this, SLOT(resetButtonDownCount()));
    connect(setstick, SIGNAL(setChangeActivated(int)), this, SLOT(setActiveSetNumber(int)));
    connect(setstick, SIGNAL(setChangeActivated(int)), this, SLOT(propogateSetChange(int)));
    connect(setstick, SIGNAL(setAssignmentButtonChanged(int,int,int,int)), this, SLOT(changeSetButtonAssociation(int,int,int,int)));

    connect(setstick, SIGNAL(setAssignmentAxisChanged(int,int,int,int,int)), this, SLOT(changeSetAxisButtonAssociation(int,int,int,int,int)));
    connect(setstick, SIGNAL(setAssignmentDPadChanged(int,int,int,int,int)), this, SLOT(changeSetDPadButtonAssociation(int,int,int,int,int)));
    connect(setstick, SIGNAL(setAssignmentVDPadChanged(int,int,int,int,int)), this, SLOT(changeSetVDPadButtonAssociation(int,int,int,int,int)));
    connect(setstick, SIGNAL(setAssignmentStickChanged(int,int,int,int,int)), this, SLOT(changeSetStickButtonAssociation(int,int,int,int,int)));
    connect(setstick, SIGNAL(setAssignmentAxisThrottleChanged(int,int)), this, SLOT(propogateSetAxisThrottleChange(int, int)));

    connect(setstick, SIGNAL(setButtonClick(int,int)), this, SLOT(buttonDownEvent(int,int)));

    connect(setstick, SIGNAL(setButtonRelease(int,int)), this, SLOT(buttonUpEvent(int,int)));

    connect(setstick, SIGNAL(setAxisButtonClick(int,int,int)), this, SLOT(axisButtonDownEvent(int,int,int)));
    connect(setstick, SIGNAL(setAxisButtonRelease(int,int,int)), this, SLOT(axisButtonUpEvent(int,int,int)));
    connect(setstick, SIGNAL(setAxisActivated(int,int, int)), this, SLOT(axisActivatedEvent(int,int,int)));
    connect(setstick, SIGNAL(setAxisReleased(int,int,int)), this, SLOT(axisReleasedEvent(int,int,int)));

    connect(setstick, SIGNAL(setDPadButtonClick(int,int,int)), this, SLOT(dpadButtonDownEvent(int,int,int)));
    connect(setstick, SIGNAL(setDPadButtonRelease(int,int,int)), this, SLOT(dpadButtonUpEvent(int,int,int)));

    connect(setstick, SIGNAL(setStickButtonClick(int,int,int)), this, SLOT(stickButtonDownEvent(int,int,int)));
    connect(setstick, SIGNAL(setStickButtonRelease(int,int,int)), this, SLOT(stickButtonUpEvent(int,int,int)));

    connect(setstick, SIGNAL(setButtonNameChange(int)), this, SLOT(updateSetButtonNames(int)));
    connect(setstick, SIGNAL(setAxisButtonNameChange(int,int)), this, SLOT(updateSetAxisButtonNames(int,int)));
    connect(setstick, SIGNAL(setStickButtonNameChange(int,int)), this, SLOT(updateSetStickButtonNames(int,int)));
    connect(setstick, SIGNAL(setDPadButtonNameChange(int,int)), this, SLOT(updateSetDPadButtonNames(int,int)));
    connect(setstick, SIGNAL(setVDPadButtonNameChange(int,int)), this, SLOT(updateSetVDPadButtonNames(int,int)));

    connect(setstick, SIGNAL(setAxisNameChange(int)), this, SLOT(updateSetAxisNames(int)));
    connect(setstick, SIGNAL(setStickNameChange(int)), this, SLOT(updateSetStickNames(int)));
    connect(setstick, SIGNAL(setDPadNameChange(int)), this, SLOT(updateSetDPadNames(int)));
    connect(setstick, SIGNAL(setVDPadNameChange(int)), this, SLOT(updateSetVDPadNames(int)));
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
    {
        joyNumber = index;
    }
    else
    {
        joyNumber = 0;
    }
}

void InputDevice::setDeviceKeyPressTime(unsigned int newPressTime)
{
    keyPressTime = newPressTime;
    emit propertyUpdated();
}

unsigned int InputDevice::getDeviceKeyPressTime()
{
    return keyPressTime;
}

void InputDevice::profileEdited()
{
    if (!deviceEdited)
    {
        deviceEdited = true;
        emit profileUpdated();
    }
}

bool InputDevice::isDeviceEdited()
{
    return deviceEdited;
}

void InputDevice::revertProfileEdited()
{
    deviceEdited = false;
}

QString InputDevice::getStringIdentifier()
{
    QString identifier;
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
    connect(this, SIGNAL(propertyUpdated()), this, SLOT(profileEdited()));
}

void InputDevice::disconnectPropertyUpdatedConnection()
{
    disconnect(this, SIGNAL(propertyUpdated()), this, SLOT(profileEdited()));
}

void InputDevice::setKeyRepeatStatus(bool enabled)
{
    keyRepeatEnabled = enabled;
}

void InputDevice::setKeyRepeatDelay(int delay)
{
    if (delay >= 250 && delay <= 1000)
    {
        keyRepeatDelay = delay;
    }
}

void InputDevice::setKeyRepeatRate(int rate)
{
    if (rate >= 20 && rate <= 200)
    {
        keyRepeatRate = rate;
    }
}

bool InputDevice::isKeyRepeatEnabled()
{
    return keyRepeatEnabled;
}

int InputDevice::getKeyRepeatDelay()
{
    int tempKeyRepeatDelay = DEFAULTKEYREPEATDELAY;
    if (keyRepeatDelay != 0)
    {
        tempKeyRepeatDelay = keyRepeatDelay;
    }

    return tempKeyRepeatDelay;
}

int InputDevice::getKeyRepeatRate()
{
    int tempKeyRepeatRate = DEFAULTKEYREPEATRATE;
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

QString InputDevice::getProfileName()
{
    return profileName;
}

int InputDevice::getButtonDownCount()
{
    return buttonDownCount;
}

#ifdef USE_SDL_2
QString InputDevice::getSDLPlatform()
{
    QString temp = SDL_GetPlatform();
    return temp;
}

#endif

/**
 * @brief Check if device is using the SDL Game Controller API
 * @return Status showing if device is using the Game Controller API
 */
bool InputDevice::isGameController()
{
    return false;
}

bool InputDevice::hasCalibrationThrottle(int axisNum)
{
    bool result = false;
    if (cali.contains(axisNum))
    {
        result = true;
    }

    return result;
}

JoyAxis::ThrottleTypes InputDevice::getCalibrationThrottle(int axisNum)
{
    return cali.value(axisNum);
}

void InputDevice::setCalibrationThrottle(int axisNum, JoyAxis::ThrottleTypes throttle)
{
    if (!cali.contains(axisNum))
    {
        for (int i=0; i < NUMBER_JOYSETS; i++)
        {
            joystick_sets.value(i)->setAxisThrottle(axisNum, throttle);
        }

        cali.insert(axisNum, throttle);
    }
}

void InputDevice::setCalibrationStatus(int axisNum, JoyAxis::ThrottleTypes throttle)
{
    if (!cali.contains(axisNum))
    {
        cali.insert(axisNum, throttle);
    }
}

void InputDevice::removeCalibrationStatus(int axisNum)
{
    if (cali.contains(axisNum))
    {
        cali.remove(axisNum);
    }
}

void InputDevice::sendLoadProfileRequest(QString location)
{
    if (!location.isEmpty())
    {
        emit requestProfileLoad(location);
    }
}

AntiMicroSettings* InputDevice::getSettings()
{
    return settings;
}

bool InputDevice::isKnownController()
{
    bool result = false;
    if (isGameController())
    {
        result = true;
    }
    else
    {
        settings->beginGroup("Mappings");
        if (settings->contains(getGUIDString()))
        {
            result = true;
        }
        else if (settings->contains(QString("%1%2").arg(getGUIDString()).arg("Disabled")))
        {
            result = true;
        }

        settings->endGroup();
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
    for (int i=0; i < currentSet->getNumberSticks(); i++)
    {
        JoyControlStick *tempStick = currentSet->getJoyStick(i);
        if (tempStick && tempStick->hasPendingEvent())
        {
            tempStick->activatePendingEvent();
        }
    }
}

void InputDevice::activatePossibleAxisEvents()
{
    SetJoystick *currentSet = getActiveSetJoystick();
    for (int i=0; i < currentSet->getNumberAxes(); i++)
    {
        JoyAxis *tempAxis = currentSet->getJoyAxis(i);
        if (tempAxis && tempAxis->hasPendingEvent())
        {
            tempAxis->activatePendingEvent();
        }
    }
}

void InputDevice::activatePossibleDPadEvents()
{
    SetJoystick *currentSet = getActiveSetJoystick();
    for (int i=0; i < currentSet->getNumberHats(); i++)
    {
        JoyDPad *tempDPad = currentSet->getJoyDPad(i);
        if (tempDPad && tempDPad->hasPendingEvent())
        {
            tempDPad->activatePendingEvent();
        }
    }
}

void InputDevice::activatePossibleVDPadEvents()
{
    SetJoystick *currentSet = getActiveSetJoystick();
    for (int i=0; i < currentSet->getNumberVDPads(); i++)
    {
        VDPad *tempVDPad = currentSet->getVDPad(i);
        if (tempVDPad && tempVDPad->hasPendingEvent())
        {
            tempVDPad->activatePendingEvent();
        }
    }
}

void InputDevice::activatePossibleButtonEvents()
{
    SetJoystick *currentSet = getActiveSetJoystick();
    for (int i=0; i < currentSet->getNumberButtons(); i++)
    {
        JoyButton *tempButton = currentSet->getJoyButton(i);
        if (tempButton && tempButton->hasPendingEvent())
        {
            tempButton->activatePendingEvent();
        }
    }
}

bool InputDevice::elementsHaveNames()
{
    bool result = false;

    SetJoystick *tempSet = getActiveSetJoystick();
    for (int i=0; i < getNumberButtons() && !result; i++)
    {
        JoyButton *button = tempSet->getJoyButton(i);
        if (button && !button->getButtonName().isEmpty())
        {
            result = true;
        }
    }

    for (int i=0; i < getNumberAxes() && !result; i++)
    {
        JoyAxis *axis = tempSet->getJoyAxis(i);
        if (axis)
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

    for (int i=0; i < getNumberSticks() && !result; i++)
    {
        JoyControlStick *stick = tempSet->getJoyStick(i);
        if (stick)
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

    for (int i=0; i < getNumberHats() && !result; i++)
    {
        JoyDPad *dpad = tempSet->getJoyDPad(i);
        if (dpad)
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

    for (int i=0; i < getNumberVDPads() && !result; i++)
    {
        VDPad *vdpad = getActiveSetJoystick()->getVDPad(i);
        if (vdpad)
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
                if (button && !button->getButtonName().isEmpty())
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
    bool result = false;

    if (tempGUID.contains(emptyGUID))
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
    bool result = false;
    if (tempGUID == getGUIDString())
    {
        result = true;
    }

    return result;
}

QString InputDevice::getRawGUIDString()
{
    QString temp = getGUIDString();
    return temp;
}

void InputDevice::haltServices()
{
    emit requestWait();
}

void InputDevice::finalRemoval()
{
    this->closeSDLDevice();
    this->deleteLater();
}

void InputDevice::setRawAxisDeadZone(int deadZone)
{
    if (deadZone > 0 && deadZone <= JoyAxis::AXISMAX)
    {
        this->rawAxisDeadZone = deadZone;
    }
    else
    {
        this->rawAxisDeadZone = RAISEDDEADZONE;
    }
}

int InputDevice::getRawAxisDeadZone()
{
    return rawAxisDeadZone;
}

void InputDevice::rawAxisEvent(int index, int value)
{
    emit rawAxisMoved(index, value);
}
