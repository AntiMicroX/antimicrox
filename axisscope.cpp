#include <QPainter>

#include "axisscope.h"

AxisScope::AxisScope(JoyAxis *axis, QWidget *parent) :
    QWidget(parent)
{
    this->axis = axis;
}

void AxisScope::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawRect(20, 30, 50, 50);

    painter.drawEllipse(60, 60, 60, 60);
    painter.drawLine(10, 10, 50, 10);
    painter.drawLine(5, 5, 20, 20);

    QBrush dude();
    dude.setColor(Qt::red);
    painter.setBrush(dude);
    painter.setOpacity(0.5);
    painter.drawEllipse(60, 60, 60, 60);
    painter.setOpacity(1.0);


}
