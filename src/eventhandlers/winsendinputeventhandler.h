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

#ifndef WINSENDINPUTEVENTHANDLER_H
#define WINSENDINPUTEVENTHANDLER_H

#include <QObject>

#include "baseeventhandler.h"

#include <joybuttonslot.h>

/**
 * @brief Input event handler class for Windows
 *
 * It utilizes SendInput functions
 *
 */
class WinSendInputEventHandler : public BaseEventHandler
{
    Q_OBJECT
  public:
    explicit WinSendInputEventHandler(QObject *parent = 0);

    virtual bool init() override;
    virtual bool cleanup() override;
    virtual void sendKeyboardEvent(JoyButtonSlot *slot, bool pressed) override;
    virtual void sendMouseButtonEvent(JoyButtonSlot *slot, bool pressed) override;
    virtual void sendMouseEvent(int xDis, int yDis) override;
    virtual void sendMouseSpringEvent(int xDis, int yDis, int width, int height) override;
    virtual void sendTextEntryEvent(QString maintext) override;

    virtual QString getName() override;
    virtual QString getIdentifier() override;

  signals:

  public slots:
};

#endif // WINSENDINPUTEVENTHANDLER_H
