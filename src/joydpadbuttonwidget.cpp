#include "joydpadbuttonwidget.h"

JoyDPadButtonWidget::JoyDPadButtonWidget(JoyButton *button, bool displayNames, QWidget *parent) :
    JoyButtonWidget(button, displayNames, parent)
{
    refreshLabel();
    //connect(button, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
    //connect(button, SIGNAL(actionNameChanged()), this, SLOT(refreshLabel()));
}

QString JoyDPadButtonWidget::generateLabel()
{
    QString temp;
    if (!button->getActionName().isEmpty() && displayNames)
    {
        temp = button->getActionName();
    }
    else
    {
        temp = button->getSlotsSummary();
    }
    temp.replace("&", "&&");
    return temp;
}
