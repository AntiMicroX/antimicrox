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

#include "joycontrolstickstatusbox.h"

#include "messagehandler.h"
#include "joycontrolstick.h"
#include "joyaxis.h"
#include "common.h"

#include <qdrawutil.h>

#include <QDebug>
#include <QPainter>
#include <QSizePolicy>
#include <QList>
#include <QLinearGradient>
#include <QPaintEvent>


JoyControlStickStatusBox::JoyControlStickStatusBox(QWidget *parent) :
    QWidget(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->stick = nullptr;
}

JoyControlStickStatusBox::JoyControlStickStatusBox(JoyControlStick *stick, QWidget *parent) :
    QWidget(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->stick = stick;

    connect(stick, &JoyControlStick::deadZoneChanged, [this](int) { update(); });
    connect(stick, &JoyControlStick::moved, [this](int, int) { update(); });
    connect(stick, &JoyControlStick::diagonalRangeChanged, [this](int) { update(); });
    connect(stick, &JoyControlStick::maxZoneChanged, [this](int) { update(); });
    connect(stick, &JoyControlStick::joyModeChanged, this, [=]() { update(); });
    connect(stick, &JoyControlStick::circleAdjustChange, this, [this](double) { update(); });
}

void JoyControlStickStatusBox::setStick(JoyControlStick *stick)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (stick != nullptr)
    {
        disconnect(stick, &JoyControlStick::deadZoneChanged, this, 0);
        disconnect(stick, &JoyControlStick::moved, this, 0);
        disconnect(stick, &JoyControlStick::diagonalRangeChanged, this, 0);
        disconnect(stick, &JoyControlStick::maxZoneChanged, this, 0);
        disconnect(stick, &JoyControlStick::joyModeChanged, this, 0);
    }

    this->stick = stick;
    connect(stick, &JoyControlStick::deadZoneChanged, [this](int) { update(); });
    connect(stick, &JoyControlStick::moved, [this](int, int) { update(); });
    connect(stick, &JoyControlStick::diagonalRangeChanged, [this](int) { update(); });
    connect(stick, &JoyControlStick::maxZoneChanged, [this](int) { update(); });
    connect(stick, &JoyControlStick::joyModeChanged, this, [=]() { update(); });

    update();
}

JoyControlStick* JoyControlStickStatusBox::getStick() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return stick;
}

int JoyControlStickStatusBox::heightForWidth(int width) const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return width;
}

QSize JoyControlStickStatusBox::sizeHint() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return QSize(-1, -1);
}

void JoyControlStickStatusBox::paintEvent(QPaintEvent *event)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_UNUSED(event);

    PadderCommon::inputDaemonMutex.lock();

    if ((stick->getJoyMode() == JoyControlStick::StandardMode) ||
        (stick->getJoyMode() == JoyControlStick::EightWayMode))
    {
        drawEightWayBox();
    }
    else if (stick->getJoyMode() == JoyControlStick::FourWayCardinal)
    {
        drawFourWayCardinalBox();
    }
    else if (stick->getJoyMode() == JoyControlStick::FourWayDiagonal)
    {
        drawFourWayDiagonalBox();
    }

    PadderCommon::inputDaemonMutex.unlock();
}

