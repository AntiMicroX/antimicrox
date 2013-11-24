#include <QDebug>
#include <QStyle>
#include <QFontMetrics>
#include <QPainter>

#include "dpadpushbutton.h"

DPadPushButton::DPadPushButton(JoyDPad *dpad, QWidget *parent) :
    QPushButton(parent)
{
    this->dpad = dpad;

    isflashing = false;
    refreshLabel();
    enableFlashes();
    connect(dpad, SIGNAL(dpadNameChanged()), this, SLOT(refreshLabel()));
}

JoyDPad* DPadPushButton::getDPad()
{
    return dpad;
}

void DPadPushButton::flash()
{
    isflashing = true;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void DPadPushButton::unflash()
{
    isflashing = false;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void DPadPushButton::refreshLabel()
{
    setText(generateLabel());
}

QString DPadPushButton::generateLabel()
{
    QString temp;
    if (!dpad->getDpadName().isEmpty())
    {
        temp.append(dpad->getName());
    }
    else
    {
        temp.append(dpad->getName(true));
    }

    return temp;
}

void DPadPushButton::disableFlashes()
{
    disconnect(dpad, SIGNAL(active(int)), this, 0);
    disconnect(dpad, SIGNAL(released(int)), this, 0);
    this->unflash();
}

void DPadPushButton::enableFlashes()
{
    connect(dpad, SIGNAL(active(int)), this, SLOT(flash()));
    connect(dpad, SIGNAL(released(int)), this, SLOT(unflash()));
}

bool DPadPushButton::isButtonFlashing()
{
    return isflashing;
}

void DPadPushButton::paintEvent(QPaintEvent *event)
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
