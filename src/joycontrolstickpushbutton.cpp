#include <QDebug>
#include <QStyle>

#include "joycontrolstickpushbutton.h"

JoyControlStickPushButton::JoyControlStickPushButton(JoyControlStick *stick, QWidget *parent) :
    QPushButton(parent)
{
    this->stick = stick;

    isflashing = false;

    setText(stick->getName());

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
    setText(stick->getName());
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

