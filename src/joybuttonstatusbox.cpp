#include <QStyle>

#include "joybuttonstatusbox.h"

JoyButtonStatusBox::JoyButtonStatusBox(JoyButton *button, QWidget *parent) :
    QPushButton(parent)
{
    this->button = button;
    isflashing = false;

    setText(QString::number(button->getRealJoyNumber()));

    connect(button, SIGNAL(clicked(int)), this, SLOT(flash()));
    connect(button, SIGNAL(released(int)), this, SLOT(unflash()));
}

JoyButton* JoyButtonStatusBox::getJoyButton()
{
    return button;
}

bool JoyButtonStatusBox::isButtonFlashing()
{
    return isflashing;
}

void JoyButtonStatusBox::flash()
{
    isflashing = true;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void JoyButtonStatusBox::unflash()
{
    isflashing = false;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}
