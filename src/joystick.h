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

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "inputdevice.h"

#include <SDL2/SDL_gamecontroller.h>

class AntiMicroSettings;

/**
 * @brief General input device (which is not a typical gamepad like steering wheels, flight sticks etc.)
 *
 * @details This is broad category of devices. (Includes Gamepads described by GameController and all of the other devices)
 * Devices in this category trigger events SDL_JOY... (like SDL_JOYBUTTONUP)
 */
class Joystick : public InputDevice
{
    Q_OBJECT

  public:
    explicit Joystick(SDL_Joystick *joyhandle, int deviceIndex, AntiMicroSettings *settings, QObject *parent);

    virtual QString getName() override;
    virtual QString getSDLName() override;
    virtual QString getGUIDString() const override; // GUID available on SDL 2.
    virtual QString getUniqueIDString() const override;
    virtual QString getVendorString() const override;
    virtual QString getProductIDString() const override;
    virtual QString getSerialString() const override;
    virtual QString getProductVersion() const override;

    virtual void closeSDLDevice() override;
    virtual SDL_JoystickID getSDLJoystickID() override;

    virtual int getNumberRawButtons() override;
    virtual int getNumberRawAxes() override;
    virtual int getNumberRawHats() override;
    virtual double getRawSensorRate(JoySensorType type) override;
    virtual bool hasRawSensor(JoySensorType type) override;

    void setCounterUniques(int counter) override;

    SDL_Joystick *getJoyhandle() const;
    virtual QString getXmlName() const override;

  private:
    SDL_Joystick *m_joyhandle;
    SDL_GameController *controller;
    SDL_JoystickID joystickID;
    int counterUniques;
};

Q_DECLARE_METATYPE(Joystick *)

#endif // JOYSTICK_H
