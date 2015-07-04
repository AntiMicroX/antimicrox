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

#include "joystick.h"

const QString Joystick::xmlName = "joystick";

Joystick::Joystick(SDL_Joystick *joyhandle, int deviceIndex,
                   AntiMicroSettings *settings, QObject *parent) :
    InputDevice(deviceIndex, settings, parent)
{
    this->joyhandle = joyhandle;
#ifdef USE_SDL_2
    joystickID = SDL_JoystickInstanceID(joyhandle);
#else
    joyNumber = SDL_JoystickIndex(joyhandle);
#endif

    for (int i=0; i < NUMBER_JOYSETS; i++)
    {
        SetJoystick *setstick = new SetJoystick(this, i, this);
        joystick_sets.insert(i, setstick);
        enableSetConnections(setstick);
    }
}

QString Joystick::getName()
{
    return QString(tr("Joystick")).append(" ").append(QString::number(getRealJoyNumber()));
}

QString Joystick::getSDLName()
{
    QString temp;
#ifdef USE_SDL_2
    if (joyhandle)
    {
        temp = SDL_JoystickName(joyhandle);
    }
#else
    temp = SDL_JoystickName(joyNumber);
#endif
    return temp;
}

QString Joystick::getGUIDString()
{
    QString temp;

#ifdef USE_SDL_2
    SDL_JoystickGUID tempGUID = SDL_JoystickGetGUID(joyhandle);
    char guidString[65] = {'0'};
    SDL_JoystickGetGUIDString(tempGUID, guidString, sizeof(guidString));
    temp = QString(guidString);
#endif

    // Not available on SDL 1.2. Return empty string in that case.
    return temp;
}

QString Joystick::getXmlName()
{
    return this->xmlName;
}

void Joystick::closeSDLDevice()
{
#ifdef USE_SDL_2
    if (joyhandle && SDL_JoystickGetAttached(joyhandle))
    {
        SDL_JoystickClose(joyhandle);
    }
#else
    if (joyhandle && SDL_JoystickOpened(joyNumber))
    {
        SDL_JoystickClose(joyhandle);
    }
#endif
}

int Joystick::getNumberRawButtons()
{
    int numbuttons = SDL_JoystickNumButtons(joyhandle);
    return numbuttons;
}

int Joystick::getNumberRawAxes()
{
    int numaxes = SDL_JoystickNumAxes(joyhandle);
    return numaxes;
}

int Joystick::getNumberRawHats()
{
    int numhats = SDL_JoystickNumHats(joyhandle);
    return numhats;
}

#ifdef USE_SDL_2
SDL_JoystickID Joystick::getSDLJoystickID()
{
    return joystickID;
}
#endif
