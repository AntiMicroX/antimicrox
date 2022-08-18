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

#ifndef XTESTEVENTHANDLER_H
#define XTESTEVENTHANDLER_H

#include "baseeventhandler.h"

class JoyButtonSlot;

class XTestEventHandler : public BaseEventHandler
{
    Q_OBJECT

  public:
    explicit XTestEventHandler(QObject *parent = nullptr);
    virtual ~XTestEventHandler();

    bool init() override;
    bool cleanup() override;

    void sendKeyboardEvent(JoyButtonSlot *slot, bool pressed) override;
    void sendMouseButtonEvent(JoyButtonSlot *slot, bool pressed) override;
    void sendMouseEvent(int xDis, int yDis) override;
    void sendMouseAbsEvent(int xDis, int yDis, int screen) override;

    void sendMouseSpringEvent(int xDis, int yDis, int width, int height) override;

    void sendTextEntryEvent(QString maintext) override;

    QString getName() override;
    QString getIdentifier() override;
    void printPostMessages() override;
};

#endif // XTESTEVENTHANDLER_H
