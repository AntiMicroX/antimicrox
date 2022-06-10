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

#include "joycontrolstickstatusbox.h"

#include "common.h"
#include "globalvariables.h"
#include "joyaxis.h"
#include "joycontrolstick.h"

#include <qdrawutil.h>

#include <QDebug>
#include <QLinearGradient>
#include <QList>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QSizePolicy>

JoyControlStickStatusBox::JoyControlStickStatusBox(QWidget *parent)
    : QWidget(parent)
    , m_stick(nullptr)
{
}

JoyControlStickStatusBox::JoyControlStickStatusBox(JoyControlStick *stick, QWidget *parent)
    : QWidget(parent)
    , m_stick(nullptr)
{
    setStick(stick);
}

void JoyControlStickStatusBox::setStick(JoyControlStick *stick)
{
    if (m_stick != nullptr)
    {
        disconnect(stick, SIGNAL(deadZoneChanged(int)), this, nullptr);
        disconnect(stick, SIGNAL(moved(int, int)), this, nullptr);
        disconnect(stick, SIGNAL(diagonalRangeChanged(int)), this, nullptr);
        disconnect(stick, SIGNAL(maxZoneChanged(int)), this, nullptr);
        disconnect(stick, SIGNAL(modifierZoneChanged(int)), this, nullptr);
        disconnect(stick, SIGNAL(joyModeChanged()), this, nullptr);
        disconnect(stick, SIGNAL(circleAdjustChange(double)), this, nullptr);
    }

    m_stick = stick;
    connect(stick, SIGNAL(deadZoneChanged(int)), this, SLOT(update()));
    connect(stick, SIGNAL(moved(int, int)), this, SLOT(update()));
    connect(stick, SIGNAL(diagonalRangeChanged(int)), this, SLOT(update()));
    connect(stick, SIGNAL(maxZoneChanged(int)), this, SLOT(update()));
    connect(stick, SIGNAL(modifierZoneChanged(int)), this, SLOT(update()));
    connect(stick, SIGNAL(joyModeChanged()), this, SLOT(update()));
    connect(stick, SIGNAL(circleAdjustChange(double)), this, SLOT(update()));

    update();
}

JoyControlStick *JoyControlStickStatusBox::getStick() const { return m_stick; }

int JoyControlStickStatusBox::heightForWidth(int width) const { return width; }

QSize JoyControlStickStatusBox::sizeHint() const { return QSize(-1, -1); }

void JoyControlStickStatusBox::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    PadderCommon::inputDaemonMutex.lock();

    if (m_stick == nullptr || m_stick->getJoyMode() == JoyControlStick::StandardMode ||
        m_stick->getJoyMode() == JoyControlStick::EightWayMode)
    {
        drawEightWayBox();
    } else if (m_stick->getJoyMode() == JoyControlStick::FourWayCardinal)
    {
        drawFourWayCardinalBox();
    } else if (m_stick->getJoyMode() == JoyControlStick::FourWayDiagonal)
    {
        drawFourWayDiagonalBox();
    }

    PadderCommon::inputDaemonMutex.unlock();
}

