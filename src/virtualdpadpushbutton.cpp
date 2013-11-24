#include <QDebug>
#include <QStyle>
#include <QFontMetrics>
#include <QPainter>

#include "virtualdpadpushbutton.h"

VirtualDPadPushButton::VirtualDPadPushButton(VDPad *vdpad, QWidget *parent) :
    QPushButton(parent)
{
    this->vdpad = vdpad;

    isflashing = false;
    refreshLabel();

    connect(vdpad, SIGNAL(active(int)), this, SLOT(flash()));
    connect(vdpad, SIGNAL(released(int)), this, SLOT(unflash()));
    connect(vdpad, SIGNAL(dpadNameChanged()), this, SLOT(refreshLabel()));
}

VDPad* VirtualDPadPushButton::getVDPad()
{
    return vdpad;
}

void VirtualDPadPushButton::flash()
{
    isflashing = true;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void VirtualDPadPushButton::unflash()
{
    isflashing = false;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void VirtualDPadPushButton::refreshLabel()
{
    setText(generateLabel());
}

QString VirtualDPadPushButton::generateLabel()
{
    QString temp;

    if (!vdpad->getDpadName().isEmpty())
    {
        temp.append(vdpad->getName());
    }
    else
    {
        temp.append(vdpad->getName(true));
    }

    return temp;
}

void VirtualDPadPushButton::disableFlashes()
{
    disconnect(vdpad, SIGNAL(active(int)), this, 0);
    disconnect(vdpad, SIGNAL(released(int)), this, 0);
    this->unflash();
}

void VirtualDPadPushButton::enableFlashes()
{
    connect(vdpad, SIGNAL(active(int)), this, SLOT(flash()));
    connect(vdpad, SIGNAL(released(int)), this, SLOT(unflash()));
}

bool VirtualDPadPushButton::isButtonFlashing()
{
    return isflashing;
}

void VirtualDPadPushButton::paintEvent(QPaintEvent *event)
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
