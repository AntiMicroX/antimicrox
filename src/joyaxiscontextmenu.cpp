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

#include "joyaxiscontextmenu.h"

#include "antkeymapper.h"
#include "common.h"
#include "inputdevice.h"
#include "joyaxis.h"
#include "mousedialog/mouseaxissettingsdialog.h"

#include <QActionGroup>
#include <QDebug>
#include <QWidget>

JoyAxisContextMenu::JoyAxisContextMenu(JoyAxis *axis, QWidget *parent)
    : QMenu(parent)
    , helper(axis)
{
    this->axis = axis;
    getHelperLocal().moveToThread(axis->thread());

    connect(this, &JoyAxisContextMenu::aboutToHide, this, &JoyAxisContextMenu::deleteLater);
}

void JoyAxisContextMenu::buildMenu()
{
    bool actAsTrigger = false;

    PadderCommon::inputDaemonMutex.lock();

    if ((axis->getThrottle() == static_cast<int>(JoyAxis::PositiveThrottle)) ||
        (axis->getThrottle() == static_cast<int>(JoyAxis::PositiveHalfThrottle)))
    {
        actAsTrigger = true;
    }

    PadderCommon::inputDaemonMutex.unlock();

    if (actAsTrigger)
        buildTriggerMenu();
    else
        buildAxisMenu();
}

