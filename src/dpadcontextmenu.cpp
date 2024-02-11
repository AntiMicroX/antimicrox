/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail.com>
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

#include "dpadcontextmenu.h"

#include "antkeymapper.h"
#include "common.h"
#include "inputdevice.h"
#include "joydpad.h"
#include "mousedialog/mousedpadsettingsdialog.h"

#include <QActionGroup>
#include <QDebug>
#include <QHash>

DPadContextMenu::DPadContextMenu(JoyDPad *dpad, QWidget *parent)
    : QMenu(parent)
    , helper(dpad)
{
    this->dpad = dpad;
    getHelper().moveToThread(dpad->thread());

    connect(this, &DPadContextMenu::aboutToHide, this, &DPadContextMenu::deleteLater);
}

/**
 * @brief Generate the context menu that will be shown to a user when the person
 *     right clicks on the DPad settings button.
 */
void DPadContextMenu::buildMenu()
{
    QActionGroup *presetGroup = new QActionGroup(this);
    QAction *action = nullptr;
    int presetMode = 0;
    int currentPreset = getPresetIndex();

    generateActionPreset(action, tr("Mouse (Normal)"), currentPreset, presetMode, presetGroup);
    generateActionPreset(action, tr("Mouse (Inverted Horizontal)"), currentPreset, presetMode, presetGroup);
    generateActionPreset(action, tr("Mouse (Inverted Vertical)"), currentPreset, presetMode, presetGroup);
    generateActionPreset(action, tr("Mouse (Inverted Horizontal + Vertical)"), currentPreset, presetMode, presetGroup);
    generateActionPreset(action, tr("Arrows"), currentPreset, presetMode, presetGroup);
    generateActionPreset(action, tr("Keys: W | A | S | D"), currentPreset, presetMode, presetGroup);
    generateActionPreset(action, tr("NumPad"), currentPreset, presetMode, presetGroup);
    generateActionPreset(action, tr("None"), currentPreset, presetMode, presetGroup);

    this->addSeparator();

    QActionGroup *modesGroup = new QActionGroup(this);

    generateActionMode(modesGroup, action, tr("Standard"), static_cast<int>(dpad->getJoyMode()),
                       static_cast<int>(JoyDPad::StandardMode));
    generateActionMode(modesGroup, action, tr("Eight Way"), static_cast<int>(dpad->getJoyMode()),
                       static_cast<int>(JoyDPad::EightWayMode));
    generateActionMode(modesGroup, action, tr("4 Way Cardinal"), static_cast<int>(dpad->getJoyMode()),
                       static_cast<int>(JoyDPad::FourWayCardinal));
    generateActionMode(modesGroup, action, tr("4 Way Diagonal"), static_cast<int>(dpad->getJoyMode()),
                       static_cast<int>(JoyDPad::FourWayDiagonal));

    this->addSeparator();

    action = this->addAction(tr("Mouse Settings"));
    action->setCheckable(false);

    connect(action, &QAction::triggered, this, &DPadContextMenu::openMouseSettingsDialog);
}

void DPadContextMenu::generateActionPreset(QAction *action, QString actionText, int currentPreset, int &presetMode,
                                           QActionGroup *presetGroup)
{
    action = this->addAction(actionText);
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode + 1);
    action->setData(QVariant(presetMode));

    connect(action, &QAction::triggered, this, [this, action] { setDPadPreset(action); });

    presetGroup->addAction(action);
    presetMode++;
}

void DPadContextMenu::generateActionMode(QActionGroup *modesGroup, QAction *action, QString actionText, int currentPreset,
                                         int presetMode)
{
    action = this->addAction(actionText);
    action->setCheckable(true);
    action->setChecked(currentPreset == presetMode);
    action->setData(QVariant(presetMode));

    connect(action, &QAction::triggered, this, [this, action] { setDPadMode(action); });

    modesGroup->addAction(action);
}

/**
 * @brief Set the appropriate mode for a DPad based on the item chosen.
 */
