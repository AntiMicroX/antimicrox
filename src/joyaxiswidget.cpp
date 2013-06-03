#include <QDebug>
#include <QStyle>

#include "joyaxiswidget.h"

JoyAxisWidget::JoyAxisWidget(JoyAxis *axis, QWidget *parent) :
    QPushButton(parent)
{
    this->axis = axis;

    isflashing = false;

    setText(axis->getName());

    connect(axis, SIGNAL(active(int)), this, SLOT(flash()));
    connect(axis, SIGNAL(released(int)), this, SLOT(unflash()));
}

JoyAxis* JoyAxisWidget::getAxis()
{
    return axis;
}

void JoyAxisWidget::flash()
{
    isflashing = true;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void JoyAxisWidget::unflash()
{
    isflashing = false;

    this->style()->unpolish(this);
    this->style()->polish(this);

    emit flashed(isflashing);
}

void JoyAxisWidget::refreshLabel()
{
    setText(axis->getName());
}

void JoyAxisWidget::disableFlashes()
{
    disconnect(axis, SIGNAL(active(int)), 0, 0);
    disconnect(axis, SIGNAL(released(int)), 0, 0);
    this->unflash();
}

void JoyAxisWidget::enableFlashes()
{
    connect(axis, SIGNAL(active(int)), this, SLOT(flash()));
    connect(axis, SIGNAL(released(int)), this, SLOT(unflash()));
}

bool JoyAxisWidget::isButtonFlashing()
{
    return isflashing;
}
