#include <QDebug>
#include <QStyle>

#include "joycontrolstickbuttonpushbutton.h"

JoyControlStickButtonPushButton::JoyControlStickButtonPushButton(QWidget *parent) :
    QPushButton(parent)
{
    this->button = 0;
    this->isflashing = false;
}

JoyControlStickButtonPushButton::JoyControlStickButtonPushButton(JoyControlStickButton *button, QWidget *parent) :
    QPushButton(parent)
{
    this->button = button;
    this->isflashing = false;

    refreshLabel();
    enableFlashes();
    connect(button, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
}

JoyControlStickButton* JoyControlStickButtonPushButton::getButton()
{
    return button;
}

void JoyControlStickButtonPushButton::JoyControlStickButtonPushButton::setButton(JoyControlStickButton *button)
{
    disableFlashes();
    if (this->button)
    {
        disconnect(this->button, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
    }

    this->button = button;
    refreshLabel();
    enableFlashes();
    connect(button, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
}

bool JoyControlStickButtonPushButton::isButtonFlashing()
{
    return isflashing;
}

void JoyControlStickButtonPushButton::flash()
{
    isflashing = true;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void JoyControlStickButtonPushButton::unflash()
{
    isflashing = false;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void JoyControlStickButtonPushButton::refreshLabel()
{
    if (button)
    {
        setText(button->getSlotsSummary().replace("&", "&&"));
    }
}

void JoyControlStickButtonPushButton::disableFlashes()
{
    if (button)
    {
        disconnect(button, SIGNAL(clicked(int)), this, 0);
        disconnect(button, SIGNAL(released(int)), this, 0);
    }
    this->unflash();
}

void JoyControlStickButtonPushButton::enableFlashes()
{
    if (button)
    {
        connect(button, SIGNAL(clicked(int)), this, SLOT(flash()));
        connect(button, SIGNAL(released(int)), this, SLOT(unflash()));
    }
}
