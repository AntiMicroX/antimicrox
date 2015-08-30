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

#include "gamecontrollerset.h"
#include <inputdevice.h>

GameControllerSet::GameControllerSet(InputDevice *device, int index, QObject *parent) :
    SetJoystick(device, index, false, parent)
{
    reset();
}

void GameControllerSet::reset()
{
    SetJoystick::reset();
    populateSticksDPad();
}

void GameControllerSet::populateSticksDPad()
{
    // Left Stick Assignment
    JoyAxis *axisX = getJoyAxis(SDL_CONTROLLER_AXIS_LEFTX);
    JoyAxis *axisY = getJoyAxis(SDL_CONTROLLER_AXIS_LEFTY);
    JoyControlStick *stick1 = new JoyControlStick(axisX, axisY, 0, index, this);
    //stick1->setStickDelay(10);
    stick1->setDefaultStickName("L Stick");
    addControlStick(0, stick1);

    // Right Stick Assignment
    axisX = getJoyAxis(SDL_CONTROLLER_AXIS_RIGHTX);
    axisY = getJoyAxis(SDL_CONTROLLER_AXIS_RIGHTY);
    JoyControlStick *stick2 = new JoyControlStick(axisX, axisY, 1, index, this);
    stick2->setDefaultStickName("R Stick");
    addControlStick(1, stick2);

    // Assign DPad buttons as a virtual DPad. Allows rougelike controls
    // to be assigned.
    JoyButton *buttonUp = getJoyButton(SDL_CONTROLLER_BUTTON_DPAD_UP);
    JoyButton *buttonDown = getJoyButton(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    JoyButton *buttonLeft = getJoyButton(SDL_CONTROLLER_BUTTON_DPAD_LEFT);
    JoyButton *buttonRight = getJoyButton(SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
    GameControllerDPad *controllerDPad = new GameControllerDPad(buttonUp, buttonDown, buttonLeft, buttonRight, 0, index, this, this);
    controllerDPad->setDefaultDPadName("DPad");
    //controllerDPad->setDPadDelay(10);
    addVDPad(0, controllerDPad);

    // Give default names to buttons
    getJoyButton(SDL_CONTROLLER_BUTTON_A)->setDefaultButtonName("A");
    getJoyButton(SDL_CONTROLLER_BUTTON_B)->setDefaultButtonName("B");
    getJoyButton(SDL_CONTROLLER_BUTTON_X)->setDefaultButtonName("X");
    getJoyButton(SDL_CONTROLLER_BUTTON_Y)->setDefaultButtonName("Y");
    getJoyButton(SDL_CONTROLLER_BUTTON_BACK)->setDefaultButtonName(tr("Back"));
    getJoyButton(SDL_CONTROLLER_BUTTON_GUIDE)->setDefaultButtonName(tr("Guide"));
    getJoyButton(SDL_CONTROLLER_BUTTON_START)->setDefaultButtonName(tr("Start"));
    getJoyButton(SDL_CONTROLLER_BUTTON_LEFTSTICK)->setDefaultButtonName(tr("LS Click"));
    getJoyButton(SDL_CONTROLLER_BUTTON_RIGHTSTICK)->setDefaultButtonName(tr("RS Click"));
    getJoyButton(SDL_CONTROLLER_BUTTON_LEFTSHOULDER)->setDefaultButtonName(tr("L Shoulder"));
    getJoyButton(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)->setDefaultButtonName(tr("R Shoulder"));

    // Give default names to triggers
    getJoyAxis(SDL_CONTROLLER_AXIS_TRIGGERLEFT)->setDefaultAxisName(tr("L Trigger"));
    getJoyAxis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT)->setDefaultAxisName(tr("R Trigger"));
}

void GameControllerSet::readJoystickConfig(QXmlStreamReader *xml,
                                           QHash<unsigned int, SDL_GameControllerButton> &buttons,
                                           QHash<unsigned int, SDL_GameControllerAxis> &axes,
                                           QList<SDL_GameControllerButtonBind> &hatButtons)
{
    if (xml->isStartElement() && xml->name() == "set")
    {
        xml->readNextStartElement();

        while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != "set"))
        {
            bool dpadExists = false;
            bool vdpadExists = false;

            if (xml->name() == "button" && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyButton *button = 0;
                if (buttons.contains(index-1))
                {
                    SDL_GameControllerButton current = buttons.value(index-1);
                    button = getJoyButton(current);
                }

                if (button)
                {
                    button->readConfig(xml);
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if (xml->name() == "axis" && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                GameControllerTrigger *trigger = 0;
                if (axes.contains(index-1))
                {
                    SDL_GameControllerAxis current = axes.value(index-1);
                    trigger = static_cast<GameControllerTrigger*>(getJoyAxis((int)current));
                }

                if (trigger)
                {
                    trigger->readJoystickConfig(xml);
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if (xml->name() == "dpad" && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                index = index - 1;
                bool found = false;
                QListIterator<SDL_GameControllerButtonBind> iter(hatButtons);
                SDL_GameControllerButtonBind current;
                while (iter.hasNext())
                {
                    current = iter.next();
                    if (current.value.hat.hat == index)
                    {
                        found = true;
                        iter.toBack();
                    }
                }

                VDPad *dpad = 0;
                if (found)
                {
                    dpad = getVDPad(0);
                }

                if (dpad && !vdpadExists)
                {
                    dpadExists = true;
                    dpad->readConfig(xml);
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if (xml->name() == "stick" && xml->isStartElement())
            {
                int stickIndex = xml->attributes().value("index").toString().toInt();

                if (stickIndex > 0)
                {
                    stickIndex -= 1;
                    JoyControlStick *stick = getJoyStick(stickIndex);
                    if (stick)
                    {
                        stick->readConfig(xml);
                    }
                    else
                    {
                        xml->skipCurrentElement();
                    }
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if (xml->name() == "vdpad" && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                index = index - 1;
                bool found = false;
                QListIterator<SDL_GameControllerButtonBind> iter(hatButtons);
                SDL_GameControllerButtonBind current;
                while (iter.hasNext())
                {
                    current = iter.next();
                    if (current.value.hat.hat == index)
                    {
                        found = true;
                        iter.toBack();
                    }
                }

                VDPad *dpad = 0;
                if (found)
                {
                    dpad = getVDPad(0);
                }

                if (dpad && !dpadExists)
                {
                    vdpadExists = true;

                    dpad->readConfig(xml);
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if (xml->name() == "name" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                if (!temptext.isEmpty())
                {
                    setName(temptext);
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
}

void GameControllerSet::readConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && xml->name() == "set")
    {
        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != "set"))
        {
            if (xml->name() == "button" && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyButton *button = getJoyButton(index-1);
                if (button)
                {
                    button->readConfig(xml);
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if (xml->name() == "trigger" && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                GameControllerTrigger *axis = qobject_cast<GameControllerTrigger*>(getJoyAxis((index-1) + SDL_CONTROLLER_AXIS_TRIGGERLEFT));
                if (axis)
                {
                    axis->readConfig(xml);
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if (xml->name() == "stick" && xml->isStartElement())
            {
                int stickIndex = xml->attributes().value("index").toString().toInt();

                if (stickIndex > 0)
                {
                    stickIndex -= 1;
                    JoyControlStick *stick = getJoyStick(stickIndex);
                    if (stick)
                    {
                        stick->readConfig(xml);
                    }
                    else
                    {
                        xml->skipCurrentElement();
                    }
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if (xml->name() == "dpad" && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                GameControllerDPad *vdpad = qobject_cast<GameControllerDPad*>(getVDPad(index-1));
                if (vdpad)
                {
                    vdpad->readConfig(xml);
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if (xml->name() == "name" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                if (!temptext.isEmpty())
                {
                    setName(temptext);
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
}

void GameControllerSet::refreshAxes()
{
    deleteAxes();

    for (int i=0; i < device->getNumberRawAxes(); i++)
    {
        if (i == SDL_CONTROLLER_AXIS_TRIGGERLEFT ||
            i == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
        {
            GameControllerTrigger *trigger = new GameControllerTrigger(i, index, this, this);
            axes.insert(i, trigger);
            enableAxisConnections(trigger);
        }
        else
        {
            JoyAxis *axis = new JoyAxis(i, index, this, this);
            axes.insert(i, axis);
            enableAxisConnections(axis);
        }
    }
}
