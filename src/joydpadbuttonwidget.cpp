#include "joydpadbuttonwidget.h"

JoyDPadButtonWidget::JoyDPadButtonWidget(JoyButton *button, QWidget *parent) :
    JoyButtonWidget(button, parent)
{
    refreshLabel();
}

QString JoyDPadButtonWidget::generateLabel()
{
    QString temp;
    temp = button->getSlotsSummary().replace("&", "&&");
    return temp;
}

void JoyDPadButtonWidget::refreshLabel()
{
    setText(generateLabel());
}
