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

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <QObject>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "inputdevice.h"

class Joystick : public InputDevice
{
    Q_OBJECT
public:
    explicit Joystick(SDL_Joystick *joyhandle, int deviceIndex, AntiMicroSettings *settings, QObject *parent=0);

    virtual QString getName();
    virtual QString getSDLName();
    virtual QString getGUIDString(); // GUID available on SDL 2.
    virtual QString getXmlName();
    virtual void closeSDLDevice();
#ifdef USE_SDL_2
    virtual SDL_JoystickID getSDLJoystickID();
#endif

    virtual int getNumberRawButtons();
    virtual int getNumberRawAxes();
    virtual int getNumberRawHats();

    static const QString xmlName;

protected:
    SDL_Joystick *joyhandle;

signals:

public slots:

};

Q_DECLARE_METATYPE(Joystick*)

#endif // JOYSTICK_H
