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

// holds information about gamecontrollers

class GameController : public InputDevice
{
    Q_OBJECT

  public:
    explicit GameController(SDL_GameController *controller, int deviceIndex, AntiMicroSettings *settings, int counterUniques,
                            QObject *parent);

    virtual QString getName() override;
    virtual QString getSDLName() override;
    virtual QString getXmlName() override;

    // GUID available on SDL 2.
    virtual QString getGUIDString() override;
    virtual QString getVendorString() override;
    virtual QString getProductIDString() override;
    virtual QString getUniqueIDString() override;
    virtual QString getProductVersion() override;
    virtual QString getRawGUIDString() override;
    virtual QString getRawUniqueIDString() override;
    virtual QString getRawVendorString() override;
    virtual QString getRawProductIDString() override;
    virtual QString getRawProductVersion() override;

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

    void fillContainers(QHash<int, SDL_GameControllerButton> &buttons, QHash<int, SDL_GameControllerAxis> &axes,
                        QList<SDL_GameControllerButtonBind> &hatButtons);

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
};

#endif // GAMECONTROLLER_H
