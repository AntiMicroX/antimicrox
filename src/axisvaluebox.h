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

#ifndef AXISVALUEBOX_H
#define AXISVALUEBOX_H

#include <QWidget>

class AxisValueBox : public QWidget
{
    Q_OBJECT
public:
    explicit AxisValueBox(QWidget *parent = 0);

    int getDeadZone();
    int getMaxZone();
    int getJoyValue();
    int getThrottle();

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void paintEvent(QPaintEvent *event);

    int deadZone;
    int maxZone;
    int joyValue;
    int throttle;
    int boxwidth;
    int boxheight;
    int lboxstart;
    int lboxend;
    int rboxstart;
    int rboxend;
    int singlewidth;
    int singleend;

signals:
    
public slots:
    void setThrottle(int throttle);
    void setValue(int value);
    void setDeadZone(int deadZone);
    void setMaxZone(int maxZone);

};

#endif // AXISVALUEBOX_H