void JoyControlStickStatusBox::drawEightWayBox()
{
    QPainter paint(this);
    paint.setRenderHint(QPainter::Antialiasing, true);

    int side = qMin(width() - 2, height() - 2);

    QPixmap pix(side, side);
    pix.fill(Qt::transparent);
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Draw outline circle
    QPen penny;
    penny.setColor(Qt::black);
    penny.setWidth(0);
    painter.setPen(penny);
    painter.setBrush(Qt::NoBrush);

    painter.save();
    painter.scale(side / static_cast<double>(GlobalVariables::JoyAxis::AXISMAX * 2.0),
                  side / static_cast<double>(GlobalVariables::JoyAxis::AXISMAX * 2.0));
    painter.translate(GlobalVariables::JoyAxis::AXISMAX, GlobalVariables::JoyAxis::AXISMAX);

    painter.drawEllipse(-GlobalVariables::JoyAxis::AXISMAX, -GlobalVariables::JoyAxis::AXISMAX,
                        GlobalVariables::JoyAxis::AXISMAX * 2, GlobalVariables::JoyAxis::AXISMAX * 2);

    // Draw diagonal zones
    if (m_stick != nullptr)
    {
        QList<double> anglesList = m_stick->getDiagonalZoneAngles();
        int diagonalRange = m_stick->getDiagonalRange();

        penny.setWidth(0);
        penny.setColor(Qt::black);
        painter.setPen(penny);
        painter.setBrush(QBrush(Qt::green));

        painter.drawPie(-GlobalVariables::JoyAxis::AXISMAX, -GlobalVariables::JoyAxis::AXISMAX,
                        GlobalVariables::JoyAxis::AXISMAX * 2, GlobalVariables::JoyAxis::AXISMAX * 2,
                        static_cast<int>(anglesList.value(2)) * 16, diagonalRange * 16);
        painter.drawPie(-GlobalVariables::JoyAxis::AXISMAX, -GlobalVariables::JoyAxis::AXISMAX,
                        GlobalVariables::JoyAxis::AXISMAX * 2, GlobalVariables::JoyAxis::AXISMAX * 2,
                        static_cast<int>(anglesList.value(4)) * 16, diagonalRange * 16);
        painter.drawPie(-GlobalVariables::JoyAxis::AXISMAX, -GlobalVariables::JoyAxis::AXISMAX,
                        GlobalVariables::JoyAxis::AXISMAX * 2, GlobalVariables::JoyAxis::AXISMAX * 2,
                        static_cast<int>(anglesList.value(6)) * 16, diagonalRange * 16);
        painter.drawPie(-GlobalVariables::JoyAxis::AXISMAX, -GlobalVariables::JoyAxis::AXISMAX,
                        GlobalVariables::JoyAxis::AXISMAX * 2, GlobalVariables::JoyAxis::AXISMAX * 2,
                        static_cast<int>(anglesList.value(8)) * 16, diagonalRange * 16);

        // Draw modifier zone circle
        int modifierZone = m_stick->getModifierZone();
        int maxZone = m_stick->getMaxZone();
        penny.setWidth(0);
        penny.setColor(Qt::blue);
        painter.setOpacity(0.5);
        painter.setPen(penny);
        painter.setBrush(QBrush(Qt::yellow));

        if (m_stick->getModifierZoneInverted())
        {
            painter.drawEllipse(-modifierZone, -modifierZone, modifierZone * 2, modifierZone * 2);
        } else
        {
            QPainterPath modifierZonePath;
            modifierZonePath.addEllipse(QPoint(0, 0), maxZone, maxZone);
            modifierZonePath.addEllipse(QPoint(0, 0), modifierZone, modifierZone);
            painter.drawPath(modifierZonePath);
        }
    }

    // Draw deadzone circle
    penny.setWidth(0);
    penny.setColor(Qt::blue);
    painter.setOpacity(1);
    painter.setPen(penny);
    painter.setBrush(QBrush(Qt::red));
    int deadZone = m_stick != nullptr ? m_stick->getDeadZone() : 0;
    painter.drawEllipse(-deadZone, -deadZone, deadZone * 2, deadZone * 2);

    painter.restore();

    painter.save();
    penny.setWidth(0);
    penny.setColor(Qt::gray);
    painter.setPen(penny);
    painter.scale(side / 2.0, side / 2.0);
    painter.translate(1, 1);
    // Draw Y line
    painter.drawLine(0, -1, 0, 1);
    // Draw X line
    painter.drawLine(-1, 0, 1, 0);
    painter.restore();

    painter.save();
    painter.scale(side / static_cast<double>(GlobalVariables::JoyAxis::AXISMAX * 2.0),
                  side / static_cast<double>(GlobalVariables::JoyAxis::AXISMAX * 2.0));
    painter.translate(GlobalVariables::JoyAxis::AXISMAX, GlobalVariables::JoyAxis::AXISMAX);

    penny.setWidth(0);
    painter.setBrush(QBrush(Qt::black));
    penny.setColor(Qt::black);
    painter.setPen(penny);

    if (m_stick != nullptr)
    {
        // Draw raw crosshair
        int linexstart = m_stick->getXCoordinate() - 1000;
        int lineystart = m_stick->getYCoordinate() - 1000;

        if (linexstart < GlobalVariables::JoyAxis::AXISMIN)
            linexstart = GlobalVariables::JoyAxis::AXISMIN;

        if (lineystart < GlobalVariables::JoyAxis::AXISMIN)
            lineystart = GlobalVariables::JoyAxis::AXISMIN;

        painter.drawRect(linexstart, lineystart, 2000, 2000);

        painter.setBrush(QBrush(Qt::darkBlue));
        penny.setColor(Qt::darkBlue);
        painter.setPen(penny);

        // Draw adjusted crosshair
        linexstart = m_stick->getCircleXCoordinate() - 1000;
        lineystart = m_stick->getCircleYCoordinate() - 1000;
        if (linexstart < GlobalVariables::JoyAxis::AXISMIN)
            linexstart = GlobalVariables::JoyAxis::AXISMIN;

        if (lineystart < GlobalVariables::JoyAxis::AXISMIN)
            lineystart = GlobalVariables::JoyAxis::AXISMIN;

        painter.drawRect(linexstart, lineystart, 2000, 2000);
    }

    // Reset pen
    painter.restore();
    penny.setColor(Qt::black);
    painter.setPen(penny);

    // Draw primary pixmap
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    painter.setPen(Qt::NoPen);
    painter.fillRect(0, 0, side, side, palette().window().color());
    paint.drawPixmap(pix.rect(), pix);

    paint.save();
    paint.scale(side / static_cast<double>(GlobalVariables::JoyAxis::AXISMAX * 2.0),
                side / static_cast<double>(GlobalVariables::JoyAxis::AXISMAX * 2.0));
    paint.translate(GlobalVariables::JoyAxis::AXISMAX, GlobalVariables::JoyAxis::AXISMAX);

    // Draw max zone and initial inner clear circle
    int maxzone = m_stick != nullptr ? m_stick->getMaxZone() : GlobalVariables::JoyControlStick::DEFAULTMAXZONE;
    int diffmaxzone = GlobalVariables::JoyAxis::AXISMAX - maxzone;
    paint.setOpacity(0.5);
    paint.setBrush(Qt::darkGreen);
    paint.drawEllipse(-GlobalVariables::JoyAxis::AXISMAX, -GlobalVariables::JoyAxis::AXISMAX,
                      GlobalVariables::JoyAxis::AXISMAX * 2, GlobalVariables::JoyAxis::AXISMAX * 2);
    paint.setCompositionMode(QPainter::CompositionMode_Clear);
    paint.setPen(Qt::NoPen);
    paint.drawEllipse(-GlobalVariables::JoyAxis::AXISMAX + diffmaxzone, -GlobalVariables::JoyAxis::AXISMAX + diffmaxzone,
                      GlobalVariables::JoyAxis::AXISMAX * 2 - (diffmaxzone * 2),
                      GlobalVariables::JoyAxis::AXISMAX * 2 - (diffmaxzone * 2));

    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    paint.setOpacity(1.0);
    paint.restore();

    // Re-draw pixmap so the inner circle will be transparent
    paint.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    paint.drawPixmap(pix.rect(), pix);
    paint.setCompositionMode(QPainter::CompositionMode_SourceOver);
}

