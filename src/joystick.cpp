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

#include "joystick.h"

#include "antimicrosettings.h"
#include "globalvariables.h"

#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_version.h>

#include <QDebug>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

Joystick::Joystick(SDL_Joystick *joyhandle, int deviceIndex, AntiMicroSettings *settings, QObject *parent)
    : InputDevice(joyhandle, deviceIndex, settings, parent)
{
    m_joyhandle = joyhandle;
    controller = SDL_GameControllerOpen(deviceIndex);
    joystickID = SDL_JoystickInstanceID(joyhandle);

    for (int i = 0; i < GlobalVariables::InputDevice::NUMBER_JOYSETS; i++)
    {
        SetJoystick *setstick = new SetJoystick(this, i, this);
        getJoystick_sets().insert(i, setstick);
        enableSetConnections(setstick);
    }
    INFO() << "Created new Joystick:\n" << getDescription();
}

QString Joystick::getXmlName() const { return GlobalVariables::Joystick::xmlName; }

QString Joystick::getName() { return QString(tr("Joystick")).append(" ").append(QString::number(getRealJoyNumber())); }

QString Joystick::getSDLName()
{
    QString temp = QString();

    if (m_joyhandle != nullptr)
    {
        temp = SDL_JoystickName(m_joyhandle);
    }

    return temp;
}

QString Joystick::getGUIDString() const
{
    QString temp = QString();

    SDL_JoystickGUID tempGUID = SDL_JoystickGetGUID(m_joyhandle);
    char guidString[65] = {'0'};
    SDL_JoystickGetGUIDString(tempGUID, guidString, sizeof(guidString));
    temp = QString(guidString);

    // Not available on SDL 1.2. Return empty string in that case.
    return temp;
}

QString Joystick::getVendorString() const
{
    QString temp = QString();

    if (m_joyhandle != nullptr)
    {
        Uint16 tempVendor = SDL_JoystickGetVendor(m_joyhandle);
        char buffer[50];
        sprintf(buffer, "%u", tempVendor);

        temp = QString(buffer);
    }

    return temp;
}

QString Joystick::getProductIDString() const
{
    QString temp = QString();

    if (m_joyhandle != nullptr)
    {
        Uint16 tempProduct = SDL_JoystickGetProduct(m_joyhandle);
        char buffer[50];
        sprintf(buffer, "%u", tempProduct);

        temp = QString(buffer);
    }

    return temp;
}

QString Joystick::getSerialString() const
{
    QString temp = QString();
#if SDL_VERSION_ATLEAST(2, 0, 14)
    if (m_joyhandle != nullptr)
    {
        const char *serial = SDL_JoystickGetSerial(m_joyhandle);
        temp = QString(serial).remove(QRegularExpression("[^A-Za-z0-9]"));
    }
#endif

    return temp;
}

QString Joystick::getProductVersion() const
{
    QString temp = QString();

    if (m_joyhandle != nullptr)
    {
        Uint16 tempProductVersion = SDL_JoystickGetProductVersion(m_joyhandle);
        char buffer[50];
        sprintf(buffer, "%u", tempProductVersion);

        temp = QString(buffer);
    }

    return temp;
}

QString Joystick::getUniqueIDString() const
{
    return (getGUIDString() + getVendorString() + getProductIDString()) + getSerialString();
}

void Joystick::closeSDLDevice()
{
    if ((m_joyhandle != nullptr) && SDL_JoystickGetAttached(m_joyhandle))
    {
        SDL_JoystickClose(m_joyhandle);
    }
}

int Joystick::getNumberRawButtons()
{
    int numbuttons = SDL_JoystickNumButtons(m_joyhandle);
    return numbuttons;
}

int Joystick::getNumberRawAxes()
{
    int numaxes = SDL_JoystickNumAxes(m_joyhandle);
    return numaxes;
}

int Joystick::getNumberRawHats()
{
    int numhats = SDL_JoystickNumHats(m_joyhandle);
    return numhats;
}

double Joystick::getRawSensorRate(JoySensorType _) { return 0; }

bool Joystick::hasRawSensor(JoySensorType _) { return false; }

void Joystick::setCounterUniques(int counter) { counterUniques = counter; }

SDL_JoystickID Joystick::getSDLJoystickID() { return joystickID; }

SDL_Joystick *Joystick::getJoyhandle() const { return m_joyhandle; }
