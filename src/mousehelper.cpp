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

#include "mousehelper.h"

#include "messagehandler.h"

#include <QDesktopWidget>
#include <QDebug>

MouseHelper::MouseHelper(QObject *parent) :
    QObject(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    springMouseMoving = false;
    previousCursorLocation[0] = 0;
    previousCursorLocation[1] = 0;
    pivotPoint[0] = -1;
    pivotPoint[1] = -1;
    mouseTimer.setParent(this);
    mouseTimer.setSingleShot(true);
    QObject::connect(&mouseTimer, &QTimer::timeout, this, &MouseHelper::resetSpringMouseMoving);
}

void MouseHelper::resetSpringMouseMoving()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    springMouseMoving = false;
}

void MouseHelper::initDeskWid()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (deskWid == nullptr) deskWid = new QDesktopWidget;
}

void MouseHelper::deleteDeskWid()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (deskWid != nullptr)
    {
        delete deskWid;
        deskWid = nullptr;
    }
}

QDesktopWidget* MouseHelper::getDesktopWidget() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return deskWid;
}
