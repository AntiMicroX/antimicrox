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

#ifndef GAMECONTROLLERTRIGGER_H
#define GAMECONTROLLERTRIGGER_H

#include <QObject>
#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <SDL2/SDL_gamecontroller.h>

#include "gamecontrollertriggerbutton.h"
#include <joyaxis.h>

class GameControllerTrigger : public JoyAxis
{
    Q_OBJECT
public:
    explicit GameControllerTrigger(int index, int originset, SetJoystick *parentSet, QObject *parent = 0);

    virtual QString getXmlName();
    virtual QString getPartialName(bool forceFullFormat, bool displayNames);

    virtual int getDefaultDeadZone();
    virtual int getDefaultMaxZone();
    virtual ThrottleTypes getDefaultThrottle();

    void readJoystickConfig(QXmlStreamReader *xml);

    virtual void writeConfig(QXmlStreamWriter *xml);

    static const int AXISDEADZONE;
    static const int AXISMAXZONE;
    static const ThrottleTypes DEFAULTTHROTTLE;

    static const QString xmlName;

protected:
    void correctJoystickThrottle();

signals:

public slots:

};

#endif // GAMECONTROLLERTRIGGER_H
