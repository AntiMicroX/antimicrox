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

#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <SDL2/SDL_gamecontroller.h>
#include <inputdevice.h>

class QXmlStreamReader;
class QXmlStreamWriter;
class AntiMicroSettings;

/**
 * @brief Represents a typical gamepad (like PS4 or Xbox gamepad)
 *
 * @details It represents typical gamepads with two analog sticks, one dpad, two shoulder buttons on each side (one of them
possibly a trigger axis and not just a button), 4 buttons on the right and some buttons in the middle of the pad.

SDL2 maps the axis and buttons of devices it knows (and XInput Devices on Windows) to standardized button/axis names based on
xbox360/xinput controllers. It often uses SDL bindings to figure out how gamepad should look like. Every device in this
category releases events SDL_CONTROLLER... (like SDL_CONTROLLERBUTTONUP)

Each SDL Gamecontroller device can also be used as a bit more general-purposed Joystick device.
 *
 * @see https://wiki.libsdl.org/CategoryGameController
 * @see https://wiki.libsdl.org/SDL_IsGameController
 */
class GameController : public InputDevice
{
    Q_OBJECT

  public:
    explicit GameController(SDL_GameController *controller, int deviceIndex, AntiMicroSettings *settings, int counterUniques,
                            QObject *parent);

    virtual QString getName() override;
    virtual QString getSDLName() override;
    virtual QString getXmlName() const override;

    // GUID available on SDL 2.
    virtual QString getGUIDString() const override;
    virtual QString getVendorString() const override;
    virtual QString getProductIDString() const override;
    virtual QString getSerialString() const override;
    virtual QString getUniqueIDString() const override;
    virtual QString getRawGUIDString() const override;
    virtual QString getProductVersion() const override;
    virtual QString getRawUniqueIDString() const override;
    virtual QString getRawVendorString() const override;
    virtual QString getRawProductIDString() const override;
    virtual QString getRawProductVersion() const override;

    virtual bool isGameController() override;
    virtual void closeSDLDevice() override;
    virtual SDL_JoystickID getSDLJoystickID() override;

    virtual int getNumberRawButtons() override;
    virtual int getNumberRawAxes() override;
    virtual int getNumberRawHats() override;
    virtual double getRawSensorRate(JoySensorType type) override;
    virtual bool hasRawSensor(JoySensorType type) override;
    void setCounterUniques(int counter) override;

    QString getBindStringForAxis(int index, bool trueIndex = true);
    QString getBindStringForButton(int index, bool trueIndex = true);

    SDL_GameControllerButtonBind getBindForAxis(int index);
    SDL_GameControllerButtonBind getBindForButton(int index);

    // bool isRelevantGUID(QString tempGUID);
    bool isRelevantUniqueID(QString tempUniqueID);
    void rawButtonEvent(int index, bool pressed);
    void rawAxisEvent(int index, int value);
    void rawDPadEvent(int index, int value);

    QHash<int, bool> const &getRawbuttons();
    QHash<int, int> const &getAxisvalues();
    QHash<int, int> const &getDpadvalues();

    SDL_GameController *getController() const;
    virtual SDL_GameControllerType getControllerType() const override;

  protected slots:
    virtual void axisActivatedEvent(int setindex, int axisindex, int value) override;
    virtual void buttonClickEvent(int buttonindex) override;
    virtual void buttonReleaseEvent(int buttonindex) override;

  private:
    QHash<int, bool> rawbuttons;
    QHash<int, int> axisvalues;
    QHash<int, int> dpadvalues;
    int counterUniques;

    SDL_JoystickID joystickID;
    SDL_GameController *controller;
    SDL_GameControllerType m_controller_type;

    void enableSensors();
};

#endif // GAMECONTROLLER_H