void DPadContextMenu::setDPadMode(QAction *action)
{
    int item = action->data().toInt();
    dpad->setJoyMode(static_cast<JoyDPad::JoyMode>(item));
}

/**
 * @brief Assign the appropriate slots to DPad buttons based on the preset item
 *     that was chosen.
 */
void DPadContextMenu::setDPadPreset(QAction *action)
{
    int item = action->data().toInt();

    JoyButtonSlot *upButtonSlot = nullptr;
    JoyButtonSlot *downButtonSlot = nullptr;
    JoyButtonSlot *leftButtonSlot = nullptr;
    JoyButtonSlot *rightButtonSlot = nullptr;
    JoyButtonSlot *upLeftButtonSlot = nullptr;
    JoyButtonSlot *upRightButtonSlot = nullptr;
    JoyButtonSlot *downLeftButtonSlot = nullptr;
    JoyButtonSlot *downRightButtonSlot = nullptr;

    switch (item)
    {
    case 0:

        PadderCommon::inputDaemonMutex.lock();

        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        dpad->setJoyMode(JoyDPad::StandardMode);

        PadderCommon::inputDaemonMutex.unlock();

        break;

    case 1:

        PadderCommon::inputDaemonMutex.lock();

        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        dpad->setJoyMode(JoyDPad::StandardMode);

        PadderCommon::inputDaemonMutex.unlock();

        break;

    case 2:

        PadderCommon::inputDaemonMutex.lock();

        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        dpad->setJoyMode(JoyDPad::StandardMode);

        PadderCommon::inputDaemonMutex.unlock();

        break;

    case 3:

        PadderCommon::inputDaemonMutex.lock();

        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        dpad->setJoyMode(JoyDPad::StandardMode);

        PadderCommon::inputDaemonMutex.unlock();

        break;

    case 4:

        PadderCommon::inputDaemonMutex.lock();

        upButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Up), Qt::Key_Up,
                                         JoyButtonSlot::JoyKeyboard, this);
        downButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Down), Qt::Key_Down,
                                           JoyButtonSlot::JoyKeyboard, this);
        leftButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Left), Qt::Key_Left,
                                           JoyButtonSlot::JoyKeyboard, this);
        rightButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Right), Qt::Key_Right,
                                            JoyButtonSlot::JoyKeyboard, this);
        dpad->setJoyMode(JoyDPad::StandardMode);

        PadderCommon::inputDaemonMutex.unlock();

        break;

    case 5:

        PadderCommon::inputDaemonMutex.lock();

        upButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_W), Qt::Key_W,
                                         JoyButtonSlot::JoyKeyboard, this);
        downButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_S), Qt::Key_S,
                                           JoyButtonSlot::JoyKeyboard, this);
        leftButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_A), Qt::Key_A,
                                           JoyButtonSlot::JoyKeyboard, this);
        rightButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_D), Qt::Key_D,
                                            JoyButtonSlot::JoyKeyboard, this);
        dpad->setJoyMode(JoyDPad::StandardMode);

        PadderCommon::inputDaemonMutex.unlock();

        break;

    case 6:

        PadderCommon::inputDaemonMutex.lock();

        if ((dpad->getJoyMode() == JoyDPad::StandardMode) || (dpad->getJoyMode() == JoyDPad::FourWayCardinal))
        {
            upButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_8),
                                             QtKeyMapperBase::AntKey_KP_8, JoyButtonSlot::JoyKeyboard, this);
            downButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_2),
                                               QtKeyMapperBase::AntKey_KP_2, JoyButtonSlot::JoyKeyboard, this);
            leftButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_4),
                                               QtKeyMapperBase::AntKey_KP_4, JoyButtonSlot::JoyKeyboard, this);
            rightButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_6),
                                                QtKeyMapperBase::AntKey_KP_6, JoyButtonSlot::JoyKeyboard, this);
        } else if (dpad->getJoyMode() == JoyDPad::EightWayMode)
        {
            upButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_8),
                                             QtKeyMapperBase::AntKey_KP_8, JoyButtonSlot::JoyKeyboard, this);
            downButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_2),
                                               QtKeyMapperBase::AntKey_KP_2, JoyButtonSlot::JoyKeyboard, this);
            leftButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_4),
                                               QtKeyMapperBase::AntKey_KP_4, JoyButtonSlot::JoyKeyboard, this);
            rightButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_6),
                                                QtKeyMapperBase::AntKey_KP_6, JoyButtonSlot::JoyKeyboard, this);

            upLeftButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_7),
                                                 QtKeyMapperBase::AntKey_KP_7, JoyButtonSlot::JoyKeyboard, this);
            upRightButtonSlot =
                new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_9),
                                  QtKeyMapperBase::AntKey_KP_9, JoyButtonSlot::JoyKeyboard, this);
            downLeftButtonSlot =
                new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_1),
                                  QtKeyMapperBase::AntKey_KP_1, JoyButtonSlot::JoyKeyboard, this);
            downRightButtonSlot =
                new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_3),
                                  QtKeyMapperBase::AntKey_KP_3, JoyButtonSlot::JoyKeyboard, this);
        } else if (dpad->getJoyMode() == JoyDPad::FourWayDiagonal)
        {
            upLeftButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_7),
                                                 QtKeyMapperBase::AntKey_KP_7, JoyButtonSlot::JoyKeyboard, this);
            upRightButtonSlot =
                new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_9),
                                  QtKeyMapperBase::AntKey_KP_9, JoyButtonSlot::JoyKeyboard, this);
            downLeftButtonSlot =
                new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_1),
                                  QtKeyMapperBase::AntKey_KP_1, JoyButtonSlot::JoyKeyboard, this);
            downRightButtonSlot =
                new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_3),
                                  QtKeyMapperBase::AntKey_KP_3, JoyButtonSlot::JoyKeyboard, this);
        }

        PadderCommon::inputDaemonMutex.unlock();

        break;

    case 7:

        QMetaObject::invokeMethod(&helper, "clearButtonsSlotsEventReset", Qt::BlockingQueuedConnection);

        break;
    }

    QHash<JoyDPadButton::JoyDPadDirections, JoyButtonSlot *> tempHash;
    tempHash.insert(JoyDPadButton::DpadUp, upButtonSlot);
    tempHash.insert(JoyDPadButton::DpadDown, downButtonSlot);
    tempHash.insert(JoyDPadButton::DpadLeft, leftButtonSlot);
    tempHash.insert(JoyDPadButton::DpadRight, rightButtonSlot);
    tempHash.insert(JoyDPadButton::DpadLeftUp, upLeftButtonSlot);
    tempHash.insert(JoyDPadButton::DpadRightUp, upRightButtonSlot);
    tempHash.insert(JoyDPadButton::DpadLeftDown, downLeftButtonSlot);
    tempHash.insert(JoyDPadButton::DpadRightDown, downRightButtonSlot);

    getHelper().setPendingSlots(&tempHash);
    QMetaObject::invokeMethod(&helper, "setFromPendingSlots", Qt::BlockingQueuedConnection);
}

