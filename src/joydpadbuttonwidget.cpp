#include "joydpadbuttonwidget.h"

JoyDPadButtonWidget::JoyDPadButtonWidget(JoyButton *button, QWidget *parent) :
    JoyButtonWidget(button, parent)
{
    refreshLabel();
    connect(button, SIGNAL(slotsChanged()), this, SLOT(refreshLabel()));
    connect(button, SIGNAL(actionNameChanged()), this, SLOT(refreshLabel()));
}

QString JoyDPadButtonWidget::generateLabel()
{
    QString temp;
    if (!button->getActionName().isEmpty())
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

void JoyDPadButtonWidget::refreshLabel()
{
    setText(generateLabel());
}
