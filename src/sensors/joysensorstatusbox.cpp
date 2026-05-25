/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2022 Max Maisel <max.maisel@posteo.de>
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

#include "joysensorstatusbox.h"

#include "common.h"
#include "globalvariables.h"
#include "joyaxis.h"
#include "joysensor.h"

#include <qdrawutil.h>

#include <QDebug>
#include <QLinearGradient>
#include <QList>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QSizePolicy>

JoySensorStatusBox::JoySensorStatusBox(QWidget *parent)
    : QWidget(parent)
    , m_sensor(nullptr)
{
}

/**
 * @brief Sets the to be visualized sensor.
 *
 * It disconnects all events to the previous sensor if there was any, and
 * connects them to the new sensor.
 */
void JoySensorStatusBox::setSensor(JoySensor *sensor)
{
    if (m_sensor != nullptr)
    {
        disconnect(m_sensor, SIGNAL(deadZoneChanged(double)), this, nullptr);
        disconnect(m_sensor, SIGNAL(moved(float, float, float)), this, nullptr);
        disconnect(m_sensor, SIGNAL(diagonalRangeChanged(double)), this, nullptr);
        disconnect(m_sensor, SIGNAL(maxZoneChanged(double)), this, nullptr);
    }

    m_sensor = sensor;
    connect(m_sensor, SIGNAL(deadZoneChanged(double)), this, SLOT(update()));
    connect(m_sensor, SIGNAL(moved(float, float, float)), this, SLOT(update()));
    connect(m_sensor, SIGNAL(diagonalRangeChanged(double)), this, SLOT(update()));
    connect(m_sensor, SIGNAL(maxZoneChanged(double)), this, SLOT(update()));

    update();
}

/**
 * @brief Get the visualized sensor object
 */
JoySensor *JoySensorStatusBox::getSensor() const { return m_sensor; }

QSize JoySensorStatusBox::sizeHint() const { return QSize(-1, -1); }

void JoySensorStatusBox::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    PadderCommon::inputDaemonMutex.lock();
    drawArtificialHorizon();
    PadderCommon::inputDaemonMutex.unlock();
}

/**
 * @brief draws the artificial horizon instrument on the screen
 */