/**
 * @brief Find the appropriate menu item index for the currently assigned
 *     slots that are assigned to a DPad.
 * @return Menu index that corresponds to the currently assigned preset choice.
 *    0 means that no matching preset was found.
 */
int DPadContextMenu::getPresetIndex()
{
    int result = 0;

    PadderCommon::inputDaemonMutex.lock();

    JoyDPadButton *upButton = dpad->getJoyButton(JoyDPadButton::DpadUp);
    QList<JoyButtonSlot *> *upslots = upButton->getAssignedSlots();

    JoyDPadButton *downButton = dpad->getJoyButton(JoyDPadButton::DpadDown);
    QList<JoyButtonSlot *> *downslots = downButton->getAssignedSlots();

    JoyDPadButton *leftButton = dpad->getJoyButton(JoyDPadButton::DpadLeft);
    QList<JoyButtonSlot *> *leftslots = leftButton->getAssignedSlots();

    JoyDPadButton *rightButton = dpad->getJoyButton(JoyDPadButton::DpadRight);
    QList<JoyButtonSlot *> *rightslots = rightButton->getAssignedSlots();

    if ((upslots->length() == 1) && (downslots->length() == 1) && (leftslots->length() == 1) && (rightslots->length() == 1))
    {
        JoyButtonSlot *upslot = upslots->at(0);
        JoyButtonSlot *downslot = downslots->at(0);
        JoyButtonSlot *leftslot = leftslots->at(0);
        JoyButtonSlot *rightslot = rightslots->at(0);

        if ((upslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
            (upslot->getSlotCode() == JoyButtonSlot::MouseUp) &&
            (downslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
            (downslot->getSlotCode() == JoyButtonSlot::MouseDown) &&
            (leftslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
            (leftslot->getSlotCode() == JoyButtonSlot::MouseLeft) &&
            (rightslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
            (rightslot->getSlotCode() == JoyButtonSlot::MouseRight))
        {
            result = 1;
        } else if ((upslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (upslot->getSlotCode() == JoyButtonSlot::MouseUp) &&
                   (downslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (downslot->getSlotCode() == JoyButtonSlot::MouseDown) &&
                   (leftslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (leftslot->getSlotCode() == JoyButtonSlot::MouseRight) &&
                   (rightslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (rightslot->getSlotCode() == JoyButtonSlot::MouseLeft))
        {
            result = 2;
        } else if ((upslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (upslot->getSlotCode() == JoyButtonSlot::MouseDown) &&
                   (downslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (downslot->getSlotCode() == JoyButtonSlot::MouseUp) &&
                   (leftslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (leftslot->getSlotCode() == JoyButtonSlot::MouseLeft) &&
                   (rightslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (rightslot->getSlotCode() == JoyButtonSlot::MouseRight))
        {
            result = 3;
        } else if ((upslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (upslot->getSlotCode() == JoyButtonSlot::MouseDown) &&
                   (downslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (downslot->getSlotCode() == JoyButtonSlot::MouseUp) &&
                   (leftslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (leftslot->getSlotCode() == JoyButtonSlot::MouseRight) &&
                   (rightslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (rightslot->getSlotCode() == JoyButtonSlot::MouseLeft))
        {
            result = 4;
        } else if ((upslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (upslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Up)) &&
                   (downslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (downslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Down)) &&
                   (leftslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (leftslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Left)) &&
                   (rightslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (rightslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Right)))
        {
            result = 5;
        } else if ((upslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (upslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_W)) &&
                   (downslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (downslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_S)) &&
                   (leftslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (leftslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_A)) &&
                   (rightslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (rightslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_D)))
        {
            result = 6;
        } else if ((upslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (upslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_8)) &&
                   (downslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (downslot->getSlotCode() ==
                    AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_2)) &&
                   (leftslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (leftslot->getSlotCode() ==
                    AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_4)) &&
                   (rightslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (rightslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_6)))
        {
            result = 7;
        }
    } else if ((upslots->length() == 0) && (downslots->length() == 0) && (leftslots->length() == 0) &&
               (rightslots->length() == 0))
    {
        result = 8;
    }

    PadderCommon::inputDaemonMutex.unlock();

    return result;
}

/**
 * @brief Open a mouse settings dialog for changing the mouse speed settings
 *     for all DPad buttons.
 */
void DPadContextMenu::openMouseSettingsDialog()
{
    MouseDPadSettingsDialog *dialog = new MouseDPadSettingsDialog(dpad, parentWidget());
    dialog->show();
}

DPadContextMenuHelper &DPadContextMenu::getHelper() { return helper; }
