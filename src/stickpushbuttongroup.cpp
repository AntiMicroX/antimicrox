#include <QHash>

#include "stickpushbuttongroup.h"
#include "buttoneditdialog.h"
#include "joycontrolstickeditdialog.h"

StickPushButtonGroup::StickPushButtonGroup(JoyControlStick *stick, bool displayNames, QWidget *parent) :
    QGridLayout(parent)
{
    this->stick = stick;
    this->displayNames = displayNames;

    generateButtons();
    changeButtonLayout();

    connect(stick, SIGNAL(joyModeChanged()), this, SLOT(changeButtonLayout()));
}

void StickPushButtonGroup::generateButtons()
{
    QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton*> *stickButtons = stick->getButtons();

    JoyControlStickButton *button = 0;
    JoyControlStickButtonPushButton *pushbutton = 0;

    button = stickButtons->value(JoyControlStick::StickLeftUp);
    upLeftButton = new JoyControlStickButtonPushButton(button, displayNames, parentWidget());
    pushbutton = upLeftButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openStickButtonDialog()));
    button->establishPropertyUpdatedConnections();
    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 0, 0);

    button = stickButtons->value(JoyControlStick::StickUp);
    upButton = new JoyControlStickButtonPushButton(button, displayNames, parentWidget());
    pushbutton = upButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openStickButtonDialog()));
    button->establishPropertyUpdatedConnections();
    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 0, 1);

    button = stickButtons->value(JoyControlStick::StickRightUp);
    upRightButton = new JoyControlStickButtonPushButton(button, displayNames, parentWidget());
    pushbutton = upRightButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openStickButtonDialog()));
    button->establishPropertyUpdatedConnections();
    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 0, 2);

    button = stickButtons->value(JoyControlStick::StickLeft);
    leftButton = new JoyControlStickButtonPushButton(button, displayNames, parentWidget());
    pushbutton = leftButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openStickButtonDialog()));
    button->establishPropertyUpdatedConnections();
    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 1, 0);

    stickWidget = new JoyControlStickPushButton(stick, displayNames, parentWidget());
    stickWidget->setIcon(QIcon::fromTheme(QString::fromUtf8("games-config-options")));
    connect(stickWidget, SIGNAL(clicked()), this, SLOT(showStickDialog()));

    addWidget(stickWidget, 1, 1);

    button = stickButtons->value(JoyControlStick::StickRight);
    rightButton = new JoyControlStickButtonPushButton(button, displayNames, parentWidget());
    pushbutton = rightButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openStickButtonDialog()));
    button->establishPropertyUpdatedConnections();
    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 1, 2);

    button = stickButtons->value(JoyControlStick::StickLeftDown);
    downLeftButton = new JoyControlStickButtonPushButton(button, displayNames, parentWidget());
    pushbutton = downLeftButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openStickButtonDialog()));
    button->establishPropertyUpdatedConnections();

    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 2, 0);

    button = stickButtons->value(JoyControlStick::StickDown);
    downButton = new JoyControlStickButtonPushButton(button, displayNames, parentWidget());
    pushbutton = downButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openStickButtonDialog()));
    button->establishPropertyUpdatedConnections();
    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 2, 1);

    button = stickButtons->value(JoyControlStick::StickRightDown);
    downRightButton = new JoyControlStickButtonPushButton(button, displayNames, parentWidget());
    pushbutton = downRightButton;
    connect(pushbutton, SIGNAL(clicked()), this, SLOT(openStickButtonDialog()));
    button->establishPropertyUpdatedConnections();
    connect(button, SIGNAL(slotsChanged()), this, SLOT(propogateSlotsChanged()));

    addWidget(pushbutton, 2, 2);
}

void StickPushButtonGroup::changeButtonLayout()
{
    if (stick->getJoyMode() == JoyControlStick::StandardMode ||
        stick->getJoyMode() == JoyControlStick::EightWayMode ||
        stick->getJoyMode() == JoyControlStick::FourWayCardinal)
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

    if (stick->getJoyMode() == JoyControlStick::EightWayMode ||
        stick->getJoyMode() == JoyControlStick::FourWayDiagonal)
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

void StickPushButtonGroup::propogateSlotsChanged()
{
    emit buttonSlotChanged();
}

JoyControlStick* StickPushButtonGroup::getStick()
{
    return stick;
}

void StickPushButtonGroup::openStickButtonDialog()
{
    JoyControlStickButtonPushButton *pushbutton = static_cast<JoyControlStickButtonPushButton*>(sender());
    ButtonEditDialog *dialog = new ButtonEditDialog(pushbutton->getButton(), parentWidget());
    dialog->show();
}

void StickPushButtonGroup::showStickDialog()
{
    JoyControlStickEditDialog *dialog = new JoyControlStickEditDialog(stick, parentWidget());
    dialog->show();
}

void StickPushButtonGroup::toggleNameDisplay()
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

    stickWidget->toggleNameDisplay();
}
