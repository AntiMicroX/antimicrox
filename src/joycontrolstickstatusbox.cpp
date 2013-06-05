#include <QDebug>
#include <QPainter>
#include <qdrawutil.h>
#include <QSizePolicy>
#include <QList>

#include "joycontrolstickstatusbox.h"

JoyControlStickStatusBox::JoyControlStickStatusBox(QWidget *parent) :
    QWidget(parent)
{
    this->stick = 0;
    xCoor = 0;
    yCoor = 0;

    /*QSizePolicy sizePolicy1;
    sizePolicy1.setHorizontalPolicy(QSizePolicy::Preferred);
    sizePolicy1.setVerticalPolicy(QSizePolicy::Preferred);
    sizePolicy1.setHeightForWidth(true);
    this->setSizePolicy(sizePolicy1);*/
}

JoyControlStickStatusBox::JoyControlStickStatusBox(JoyControlStick *stick, QWidget *parent) :
    QWidget(parent)
{
    this->stick = stick;
    xCoor = stick->getXCoordinate();
    yCoor = stick->getYCoordinate();

    connect(stick, SIGNAL(deadZoneChanged(int)), this, SLOT(update()));
    connect(stick, SIGNAL(moved(int,int)), this, SLOT(update()));
    connect(stick, SIGNAL(diagonalRangeChanged(int)), this, SLOT(update()));
}

void JoyControlStickStatusBox::setStick(JoyControlStick *stick)
{
    if (stick)
    {
        disconnect(stick, SIGNAL(deadZoneChanged(int)), this, 0);
        disconnect(stick, SIGNAL(moved(int,int)), this, 0);
        disconnect(stick, SIGNAL(diagonalRangeChanged(int)), this, 0);
    }

    this->stick = stick;
    connect(stick, SIGNAL(deadZoneChanged(int)), this, SLOT(update()));
    connect(stick, SIGNAL(moved(int,int)), this, SLOT(update()));
    connect(stick, SIGNAL(diagonalRangeChanged(int)), this, SLOT(update()));
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

    QPainter paint (this);

    int side = qMin(width()-2, height()-2);

    //qDebug() << "WIDTH: " << width() << endl;
    //qDebug() << "HEIGHT: " << height() << endl;
    //qDebug() << "SIDE: " << side << endl;
    paint.save();
    paint.setPen(Qt::gray);
    paint.scale(side / 2.0, side / 2.0);
    paint.translate(1, 1);
    // Y line
    paint.drawLine(0, -1, 0, 1);
    // X line
    paint.drawLine(-1, 0, 1, 0);

    paint.restore();

    paint.save();
    //paint.translate(-JoyAxis::AXISMAX, -JoyAxis::AXISMAX);
    paint.scale(side / (double)(JoyAxis::AXISMAX*2.0), side / (double)(JoyAxis::AXISMAX*2.0));
    paint.translate(JoyAxis::AXISMAX, JoyAxis::AXISMAX);
    paint.drawRect(-JoyAxis::AXISMAX, -JoyAxis::AXISMAX, (JoyAxis::AXISMAX*2.0), (JoyAxis::AXISMAX*2.0));

    // Draw diagonal zones
    paint.setPen(Qt::blue);
    paint.setBrush(QBrush(Qt::green));
    QList<int> anglesList = stick->getDiagonalZoneAngles();
    paint.drawPie(-JoyAxis::AXISMAX, -JoyAxis::AXISMAX, JoyAxis::AXISMAX*2, JoyAxis::AXISMAX*2, anglesList.value(2)*16, stick->getDiagonalRange()*16);
    paint.drawPie(-JoyAxis::AXISMAX, -JoyAxis::AXISMAX, JoyAxis::AXISMAX*2, JoyAxis::AXISMAX*2, anglesList.value(4)*16, stick->getDiagonalRange()*16);
    paint.drawPie(-JoyAxis::AXISMAX, -JoyAxis::AXISMAX, JoyAxis::AXISMAX*2, JoyAxis::AXISMAX*2, anglesList.value(6)*16, stick->getDiagonalRange()*16);
    paint.drawPie(-JoyAxis::AXISMAX, -JoyAxis::AXISMAX, JoyAxis::AXISMAX*2, JoyAxis::AXISMAX*2, anglesList.value(8)*16, stick->getDiagonalRange()*16);

    // Draw deadzone circle
    paint.setPen(Qt::blue);
    paint.setBrush(QBrush(Qt::red));
    paint.drawEllipse(-stick->getDeadZone(), -stick->getDeadZone(), stick->getDeadZone()*2, stick->getDeadZone()*2);

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
    paint.drawLine(linexstart, stick->getYCoordinate(), linexend, stick->getYCoordinate());
    paint.drawLine(stick->getXCoordinate(), lineystart, stick->getXCoordinate(), lineyend);

    paint.restore();
}