void JoySensorStatusBox::drawArtificialHorizon()
{
    QPainter paint(this);
    paint.setRenderHint(QPainter::Antialiasing, true);

    int side = qMin(width(), height());

    QPen pen;
    QPixmap pix(side, side);
    pix.fill(Qt::transparent);
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Switch to centric coordinate system
    painter.translate(side / 2.0, side / 2.0);
    painter.scale(side * 0.45, -side * 0.45);
    painter.save();

    // Draw moving instrument parts
    QPainterPath clippingPath;
    clippingPath.addEllipse(QPointF(0, 0), 1, 1);
    painter.setClipPath(clippingPath);
    JoySensorType type;

    double pitch, roll, yaw;
    if (m_sensor != nullptr)
    {
        type = m_sensor->getType();
        if (type == ACCELEROMETER)
        {
            pitch = -JoySensor::radToDeg(m_sensor->calculatePitch());
            roll = JoySensor::radToDeg(m_sensor->calculateRoll());
            yaw = 0;
        } else
        {
            pitch = -m_sensor->getXCoordinate();
            roll = m_sensor->getYCoordinate();
            yaw = -m_sensor->getZCoordinate();
        }
    } else
    {
        type = ACCELEROMETER;
        pitch = 0;
        roll = 0;
        yaw = 0;
    }

    pitch = qBound(-180.0, pitch, 180.0);
    roll = qBound(-180.0, roll, 180.0);
    yaw = qBound(-180.0, yaw, 180.0);
    painter.translate(yaw / 90, pitch / 90);
    painter.rotate(roll);

    pen.setColor(Qt::transparent);
    painter.setPen(pen);
    painter.setBrush(QBrush(QColor(64, 128, 255)));
    painter.drawRect(QRectF(-10, 0, 20, 10));
    painter.setBrush(QBrush(Qt::black));
    painter.drawRect(QRectF(-10, -10, 20, 10));

    // Draw dead zone
    pen.setColor(Qt::red);
    pen.setWidthF(0.02);
    painter.setPen(pen);
    painter.setBrush(QBrush(QColor(255, 0, 0, 128)));
    double deadZone = m_sensor != nullptr ? m_sensor->getDeadZone() : 0.0;
    painter.drawEllipse(QPointF(0, 0), deadZone / 90, deadZone / 90);

    // Draw max zone
    QPainterPath maxZonePath;
    double maxZone = m_sensor != nullptr ? m_sensor->getMaxZone() : 0.0;
    maxZonePath.addEllipse(QPointF(0, 0), 10, 10);
    maxZonePath.addEllipse(QPointF(0, 0), maxZone / 90, maxZone / 90);
    pen.setColor(Qt::darkGreen);
    pen.setWidthF(0.02);
    painter.setPen(pen);
    painter.setBrush(QBrush(QColor(0, 128, 0, 128)));
    painter.drawPath(maxZonePath);

    // Draw diagonal zones
    pen.setColor(Qt::green);
    painter.setPen(pen);
    painter.setBrush(QBrush(QColor(0, 255, 0, 128)));

    double diagonalRange = m_sensor != nullptr ? m_sensor->getDiagonalRange() : 0.0;
    if (type == GYROSCOPE)
    {
        for (int i = 0; i < 4; ++i)
        {
            painter.drawPie(QRectF(-maxZone / 90, -maxZone / 90, 2 * maxZone / 90, 2 * maxZone / 90),
                            (45 + 90 * i - diagonalRange / 2) * 16, diagonalRange * 16);
        }
    } else
    {
        painter.drawPie(QRectF(-maxZone / 90, -maxZone / 90, 2 * maxZone / 90, 2 * maxZone / 90),
                        (135 - diagonalRange / 2) * 16, (diagonalRange + 90) * 16);
        painter.drawPie(QRectF(-maxZone / 90, -maxZone / 90, 2 * maxZone / 90, 2 * maxZone / 90),
                        (-45 - diagonalRange / 2) * 16, (diagonalRange + 90) * 16);
    }

    // Pitch scale: 30deg per line
    pen.setColor(Qt::white);
    pen.setWidthF(0.025);
    painter.setPen(pen);
    painter.setBrush(QBrush(Qt::transparent));
    for (int j = -180; j <= 180; j += 30)
    {
        painter.drawLine(QPointF(-10, j / 90.0), QPointF(10, j / 90.0));
    }

    // Yaw scale: 30deg per line
    if (type == GYROSCOPE)
    {
        pen.setColor(Qt::white);
        pen.setWidthF(0.025);
        painter.setPen(pen);
        painter.setBrush(QBrush(Qt::transparent));
        for (int j = -180; j <= 180; j += 30)
        {
            painter.drawLine(QPointF(j / 90.0, -10), QPointF(j / 90.0, 10));
        }
    }

    // Draw fixed instrument parts
    painter.restore();
    painter.save();
    pen.setColor(QColor(80, 80, 80));
    pen.setWidthF(0.2);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(QPointF(0, 0), 1, 1);

    // Draw scale
    pen.setWidthF(0.05);
    pen.setColor(Qt::yellow);
    painter.setPen(pen);

    painter.drawLine(QPointF(0.3, 0), QPointF(0.2, 0));
    painter.drawLine(QPointF(-0.3, 0), QPointF(-0.2, 0));
    painter.drawArc(QRectF(-0.2, -0.2, 0.4, 0.4), 0 * 16, 180 * 16);
    painter.drawPoint(QPointF(0, 0));

    pen.setColor(Qt::white);
    painter.setPen(pen);
    for (int j = 0; j < 19; ++j)
    {
        painter.drawLine(QPointF(1, 0), QPointF(0.9, 0));
        painter.rotate(10.0);
    }

    // Draw dead zone
    painter.restore();
    pen.setColor(Qt::red);
    pen.setWidthF(0.1);
    painter.setPen(pen);
    painter.setOpacity(0.5);
    painter.drawArc(QRectF(-1, -1, 2, 2), -16 * deadZone, 16 * deadZone * 2);
    painter.drawArc(QRectF(-1, -1, 2, 2), 16 * (180 - deadZone), 16 * deadZone * 2);

    // Draw max zone
    pen.setColor(Qt::darkGreen);
    painter.setPen(pen);
    double tmpMaxZone = std::min(maxZone, 90.0);
    painter.drawArc(QRectF(-1, -1, 2, 2), 16 * (90 - (90 - tmpMaxZone)), 16 * (90 - tmpMaxZone) * 2);
    painter.drawArc(QRectF(-1, -1, 2, 2), 16 * (270 - (90 - tmpMaxZone)), 16 * (90 - tmpMaxZone) * 2);

    // Draw to window
    paint.setCompositionMode(QPainter::CompositionMode_SourceOver);
    paint.drawPixmap(pix.rect(), pix);
}
