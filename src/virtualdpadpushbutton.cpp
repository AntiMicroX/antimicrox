#include <QDebug>
#include <QStyle>

#include "virtualdpadpushbutton.h"

VirtualDPadPushButton::VirtualDPadPushButton(VDPad *vdpad, QWidget *parent) :
    FlashButtonWidget(parent)
{
    this->vdpad = vdpad;

    isflashing = false;
    displayNames = false;
    leftAlignText = false;

    refreshLabel();

    connect(vdpad, SIGNAL(active(int)), this, SLOT(flash()));
    connect(vdpad, SIGNAL(released(int)), this, SLOT(unflash()));
    connect(vdpad, SIGNAL(dpadNameChanged()), this, SLOT(refreshLabel()));
}

VDPad* VirtualDPadPushButton::getVDPad()
{
    return vdpad;
}

QString VirtualDPadPushButton::generateLabel()
{
    QString temp;

    if (!vdpad->getDpadName().isEmpty())
    {
        temp.append(vdpad->getName(false, displayNames));
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
