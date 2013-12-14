#include <QDebug>
#include <QStyle>

#include "joyaxiswidget.h"

JoyAxisWidget::JoyAxisWidget(JoyAxis *axis, QWidget *parent) :
    FlashButtonWidget(parent)
{
    this->axis = axis;

    refreshLabel();

    JoyAxisButton *nAxisButton = axis->getNAxisButton();
    JoyAxisButton *pAxisButton = axis->getPAxisButton();

    connect(axis, SIGNAL(active(int)), this, SLOT(flash()));
    connect(axis, SIGNAL(released(int)), this, SLOT(unflash()));
    connect(axis, SIGNAL(throttleChanged()), this, SLOT(refreshLabel()));
    connect(axis, SIGNAL(axisNameChanged()), this, SLOT(refreshLabel()));
    connect(nAxisButton, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
    connect(nAxisButton, SIGNAL(actionNameChanged()), this, SLOT(refreshLabel()));
    connect(pAxisButton, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
    connect(pAxisButton, SIGNAL(actionNameChanged()), this, SLOT(refreshLabel()));
}

JoyAxis* JoyAxisWidget::getAxis()
{
    return axis;
}

void JoyAxisWidget::disableFlashes()
{
    disconnect(axis, SIGNAL(active(int)), this, 0);
    disconnect(axis, SIGNAL(released(int)), this, 0);
    this->unflash();
}

void JoyAxisWidget::enableFlashes()
{
    connect(axis, SIGNAL(active(int)), this, SLOT(flash()));
    connect(axis, SIGNAL(released(int)), this, SLOT(unflash()));
}

QString JoyAxisWidget::generateLabel()
{
    QString temp;
    temp = axis->getName(false, displayNames).replace("&", "&&");
    return temp;
}
