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

#include "gamecontrollermappingdialoghelper.h"

#include "globalvariables.h"
#include "messagehandler.h"
#include "inputdevice.h"

#include <QDebug>

GameControllerMappingDialogHelper::GameControllerMappingDialogHelper(InputDevice *device,
                                                                     QObject *parent) :
    QObject(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->device = device;
}


void GameControllerMappingDialogHelper::raiseDeadZones()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    device->setRawAxisDeadZone(GlobalVariables::InputDevice::RAISEDDEADZONE);
    device->getActiveSetJoystick()->raiseAxesDeadZones();
}

void GameControllerMappingDialogHelper::raiseDeadZones(int deadZone)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    device->getActiveSetJoystick()->raiseAxesDeadZones(deadZone);
    device->setRawAxisDeadZone(deadZone);
}

void GameControllerMappingDialogHelper::setupDeadZones()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    device->getActiveSetJoystick()->setIgnoreEventState(true);
    device->getActiveSetJoystick()->release();

    device->getActiveSetJoystick()->currentAxesDeadZones(&originalAxesDeadZones);
    device->getActiveSetJoystick()->raiseAxesDeadZones();

    device->setRawAxisDeadZone(GlobalVariables::InputDevice::RAISEDDEADZONE);
}

void GameControllerMappingDialogHelper::restoreDeviceDeadZones()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    device->getActiveSetJoystick()->setIgnoreEventState(false);
    device->getActiveSetJoystick()->release();
    device->getActiveSetJoystick()->setAxesDeadZones(&originalAxesDeadZones);

    device->setRawAxisDeadZone(GlobalVariables::InputDevice::RAISEDDEADZONE);
}
