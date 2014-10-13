#include "joydpadbuttonwidget.h"

JoyDPadButtonWidget::JoyDPadButtonWidget(JoyButton *button, bool displayNames, QWidget *parent) :
    JoyButtonWidget(button, displayNames, parent)
{
    // Ensure that JoyDPadButtonWidget::generateLabel is called.
    refreshLabel();
}

/**
 * @brief Generate the string that will be displayed on the button
 * @return Display string
 */
QString JoyDPadButtonWidget::generateLabel()
{
    QString temp;
    if (!button->getActionName().isEmpty() && displayNames)
    {
        temp = button->getActionName();
    }
    else
    {
        temp = button->getActiveZoneSummary();
    }
    temp.replace("&", "&&");
    return temp;
}
