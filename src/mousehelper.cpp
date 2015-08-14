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

MouseHelper::MouseHelper(QObject *parent) :
    QObject(parent)
{
    springMouseMoving = false;
    previousCursorLocation[0] = 0;
    previousCursorLocation[1] = 0;
    pivotPoint[0] = -1;
    pivotPoint[1] = -1;
    mouseTimer.setParent(this);
    mouseTimer.setSingleShot(true);
    QObject::connect(&mouseTimer, SIGNAL(timeout()), this, SLOT(resetSpringMouseMoving()));
}

void MouseHelper::resetSpringMouseMoving()
{
    springMouseMoving = false;
}

void MouseHelper::initDeskWid()
{
    if (!deskWid)
    {
        deskWid = new QDesktopWidget;
    }
}

void MouseHelper::deleteDeskWid()
{
    if (deskWid)
    {
        delete deskWid;
        deskWid = 0;
    }
}

QDesktopWidget* MouseHelper::getDesktopWidget()
{
    return deskWid;
}
