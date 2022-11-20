/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "joybuttoncontextmenu.h"

#include "common.h"
#include "globalvariables.h"
#include "inputdevice.h"
#include "joybuttontypes/joybutton.h"

#include <QActionGroup>
#include <QDebug>
#include <QWidget>

JoyButtonContextMenu::JoyButtonContextMenu(JoyButton *button, QWidget *parent)
    : QMenu(parent)
{
    this->button = button;

    connect(this, &JoyButtonContextMenu::aboutToHide, this, &JoyButtonContextMenu::deleteLater);
}

void JoyButtonContextMenu::buildMenu()
{
    PadderCommon::inputDaemonMutex.lock();

    QAction *action = this->addAction(tr("Toggle"));
    action->setCheckable(true);
    action->setChecked(button->getToggleState());
    connect(action, &QAction::triggered, this, &JoyButtonContextMenu::switchToggle);

    action = this->addAction(tr("Turbo"));
    action->setCheckable(true);
    action->setChecked(button->isUsingTurbo());
    connect(action, &QAction::triggered, this, &JoyButtonContextMenu::switchTurbo);

    this->addSeparator();

    action = this->addAction(tr("Clear"));
    action->setCheckable(false);
    connect(action, &QAction::triggered, this, &JoyButtonContextMenu::clearButton);

    this->addSeparator();

    QMenu *setSectionMenu = this->addMenu(tr("Set Select"));
    action = setSectionMenu->addAction(tr("Disabled"));

    if (button->getChangeSetCondition() == JoyButton::SetChangeDisabled)
    {
        action->setCheckable(true);
        action->setChecked(true);
    }

    connect(action, &QAction::triggered, this, &JoyButtonContextMenu::disableSetMode);
    setSectionMenu->addSeparator();

    for (int i = 0; i < GlobalVariables::InputDevice::NUMBER_JOYSETS; i++)
    {
        QMenu *tempSetMenu = setSectionMenu->addMenu(tr("Set %1").arg(i + 1));
        int setSelection = i * 3;

        if (i == button->getSetSelection())
        {
            QFont tempFont = tempSetMenu->menuAction()->font();
            tempFont.setBold(true);
            tempSetMenu->menuAction()->setFont(tempFont);
        }

        QActionGroup *tempGroup = new QActionGroup(tempSetMenu);
        createActionForGroup(tempGroup, tr("Set %1 1W"), action, tempSetMenu, setSelection, i, 0, 1);
        createActionForGroup(tempGroup, tr("Set %1 2W"), action, tempSetMenu, setSelection, i, 1, 2);
        createActionForGroup(tempGroup, tr("Set %1 WH"), action, tempSetMenu, setSelection, i, 2, 3);

        if (i == button->getParentSet()->getIndex())
            tempSetMenu->setEnabled(false);
    }

    PadderCommon::inputDaemonMutex.unlock();
}

void JoyButtonContextMenu::createActionForGroup(QActionGroup *tempGroup, QString actionText, QAction *action,
                                                QMenu *tempSetMenu, int setSelection, int currentSelection, int setDataInc,
                                                int setCondition)
{
    action = tempSetMenu->addAction(actionText.arg(currentSelection + 1));
    action->setData(QVariant(setSelection + setDataInc));
    action->setCheckable(true);

    if ((button->getSetSelection() == currentSelection) &&
        (button->getChangeSetCondition() == static_cast<JoyButton::SetChangeCondition>(setCondition)))
    {
        action->setChecked(true);
    }

    connect(action, &QAction::triggered, this, [this, action]() { switchSetMode(action); });

    tempGroup->addAction(action);
}

void JoyButtonContextMenu::switchToggle()
{
    PadderCommon::inputDaemonMutex.lock();
    button->setToggle(!button->getToggleState());
    PadderCommon::inputDaemonMutex.unlock();
}

void JoyButtonContextMenu::switchTurbo()
{
    PadderCommon::inputDaemonMutex.lock();
    button->setUseTurbo(!button->isUsingTurbo());
    PadderCommon::inputDaemonMutex.unlock();
}

void JoyButtonContextMenu::switchSetMode(QAction *action)
{
    int item = action->data().toInt();
    int setSelection = item / 3;
    int setChangeCondition = item % 3;
    JoyButton::SetChangeCondition temp = JoyButton::SetChangeOneWay;

    switch (setChangeCondition)
    {
    case 0:
        temp = JoyButton::SetChangeOneWay;
        break;

    case 1:
        temp = JoyButton::SetChangeTwoWay;
        break;

    case 2:
        temp = JoyButton::SetChangeWhileHeld;
        break;
    }

    PadderCommon::inputDaemonMutex.lock();

    // First, remove old condition for the button in both sets.
    // After that, make the new assignment.
    button->setChangeSetCondition(JoyButton::SetChangeDisabled);
    button->setChangeSetSelection(setSelection);
    button->setChangeSetCondition(temp);
    PadderCommon::inputDaemonMutex.unlock();
}

void JoyButtonContextMenu::disableSetMode()
{
    PadderCommon::inputDaemonMutex.lock();
    button->setChangeSetCondition(JoyButton::SetChangeDisabled);
    PadderCommon::inputDaemonMutex.unlock();
}

void JoyButtonContextMenu::clearButton() { QMetaObject::invokeMethod(button, "clearSlotsEventReset"); }
