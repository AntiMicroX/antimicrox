#include "dpadpushbutton.h"

DPadPushButton::DPadPushButton(JoyDPad *dpad, bool displayNames, QWidget *parent) :
    FlashButtonWidget(displayNames, parent)
{
    this->dpad = dpad;

    refreshLabel();
    enableFlashes();
    connect(dpad, SIGNAL(dpadNameChanged()), this, SLOT(refreshLabel()));
}

JoyDPad* DPadPushButton::getDPad()
{
    return dpad;
}

QString DPadPushButton::generateLabel()
{
    QString temp;
    if (!dpad->getDpadName().isEmpty())
    {
        temp.append(dpad->getName(false, displayNames));
    }
    else
    {
        temp.append(dpad->getName());
    }

    return temp;
}

void DPadPushButton::disableFlashes()
{
    disconnect(dpad, SIGNAL(active(int)), this, SLOT(flash()));
    disconnect(dpad, SIGNAL(released(int)), this, SLOT(unflash()));
    this->unflash();
}

void DPadPushButton::enableFlashes()
{
    connect(dpad, SIGNAL(active(int)), this, SLOT(flash()));
    connect(dpad, SIGNAL(released(int)), this, SLOT(unflash()));
}
