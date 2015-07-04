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

#ifndef JOYCONTROLSTICKSTATUSBOX_H
#define JOYCONTROLSTICKSTATUSBOX_H

#include <QWidget>
#include <QSize>

#include "joycontrolstick.h"
#include "joyaxis.h"

class JoyControlStickStatusBox : public QWidget
{
    Q_OBJECT
public:
    explicit JoyControlStickStatusBox(QWidget *parent = 0);
    explicit JoyControlStickStatusBox(JoyControlStick *stick, QWidget *parent = 0);

    void setStick(JoyControlStick *stick);

    JoyControlStick* getStick();

    virtual int heightForWidth(int width) const;
    QSize sizeHint() const;

protected:
    virtual void paintEvent(QPaintEvent *event);
    void drawEightWayBox();
    void drawFourWayCardinalBox();
    void drawFourWayDiagonalBox();

    JoyControlStick *stick;

signals:
    
public slots:
    
};

#endif // JOYCONTROLSTICKSTATUSBOX_H
