#include <QDebug>
#include <QStyle>

#include "joybuttonwidget.h"

JoyButtonWidget::JoyButtonWidget(JoyButton *button, QWidget *parent) :
    FlashButtonWidget(parent)
{
    this->button = button;

    refreshLabel();

    connect(button, SIGNAL(clicked(int)), this, SLOT(flash()));
    connect(button, SIGNAL(released(int)), this, SLOT(unflash()));
    connect(button, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
    connect(button, SIGNAL(actionNameChanged()), this, SLOT(refreshLabel()));
    connect(button, SIGNAL(buttonNameChanged()), this, SLOT(refreshLabel()));
}

JoyButton* JoyButtonWidget::getJoyButton()
{
    return button;
}

void JoyButtonWidget::disableFlashes()
{
    disconnect(button, SIGNAL(clicked(int)), this, SLOT(flash()));
    disconnect(button, SIGNAL(released(int)), this, SLOT(unflash()));
    this->unflash();
}

void JoyButtonWidget::enableFlashes()
{
    connect(button, SIGNAL(clicked(int)), this, SLOT(flash()));
    connect(button, SIGNAL(released(int)), this, SLOT(unflash()));
}

QString JoyButtonWidget::generateLabel()
{
    QString temp;
    temp = button->getName(false, displayNames).replace("&", "&&");
    return temp;
}
