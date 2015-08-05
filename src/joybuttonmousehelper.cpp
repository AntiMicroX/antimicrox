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

//#include <QDebug>
#include <QList>

#include "joybutton.h"
#include "joybuttonmousehelper.h"

JoyButtonMouseHelper::JoyButtonMouseHelper(QObject *parent) :
    QObject(parent)
{
    firstSpringEvent = false;
}

/**
 * @brief Perform mouse movement in cursor mode.
 */
void JoyButtonMouseHelper::moveMouseCursor()
{
    int finalx = 0;
    int finaly = 0;
    int elapsedTime = 5;
    JoyButton::moveMouseCursor(finalx, finaly, elapsedTime);
    if (finalx != 0 || finaly != 0)
    {
        emit mouseCursorMoved(finalx, finaly, elapsedTime);
    }
}

/**
 * @brief Perform mouse movement in spring mode.
 */
void JoyButtonMouseHelper::moveSpringMouse()
{
    int finalx = 0;
    int finaly = 0;
    bool hasMoved = false;
    JoyButton::moveSpringMouse(finalx, finaly, hasMoved);
    if (hasMoved)
    {
        emit mouseSpringMoved(finalx, finaly);
    }
}

/**
 * @brief Perform mouse events for all buttons and slots.
 */
void JoyButtonMouseHelper::mouseEvent()
{
    if (!JoyButton::hasCursorEvents() && !JoyButton::hasSpringEvents())
    {
        QList<JoyButton*> *buttonList = JoyButton::getPendingMouseButtons();
        QListIterator<JoyButton*> iter(*buttonList);
        while (iter.hasNext())
        {
            JoyButton *temp = iter.next();
            temp->mouseEvent();
        }
    }

    moveMouseCursor();

    if (JoyButton::hasSpringEvents())
    {
        moveSpringMouse();
    }

    JoyButton::restartLastMouseTime();
    firstSpringEvent = false;
}

void JoyButtonMouseHelper::resetButtonMouseDistances()
{
    QList<JoyButton*> *buttonList = JoyButton::getPendingMouseButtons();
    QListIterator<JoyButton*> iter(*buttonList);
    while (iter.hasNext())
    {
        JoyButton *temp = iter.next();
        temp->resetAccelerationDistances();
    }
}

void JoyButtonMouseHelper::setFirstSpringStatus(bool status)
{
    firstSpringEvent = status;
}

bool JoyButtonMouseHelper::getFirstSpringStatus()
{
    return firstSpringEvent;
}

void JoyButtonMouseHelper::carryGamePollRateUpdate(unsigned int pollRate)
{
    emit gamepadRefreshRateUpdated(pollRate);
}

void JoyButtonMouseHelper::carryMouseRefreshRateUpdate(unsigned int refreshRate)
{
    emit mouseRefreshRateUpdated(refreshRate);
}

void JoyButtonMouseHelper::changeThread(QThread *thread)
{
    JoyButton::setStaticMouseThread(thread);
}
