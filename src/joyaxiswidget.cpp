#include <QDebug>
#include <QStyle>
#include <QFontMetrics>
#include <QPainter>

#include "joyaxiswidget.h"

JoyAxisWidget::JoyAxisWidget(JoyAxis *axis, QWidget *parent) :
    QPushButton(parent)
{
    this->axis = axis;

    isflashing = false;

    setText(generateLabel());

    connect(axis, SIGNAL(active(int)), this, SLOT(flash()));
    connect(axis, SIGNAL(released(int)), this, SLOT(unflash()));
}

JoyAxis* JoyAxisWidget::getAxis()
{
    return axis;
}

void JoyAxisWidget::flash()
{
    isflashing = true;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void JoyAxisWidget::unflash()
{
    isflashing = false;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void JoyAxisWidget::refreshLabel()
{
    setText(generateLabel());
}

void JoyAxisWidget::disableFlashes()
{
    disconnect(axis, SIGNAL(active(int)), 0, 0);
    disconnect(axis, SIGNAL(released(int)), 0, 0);
    this->unflash();
}

void JoyAxisWidget::enableFlashes()
{
    connect(axis, SIGNAL(active(int)), this, SLOT(flash()));
    connect(axis, SIGNAL(released(int)), this, SLOT(unflash()));
}

bool JoyAxisWidget::isButtonFlashing()
{
    return isflashing;
}

QString JoyAxisWidget::generateLabel()
{
    QString temp;
    temp = axis->getName().replace("&", "&&");
    return temp;
}

void JoyAxisWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    QFontMetrics fm = this->fontMetrics();
    //QString temp = stick->getName();
    QFont tempWidgetFont = this->font();
    //QFontMetrics fm(this->font());
    //QString temp = fm.elidedText(stick->getName(), Qt::ElideRight, this->width());
    //this->setText(temp);
    //qDebug() << "FM WIDTH B4: " << fm.width(stick->getName()) << " " << text();
    QFont tempScaledFont = painter.font();

    while ((this->width() < fm.width(text())) && tempScaledFont.pointSize() >= 6)
    {
        tempScaledFont.setPointSize(painter.font().pointSize()-2);
        painter.setFont(tempScaledFont);
        fm = painter.fontMetrics();
        //qDebug() << "TEMP SIZE: " << tempScaledFont.pointSize() << endl;
    }
    //qDebug() << "FM WIDTH NOW: " << fm.width(stick->getName()) << " " << text();

    this->setFont(tempScaledFont);
    QPushButton::paintEvent(event);
    this->setFont(tempWidgetFont);
}
