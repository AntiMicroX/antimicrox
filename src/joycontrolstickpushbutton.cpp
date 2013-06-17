#include <QDebug>
#include <QStyle>
#include <QFontMetrics>
#include <QPainter>

#include "joycontrolstickpushbutton.h"

JoyControlStickPushButton::JoyControlStickPushButton(JoyControlStick *stick, QWidget *parent) :
    QPushButton(parent)
{
    this->stick = stick;

    isflashing = false;

    refreshLabel();

    connect(stick, SIGNAL(active(int, int)), this, SLOT(flash()));
    connect(stick, SIGNAL(released(int, int)), this, SLOT(unflash()));
}

JoyControlStick* JoyControlStickPushButton::getStick()
{
    return stick;
}

void JoyControlStickPushButton::flash()
{
    isflashing = true;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void JoyControlStickPushButton::unflash()
{
    isflashing = false;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void JoyControlStickPushButton::refreshLabel()
{
    setText(stick->getName().replace("&", "&&"));
}

void JoyControlStickPushButton::disableFlashes()
{
    disconnect(stick, SIGNAL(active(int, int)), 0, 0);
    disconnect(stick, SIGNAL(released(int, int)), 0, 0);
    this->unflash();
}

void JoyControlStickPushButton::enableFlashes()
{
    connect(stick, SIGNAL(active(int, int)), this, SLOT(flash()));
    connect(stick, SIGNAL(released(int, int)), this, SLOT(unflash()));
}

bool JoyControlStickPushButton::isButtonFlashing()
{
    return isflashing;
}

void JoyControlStickPushButton::paintEvent(QPaintEvent *event)
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

    while ((this->width() < fm.width(stick->getName())) && tempScaledFont.pointSize() >= 6)
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