void JoyAxisContextMenu::buildAxisMenu()
{
    QActionGroup *presetGroup = new QActionGroup(this);
    int presetMode = 0;
    int currentPreset = getPresetIndex();

    QAction *action = this->addAction(tr("Mouse (Horizontal)"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode + 1);
    action->setData(QVariant(presetMode));

    connect(action, &QAction::triggered, this, [this, action]() { setAxisPreset(action); });

    presetGroup->addAction(action);
    presetMode++;

    action = this->addAction(tr("Mouse (Inverted Horizontal)"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode + 1);
    action->setData(QVariant(presetMode));

    connect(action, &QAction::triggered, this, [this, action]() { setAxisPreset(action); });

    presetGroup->addAction(action);
    presetMode++;

    action = this->addAction(tr("Mouse (Vertical)"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode + 1);
    action->setData(QVariant(presetMode));

    connect(action, &QAction::triggered, this, [this, action]() { setAxisPreset(action); });

    presetGroup->addAction(action);
    presetMode++;

    action = this->addAction(tr("Mouse (Inverted Vertical)"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode + 1);
    action->setData(QVariant(presetMode));

    connect(action, &QAction::triggered, this, [this, action]() { setAxisPreset(action); });

    presetGroup->addAction(action);
    presetMode++;

    action = this->addAction(tr("Arrows: Up | Down"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode + 1);
    action->setData(QVariant(presetMode));

    connect(action, &QAction::triggered, this, [this, action]() { setAxisPreset(action); });

    presetGroup->addAction(action);
    presetMode++;

    action = this->addAction(tr("Arrows: Left | Right"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode + 1);
    action->setData(QVariant(presetMode));

    connect(action, &QAction::triggered, this, [this, action]() { setAxisPreset(action); });

    presetGroup->addAction(action);
    presetMode++;

    action = this->addAction(tr("Keys: W | S"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode + 1);
    action->setData(QVariant(presetMode));

    connect(action, &QAction::triggered, this, [this, action]() { setAxisPreset(action); });

    presetGroup->addAction(action);
    presetMode++;

    action = this->addAction(tr("Keys: A | D"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode + 1);
    action->setData(QVariant(presetMode));

    connect(action, &QAction::triggered, this, [this, action]() { setAxisPreset(action); });

    presetGroup->addAction(action);
    presetMode++;

    action = this->addAction(tr("NumPad: KP_8 | KP_2"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode + 1);
    action->setData(QVariant(presetMode));

    connect(action, &QAction::triggered, this, [this, action]() { setAxisPreset(action); });

    presetGroup->addAction(action);

    presetMode++;
    action = this->addAction(tr("NumPad: KP_4 | KP_6"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode + 1);
    action->setData(QVariant(presetMode));
    connect(action, &QAction::triggered, this, [this, action]() { setAxisPreset(action); });

    presetGroup->addAction(action);
    presetMode++;

    action = this->addAction(tr("None"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode + 1);
    action->setData(QVariant(presetMode));

    connect(action, &QAction::triggered, this, [this, action]() { setAxisPreset(action); });

    presetGroup->addAction(action);

    this->addSeparator();

    action = this->addAction(tr("Mouse Settings"));
    action->setCheckable(false);

    connect(action, &QAction::triggered, this, &JoyAxisContextMenu::openMouseSettingsDialog);
}

int JoyAxisContextMenu::getPresetIndex()
{
    int result = 0;

    JoyAxisButton *naxisbutton = axis->getNAxisButton();
    QList<JoyButtonSlot *> *naxisslots = naxisbutton->getAssignedSlots();
    JoyAxisButton *paxisbutton = axis->getPAxisButton();
    QList<JoyButtonSlot *> *paxisslots = paxisbutton->getAssignedSlots();

    if ((naxisslots->length() == 1) && (paxisslots->length() == 1))
    {
        JoyButtonSlot *nslot = naxisslots->at(0);
        JoyButtonSlot *pslot = paxisslots->at(0);

        if ((nslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
            (nslot->getSlotCode() == JoyButtonSlot::MouseLeft) &&
            (pslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) && (pslot->getSlotCode() == JoyButtonSlot::MouseRight))
        {
            result = 1;
        } else if ((nslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (nslot->getSlotCode() == JoyButtonSlot::MouseRight) &&
                   (pslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (pslot->getSlotCode() == JoyButtonSlot::MouseLeft))
        {
            result = 2;
        } else if ((nslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (nslot->getSlotCode() == JoyButtonSlot::MouseUp) &&
                   (pslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (pslot->getSlotCode() == JoyButtonSlot::MouseDown))
        {
            result = 3;
        } else if ((nslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (nslot->getSlotCode() == JoyButtonSlot::MouseDown) &&
                   (pslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (pslot->getSlotCode() == JoyButtonSlot::MouseUp))
        {
            result = 4;
        } else if ((nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (nslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Up)) &&
                   (pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (pslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Down)))
        {
            result = 5;
        } else if ((nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (nslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Left)) &&
                   (pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (pslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Right)))
        {
            result = 6;
        } else if ((nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (nslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_W)) &&
                   (pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (pslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_S)))
        {
            result = 7;
        } else if ((nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (nslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_A)) &&
                   (pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (pslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_D)))
        {
            result = 8;
        } else if ((nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (nslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_8)) &&
                   (pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (pslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_2)))
        {
            result = 9;
        } else if ((nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (nslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_4)) &&
                   (pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (pslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_6)))
        {
            result = 10;
        }
    } else if ((naxisslots->length() == 0) && (paxisslots->length() == 0))
    {
        result = 11;
    }

    PadderCommon::inputDaemonMutex.unlock();

    return result;
}

void JoyAxisContextMenu::setAxisPreset(QAction *action)
{
    int item = action->data().toInt();

    JoyButtonSlot *nbuttonslot = nullptr;
    JoyButtonSlot *pbuttonslot = nullptr;

    switch (item)
    {
    case 0:
        nbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        break;

    case 1:
        nbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        break;

    case 2:
        nbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        break;

    case 3:
        nbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        break;

    case 4:
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Up), Qt::Key_Up,
                                        JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Down), Qt::Key_Down,
                                        JoyButtonSlot::JoyKeyboard, this);
        break;

    case 5:
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Left), Qt::Key_Left,
                                        JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Right), Qt::Key_Right,
                                        JoyButtonSlot::JoyKeyboard, this);
        break;

    case 6:
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_W), Qt::Key_W,
                                        JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_S), Qt::Key_S,
                                        JoyButtonSlot::JoyKeyboard, this);
        break;

    case 7:
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_A), Qt::Key_A,
                                        JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_D), Qt::Key_D,
                                        JoyButtonSlot::JoyKeyboard, this);
        break;

    case 8:
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_8),
                                        QtKeyMapperBase::AntKey_KP_8, JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_2),
                                        QtKeyMapperBase::AntKey_KP_2, JoyButtonSlot::JoyKeyboard, this);
        break;

    case 9:
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_4),
                                        QtKeyMapperBase::AntKey_KP_4, JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_6),
                                        QtKeyMapperBase::AntKey_KP_6, JoyButtonSlot::JoyKeyboard, this);
        break;

    case 10:
        QMetaObject::invokeMethod(&helper, "clearAndResetAxisButtons", Qt::BlockingQueuedConnection);
        break;
    }

    if (nbuttonslot != nullptr)
    {
        QMetaObject::invokeMethod(&helper, "setNAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, nbuttonslot->getSlotCode()), Q_ARG(int, nbuttonslot->getSlotCodeAlias()),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, nbuttonslot->getSlotMode()));

        nbuttonslot->deleteLater();
    }

    if (pbuttonslot != nullptr)
    {
        QMetaObject::invokeMethod(&helper, "setPAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, nbuttonslot->getSlotCode()), Q_ARG(int, nbuttonslot->getSlotCodeAlias()),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, nbuttonslot->getSlotMode()));

        pbuttonslot->deleteLater();
    }
}

void JoyAxisContextMenu::openMouseSettingsDialog()
{
    MouseAxisSettingsDialog *dialog = new MouseAxisSettingsDialog(this->axis, parentWidget());
    dialog->show();
}

void JoyAxisContextMenu::buildTriggerMenu()
{
    QActionGroup *presetGroup = new QActionGroup(this);
    int presetMode = 0;
    int currentPreset = getTriggerPresetIndex();

    QAction *action = this->addAction(tr("Left Mouse Button"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode + 1);
    action->setData(QVariant(presetMode));

    connect(action, &QAction::triggered, this, [this, action]() { setTriggerPreset(action); });

    presetGroup->addAction(action);
    presetMode++;

    action = this->addAction(tr("Right Mouse Button"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode + 1);
    action->setData(QVariant(presetMode));

    connect(action, &QAction::triggered, this, [this, action]() { setTriggerPreset(action); });

    presetGroup->addAction(action);
    presetMode++;

    action = this->addAction(tr("None"));
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode + 1);
    action->setData(QVariant(presetMode));

    connect(action, &QAction::triggered, this, [this, action]() { setTriggerPreset(action); });

    presetGroup->addAction(action);

    this->addSeparator();

    action = this->addAction(tr("Mouse Settings"));
    action->setCheckable(false);
    connect(action, &QAction::triggered, this, &JoyAxisContextMenu::openMouseSettingsDialog);
}

int JoyAxisContextMenu::getTriggerPresetIndex()
{
    int result = 0;

    PadderCommon::inputDaemonMutex.lock();

    JoyAxisButton *paxisbutton = axis->getPAxisButton();
    QList<JoyButtonSlot *> *paxisslots = paxisbutton->getAssignedSlots();

    if (paxisslots->length() == 1)
    {
        JoyButtonSlot *pslot = paxisslots->at(0);

        if ((pslot->getSlotMode() == JoyButtonSlot::JoyMouseButton) && (pslot->getSlotCode() == JoyButtonSlot::MouseLB))
        {
            result = 1;
        } else if ((pslot->getSlotMode() == JoyButtonSlot::JoyMouseButton) &&
                   (pslot->getSlotCode() == JoyButtonSlot::MouseRB))
        {
            result = 2;
        }
    } else if (paxisslots->length() == 0)
    {
        result = 3;
    }

    PadderCommon::inputDaemonMutex.unlock();

    return result;
}

void JoyAxisContextMenu::setTriggerPreset(QAction *action)
{
    int item = action->data().toInt();

    JoyButtonSlot *pbuttonslot = nullptr;

    switch (item)
    {
    case 0:
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseLB, JoyButtonSlot::JoyMouseButton, this);
        break;

    case 1:
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseRB, JoyButtonSlot::JoyMouseButton, this);
        break;

    case 2:
        JoyAxisButton *pbutton = axis->getPAxisButton();
        QMetaObject::invokeMethod(pbutton, "clearSlotsEventReset", Qt::BlockingQueuedConnection);
        break;
    }

    if (pbuttonslot != nullptr)
    {
        QMetaObject::invokeMethod(&helper, "setPAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, pbuttonslot->getSlotCode()), Q_ARG(int, pbuttonslot->getSlotCodeAlias()),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, pbuttonslot->getSlotMode()));

        pbuttonslot->deleteLater();
    }
}

JoyAxisContextMenuHelper &JoyAxisContextMenu::getHelperLocal() { return helper; }
