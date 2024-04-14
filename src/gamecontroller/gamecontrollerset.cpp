/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail.
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

#include "gamecontrollerset.h"

#include "gamecontroller.h"
#include "gamecontrollerdpad.h"
#include "gamecontrollertrigger.h"
#include "haptictriggerps5.h"
#include "inputdevice.h"
#include "joycontrolstick.h"
#include "joysensor.h"
#include "xml/joyaxisxml.h"
#include "xml/joybuttonxml.h"
#include "xml/joydpadxml.h"

#include <QDebug>
#include <QXmlStreamReader>

GameControllerSet::GameControllerSet(InputDevice *device, int index, QObject *parent)
    : SetJoystick(device, index, false, parent)
{
    resetSticks();
    applyHapticTrigger();
}

void GameControllerSet::reset() { resetSticks(); }

/**
 * @brief Applies haptic feedback to the triggers of the controller.
 *
 * This fetches the current haptic feedback effects from all triggers of the
 * controller, builds a low level message and sends the message to the controller.
 */
void GameControllerSet::applyHapticTrigger()
{
    GameController *controller = qobject_cast<GameController *>(getInputDevice());
    HapticTriggerPs5 *left_effect = getJoyAxis(SDL_CONTROLLER_AXIS_TRIGGERLEFT)->getHapticTrigger();
    HapticTriggerPs5 *right_effect = getJoyAxis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT)->getHapticTrigger();

    if (left_effect == nullptr || right_effect == nullptr)
        return;

    HapticTriggerPs5::send(controller->getController(), *left_effect, *right_effect);
}

void GameControllerSet::resetSticks()
{
    SetJoystick::reset();
    populateSticksDPad();
}

