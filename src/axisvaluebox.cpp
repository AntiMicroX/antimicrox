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

#include <QPainter>
#include <qdrawutil.h>

#include "axisvaluebox.h"
#include "joyaxis.h"

AxisValueBox::AxisValueBox(QWidget *parent) :
    QWidget(parent)
{
    deadZone = 0;
    maxZone = 0;
    joyValue = 0;
    throttle = 0;
    lboxstart = 0;
    lboxend = 0;
    rboxstart = 0;
    rboxend = 0;
}

void AxisValueBox::setThrottle(int throttle)
{
    if (throttle <= JoyAxis::PositiveHalfThrottle && throttle >= JoyAxis::NegativeHalfThrottle)
    {
        this->throttle = throttle;
        setValue(joyValue);
    }
    update();
}

void AxisValueBox::setValue(int value)
{
    if (value >= JoyAxis::AXISMIN && value <= JoyAxis::AXISMAX)
    {
        if (throttle == JoyAxis::NormalThrottle)
        {
            this->joyValue = value;
        }
        else if (throttle == JoyAxis::NegativeThrottle)
        {
            this->joyValue = (value + JoyAxis::AXISMIN) / 2;
        }
        else if (throttle == JoyAxis::PositiveThrottle)
        {
            this->joyValue = (value + JoyAxis::AXISMAX) / 2;
        }
        else if (throttle == JoyAxis::NegativeHalfThrottle)
        {
            this->joyValue = value <= 0 ? value : -value;
        }
        else if (throttle == JoyAxis::PositiveHalfThrottle)
        {
            this->joyValue = value >= 0 ? value : -value;
        }
    }
    update();
}

void AxisValueBox::setDeadZone(int deadZone)
{
    if (deadZone >= JoyAxis::AXISMIN && deadZone <= JoyAxis::AXISMAX)
    {
        this->deadZone = deadZone;
    }
    update();
}

int AxisValueBox::getDeadZone()
{
    return deadZone;
}

void AxisValueBox::setMaxZone(int maxZone)
{
    if (maxZone >= JoyAxis::AXISMIN && maxZone <= JoyAxis::AXISMAX)
    {
        this->maxZone = maxZone;
    }
    update();
}

int AxisValueBox::getMaxZone()
{
    return maxZone;
}

int AxisValueBox::getJoyValue()
{
    return joyValue;
}

int AxisValueBox::getThrottle()
{
    return throttle;
}

void AxisValueBox::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    boxwidth = (this->width() / 2) - 5;
    boxheight = this->height() - 4;

    lboxstart = 0;
    lboxend = lboxstart + boxwidth;

    rboxstart = lboxend + 10;
    rboxend = rboxstart + boxwidth;

    singlewidth = this->width();
    singleend = lboxstart + singlewidth;
}

void AxisValueBox::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter paint (this);

    paint.setPen(palette().base().color());
    paint.setBrush(palette().base().color());
    QBrush brush(palette().light().color());

    if (throttle == 0)
    {
        qDrawShadeRect(&paint, lboxstart, 0, lboxend, height(), palette(), true, 2, 0, &brush);
        qDrawShadeRect(&paint, rboxstart, 0, rboxend, height(), palette(), true, 2, 0, &brush);
    }
    else
    {
        qDrawShadeRect(&paint, lboxstart, 0, singlewidth, height(), palette(), true, 2, 0, &brush);
    }

    QColor innerColor;
    if (abs(joyValue) <= deadZone)
    {
        innerColor = Qt::gray;
    }
    else if (abs(joyValue) >= maxZone)
    {
        innerColor = Qt::red;
    }
    else
    {
        innerColor = Qt::blue;
    }
    paint.setPen(innerColor);
    paint.setBrush(innerColor);

    int barwidth = (throttle == 0) ? boxwidth : singlewidth;
    int barlength = abs((barwidth - 2) * joyValue) / JoyAxis::AXISMAX;

    if (joyValue > 0)
    {
        paint.drawRect(((throttle == 0) ? rboxstart : lboxstart) + 2, 2, barlength, boxheight);
    }
    else if (joyValue < 0)
    {
        paint.drawRect(lboxstart + barwidth - 2 - barlength, 2, barlength, boxheight);
    }

    // Draw marker for deadZone
    int deadLine = abs((barwidth - 2) * deadZone) / JoyAxis::AXISMAX;
    int maxLine = abs((barwidth - 2) * maxZone) / JoyAxis::AXISMAX;

    paint.setPen(Qt::blue);
    brush.setColor(Qt::blue);
    QBrush maxBrush(Qt::red);

    if (throttle == JoyAxis::NormalThrottle)
    {
        qDrawPlainRect(&paint, rboxstart + 2 + deadLine, 2, 4, boxheight + 2, Qt::black, 1, &brush);
        qDrawPlainRect(&paint, lboxend - deadLine - 2, 2, 4, boxheight + 2, Qt::black, 1, &brush);

        paint.setPen(Qt::red);
        qDrawPlainRect(&paint, rboxstart + 2 + maxLine, 2, 4, boxheight + 2, Qt::black, 1, &maxBrush);
        qDrawPlainRect(&paint, lboxend - maxLine - 2, 2, 4, boxheight + 2, Qt::black, 1, &maxBrush);
    }
    else if (throttle == JoyAxis::PositiveThrottle || JoyAxis::PositiveHalfThrottle)
    {
        qDrawPlainRect(&paint, lboxstart + deadLine - 2, 2, 4, boxheight + 2, Qt::black, 1, &brush);
        paint.setPen(Qt::red);
        qDrawPlainRect(&paint, lboxstart + maxLine, 2, 4, boxheight + 2, Qt::black, 1, &maxBrush);
    }

    else if (throttle == JoyAxis::NegativeThrottle || throttle == JoyAxis::NegativeHalfThrottle)
    {
        qDrawPlainRect(&paint, singleend - deadLine - 2, 2, 4, boxheight + 2, Qt::black, 1, &brush);
        paint.setPen(Qt::red);
        qDrawPlainRect(&paint, singleend - maxLine, 2, 4, boxheight + 2, Qt::black, 1, &maxBrush);
    }
}
