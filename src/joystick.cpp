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

#include "joystick.h"

#include "messagehandler.h"
#include "antimicrosettings.h"

#include <QDebug>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

const QString Joystick::xmlName = "joystick";

Joystick::Joystick(SDL_Joystick *joyhandle, int deviceIndex,
                   AntiMicroSettings *settings, QObject *parent) :
    InputDevice(deviceIndex, settings, parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->joyhandle = joyhandle;

    joystickID = SDL_JoystickInstanceID(joyhandle);

    for (int i = 0; i < NUMBER_JOYSETS; i++)
    {
        SetJoystick *setstick = new SetJoystick(this, i, this);
        getJoystick_sets().insert(i, setstick);
        enableSetConnections(setstick);
    }
}

QString Joystick::getName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return QString(trUtf8("Joystick")).append(" ").append(QString::number(getRealJoyNumber()));
}

QString Joystick::getSDLName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();

    if (joyhandle != nullptr)
    {
        temp = SDL_JoystickName(joyhandle);
    }

    return temp;
}

QString Joystick::getGUIDString()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();

    SDL_JoystickGUID tempGUID = SDL_JoystickGetGUID(joyhandle);
    char guidString[65] = {'0'};
    SDL_JoystickGetGUIDString(tempGUID, guidString, sizeof(guidString));
    temp = QString(guidString);

    // Not available on SDL 1.2. Return empty string in that case.
    return temp;
}

QString Joystick::getXmlName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return this->xmlName;
}

void Joystick::closeSDLDevice()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((joyhandle != nullptr) && SDL_JoystickGetAttached(joyhandle))
    {
        SDL_JoystickClose(joyhandle);
    }
}

int Joystick::getNumberRawButtons()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int numbuttons = SDL_JoystickNumButtons(joyhandle);
    return numbuttons;
}

int Joystick::getNumberRawAxes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int numaxes = SDL_JoystickNumAxes(joyhandle);
    return numaxes;
}

int Joystick::getNumberRawHats()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int numhats = SDL_JoystickNumHats(joyhandle);
    return numhats;
}

SDL_JoystickID Joystick::getSDLJoystickID()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return joystickID;
}

SDL_Joystick* Joystick::getJoyhandle() const {

    return joyhandle;
}
