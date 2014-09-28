#include <QDebug>
#include <QPainter>
#include <qdrawutil.h>
#include <QSizePolicy>
#include <QList>
#include <QLinearGradient>

#include "joycontrolstickstatusbox.h"

JoyControlStickStatusBox::JoyControlStickStatusBox(QWidget *parent) :
    QWidget(parent)
{
    this->stick = 0;
}

JoyControlStickStatusBox::JoyControlStickStatusBox(JoyControlStick *stick, QWidget *parent) :
    QWidget(parent)
{
    this->stick = stick;

    connect(stick, SIGNAL(deadZoneChanged(int)), this, SLOT(update()));
    connect(stick, SIGNAL(moved(int,int)), this, SLOT(update()));
    connect(stick, SIGNAL(diagonalRangeChanged(int)), this, SLOT(update()));
    connect(stick, SIGNAL(maxZoneChanged(int)), this, SLOT(update()));
    connect(stick, SIGNAL(joyModeChanged()), this, SLOT(update()));
    connect(stick, SIGNAL(circleAdjustChange(double)), this, SLOT(update()));
}

void JoyControlStickStatusBox::setStick(JoyControlStick *stick)
{
    if (stick)
    {
        disconnect(stick, SIGNAL(deadZoneChanged(int)), this, 0);
        disconnect(stick, SIGNAL(moved(int,int)), this, 0);
        disconnect(stick, SIGNAL(diagonalRangeChanged(int)), this, 0);
        disconnect(stick, SIGNAL(maxZoneChanged(int)), this, 0);
        disconnect(stick, SIGNAL(joyModeChanged()), this, 0);
    }

    this->stick = stick;
    connect(stick, SIGNAL(deadZoneChanged(int)), this, SLOT(update()));
    connect(stick, SIGNAL(moved(int,int)), this, SLOT(update()));
    connect(stick, SIGNAL(diagonalRangeChanged(int)), this, SLOT(update()));
    connect(stick, SIGNAL(maxZoneChanged(int)), this, SLOT(update()));
    connect(stick, SIGNAL(joyModeChanged()), this, SLOT(update()));
}

JoyControlStick* JoyControlStickStatusBox::getStick()
{
    return stick;
}

int JoyControlStickStatusBox::heightForWidth(int width) const
{
    return width;
}

QSize JoyControlStickStatusBox::sizeHint() const
{
    return QSize(-1, -1);
}

void JoyControlStickStatusBox::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if (stick->getJoyMode() == JoyControlStick::StandardMode ||
        stick->getJoyMode() == JoyControlStick::EightWayMode)
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

    /*QPainter paint (this);
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
    QList<int> anglesList = stick->getDiagonalZoneAngles();
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
    penny.setColor(Qt::black);
    painter.setPen(penny);

    // Draw crosshair
    int linexstart = stick->getXCoordinate()-2000, linexend = stick->getXCoordinate()+2000;
    int lineystart = stick->getYCoordinate()-2000, lineyend = stick->getYCoordinate()+2000;
    if (linexstart < JoyAxis::AXISMIN)
    {
        linexstart = JoyAxis::AXISMIN;
    }
    if (linexend > JoyAxis::AXISMAX)
    {
        linexend = JoyAxis::AXISMAX;
    }
    if (lineystart < JoyAxis::AXISMIN)
    {
        lineystart = JoyAxis::AXISMIN;
    }
    if (lineyend > JoyAxis::AXISMAX)
    {
        lineyend = JoyAxis::AXISMAX;
    }
    painter.drawLine(linexstart, stick->getYCoordinate(), linexend, stick->getYCoordinate());
    painter.drawLine(stick->getXCoordinate(), lineystart, stick->getXCoordinate(), lineyend);

    painter.restore();

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
    paint.setCompositionMode(QPainter::CompositionMode_SourceOver);*/
}

void JoyControlStickStatusBox::drawEightWayBox()
{
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
    QList<int> anglesList = stick->getDiagonalZoneAngles();
    QListIterator<int> iter(anglesList);
    //qDebug() << "LIST START";
    //qDebug() << "DIAGONAL RANGE: " << stick->getDiagonalRange();
    //while (iter.hasNext())
    //{
    //    qDebug() << "ANGLE: " << iter.next();
    //}
    //qDebug() << "LIST END";
    //qDebug();

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
    penny.setColor(Qt::black);
    painter.setPen(penny);

    // Draw crosshair
    int linexstart = stick->getXCoordinate()-2000, linexend = stick->getXCoordinate()+2000;
    int lineystart = stick->getYCoordinate()-2000, lineyend = stick->getYCoordinate()+2000;
    if (linexstart < JoyAxis::AXISMIN)
    {
        linexstart = JoyAxis::AXISMIN;
    }
    if (linexend > JoyAxis::AXISMAX)
    {
        linexend = JoyAxis::AXISMAX;
    }
    if (lineystart < JoyAxis::AXISMIN)
    {
        lineystart = JoyAxis::AXISMIN;
    }
    if (lineyend > JoyAxis::AXISMAX)
    {
        lineyend = JoyAxis::AXISMAX;
    }
    painter.drawLine(linexstart, stick->getYCoordinate(), linexend, stick->getYCoordinate());
    painter.drawLine(stick->getXCoordinate(), lineystart, stick->getXCoordinate(), lineyend);

    painter.restore();

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
    penny.setColor(Qt::black);
    painter.setPen(penny);

    // Draw crosshair
    int linexstart = stick->getXCoordinate()-2000, linexend = stick->getXCoordinate()+2000;
    int lineystart = stick->getYCoordinate()-2000, lineyend = stick->getYCoordinate()+2000;
    if (linexstart < JoyAxis::AXISMIN)
    {
        linexstart = JoyAxis::AXISMIN;
    }
    if (linexend > JoyAxis::AXISMAX)
    {
        linexend = JoyAxis::AXISMAX;
    }
    if (lineystart < JoyAxis::AXISMIN)
    {
        lineystart = JoyAxis::AXISMIN;
    }
    if (lineyend > JoyAxis::AXISMAX)
    {
        lineyend = JoyAxis::AXISMAX;
    }
    painter.drawLine(linexstart, stick->getYCoordinate(), linexend, stick->getYCoordinate());
    painter.drawLine(stick->getXCoordinate(), lineystart, stick->getXCoordinate(), lineyend);

    painter.restore();

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
    penny.setColor(Qt::black);
    painter.setPen(penny);

    // Draw crosshair
    int linexstart = stick->getXCoordinate()-2000, linexend = stick->getXCoordinate()+2000;
    int lineystart = stick->getYCoordinate()-2000, lineyend = stick->getYCoordinate()+2000;
    if (linexstart < JoyAxis::AXISMIN)
    {
        linexstart = JoyAxis::AXISMIN;
    }
    if (linexend > JoyAxis::AXISMAX)
    {
        linexend = JoyAxis::AXISMAX;
    }
    if (lineystart < JoyAxis::AXISMIN)
    {
        lineystart = JoyAxis::AXISMIN;
    }
    if (lineyend > JoyAxis::AXISMAX)
    {
        lineyend = JoyAxis::AXISMAX;
    }
    painter.drawLine(linexstart, stick->getYCoordinate(), linexend, stick->getYCoordinate());
    painter.drawLine(stick->getXCoordinate(), lineystart, stick->getXCoordinate(), lineyend);

    painter.restore();

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