void GameControllerSet::populateSticksDPad()
{ // Left Stick Assignment
    JoyAxis *axisX = getJoyAxis(SDL_CONTROLLER_AXIS_LEFTX);
    JoyAxis *axisY = getJoyAxis(SDL_CONTROLLER_AXIS_LEFTY);
    JoyControlStick *stick1 = new JoyControlStick(axisX, axisY, 0, getIndex(), this);
    stick1->setDefaultStickName("L Stick");
    addControlStick(0, stick1);

    // Right Stick Assignment
    axisX = getJoyAxis(SDL_CONTROLLER_AXIS_RIGHTX);
    axisY = getJoyAxis(SDL_CONTROLLER_AXIS_RIGHTY);
    JoyControlStick *stick2 = new JoyControlStick(axisX, axisY, 1, getIndex(), this);
    stick2->setDefaultStickName("R Stick");
    addControlStick(1, stick2);

    // Assign DPad buttons as a virtual DPad. Allows rougelike controls
    // to be assigned.
    JoyButton *buttonUp = getJoyButton(SDL_CONTROLLER_BUTTON_DPAD_UP);
    JoyButton *buttonDown = getJoyButton(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    JoyButton *buttonLeft = getJoyButton(SDL_CONTROLLER_BUTTON_DPAD_LEFT);
    JoyButton *buttonRight = getJoyButton(SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
    GameControllerDPad *controllerDPad =
        new GameControllerDPad(buttonUp, buttonDown, buttonLeft, buttonRight, 0, getIndex(), this, this);
    controllerDPad->setDefaultDPadName("DPad");
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

template <typename T> void readConf(T *x, QXmlStreamReader *xml)
{
    if (x != nullptr)
    {
        x->readConfig(xml);
    } else
    {
        xml->skipCurrentElement();
    }
}

void GameControllerSet::readConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && (xml->name().toString() == "set"))
    {
        xml->readNextStartElement();

        while (!xml->atEnd() && (!xml->isEndElement() && xml->name().toString() != "set"))
        {
            if ((xml->name().toString() == "button") && xml->isStartElement())
            {
                getElemFromXml("button", xml);
            } else if ((xml->name().toString() == "trigger") && xml->isStartElement())
            {
                getElemFromXml("trigger", xml);
            } else if ((xml->name().toString() == "stick") && xml->isStartElement())
            {
                getElemFromXml("stick", xml);
            } else if ((xml->name().toString() == "sensor") && xml->isStartElement())
            {
                getElemFromXml("sensor", xml);
            } else if ((xml->name().toString() == "dpad") && xml->isStartElement())
            {
                getElemFromXml("dpad", xml);
            } else if ((xml->name().toString() == "name") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();

                if (!temptext.isEmpty())
                {
                    setName(temptext);
                }
            } else
            {
                // If none of the above, skip the element
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }
    }
}

void GameControllerSet::getElemFromXml(QString elemName, QXmlStreamReader *xml)
{
    int index = xml->attributes().value("index").toString().toInt();

    if (elemName == "button")
    {
        JoyButton *button = getJoyButton(index - 1);
        JoyButtonXml *joyButtonXml = new JoyButtonXml(button, this);
        readConf(joyButtonXml, xml);
    } else if (elemName == "dpad")
    {
        GameControllerDPad *vdpad = qobject_cast<GameControllerDPad *>(getVDPad(index - 1));
        JoyDPadXml<GameControllerDPad> *dpadXml = new JoyDPadXml<GameControllerDPad>(vdpad);
        readConf(dpadXml, xml);

        // if (!dpadXml.isNull()) delete dpadXml;
    } else if (elemName == "trigger")
    {
        GameControllerTrigger *axis = nullptr;
        JoyAxisXml *triggerAxisXml = nullptr;

        switch (index - 1)
        {
        // for older profiles
        case 0:
            axis = qobject_cast<GameControllerTrigger *>(getJoyAxis(SDL_CONTROLLER_AXIS_TRIGGERLEFT));
            triggerAxisXml = new JoyAxisXml(axis, this);
            readConf(triggerAxisXml, xml);
            break;

        // for older profiles
        case 1:
            axis = qobject_cast<GameControllerTrigger *>(getJoyAxis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT));
            triggerAxisXml = new JoyAxisXml(axis, this);
            readConf(triggerAxisXml, xml);
            break;

        case 4:
            axis = qobject_cast<GameControllerTrigger *>(getJoyAxis(SDL_CONTROLLER_AXIS_TRIGGERLEFT));
            triggerAxisXml = new JoyAxisXml(axis, this);
            readConf(triggerAxisXml, xml);
            break;

        case 5:
            axis = qobject_cast<GameControllerTrigger *>(getJoyAxis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT));
            triggerAxisXml = new JoyAxisXml(axis, this);
            readConf(triggerAxisXml, xml);
            break;
        }
    } else if (elemName == "stick")
    {
        if (index > 0)
        {
            index -= 1;
            JoyControlStick *stick = getJoyStick(index);
            readConf(stick, xml);
        } else
        {
            xml->skipCurrentElement();
        }
    } else if (elemName == "sensor")
    {
        int type = xml->attributes().value("type").toString().toInt();
        JoySensor *sensor = getSensor(static_cast<JoySensorType>(type));
        readConf(sensor, xml);
    }
}

void GameControllerSet::refreshAxes()
{
    deleteAxes();

    for (int i = 0; i < getInputDevice()->getNumberRawAxes(); i++)
    {
        if ((i == static_cast<int>(SDL_CONTROLLER_AXIS_TRIGGERLEFT)) ||
            (i == static_cast<int>(SDL_CONTROLLER_AXIS_TRIGGERRIGHT)))
        {
            GameControllerTrigger *trigger = new GameControllerTrigger(i, getIndex(), this, this);
            getAxes()->insert(i, trigger);
            enableAxisConnections(trigger);
            connect(trigger, &JoyAxis::hapticTriggerChanged, this, &GameControllerSet::applyHapticTrigger);
        } else
        {
            JoyAxis *axis = new JoyAxis(i, getIndex(), this, this);
            getAxes()->insert(i, axis);
            enableAxisConnections(axis);
        }
    }
}
