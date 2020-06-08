/* antimicroX Gamepad to KB+M event mapper
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


#ifndef AXISVALUEBOX_H
#define AXISVALUEBOX_H

#include <QWidget>

class QResizeEvent;
class QPaintEvent;
class JoyAxis;

class AxisValueBox : public QWidget
{
    Q_OBJECT
public:
    explicit AxisValueBox(QWidget *parent = nullptr);

    int getDeadZone();
    int getMaxZone();
    int getJoyValue();
    int getThrottle();
    int getMinAxValue();
    int getMaxAxValue();

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void paintEvent(QPaintEvent *event);
    
public slots:
    void setThrottle(int throttle);
    void setValue(int value);
    void setValue(JoyAxis* axis, int value);
    void setDeadZone(int deadZone);
    void setDeadZone(JoyAxis* axis, int deadZone);
    void setMaxZone(int maxZone);
    void setMaxZone(JoyAxis* axis, int deadZone);

private:
    JoyAxis* m_axis;
    int m_deadZone;
    int m_maxZone;
    int joyValue;
    int m_throttle;
    int boxwidth;
    int boxheight;
    int lboxstart;
    int lboxend;
    int rboxstart;
    int rboxend;
    int singlewidth;
    int singleend;

};

#endif // AXISVALUEBOX_H
