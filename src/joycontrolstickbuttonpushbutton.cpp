#include "joycontrolstickbuttonpushbutton.h"

JoyControlStickButtonPushButton::JoyControlStickButtonPushButton(JoyControlStickButton *button, bool displayNames, QWidget *parent) :
    FlashButtonWidget(displayNames, parent)
{
    this->button = button;

    refreshLabel();
    enableFlashes();
    connect(button, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
    connect(button, SIGNAL(actionNameChanged()), this, SLOT(refreshLabel()));
    connect(button, SIGNAL(activeZoneChanged()), this, SLOT(refreshLabel()), Qt::QueuedConnection);
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
        disconnect(this->button, SIGNAL(activeZoneChanged()), this, SLOT(refreshLabel()));
    }

    this->button = button;
    refreshLabel();
    enableFlashes();
    connect(button, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
    connect(button, SIGNAL(actionNameChanged()), this, SLOT(refreshLabel()));
    connect(button, SIGNAL(activeZoneChanged()), this, SLOT(refreshLabel()), Qt::QueuedConnection);
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
        connect(button, SIGNAL(clicked(int)), this, SLOT(flash()), Qt::QueuedConnection);
        connect(button, SIGNAL(released(int)), this, SLOT(unflash()), Qt::QueuedConnection);
    }
}

/**
 * @brief Generate the string that will be displayed on the button
 * @return Display string
 */
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
            temp = button->getActiveZoneSummary().replace("&", "&&");
        }
    }

    return temp;
}
