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

#include "joybuttonmousehelper.h"

#include "globalvariables.h"
#include "joybuttontypes/joybutton.h"

#include <QDebug>
#include <QList>
#include <QThread>

JoyButtonMouseHelper::JoyButtonMouseHelper(QObject *parent)
    : QObject(parent)
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
    JoyButton::moveMouseCursor(finalx, finaly, elapsedTime, &GlobalVariables::JoyButton::mouseHistoryX,
                               &GlobalVariables::JoyButton::mouseHistoryY, JoyButton::getTestOldMouseTime(),
                               JoyButton::getStaticMouseEventTimer(), GlobalVariables::JoyButton::mouseRefreshRate,
                               GlobalVariables::JoyButton::mouseHistorySize, JoyButton::getCursorXSpeeds(),
                               JoyButton::getCursorYSpeeds(), GlobalVariables::JoyButton::cursorRemainderX,
                               GlobalVariables::JoyButton::cursorRemainderY, GlobalVariables::JoyButton::weightModifier,
                               GlobalVariables::JoyButton::IDLEMOUSEREFRESHRATE, JoyButton::getPendingMouseButtons());

    if ((finalx != 0) || (finaly != 0))
        emit mouseCursorMoved(finalx, finaly, elapsedTime);
}

/**
 * @brief Perform mouse movement in spring mode.
 */
void JoyButtonMouseHelper::moveSpringMouse()
{
    int finalx = 0;
    int finaly = 0;
    bool hasMoved = false;
    JoyButton::moveSpringMouse(finalx, finaly, hasMoved, GlobalVariables::JoyButton::springModeScreen,
                               JoyButton::getSpringXSpeeds(), JoyButton::getSpringYSpeeds(),
                               JoyButton::getPendingMouseButtons(), GlobalVariables::JoyButton::mouseRefreshRate,
                               GlobalVariables::JoyButton::IDLEMOUSEREFRESHRATE, JoyButton::getStaticMouseEventTimer());

    if (hasMoved)
        emit mouseSpringMoved(finalx, finaly);
}

/**
 * @brief Perform mouse events for all buttons and slots.
 */
void JoyButtonMouseHelper::mouseEvent()
{
    if (!JoyButton::hasCursorEvents(JoyButton::getCursorXSpeeds(), JoyButton::getCursorYSpeeds()) &&
        !JoyButton::hasSpringEvents(JoyButton::getSpringXSpeeds(), JoyButton::getSpringYSpeeds()))
    {
        QList<JoyButton *> *buttonList = JoyButton::getPendingMouseButtons();
        QListIterator<JoyButton *> iter(*buttonList);

        while (iter.hasNext())
        {
            JoyButton *temp = iter.next();
            temp->mouseEvent();
        }
    }

    moveMouseCursor();

    if (JoyButton::hasSpringEvents(JoyButton::getSpringXSpeeds(), JoyButton::getSpringYSpeeds()))
    {
        moveSpringMouse();
    }

    JoyButton::restartLastMouseTime(JoyButton::getTestOldMouseTime());
    firstSpringEvent = false;
}

void JoyButtonMouseHelper::resetButtonMouseDistances()
{
    QList<JoyButton *> *buttonList = JoyButton::getPendingMouseButtons();

    for (JoyButton *temp : *buttonList)
    {
        temp->resetAccelerationDistances();
    }
}

void JoyButtonMouseHelper::setFirstSpringStatus(bool status) { firstSpringEvent = status; }

bool JoyButtonMouseHelper::getFirstSpringStatus() { return firstSpringEvent; }

void JoyButtonMouseHelper::carryGamePollRateUpdate(int pollRate) { emit gamepadRefreshRateUpdated(pollRate); }

void JoyButtonMouseHelper::carryMouseRefreshRateUpdate(int refreshRate) { emit mouseRefreshRateUpdated(refreshRate); }

void JoyButtonMouseHelper::changeThread(QThread *thread)
{
    JoyButton::setStaticMouseThread(thread, JoyButton::getStaticMouseEventTimer(), JoyButton::getTestOldMouseTime(),
                                    GlobalVariables::JoyButton::IDLEMOUSEREFRESHRATE, JoyButton::getMouseHelper());
}
