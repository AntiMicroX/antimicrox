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

#ifndef GAMECONTROLLERSET_H
#define GAMECONTROLLERSET_H

#include <QObject>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QHash>

#include <SDL2/SDL_gamecontroller.h>

#include <setjoystick.h>
#include "gamecontrollerdpad.h"
#include "gamecontrollertrigger.h"

class GameControllerSet : public SetJoystick
{
    Q_OBJECT
public:
    explicit GameControllerSet(InputDevice *device, int index, QObject *parent = 0);

    virtual void refreshAxes();

    virtual void readConfig(QXmlStreamReader *xml);
    virtual void readJoystickConfig(QXmlStreamReader *xml,
                            QHash<unsigned int, SDL_GameControllerButton> &buttons,
                            QHash<unsigned int, SDL_GameControllerAxis> &axes,
                            QList<SDL_GameControllerButtonBind> &hatButtons);

protected:
    void populateSticksDPad();

signals:

public slots:
    virtual void reset();
};

#endif // GAMECONTROLLERSET_H
