#include <QDebug>
#include <QStyle>

#include "joybuttonwidget.h"
#include "event.h"

JoyButtonWidget::JoyButtonWidget(QWidget *parent) :
    QPushButton(parent)
{
    isflashing = false;
}

JoyButtonWidget::JoyButtonWidget(JoyButton *button, QWidget *parent) :
    QPushButton(parent)
{
    this->button = button;

    isflashing = false;

    setText(button->getName());

    connect (button, SIGNAL(keyChanged(int)), this, SLOT(changeKeyLabel()));
    connect (button, SIGNAL(mouseChanged(int)), this, SLOT(changeMouseLabel()));
    connect (button, SIGNAL(clicked(int)), this, SLOT(flash()));
    connect (button, SIGNAL(released(int)), this, SLOT(unflash()));
}

JoyButton* JoyButtonWidget::getJoyButton()
{
    return button;
}

void JoyButtonWidget::flash()
{
    isflashing = true;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void JoyButtonWidget::unflash()
{
    isflashing = false;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void JoyButtonWidget::changeKeyLabel()
{
    setText(button->getName().replace("&", "&&"));
}

void JoyButtonWidget::changeMouseLabel()
{
    setText(button->getName().replace("&", "&&"));
}

void JoyButtonWidget::refreshLabel()
{
    setText(button->getName().replace("&", "&&"));
}

void JoyButtonWidget::disableFlashes()
{
    disconnect (button, SIGNAL(clicked(int)), 0, 0);
    disconnect (button, SIGNAL(released(int)), 0, 0);
    this->unflash();
}

void JoyButtonWidget::enableFlashes()
{
    connect (button, SIGNAL(clicked(int)), this, SLOT(flash()));
    connect (button, SIGNAL(released(int)), this, SLOT(unflash()));
}

bool JoyButtonWidget::isButtonFlashing()
{
    return isflashing;
}
