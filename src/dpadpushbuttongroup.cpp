#include <QHash>

#include "dpadpushbuttongroup.h"
#include "buttoneditdialog.h"
#include "dpadeditdialog.h"

DPadPushButtonGroup::DPadPushButtonGroup(JoyDPad *dpad, bool displayNames, QWidget *parent) :
    QGridLayout(parent)
{
    this->dpad = dpad;
    this->displayNames = displayNames;

    generateButtons();
    changeButtonLayout();

    connect(dpad, SIGNAL(joyModeChanged()), this, SLOT(changeButtonLayout()));
}

void DPadPushButtonGroup::generateButtons()
{
    QHash<int, JoyDPadButton*> *buttons = dpad->getJoyButtons();

    JoyDPadButton *button = 0;
    JoyDPadButtonWidget *pushbutton = 0;

    button = buttons->value(JoyDPadButton::DpadLeftUp);
    upLeftButton = new JoyDPadButtonWidget(button, displayNames, parentWidget());
    pushbutton = upLeftButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openDPadButtonDialog()));

    button->establishPropertyUpdatedConnections();

    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 0, 0);


    button = buttons->value(JoyDPadButton::DpadUp);
    upButton = new JoyDPadButtonWidget(button, displayNames, parentWidget());
    pushbutton = upButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openDPadButtonDialog()));
    button->establishPropertyUpdatedConnections();

    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 0, 1);

    button = buttons->value(JoyDPadButton::DpadRightUp);
    upRightButton = new JoyDPadButtonWidget(button, displayNames, parentWidget());
    pushbutton = upRightButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openDPadButtonDialog()));
    button->establishPropertyUpdatedConnections();

    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 0, 2);

    button = buttons->value(JoyDPadButton::DpadLeft);
    leftButton = new JoyDPadButtonWidget(button, displayNames, parentWidget());
    pushbutton = leftButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openDPadButtonDialog()));
    button->establishPropertyUpdatedConnections();

    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 1, 0);

    dpadWidget = new DPadPushButton(dpad, displayNames, parentWidget());
    dpadWidget->setIcon(QIcon::fromTheme(QString::fromUtf8("games-config-options")));
    connect(dpadWidget, SIGNAL(clicked()), this, SLOT(showDPadDialog()));
    addWidget(dpadWidget, 1, 1);

    button = buttons->value(JoyDPadButton::DpadRight);
    rightButton = new JoyDPadButtonWidget(button, displayNames, parentWidget());
    pushbutton = rightButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openDPadButtonDialog()));
    button->establishPropertyUpdatedConnections();

    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 1, 2);

    button = buttons->value(JoyDPadButton::DpadLeftDown);
    downLeftButton = new JoyDPadButtonWidget(button, displayNames, parentWidget());
    pushbutton = downLeftButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openDPadButtonDialog()));
    button->establishPropertyUpdatedConnections();
    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 2, 0);

    button = buttons->value(JoyDPadButton::DpadDown);
    downButton = new JoyDPadButtonWidget(button, displayNames, parentWidget());
    pushbutton = downButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openDPadButtonDialog()));
    button->establishPropertyUpdatedConnections();
    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 2, 1);

    button = buttons->value(JoyDPadButton::DpadRightDown);
    downRightButton = new JoyDPadButtonWidget(button, displayNames, parentWidget());
    pushbutton = downRightButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openDPadButtonDialog()));
    button->establishPropertyUpdatedConnections();
    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 2, 2);
}

void DPadPushButtonGroup::changeButtonLayout()
{
    if (dpad->getJoyMode() == JoyDPad::StandardMode ||
        dpad->getJoyMode() == JoyDPad::EightWayMode ||
        dpad->getJoyMode() == JoyDPad::FourWayCardinal)
    {
        upButton->setVisible(true);
        downButton->setVisible(true);
        leftButton->setVisible(true);
        rightButton->setVisible(true);
    }
    else
    {
        upButton->setVisible(false);
        downButton->setVisible(false);
        leftButton->setVisible(false);
        rightButton->setVisible(false);
    }

    if (dpad->getJoyMode() == JoyDPad::EightWayMode ||
        dpad->getJoyMode() == JoyDPad::FourWayDiagonal)
    {
        upLeftButton->setVisible(true);
        upRightButton->setVisible(true);
        downLeftButton->setVisible(true);
        downRightButton->setVisible(true);
    }
    else
    {
        upLeftButton->setVisible(false);
        upRightButton->setVisible(false);
        downLeftButton->setVisible(false);
        downRightButton->setVisible(false);
    }
}

void DPadPushButtonGroup::propogateSlotsChanged()
{
    emit buttonSlotChanged();
}

JoyDPad* DPadPushButtonGroup::getDPad()
{
    return dpad;
}

void DPadPushButtonGroup::openDPadButtonDialog()
{
    JoyButtonWidget *buttonWidget = static_cast<JoyButtonWidget*>(sender());
    JoyButton *button = buttonWidget->getJoyButton();

    ButtonEditDialog *dialog = new ButtonEditDialog(button, parentWidget());
    dialog->show();
}

void DPadPushButtonGroup::showDPadDialog()
{
    DPadEditDialog *dialog = new DPadEditDialog(dpad, parentWidget());
    dialog->show();
}

void DPadPushButtonGroup::toggleNameDisplay()
{
    displayNames = !displayNames;

    upButton->toggleNameDisplay();
    downButton->toggleNameDisplay();
    leftButton->toggleNameDisplay();
    rightButton->toggleNameDisplay();

    upLeftButton->toggleNameDisplay();
    upRightButton->toggleNameDisplay();
    downLeftButton->toggleNameDisplay();
    downRightButton->toggleNameDisplay();

    dpadWidget->toggleNameDisplay();
}
