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

#ifndef JOYBUTTONMOUSEHELPER_H
#define JOYBUTTONMOUSEHELPER_H

#include <QObject>
#include <QThread>

class JoyButtonMouseHelper : public QObject
{
    Q_OBJECT
public:
    explicit JoyButtonMouseHelper(QObject *parent = 0);
    void resetButtonMouseDistances();
    void setFirstSpringStatus(bool status);
    bool getFirstSpringStatus();
    void carryGamePollRateUpdate(unsigned int pollRate);
    void carryMouseRefreshRateUpdate(unsigned int refreshRate);

protected:
    bool firstSpringEvent;

signals:
    void mouseCursorMoved(int mouseX, int mouseY, int elapsed);
    void mouseSpringMoved(int mouseX, int mouseY);
    void gamepadRefreshRateUpdated(unsigned int pollRate);
    void mouseRefreshRateUpdated(unsigned int refreshRate);

public slots:
    void moveMouseCursor();
    void moveSpringMouse();
    void mouseEvent();
    void changeThread(QThread *thread);
};

#endif // JOYBUTTONMOUSEHELPER_H
