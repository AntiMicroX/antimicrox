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

#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <SDL2/SDL_gamecontroller.h>
#include <inputdevice.h>


class QXmlStreamReader;
class QXmlStreamWriter;
class AntiMicroSettings;


class GameController : public InputDevice
{
    Q_OBJECT

public:
    explicit GameController(SDL_GameController *controller, int deviceIndex, AntiMicroSettings *settings, QObject *parent = nullptr);

    virtual QString getName() override;
    virtual QString getSDLName() override;

    // GUID available on SDL 2.
    virtual QString getGUIDString() override;
    virtual QString getRawGUIDString() override;

    virtual QString getXmlName() override;
    virtual bool isGameController() override;
    virtual void closeSDLDevice() override;
    virtual SDL_JoystickID getSDLJoystickID() override;

    virtual int getNumberRawButtons() override;
    virtual int getNumberRawAxes() override;
    virtual int getNumberRawHats() override;

    QString getBindStringForAxis(int index, bool trueIndex=true);
    QString getBindStringForButton(int index, bool trueIndex=true);

    SDL_GameControllerButtonBind getBindForAxis(int index);
    SDL_GameControllerButtonBind getBindForButton(int index);

    bool isRelevantGUID(QString tempGUID);
    void rawButtonEvent(int index, bool pressed);
    void rawAxisEvent(int index, int value);
    void rawDPadEvent(int index, int value);

    QHash<int, bool> const& getRawbuttons();
    QHash<int, int> const& getAxisvalues();
    QHash<int, int> const& getDpadvalues();

    SDL_GameController *getController() const;

protected:
    void readJoystickConfig(QXmlStreamReader *xml);

public slots:
    virtual void readConfig(QXmlStreamReader *xml) override;
    virtual void writeConfig(QXmlStreamWriter *xml) override;

protected slots:
    virtual void axisActivatedEvent(int setindex, int axisindex, int value) override;
    virtual void buttonClickEvent(int buttonindex) override;
    virtual void buttonReleaseEvent(int buttonindex) override;

private:
    QHash<int, bool> rawbuttons;
    QHash<int, int> axisvalues;
    QHash<int, int> dpadvalues;

    SDL_JoystickID joystickID;
    SDL_GameController *controller;

    void writeXmlForButtons(SetJoystick *tempSet, QXmlStreamWriter *xml);
    void writeXmlForAxes(SetJoystick *tempSet, QXmlStreamWriter *xml);
    void writeXmlAxBtn(JoyAxis *axis, JoyAxisButton *naxisbutton, QXmlStreamWriter *xml);
    void writeXmlForSticks(SetJoystick *tempSet, QXmlStreamWriter *xml);
    void writeXmlForVDpad(QXmlStreamWriter *xml);
    void readXmlNamesShort(QString name, QXmlStreamReader *xml);
    void readXmlNamesMiddle(QString name, QXmlStreamReader *xml);
    void readXmlNamesLong(QString name, QXmlStreamReader *xml);
    void readJoystickConfigXmlLong(QList<SDL_GameControllerButtonBind>& hatButtons, bool& dpadNameExists, bool& vdpadNameExists, QXmlStreamReader *xml);
    void fillContainers(QHash<int, SDL_GameControllerButton> &buttons, QHash<int, SDL_GameControllerAxis> &axes, QList<SDL_GameControllerButtonBind> &hatButtons);

    inline void assignVariables(QXmlStreamReader *xml, int& index, int& buttonIndex, QString& temp, bool buttonDecreased);
    inline void assignVariablesShort(QXmlStreamReader *xml, int& index, QString& temp);


};

#endif // GAMECONTROLLER_H
