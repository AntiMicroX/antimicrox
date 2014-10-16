//#include <QDebug>
#include <QActionGroup>

#include "inputdevice.h"

#include "joybuttoncontextmenu.h"

JoyButtonContextMenu::JoyButtonContextMenu(JoyButton *button, QWidget *parent) :
    QMenu(parent)
{
    this->button = button;

    connect(this, SIGNAL(aboutToHide()), this, SLOT(deleteLater()));
}

void JoyButtonContextMenu::buildMenu()
{
    QAction *action = 0;

    action = this->addAction(tr("Toggle"));
    action->setCheckable(true);
    action->setChecked(button->getToggleState());
    connect(action, SIGNAL(triggered()), this, SLOT(switchToggle()));

    action = this->addAction(tr("Turbo"));
    action->setCheckable(true);
    action->setChecked(button->isUsingTurbo());
    connect(action, SIGNAL(triggered()), this, SLOT(switchToggle()));

    this->addSeparator();

    action = this->addAction(tr("Clear"));
    action->setCheckable(false);
    connect(action, SIGNAL(triggered()), this, SLOT(clearButton()));

    this->addSeparator();

    QMenu *setSectionMenu = this->addMenu(tr("Set Select"));

    action = setSectionMenu->addAction(tr("Disabled"));
    if (button->getChangeSetCondition() == JoyButton::SetChangeDisabled)
    {
        action->setCheckable(true);
        action->setChecked(true);
    }
    connect(action, SIGNAL(triggered()), this, SLOT(disableSetMode()));

    setSectionMenu->addSeparator();

    for (int i=0; i < InputDevice::NUMBER_JOYSETS; i++)
    {
        QMenu *tempSetMenu = setSectionMenu->addMenu(tr("Set %1").arg(i+1));
        int setSelection = i*3;

        if (i == button->getSetSelection())
        {
            QFont tempFont = tempSetMenu->menuAction()->font();
            tempFont.setBold(true);
            tempSetMenu->menuAction()->setFont(tempFont);
        }

        QActionGroup *tempGroup = new QActionGroup(tempSetMenu);

        action = tempSetMenu->addAction(tr("Set %1 1W").arg(i+1));
        action->setData(QVariant(setSelection + 0));
        action->setCheckable(true);
        if (button->getSetSelection() == i &&
            button->getChangeSetCondition() == JoyButton::SetChangeOneWay)
        {
            action->setChecked(true);
        }
        connect(action, SIGNAL(triggered()), this, SLOT(switchSetMode()));
        tempGroup->addAction(action);

        action = tempSetMenu->addAction(tr("Set %1 2W").arg(i+1));
        action->setData(QVariant(setSelection + 1));
        action->setCheckable(true);
        if (button->getSetSelection() == i &&
            button->getChangeSetCondition() == JoyButton::SetChangeTwoWay)
        {
            action->setChecked(true);
        }
        connect(action, SIGNAL(triggered()), this, SLOT(switchSetMode()));
        tempGroup->addAction(action);

        action = tempSetMenu->addAction(tr("Set %1 WH").arg(i+1));
        action->setData(QVariant(setSelection + 2));
        action->setCheckable(true);
        if (button->getSetSelection() == i &&
            button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld)
        {
            action->setChecked(true);
        }
        connect(action, SIGNAL(triggered()), this, SLOT(switchSetMode()));
        tempGroup->addAction(action);

        if (i == button->getParentSet()->getIndex())
        {
            tempSetMenu->setEnabled(false);
        }
    }
}

void JoyButtonContextMenu::switchToggle()
{
    button->setToggle(!button->getToggleState());
}

void JoyButtonContextMenu::switchTurbo()
{
    button->setToggle(!button->isUsingTurbo());
}

void JoyButtonContextMenu::switchSetMode()
{
    QAction *action = static_cast<QAction*>(sender());
    int item = action->data().toInt();
    int setSelection = item / 3;
    int setChangeCondition = item % 3;
    JoyButton::SetChangeCondition temp;

    if (setChangeCondition == 0)
    {
        temp = JoyButton::SetChangeOneWay;
    }
    else if (setChangeCondition == 1)
    {
        temp = JoyButton::SetChangeTwoWay;
    }
    else if (setChangeCondition == 2)
    {
        temp = JoyButton::SetChangeWhileHeld;
    }

    // First, remove old condition for the button in both sets.
    // After that, make the new assignment.
    button->setChangeSetCondition(JoyButton::SetChangeDisabled);
    button->setChangeSetSelection(setSelection);
    button->setChangeSetCondition(temp);
}

void JoyButtonContextMenu::disableSetMode()
{
    button->setChangeSetCondition(JoyButton::SetChangeDisabled);
}

void JoyButtonContextMenu::clearButton()
{
    button->clearSlotsEventReset();
}
