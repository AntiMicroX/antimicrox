/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail.com>
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

#include "axisvaluebox.h"

#include "globalvariables.h"
#include "joyaxis.h"

#include <qdrawutil.h>

#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>

AxisValueBox::AxisValueBox(QWidget *parent)
    : QWidget(parent)
{
    m_axis = nullptr;
    m_deadZone = 0;
    m_maxZone = 0;
    joyValue = 0;
    m_throttle = 0;
    lboxstart = 0;
    lboxend = 0;
    rboxstart = 0;
    rboxend = 0;
    boxwidth = 0;
    boxheight = 0;
    singlewidth = 0;
    singleend = 0;
}

void AxisValueBox::setThrottle(int throttle)
{
    qDebug() << "throttle value at start of function setThrottle: " << throttle;

    if ((throttle <= static_cast<int>(JoyAxis::PositiveHalfThrottle)) &&
        (throttle >= static_cast<int>(JoyAxis::NegativeHalfThrottle)))
    {
        qDebug() << "throttle variable has been set in setThrottle with: " << throttle;

        m_throttle = throttle;
        setValue(joyValue);
    }

    update();
}

void AxisValueBox::setValue(int value)
{
    qDebug() << "Value for axis from value box at start is: " << value;
    qDebug() << "throttle variable has value: " << m_throttle;

    if ((value >= GlobalVariables::JoyAxis::AXISMIN) && (value <= GlobalVariables::JoyAxis::AXISMAX))
    {
        qDebug() << "Value for axis from value box is between : " << GlobalVariables::JoyAxis::AXISMIN << " and "
                 << GlobalVariables::JoyAxis::AXISMAX;

        switch (m_throttle)
        {
        case -2:
            this->joyValue = (value <= 0) ? value : (-value);
            break;

        case -1:
            this->joyValue = ((value + GlobalVariables::JoyAxis::AXISMIN) / 2);
            break;

        case 0:
            this->joyValue = value;
            break;

        case 1:
            this->joyValue = (value + GlobalVariables::JoyAxis::AXISMAX) / 2;
            break;

        case 2:
            this->joyValue = (value >= 0) ? value : (-value);
            break;
        }
    }

    update();
}

void AxisValueBox::setValue(JoyAxis *axis, int value)
{
    m_axis = axis;

    qDebug() << "Value for axis from value box at start is: " << value;
    qDebug() << "throttle variable has value: " << m_throttle;

    if ((value >= GlobalVariables::JoyAxis::AXISMIN) && (value <= GlobalVariables::JoyAxis::AXISMAX))
    {
        qDebug() << "Value for axis from value box is between : " << GlobalVariables::JoyAxis::AXISMIN << " and "
                 << GlobalVariables::JoyAxis::AXISMAX;

        switch (m_throttle)
        {
        case -2:
            this->joyValue = (value <= 0) ? value : (-value);
            break;

        case -1:
            this->joyValue = ((value + GlobalVariables::JoyAxis::AXISMIN) / 2);
            break;

        case 0:
            this->joyValue = value;
            break;

        case 1:
            this->joyValue = (value + GlobalVariables::JoyAxis::AXISMAX) / 2;
            break;

        case 2:
            this->joyValue = (value >= 0) ? value : (-value);
            break;
        }
    }

    update();
}

void AxisValueBox::setDeadZone(int deadZone)
{
    if ((deadZone >= GlobalVariables::JoyAxis::AXISMIN) && (deadZone <= GlobalVariables::JoyAxis::AXISMAX))
    {
        m_deadZone = deadZone;
    }

    update();
}

void AxisValueBox::setDeadZone(JoyAxis *axis, int deadZone)
{
    m_axis = axis;

    if ((deadZone >= GlobalVariables::JoyAxis::AXISMIN) && (deadZone <= GlobalVariables::JoyAxis::AXISMAX))
    {
        m_deadZone = deadZone;
    }

    update();
}

int AxisValueBox::getDeadZone() { return m_deadZone; }

void AxisValueBox::setMaxZone(int maxZone)
{
    if ((maxZone >= GlobalVariables::JoyAxis::AXISMIN) && (maxZone <= GlobalVariables::JoyAxis::AXISMAX))
    {
        m_maxZone = maxZone;
    }

    update();
}

