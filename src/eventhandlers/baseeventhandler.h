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

#ifndef BASEEVENTHANDLER_H
#define BASEEVENTHANDLER_H

#include <QObject>
#include <QString>

#include <springmousemoveinfo.h>
#include <joybuttonslot.h>

class BaseEventHandler : public QObject
{
    Q_OBJECT
public:
    explicit BaseEventHandler(QObject *parent = 0);

    virtual bool init() = 0;
    virtual bool cleanup() = 0;

    virtual void sendKeyboardEvent(JoyButtonSlot *slot, bool pressed) = 0;
    virtual void sendMouseButtonEvent(JoyButtonSlot *slot, bool pressed) = 0;
    virtual void sendMouseEvent(int xDis, int yDis) = 0;
    virtual void sendMouseAbsEvent(int xDis, int yDis, int screen);

    virtual void sendMouseSpringEvent(unsigned int xDis, unsigned int yDis,
                                      unsigned int width, unsigned int height);
    virtual void sendMouseSpringEvent(int xDis, int yDis);

    virtual void sendTextEntryEvent(QString maintext);

    virtual QString getName() = 0;
    virtual QString getIdentifier() = 0;
    virtual void printPostMessages();
    QString getErrorString();


protected:
    QString lastErrorString;

signals:

public slots:

};

#endif // BASEEVENTHANDLER_H
