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

#ifndef XTESTEVENTHANDLER_H
#define XTESTEVENTHANDLER_H

#include "baseeventhandler.h"

#include <joybuttonslot.h>

class XTestEventHandler : public BaseEventHandler
{
    Q_OBJECT
public:
    explicit XTestEventHandler(QObject *parent = 0);

    virtual bool init();
    virtual bool cleanup();
    virtual void sendKeyboardEvent(JoyButtonSlot *slot, bool pressed);
    virtual void sendMouseButtonEvent(JoyButtonSlot *slot, bool pressed);
    virtual void sendMouseEvent(int xDis, int yDis);
    virtual void sendMouseAbsEvent(int xDis, int yDis, int screen);

    virtual QString getName();
    virtual QString getIdentifier();

    virtual void sendTextEntryEvent(QString maintext);

signals:

public slots:

};

#endif // XTESTEVENTHANDLER_H
