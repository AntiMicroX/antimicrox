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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <inputdevice.h>
#include "gamecontrollerdpad.h"
#include "gamecontrollerset.h"

class GameController : public InputDevice
{
    Q_OBJECT
public:
    explicit GameController(SDL_GameController *controller, int deviceIndex, AntiMicroSettings *settings, QObject *parent = 0);

    virtual QString getName();
    virtual QString getSDLName();

    // GUID available on SDL 2.
    virtual QString getGUIDString();
    virtual QString getRawGUIDString();

    virtual QString getXmlName();
    virtual bool isGameController();
    virtual void closeSDLDevice();
    virtual SDL_JoystickID getSDLJoystickID();

    virtual int getNumberRawButtons();
    virtual int getNumberRawAxes();
    virtual int getNumberRawHats();

    QString getBindStringForAxis(int index, bool trueIndex=true);
    QString getBindStringForButton(int index, bool trueIndex=true);

    SDL_GameControllerButtonBind getBindForAxis(int index);
    SDL_GameControllerButtonBind getBindForButton(int index);

    bool isRelevantGUID(QString tempGUID);
    void rawButtonEvent(int index, bool pressed);
    void rawAxisEvent(int index, int value);
    void rawDPadEvent(int index, int value);

    QHash<int, bool> rawbuttons;
    QHash<int, int> axisvalues;
    QHash<int, int> dpadvalues;

    static const QString xmlName;

protected:
    void readJoystickConfig(QXmlStreamReader *xml);

    SDL_GameController *controller;

signals:


public slots:
    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

protected slots:
    virtual void axisActivatedEvent(int setindex, int axisindex, int value);
    virtual void buttonClickEvent(int buttonindex);
    virtual void buttonReleaseEvent(int buttonindex);
};

#endif // GAMECONTROLLER_H
