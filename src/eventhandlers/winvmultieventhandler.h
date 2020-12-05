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

#ifndef WINVMULTIEVENTHANDLER_H
#define WINVMULTIEVENTHANDLER_H

#include <QObject>
#include <QVector>

#include "baseeventhandler.h"

#include <joybuttonslot.h>

#include <vmulticlient.h>

#include <antkeymapper.h>
#include "winsendinputeventhandler.h"


class WinVMultiEventHandler : public BaseEventHandler
{
    Q_OBJECT
public:
    explicit WinVMultiEventHandler(QObject *parent = 0);
    ~WinVMultiEventHandler();

    virtual bool init();
    virtual bool cleanup();
    virtual void sendKeyboardEvent(JoyButtonSlot *slot, bool pressed);
    virtual void sendMouseButtonEvent(JoyButtonSlot *slot, bool pressed);
    virtual void sendMouseEvent(int xDis, int yDis);
    virtual void sendMouseAbsEvent(int xDis, int yDis, int screen);
    virtual void sendMouseSpringEvent(unsigned int xDis, unsigned int yDis,
                                      unsigned int width, unsigned int height);

    // TODO: Implement text event using information from QtWinKeyMapper.
    virtual void sendTextEntryEvent(QString maintext);

    virtual QString getName();
    virtual QString getIdentifier();

protected:
    pvmulti_client vmulti;
    BYTE mouseButtons;
    BYTE shiftKeys;
    BYTE multiKeys;
    BYTE extraKeys;
    QVector<BYTE> keyboardKeys;
    WinSendInputEventHandler sendInputHandler;
    QtKeyMapperBase *nativeKeyMapper;


signals:

public slots:

};

#endif // WINVMULTIEVENTHANDLER_H