void JoyControlStickStatusBox::drawFourWayCardinalBox()
{
    QPainter paint(this);
    paint.setRenderHint(QPainter::Antialiasing, true);

    int side = qMin(width() - 2, height() - 2);

    QPixmap pix(side, side);
    pix.fill(Qt::transparent);
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Draw outline circle
    QPen penny;
    penny.setColor(Qt::black);
    penny.setWidth(0);
    painter.setPen(penny);
    painter.setBrush(Qt::NoBrush);

    painter.save();
    painter.scale(side / static_cast<double>(GlobalVariables::JoyAxis::AXISMAX * 2.0),
                  side / static_cast<double>(GlobalVariables::JoyAxis::AXISMAX * 2.0));
    painter.translate(GlobalVariables::JoyAxis::AXISMAX, GlobalVariables::JoyAxis::AXISMAX);

    painter.drawEllipse(-GlobalVariables::JoyAxis::AXISMAX, -GlobalVariables::JoyAxis::AXISMAX,
                        GlobalVariables::JoyAxis::AXISMAX * 2, GlobalVariables::JoyAxis::AXISMAX * 2);

    if (m_stick != nullptr)
    {
        // Draw diagonal zones
        QList<int> anglesList = m_stick->getFourWayCardinalZoneAngles();
        penny.setWidth(0);
        penny.setColor(Qt::black);
        painter.setPen(penny);
        painter.setOpacity(0.25);
        painter.setBrush(QBrush(Qt::black));

        painter.drawPie(-GlobalVariables::JoyAxis::AXISMAX, -GlobalVariables::JoyAxis::AXISMAX,
                        GlobalVariables::JoyAxis::AXISMAX * 2, GlobalVariables::JoyAxis::AXISMAX * 2,
                        anglesList.value(1) * 16, 90 * 16);
        painter.drawPie(-GlobalVariables::JoyAxis::AXISMAX, -GlobalVariables::JoyAxis::AXISMAX,
                        GlobalVariables::JoyAxis::AXISMAX * 2, GlobalVariables::JoyAxis::AXISMAX * 2,
                        anglesList.value(3) * 16, 90 * 16);

        painter.setOpacity(1.0);

        // Draw modifier zone circle
        int modifierZone = m_stick->getModifierZone();
        int maxZone = m_stick->getMaxZone();
        penny.setWidth(0);
        penny.setColor(Qt::blue);
        painter.setOpacity(0.5);
        painter.setPen(penny);
        painter.setBrush(QBrush(Qt::yellow));

        if (m_stick->getModifierZoneInverted())
        {
            painter.drawEllipse(-modifierZone, -modifierZone, modifierZone * 2, modifierZone * 2);
        } else
        {
            QPainterPath modifierZonePath;
            modifierZonePath.addEllipse(QPoint(0, 0), maxZone, maxZone);
            modifierZonePath.addEllipse(QPoint(0, 0), modifierZone, modifierZone);
            painter.drawPath(modifierZonePath);
        }
    }

    // Draw deadzone circle
    penny.setWidth(0);
    penny.setColor(Qt::blue);
    painter.setPen(penny);
    painter.setBrush(QBrush(Qt::red));
    int deadZone = m_stick != nullptr ? m_stick->getDeadZone() : 0;
    painter.drawEllipse(-deadZone, -deadZone, deadZone * 2, deadZone * 2);

    painter.restore();

    painter.save();
    penny.setWidth(0);
    penny.setColor(Qt::black);
    painter.setPen(penny);
    painter.setOpacity(0.5);
    painter.scale(side / 2.0, side / 2.0);
    painter.translate(1, 1);
    // Draw Y line
    painter.drawLine(0, -1, 0, 1);
    // Draw X line
    painter.drawLine(-1, 0, 1, 0);
    painter.setOpacity(1.0);
    painter.restore();

    painter.save();
    painter.scale(side / static_cast<double>(GlobalVariables::JoyAxis::AXISMAX * 2.0),
                  side / static_cast<double>(GlobalVariables::JoyAxis::AXISMAX * 2.0));
    painter.translate(GlobalVariables::JoyAxis::AXISMAX, GlobalVariables::JoyAxis::AXISMAX);
    penny.setWidth(0);
    painter.setBrush(QBrush(Qt::black));
    penny.setColor(Qt::black);
    painter.setPen(penny);

    if (m_stick != nullptr)
    {
        // Draw raw crosshair
        int linexstart = m_stick->getXCoordinate() - 1000;
        int lineystart = m_stick->getYCoordinate() - 1000;

        if (linexstart < GlobalVariables::JoyAxis::AXISMIN)
            linexstart = GlobalVariables::JoyAxis::AXISMIN;

        if (lineystart < GlobalVariables::JoyAxis::AXISMIN)
            lineystart = GlobalVariables::JoyAxis::AXISMIN;

        painter.drawRect(linexstart, lineystart, 2000, 2000);

        painter.setBrush(QBrush(Qt::darkBlue));
        penny.setColor(Qt::darkBlue);
        painter.setPen(penny);

        // Draw adjusted crosshair
        linexstart = m_stick->getCircleXCoordinate() - 1000;
        lineystart = m_stick->getCircleYCoordinate() - 1000;
        if (linexstart < GlobalVariables::JoyAxis::AXISMIN)
            linexstart = GlobalVariables::JoyAxis::AXISMIN;

        if (lineystart < GlobalVariables::JoyAxis::AXISMIN)
            lineystart = GlobalVariables::JoyAxis::AXISMIN;

        painter.drawRect(linexstart, lineystart, 2000, 2000);
    }

    // Reset pen
    painter.restore();
    penny.setColor(Qt::black);
    painter.setPen(penny);

    // Draw primary pixmap
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    painter.setPen(Qt::NoPen);
    painter.fillRect(0, 0, side, side, palette().window().color());
    paint.drawPixmap(pix.rect(), pix);

    paint.save();
    paint.scale(side / static_cast<double>(GlobalVariables::JoyAxis::AXISMAX * 2.0),
                side / static_cast<double>(GlobalVariables::JoyAxis::AXISMAX * 2.0));
    paint.translate(GlobalVariables::JoyAxis::AXISMAX, GlobalVariables::JoyAxis::AXISMAX);

    // Draw max zone and initial inner clear circle
    int maxzone = m_stick != nullptr ? m_stick->getMaxZone() : GlobalVariables::JoyControlStick::DEFAULTMAXZONE;
    int diffmaxzone = GlobalVariables::JoyAxis::AXISMAX - maxzone;
    paint.setOpacity(0.5);
    paint.setBrush(Qt::darkGreen);
    paint.drawEllipse(-GlobalVariables::JoyAxis::AXISMAX, -GlobalVariables::JoyAxis::AXISMAX,
                      GlobalVariables::JoyAxis::AXISMAX * 2, GlobalVariables::JoyAxis::AXISMAX * 2);
    paint.setCompositionMode(QPainter::CompositionMode_Clear);
    paint.setPen(Qt::NoPen);
    paint.drawEllipse(-GlobalVariables::JoyAxis::AXISMAX + diffmaxzone, -GlobalVariables::JoyAxis::AXISMAX + diffmaxzone,
                      GlobalVariables::JoyAxis::AXISMAX * 2 - (diffmaxzone * 2),
                      GlobalVariables::JoyAxis::AXISMAX * 2 - (diffmaxzone * 2));

    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    paint.setOpacity(1.0);
    paint.restore();

    // Re-draw pixmap so the inner circle will be transparent
    paint.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    paint.drawPixmap(pix.rect(), pix);
    paint.setCompositionMode(QPainter::CompositionMode_SourceOver);
}