void AxisValueBox::setMaxZone(JoyAxis *axis, int maxZone)
{
    m_axis = axis;

    if ((maxZone >= GlobalVariables::JoyAxis::AXISMIN) && (maxZone <= GlobalVariables::JoyAxis::AXISMAX))
    {
        m_maxZone = maxZone;
    }

    update();
}

int AxisValueBox::getMaxZone() { return m_maxZone; }

int AxisValueBox::getJoyValue() { return joyValue; }

int AxisValueBox::getThrottle() { return m_throttle; }

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

    QPainter paint(this);

    paint.setPen(palette().base().color());
    paint.setBrush(palette().base().color());
    QBrush brush(palette().light().color());

    if (m_throttle == 0)
    {
        qDrawShadeRect(&paint, lboxstart, 0, lboxend, height(), palette(), true, 2, 0, &brush);
        qDrawShadeRect(&paint, rboxstart, 0, rboxend, height(), palette(), true, 2, 0, &brush);
    } else
    {
        qDrawShadeRect(&paint, lboxstart, 0, singlewidth, height(), palette(), true, 2, 0, &brush);
    }

    QColor innerColor;
    if (abs(joyValue) <= m_deadZone)
    {
        innerColor = Qt::gray;
    } else if (abs(joyValue) >= m_maxZone)
    {
        innerColor = Qt::red;
    } else
    {
        innerColor = Qt::blue;
    }

    paint.setPen(innerColor);
    paint.setBrush(innerColor);

    int barwidth = (m_throttle == 0) ? boxwidth : singlewidth;
    int barlength = abs((barwidth - 2) * joyValue) / getMaxAxValue();

    if (joyValue > 0)
    {
        paint.drawRect(((m_throttle == 0) ? rboxstart : lboxstart) + 2, 2, barlength, boxheight);
    } else if (joyValue < 0)
    {
        paint.drawRect(lboxstart + barwidth - 2 - barlength, 2, barlength, boxheight);
    }

    // Draw marker for deadZone
    int deadLine = abs((barwidth - 2) * m_deadZone) / getMaxAxValue();
    int maxLine = abs((barwidth - 2) * m_maxZone) / getMaxAxValue();

    paint.setPen(Qt::blue);
    brush.setColor(Qt::blue);
    QBrush maxBrush(Qt::red);

    if (m_throttle == static_cast<int>(JoyAxis::NormalThrottle))
    {
        qDrawPlainRect(&paint, rboxstart + 2 + deadLine, 2, 4, boxheight + 2, Qt::black, 1, &brush);
        qDrawPlainRect(&paint, lboxend - deadLine - 2, 2, 4, boxheight + 2, Qt::black, 1, &brush);

        paint.setPen(Qt::red);
        qDrawPlainRect(&paint, rboxstart + 2 + maxLine, 2, 4, boxheight + 2, Qt::black, 1, &maxBrush);
        qDrawPlainRect(&paint, lboxend - maxLine - 2, 2, 4, boxheight + 2, Qt::black, 1, &maxBrush);
    } else if ((m_throttle == static_cast<int>(JoyAxis::PositiveThrottle)) ||
               (m_throttle == static_cast<int>(JoyAxis::PositiveHalfThrottle)))
    {
        qDrawPlainRect(&paint, lboxstart + deadLine - 2, 2, 4, boxheight + 2, Qt::black, 1, &brush);
        paint.setPen(Qt::red);
        qDrawPlainRect(&paint, lboxstart + maxLine, 2, 4, boxheight + 2, Qt::black, 1, &maxBrush);
    }

    else if ((m_throttle == static_cast<int>(JoyAxis::NegativeThrottle)) ||
             (m_throttle == static_cast<int>(JoyAxis::NegativeHalfThrottle)))
    {
        qDrawPlainRect(&paint, singleend - deadLine - 2, 2, 4, boxheight + 2, Qt::black, 1, &brush);
        paint.setPen(Qt::red);
        qDrawPlainRect(&paint, singleend - maxLine, 2, 4, boxheight + 2, Qt::black, 1, &maxBrush);
    }
}

int AxisValueBox::getMaxAxValue() { return GlobalVariables::JoyAxis::AXISMAX; }

int AxisValueBox::getMinAxValue() { return GlobalVariables::JoyAxis::AXISMIN; }
