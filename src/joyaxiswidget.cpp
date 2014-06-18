#include "joyaxiswidget.h"

JoyAxisWidget::JoyAxisWidget(JoyAxis *axis, bool displayNames, QWidget *parent) :
    FlashButtonWidget(displayNames, parent)
{
    this->axis = axis;

    refreshLabel();

    JoyAxisButton *nAxisButton = axis->getNAxisButton();
    JoyAxisButton *pAxisButton = axis->getPAxisButton();

    connect(axis, SIGNAL(active(int)), this, SLOT(flash()), Qt::QueuedConnection);
    connect(axis, SIGNAL(released(int)), this, SLOT(unflash()), Qt::QueuedConnection);
    connect(axis, SIGNAL(throttleChanged()), this, SLOT(refreshLabel()));
    connect(axis, SIGNAL(axisNameChanged()), this, SLOT(refreshLabel()));
    connect(nAxisButton, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
    connect(nAxisButton, SIGNAL(actionNameChanged()), this, SLOT(refreshLabel()));
    connect(pAxisButton, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
    connect(pAxisButton, SIGNAL(actionNameChanged()), this, SLOT(refreshLabel()));

    axis->establishPropertyUpdatedConnection();
    nAxisButton->establishPropertyUpdatedConnections();
    pAxisButton->establishPropertyUpdatedConnections();
}

JoyAxis* JoyAxisWidget::getAxis()
{
    return axis;
}

void JoyAxisWidget::disableFlashes()
{
    disconnect(axis, SIGNAL(active(int)), this, SLOT(flash()));
    disconnect(axis, SIGNAL(released(int)), this, SLOT(unflash()));
    this->unflash();
}

void JoyAxisWidget::enableFlashes()
{
    connect(axis, SIGNAL(active(int)), this, SLOT(flash()), Qt::QueuedConnection);
    connect(axis, SIGNAL(released(int)), this, SLOT(unflash()), Qt::QueuedConnection);
}

QString JoyAxisWidget::generateLabel()
{
    QString temp;
    temp = axis->getName(false, displayNames).replace("&", "&&");
    return temp;
}
