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

#include "gamecontrollerset.h"

#include "messagehandler.h"
#include "gamecontrollerdpad.h"
#include "gamecontrollertrigger.h"
#include "inputdevice.h"
#include "joycontrolstick.h"

#include <SDL2/SDL_gamecontroller.h>

#include <QDebug>


GameControllerSet::GameControllerSet(InputDevice *device, int index, QObject *parent) :
    SetJoystick(device, index, false, parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    resetSticks();
}


void GameControllerSet::reset()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    resetSticks();
}

void GameControllerSet::resetSticks()
{
    SetJoystick::reset();
    populateSticksDPad();
}

void GameControllerSet::populateSticksDPad()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    // Left Stick Assignment
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
    GameControllerDPad *controllerDPad = new GameControllerDPad(buttonUp, buttonDown, buttonLeft, buttonRight, 0, getIndex(), this, this);
    controllerDPad->setDefaultDPadName("DPad");
    addVDPad(0, controllerDPad);

    // Give default names to buttons
    getJoyButton(SDL_CONTROLLER_BUTTON_A)->setDefaultButtonName("A");
    getJoyButton(SDL_CONTROLLER_BUTTON_B)->setDefaultButtonName("B");
    getJoyButton(SDL_CONTROLLER_BUTTON_X)->setDefaultButtonName("X");
    getJoyButton(SDL_CONTROLLER_BUTTON_Y)->setDefaultButtonName("Y");
    getJoyButton(SDL_CONTROLLER_BUTTON_BACK)->setDefaultButtonName(trUtf8("Back"));
    getJoyButton(SDL_CONTROLLER_BUTTON_GUIDE)->setDefaultButtonName(trUtf8("Guide"));
    getJoyButton(SDL_CONTROLLER_BUTTON_START)->setDefaultButtonName(trUtf8("Start"));
    getJoyButton(SDL_CONTROLLER_BUTTON_LEFTSTICK)->setDefaultButtonName(trUtf8("LS Click"));
    getJoyButton(SDL_CONTROLLER_BUTTON_RIGHTSTICK)->setDefaultButtonName(trUtf8("RS Click"));
    getJoyButton(SDL_CONTROLLER_BUTTON_LEFTSHOULDER)->setDefaultButtonName(trUtf8("L Shoulder"));
    getJoyButton(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)->setDefaultButtonName(trUtf8("R Shoulder"));

    // Give default names to triggers
    getJoyAxis(SDL_CONTROLLER_AXIS_TRIGGERLEFT)->setDefaultAxisName(trUtf8("L Trigger"));
    getJoyAxis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT)->setDefaultAxisName(trUtf8("R Trigger"));
}


void GameControllerSet::refreshAxes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    deleteAxes();

    for (int i = 0; i < getInputDevice()->getNumberRawAxes(); i++)
    {
        if ((i == static_cast<int>(SDL_CONTROLLER_AXIS_TRIGGERLEFT)) ||
            (i == static_cast<int>(SDL_CONTROLLER_AXIS_TRIGGERRIGHT)))
        {
            GameControllerTrigger *trigger = new GameControllerTrigger(i, getIndex(), this, this);
            getAxes()->insert(i, trigger);
            enableAxisConnections(trigger);
        }
        else
        {
            JoyAxis *axis = new JoyAxis(i, getIndex(), this, this);
            getAxes()->insert(i, axis);
            enableAxisConnections(axis);
        }
    }
}