void JoyControlStickStatusBox::drawFourWayDiagonalBox()
{
    QPainter paint(this);
    paint.setRenderHint(QPainter::Antialiasing, true);

    int side = qMin(width() - 2, height() - 2);

    QPixmap pix(side, side);
    pix.fill(Qt::transparent);
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Draw outline circle
    QPen penny;
    penny.setColor(Qt::black);
    penny.setWidth(0);
    painter.setPen(penny);
    painter.setBrush(Qt::NoBrush);

    painter.save();
    painter.scale(side / static_cast<double>(GlobalVariables::JoyAxis::AXISMAX * 2.0),
                  side / static_cast<double>(GlobalVariables::JoyAxis::AXISMAX * 2.0));
    painter.translate(GlobalVariables::JoyAxis::AXISMAX, GlobalVariables::JoyAxis::AXISMAX);

    painter.drawEllipse(-GlobalVariables::JoyAxis::AXISMAX, -GlobalVariables::JoyAxis::AXISMAX,
                        GlobalVariables::JoyAxis::AXISMAX * 2, GlobalVariables::JoyAxis::AXISMAX * 2);

    if (m_stick != nullptr)
    {
        // Draw diagonal zones
        QList<int> anglesList = m_stick->getFourWayDiagonalZoneAngles();
        penny.setWidth(0);
        penny.setColor(Qt::black);
        painter.setPen(penny);
        painter.setBrush(QBrush(Qt::black));
        painter.setOpacity(0.25);

        painter.drawPie(-GlobalVariables::JoyAxis::AXISMAX, -GlobalVariables::JoyAxis::AXISMAX,
                        GlobalVariables::JoyAxis::AXISMAX * 2, GlobalVariables::JoyAxis::AXISMAX * 2,
                        anglesList.value(1) * 16, 90 * 16);
        painter.drawPie(-GlobalVariables::JoyAxis::AXISMAX, -GlobalVariables::JoyAxis::AXISMAX,
                        GlobalVariables::JoyAxis::AXISMAX * 2, GlobalVariables::JoyAxis::AXISMAX * 2,
                        anglesList.value(3) * 16, 90 * 16);

        painter.setOpacity(1.0);

        // Draw modifier zone circle
        int modifierZone = m_stick->getModifierZone();
        int maxZone = m_stick->getMaxZone();
        penny.setWidth(0);
        penny.setColor(Qt::blue);
        painter.setOpacity(0.5);
        painter.setPen(penny);
        painter.setBrush(QBrush(Qt::yellow));

        if (m_stick->getModifierZoneInverted())
        {
            painter.drawEllipse(-modifierZone, -modifierZone, modifierZone * 2, modifierZone * 2);
        } else
        {
            QPainterPath modifierZonePath;
            modifierZonePath.addEllipse(QPoint(0, 0), maxZone, maxZone);
            modifierZonePath.addEllipse(QPoint(0, 0), modifierZone, modifierZone);
            painter.drawPath(modifierZonePath);
        }
    }

    // Draw deadzone circle
    penny.setWidth(0);
    penny.setColor(Qt::blue);
    painter.setPen(penny);
    painter.setBrush(QBrush(Qt::red));
    int deadZone = m_stick != nullptr ? m_stick->getDeadZone() : 0;
    painter.drawEllipse(-deadZone, -deadZone, deadZone * 2, deadZone * 2);

    painter.restore();

    painter.save();
    penny.setWidth(0);
    penny.setColor(Qt::black);
    painter.setOpacity(0.5);
    painter.setPen(penny);
    painter.scale(side / 2.0, side / 2.0);
    painter.translate(1, 1);
    // Draw Y line
    painter.drawLine(0, -1, 0, 1);
    // Draw X line
    painter.drawLine(-1, 0, 1, 0);
    painter.setOpacity(1.0);
    painter.restore();

    painter.save();
    painter.scale(side / static_cast<double>(GlobalVariables::JoyAxis::AXISMAX * 2.0),
                  side / static_cast<double>(GlobalVariables::JoyAxis::AXISMAX * 2.0));
    painter.translate(GlobalVariables::JoyAxis::AXISMAX, GlobalVariables::JoyAxis::AXISMAX);
    penny.setWidth(0);
    painter.setBrush(QBrush(Qt::black));
    penny.setColor(Qt::black);
    painter.setPen(penny);

    // Draw raw crosshair
    if (m_stick != nullptr)
    {
        int linexstart = m_stick->getXCoordinate() - 1000;
        int lineystart = m_stick->getYCoordinate() - 1000;

        if (linexstart < GlobalVariables::JoyAxis::AXISMIN)
            linexstart = GlobalVariables::JoyAxis::AXISMIN;

        if (lineystart < GlobalVariables::JoyAxis::AXISMIN)
            lineystart = GlobalVariables::JoyAxis::AXISMIN;

        painter.drawRect(linexstart, lineystart, 2000, 2000);

        painter.setBrush(QBrush(Qt::darkBlue));
        penny.setColor(Qt::darkBlue);
        painter.setPen(penny);

        // Draw adjusted crosshair
        linexstart = m_stick->getCircleXCoordinate() - 1000;
        lineystart = m_stick->getCircleYCoordinate() - 1000;
        if (linexstart < GlobalVariables::JoyAxis::AXISMIN)
            linexstart = GlobalVariables::JoyAxis::AXISMIN;

        if (lineystart < GlobalVariables::JoyAxis::AXISMIN)
            lineystart = GlobalVariables::JoyAxis::AXISMIN;

        painter.drawRect(linexstart, lineystart, 2000, 2000);
    }

    // Reset pen
    painter.restore();
    penny.setColor(Qt::black);
    painter.setPen(penny);

    // Draw primary pixmap
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    painter.setPen(Qt::NoPen);
    painter.fillRect(0, 0, side, side, palette().window().color());
    paint.drawPixmap(pix.rect(), pix);

    paint.save();
    paint.scale(side / static_cast<double>(GlobalVariables::JoyAxis::AXISMAX * 2.0),
                side / static_cast<double>(GlobalVariables::JoyAxis::AXISMAX * 2.0));
    paint.translate(GlobalVariables::JoyAxis::AXISMAX, GlobalVariables::JoyAxis::AXISMAX);

    // Draw max zone and initial inner clear circle
    int maxzone = m_stick != nullptr ? m_stick->getMaxZone() : GlobalVariables::JoyControlStick::DEFAULTMAXZONE;
    int diffmaxzone = GlobalVariables::JoyAxis::AXISMAX - maxzone;
    paint.setOpacity(0.5);
    paint.setBrush(Qt::darkGreen);
    paint.drawEllipse(-GlobalVariables::JoyAxis::AXISMAX, -GlobalVariables::JoyAxis::AXISMAX,
                      GlobalVariables::JoyAxis::AXISMAX * 2, GlobalVariables::JoyAxis::AXISMAX * 2);
    paint.setCompositionMode(QPainter::CompositionMode_Clear);
    paint.setPen(Qt::NoPen);
    paint.drawEllipse(-GlobalVariables::JoyAxis::AXISMAX + diffmaxzone, -GlobalVariables::JoyAxis::AXISMAX + diffmaxzone,
                      GlobalVariables::JoyAxis::AXISMAX * 2 - (diffmaxzone * 2),
                      GlobalVariables::JoyAxis::AXISMAX * 2 - (diffmaxzone * 2));

    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    paint.setOpacity(1.0);
    paint.restore();

    // Re-draw pixmap so the inner circle will be transparent
    paint.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    paint.drawPixmap(pix.rect(), pix);
    paint.setCompositionMode(QPainter::CompositionMode_SourceOver);
}
