#include <QDebug>
#include <QStyle>

#include "joycontrolstickbuttonpushbutton.h"

JoyControlStickButtonPushButton::JoyControlStickButtonPushButton(JoyControlStickButton *button, QWidget *parent) :
    FlashButtonWidget(parent)
{
    this->button = button;

    refreshLabel();
    enableFlashes();
    connect(button, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
    connect(button, SIGNAL(actionNameChanged()), this, SLOT(refreshLabel()));
}

JoyControlStickButton* JoyControlStickButtonPushButton::getButton()
{
    return button;
}

void JoyControlStickButtonPushButton::setButton(JoyControlStickButton *button)
{
    disableFlashes();
    if (this->button)
    {
        disconnect(this->button, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
        disconnect(this->button, SIGNAL(actionNameChanged()), this, SLOT(refreshLabel()));
    }

    this->button = button;
    refreshLabel();
    enableFlashes();
    connect(button, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
    connect(button, SIGNAL(actionNameChanged()), this, SLOT(refreshLabel()));
}


void JoyControlStickButtonPushButton::disableFlashes()
{
    if (button)
    {
        disconnect(button, SIGNAL(clicked(int)), this, SLOT(flash()));
        disconnect(button, SIGNAL(released(int)), this, SLOT(unflash()));
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

QString JoyControlStickButtonPushButton::generateLabel()
{
    QString temp;
    if (button)
    {
        if (!button->getActionName().isEmpty() && displayNames)
        {
            temp = button->getActionName().replace("&", "&&");
        }
        else
        {
            temp = button->getSlotsSummary().replace("&", "&&");
        }
    }

    return temp;
}
