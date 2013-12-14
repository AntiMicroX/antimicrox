#include <QDebug>
#include <QStyle>

#include "joycontrolstickpushbutton.h"

JoyControlStickPushButton::JoyControlStickPushButton(JoyControlStick *stick, QWidget *parent) :
    FlashButtonWidget(parent)
{
    this->stick = stick;

    refreshLabel();

    connect(stick, SIGNAL(active(int, int)), this, SLOT(flash()));
    connect(stick, SIGNAL(released(int, int)), this, SLOT(unflash()));
    connect(stick, SIGNAL(stickNameChanged()), this, SLOT(refreshLabel()));
}

JoyControlStick* JoyControlStickPushButton::getStick()
{
    return stick;
}

QString JoyControlStickPushButton::generateLabel()
{
    QString temp;
    if (!stick->getStickName().isEmpty() && displayNames)
    {
        temp.append(stick->getStickName());
    }
    else
    {
        temp.append(tr("Stick")).append(" ").append(QString::number(stick->getRealJoyIndex()));
    }

    return temp;
}

void JoyControlStickPushButton::disableFlashes()
{
    disconnect(stick, SIGNAL(active(int, int)), this, 0);
    disconnect(stick, SIGNAL(released(int, int)), this, 0);
    this->unflash();
}

void JoyControlStickPushButton::enableFlashes()
{
    connect(stick, SIGNAL(active(int, int)), this, SLOT(flash()));
    connect(stick, SIGNAL(released(int, int)), this, SLOT(unflash()));
}