void JoyControlStickStatusBox::drawEightWayBox()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QPainter paint (this);
    paint.setRenderHint(QPainter::Antialiasing, true);

    int side = qMin(width()-2, height()-2);

    QPixmap pix(side, side);
    pix.fill(Qt::transparent);
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Draw box outline
    QPen penny;
    penny.setColor(Qt::black);
    penny.setWidth(1);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(0, 0, side-1, side-1);

    painter.save();
    painter.scale(side / (double)(JoyAxis::AXISMAX*2.0), side / (double)(JoyAxis::AXISMAX*2.0));
    painter.translate(JoyAxis::AXISMAX, JoyAxis::AXISMAX);

    // Draw diagonal zones
    QList<double> anglesList = stick->getDiagonalZoneAngles();

    penny.setWidth(0);
    penny.setColor(Qt::black);
    painter.setPen(penny);
    painter.setBrush(QBrush(Qt::green));

    painter.drawPie(-JoyAxis::AXISMAX, -JoyAxis::AXISMAX, JoyAxis::AXISMAX*2, JoyAxis::AXISMAX*2, anglesList.value(2)*16, stick->getDiagonalRange()*16);
    painter.drawPie(-JoyAxis::AXISMAX, -JoyAxis::AXISMAX, JoyAxis::AXISMAX*2, JoyAxis::AXISMAX*2, anglesList.value(4)*16, stick->getDiagonalRange()*16);
    painter.drawPie(-JoyAxis::AXISMAX, -JoyAxis::AXISMAX, JoyAxis::AXISMAX*2, JoyAxis::AXISMAX*2, anglesList.value(6)*16, stick->getDiagonalRange()*16);
    painter.drawPie(-JoyAxis::AXISMAX, -JoyAxis::AXISMAX, JoyAxis::AXISMAX*2, JoyAxis::AXISMAX*2, anglesList.value(8)*16, stick->getDiagonalRange()*16);

    // Draw deadzone circle
    penny.setWidth(0);
    penny.setColor(Qt::blue);
    painter.setPen(penny);
    painter.setBrush(QBrush(Qt::red));
    painter.drawEllipse(-stick->getDeadZone(), -stick->getDeadZone(), stick->getDeadZone()*2, stick->getDeadZone()*2);

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
    painter.scale(side / (double)(JoyAxis::AXISMAX*2.0), side / (double)(JoyAxis::AXISMAX*2.0));
    painter.translate(JoyAxis::AXISMAX, JoyAxis::AXISMAX);

    penny.setWidth(0);
    painter.setBrush(QBrush(Qt::black));
    penny.setColor(Qt::black);
    painter.setPen(penny);

    // Draw raw crosshair
    int linexstart = stick->getXCoordinate()-1000;
    int lineystart = stick->getYCoordinate()-1000;

    if (linexstart < JoyAxis::AXISMIN)
    {
        linexstart = JoyAxis::AXISMIN;
    }

    if (lineystart < JoyAxis::AXISMIN)
    {
        lineystart = JoyAxis::AXISMIN;
    }

    painter.drawRect(linexstart, lineystart, 2000, 2000);

    painter.setBrush(QBrush(Qt::darkBlue));
    penny.setColor(Qt::darkBlue);
    painter.setPen(penny);

    // Draw adjusted crosshair
    linexstart = stick->getCircleXCoordinate()-1000;
    lineystart = stick->getCircleYCoordinate()-1000;
    if (linexstart < JoyAxis::AXISMIN)
    {
        linexstart = JoyAxis::AXISMIN;
    }

    if (lineystart < JoyAxis::AXISMIN)
    {
        lineystart = JoyAxis::AXISMIN;
    }

    painter.drawRect(linexstart, lineystart, 2000, 2000);
    painter.restore();

    // Reset pen
    penny.setColor(Qt::black);
    painter.setPen(penny);

    // Draw primary pixmap
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    painter.setPen(Qt::NoPen);
    painter.fillRect(0, 0, side, side, palette().background().color());
    paint.drawPixmap(pix.rect(), pix);

    paint.save();
    paint.scale(side / (double)(JoyAxis::AXISMAX*2.0), side / (double)(JoyAxis::AXISMAX*2.0));
    paint.translate(JoyAxis::AXISMAX, JoyAxis::AXISMAX);

    // Draw max zone and initial inner clear circle
    int maxzone = stick->getMaxZone();
    int diffmaxzone = JoyAxis::AXISMAX - maxzone;
    paint.setOpacity(0.5);
    paint.setBrush(Qt::darkGreen);
    paint.drawEllipse(-JoyAxis::AXISMAX, -JoyAxis::AXISMAX, JoyAxis::AXISMAX*2, JoyAxis::AXISMAX*2);
    paint.setCompositionMode(QPainter::CompositionMode_Clear);
    paint.setPen(Qt::NoPen);
    paint.drawEllipse(-JoyAxis::AXISMAX+diffmaxzone, -JoyAxis::AXISMAX+diffmaxzone, JoyAxis::AXISMAX*2-(diffmaxzone*2), JoyAxis::AXISMAX*2-(diffmaxzone*2));

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QPainter paint(this);
    paint.setRenderHint(QPainter::Antialiasing, true);

    int side = qMin(width()-2, height()-2);

    QPixmap pix(side, side);
    pix.fill(Qt::transparent);
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Draw box outline
    QPen penny;
    penny.setColor(Qt::black);
    penny.setWidth(1);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(0, 0, side-1, side-1);

    painter.save();
    painter.scale(side / (double)(JoyAxis::AXISMAX*2.0), side / (double)(JoyAxis::AXISMAX*2.0));
    painter.translate(JoyAxis::AXISMAX, JoyAxis::AXISMAX);

    // Draw diagonal zones
    QList<int> anglesList = stick->getFourWayCardinalZoneAngles();
    penny.setWidth(0);
    penny.setColor(Qt::black);
    painter.setPen(penny);
    painter.setOpacity(0.25);
    painter.setBrush(QBrush(Qt::black));

    painter.drawPie(-JoyAxis::AXISMAX, -JoyAxis::AXISMAX, JoyAxis::AXISMAX*2, JoyAxis::AXISMAX*2, anglesList.value(1)*16, 90*16);
    painter.drawPie(-JoyAxis::AXISMAX, -JoyAxis::AXISMAX, JoyAxis::AXISMAX*2, JoyAxis::AXISMAX*2, anglesList.value(3)*16, 90*16);

    painter.setOpacity(1.0);

    // Draw deadzone circle
    penny.setWidth(0);
    penny.setColor(Qt::blue);
    painter.setPen(penny);
    painter.setBrush(QBrush(Qt::red));
    painter.drawEllipse(-stick->getDeadZone(), -stick->getDeadZone(), stick->getDeadZone()*2, stick->getDeadZone()*2);

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
    painter.scale(side / (double)(JoyAxis::AXISMAX*2.0), side / (double)(JoyAxis::AXISMAX*2.0));
    painter.translate(JoyAxis::AXISMAX, JoyAxis::AXISMAX);
    penny.setWidth(0);
    painter.setBrush(QBrush(Qt::black));
    penny.setColor(Qt::black);
    painter.setPen(penny);

    // Draw raw crosshair
    int linexstart = stick->getXCoordinate()-1000;
    int lineystart = stick->getYCoordinate()-1000;

    if (linexstart < JoyAxis::AXISMIN)
    {
        linexstart = JoyAxis::AXISMIN;
    }

    if (lineystart < JoyAxis::AXISMIN)
    {
        lineystart = JoyAxis::AXISMIN;
    }

    painter.drawRect(linexstart, lineystart, 2000, 2000);

    painter.setBrush(QBrush(Qt::darkBlue));
    penny.setColor(Qt::darkBlue);
    painter.setPen(penny);

    // Draw adjusted crosshair
    linexstart = stick->getCircleXCoordinate()-1000;
    lineystart = stick->getCircleYCoordinate()-1000;
    if (linexstart < JoyAxis::AXISMIN)
    {
        linexstart = JoyAxis::AXISMIN;
    }

    if (lineystart < JoyAxis::AXISMIN)
    {
        lineystart = JoyAxis::AXISMIN;
    }

    painter.drawRect(linexstart, lineystart, 2000, 2000);
    painter.restore();

    // Reset pen
    penny.setColor(Qt::black);
    painter.setPen(penny);

    // Draw primary pixmap
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    painter.setPen(Qt::NoPen);
    painter.fillRect(0, 0, side, side, palette().background().color());
    paint.drawPixmap(pix.rect(), pix);

    paint.save();
    paint.scale(side / (double)(JoyAxis::AXISMAX*2.0), side / (double)(JoyAxis::AXISMAX*2.0));
    paint.translate(JoyAxis::AXISMAX, JoyAxis::AXISMAX);

    // Draw max zone and initial inner clear circle
    int maxzone = stick->getMaxZone();
    int diffmaxzone = JoyAxis::AXISMAX - maxzone;
    paint.setOpacity(0.5);
    paint.setBrush(Qt::darkGreen);
    paint.drawEllipse(-JoyAxis::AXISMAX, -JoyAxis::AXISMAX, JoyAxis::AXISMAX*2, JoyAxis::AXISMAX*2);
    paint.setCompositionMode(QPainter::CompositionMode_Clear);
    paint.setPen(Qt::NoPen);
    paint.drawEllipse(-JoyAxis::AXISMAX+diffmaxzone, -JoyAxis::AXISMAX+diffmaxzone, JoyAxis::AXISMAX*2-(diffmaxzone*2), JoyAxis::AXISMAX*2-(diffmaxzone*2));

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QPainter paint(this);
    paint.setRenderHint(QPainter::Antialiasing, true);

    int side = qMin(width()-2, height()-2);

    QPixmap pix(side, side);
    pix.fill(Qt::transparent);
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Draw box outline
    QPen penny;
    penny.setColor(Qt::black);
    penny.setWidth(1);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(0, 0, side-1, side-1);

    painter.save();
    painter.scale(side / (double)(JoyAxis::AXISMAX*2.0), side / (double)(JoyAxis::AXISMAX*2.0));
    painter.translate(JoyAxis::AXISMAX, JoyAxis::AXISMAX);

    // Draw diagonal zones
    QList<int> anglesList = stick->getFourWayDiagonalZoneAngles();
    penny.setWidth(0);
    penny.setColor(Qt::black);
    painter.setPen(penny);
    painter.setBrush(QBrush(Qt::black));
    painter.setOpacity(0.25);

    painter.drawPie(-JoyAxis::AXISMAX, -JoyAxis::AXISMAX, JoyAxis::AXISMAX*2, JoyAxis::AXISMAX*2, anglesList.value(1)*16, 90*16);
    painter.drawPie(-JoyAxis::AXISMAX, -JoyAxis::AXISMAX, JoyAxis::AXISMAX*2, JoyAxis::AXISMAX*2, anglesList.value(3)*16, 90*16);

    painter.setOpacity(1.0);

    // Draw deadzone circle
    penny.setWidth(0);
    penny.setColor(Qt::blue);
    painter.setPen(penny);
    painter.setBrush(QBrush(Qt::red));
    painter.drawEllipse(-stick->getDeadZone(), -stick->getDeadZone(), stick->getDeadZone()*2, stick->getDeadZone()*2);

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
    painter.scale(side / (double)(JoyAxis::AXISMAX*2.0), side / (double)(JoyAxis::AXISMAX*2.0));
    painter.translate(JoyAxis::AXISMAX, JoyAxis::AXISMAX);
    penny.setWidth(0);
    painter.setBrush(QBrush(Qt::black));
    penny.setColor(Qt::black);
    painter.setPen(penny);

    // Draw raw crosshair
    int linexstart = stick->getXCoordinate()-1000;
    int lineystart = stick->getYCoordinate()-1000;

    if (linexstart < JoyAxis::AXISMIN)
    {
        linexstart = JoyAxis::AXISMIN;
    }

    if (lineystart < JoyAxis::AXISMIN)
    {
        lineystart = JoyAxis::AXISMIN;
    }

    painter.drawRect(linexstart, lineystart, 2000, 2000);

    painter.setBrush(QBrush(Qt::darkBlue));
    penny.setColor(Qt::darkBlue);
    painter.setPen(penny);

    // Draw adjusted crosshair
    linexstart = stick->getCircleXCoordinate()-1000;
    lineystart = stick->getCircleYCoordinate()-1000;
    if (linexstart < JoyAxis::AXISMIN)
    {
        linexstart = JoyAxis::AXISMIN;
    }

    if (lineystart < JoyAxis::AXISMIN)
    {
        lineystart = JoyAxis::AXISMIN;
    }

    painter.drawRect(linexstart, lineystart, 2000, 2000);
    painter.restore();

    // Reset pen
    penny.setColor(Qt::black);
    painter.setPen(penny);

    // Draw primary pixmap
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    painter.setPen(Qt::NoPen);
    painter.fillRect(0, 0, side, side, palette().background().color());
    paint.drawPixmap(pix.rect(), pix);

    paint.save();
    paint.scale(side / (double)(JoyAxis::AXISMAX*2.0), side / (double)(JoyAxis::AXISMAX*2.0));
    paint.translate(JoyAxis::AXISMAX, JoyAxis::AXISMAX);

    // Draw max zone and initial inner clear circle
    int maxzone = stick->getMaxZone();
    int diffmaxzone = JoyAxis::AXISMAX - maxzone;
    paint.setOpacity(0.5);
    paint.setBrush(Qt::darkGreen);
    paint.drawEllipse(-JoyAxis::AXISMAX, -JoyAxis::AXISMAX, JoyAxis::AXISMAX*2, JoyAxis::AXISMAX*2);
    paint.setCompositionMode(QPainter::CompositionMode_Clear);
    paint.setPen(Qt::NoPen);
    paint.drawEllipse(-JoyAxis::AXISMAX+diffmaxzone, -JoyAxis::AXISMAX+diffmaxzone, JoyAxis::AXISMAX*2-(diffmaxzone*2), JoyAxis::AXISMAX*2-(diffmaxzone*2));

    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    paint.setOpacity(1.0);
    paint.restore();

    // Re-draw pixmap so the inner circle will be transparent
    paint.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    paint.drawPixmap(pix.rect(), pix);
    paint.setCompositionMode(QPainter::CompositionMode_SourceOver);
}
