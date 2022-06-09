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

#include "joycontrolstickeditdialog.h"
#include "ui_joycontrolstickeditdialog.h"

#include "antkeymapper.h"
#include "buttoneditdialog.h"
#include "common.h"
#include "event.h"
#include "inputdevice.h"
#include "joybuttontypes/joycontrolstickbutton.h"
#include "joybuttontypes/joycontrolstickmodifierbutton.h"
#include "joycontrolstick.h"
#include "mousedialog/mousecontrolsticksettingsdialog.h"
#include "setjoystick.h"

#include <QDebug>
#include <QHash>
#include <QHashIterator>
#include <QList>
#include <QWidget>

JoyControlStickEditDialog::JoyControlStickEditDialog(JoyControlStick *stick, bool keypadUnlocked, QWidget *parent)
    : QDialog(parent, Qt::Window)
    , ui(new Ui::JoyControlStickEditDialog)
    , helper(stick)
{
    ui->setupUi(this);
    this->keypadUnlocked = keypadUnlocked;
    setAttribute(Qt::WA_DeleteOnClose);

    auto min_width = ui->xCoordinateLabel->fontMetrics().boundingRect(QString("X.XXXXXXXXX")).width();
    ui->xCoordinateLabel->setMinimumWidth(min_width);

    this->stick = stick;
    getHelperLocal().moveToThread(stick->thread());

    PadderCommon::inputDaemonMutex.lock();

    updateWindowTitleStickName();

    ui->deadZoneSlider->setValue(stick->getDeadZone());
    ui->deadZoneSpinBox->setValue(stick->getDeadZone());

    ui->maxZoneSlider->setValue(stick->getMaxZone());
    ui->maxZoneSpinBox->setValue(stick->getMaxZone());

    ui->modifierZoneSlider->setValue(stick->getModifierZone());
    ui->modifierZoneSpinBox->setValue(stick->getModifierZone());
    ui->modifierZoneInvertedCheckBox->setCheckState(stick->getModifierZoneInverted() ? Qt::Checked : Qt::Unchecked);

    ui->diagonalRangeSlider->setValue(stick->getDiagonalRange());
    ui->diagonalRangeSpinBox->setValue(stick->getDiagonalRange());

    QString xCoorString = QString::number(stick->getXCoordinate());
    if (stick->getCircleAdjust() > 0.0)
    {
        xCoorString.append(QString(" (%1)").arg(stick->getCircleXCoordinate()));
    }
    ui->xCoordinateLabel->setText(xCoorString);

    QString yCoorString = QString::number(stick->getYCoordinate());
    if (stick->getCircleAdjust() > 0.0)
    {
        yCoorString.append(QString(" (%1)").arg(stick->getCircleYCoordinate()));
    }
    ui->yCoordinateLabel->setText(yCoorString);

    ui->distanceLabel->setText(QString::number(stick->getAbsoluteRawDistance()));
    ui->diagonalLabel->setText(QString::number(stick->calculateBearing()));

    switch (stick->getJoyMode())
    {
    case JoyControlStick::StandardMode: {
        ui->joyModeComboBox->setCurrentIndex(0);
        break;
    }
    case JoyControlStick::EightWayMode: {
        ui->joyModeComboBox->setCurrentIndex(1);
        break;
    }
    case JoyControlStick::FourWayCardinal: {
        ui->joyModeComboBox->setCurrentIndex(2);
        ui->diagonalRangeSlider->setEnabled(false);
        ui->diagonalRangeSpinBox->setEnabled(false);
        break;
    }
    case JoyControlStick::FourWayDiagonal: {
        ui->joyModeComboBox->setCurrentIndex(3);
        ui->diagonalRangeSlider->setEnabled(false);
        ui->diagonalRangeSpinBox->setEnabled(false);
        break;
    }
    }

    ui->stickStatusBoxWidget->setStick(stick);

    selectCurrentPreset();

    ui->stickNameLineEdit->setText(stick->getStickName());
    double validDistance = stick->getDistanceFromDeadZone() * 100.0;
    ui->fromSafeZoneValueLabel->setText(QString::number(validDistance));

    double circleValue = stick->getCircleAdjust();
    ui->squareStickSlider->setValue(circleValue * 100);
    ui->squareStickSpinBox->setValue(circleValue * 100);

    int stickDelay = stick->getStickDelay();
    ui->stickDelaySlider->setValue(stickDelay * .1);
    ui->stickDelayDoubleSpinBox->setValue(stickDelay * .001);

    ui->modifierPushButton->setText(stick->getModifierButton()->getSlotsSummary());
    stick->getModifierButton()->establishPropertyUpdatedConnections();

    update();
    updateGeometry();

    PadderCommon::inputDaemonMutex.unlock();

    connect(ui->presetsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &JoyControlStickEditDialog::implementPresets);
    connect(ui->joyModeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &JoyControlStickEditDialog::implementModes);

    connect(ui->deadZoneSlider, &QSlider::valueChanged, ui->deadZoneSpinBox, &QSpinBox::setValue);
    connect(ui->maxZoneSlider, &QSlider::valueChanged, ui->maxZoneSpinBox, &QSpinBox::setValue);
    connect(ui->diagonalRangeSlider, &QSlider::valueChanged, ui->diagonalRangeSpinBox, &QSpinBox::setValue);
    connect(ui->squareStickSlider, &QSlider::valueChanged, ui->squareStickSpinBox, &QSpinBox::setValue);

    connect(ui->deadZoneSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), ui->deadZoneSlider,
            &QSlider::setValue);
    connect(ui->maxZoneSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), ui->maxZoneSlider,
            &QSlider::setValue);
    connect(ui->maxZoneSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &JoyControlStickEditDialog::checkMaxZone);
    connect(ui->diagonalRangeSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), ui->diagonalRangeSlider,
            &QSlider::setValue);
    connect(ui->squareStickSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), ui->squareStickSlider,
            &QSlider::setValue);
    connect(ui->stickDelaySlider, &QSlider::valueChanged, &helper,
            &JoyControlStickEditDialogHelper::updateControlStickDelay);

    connect(ui->deadZoneSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), stick,
            &JoyControlStick::setDeadZone);
    connect(ui->diagonalRangeSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), stick,
            &JoyControlStick::setDiagonalRange);
    connect(ui->squareStickSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &JoyControlStickEditDialog::changeCircleAdjust);
    connect(stick, &JoyControlStick::stickDelayChanged, this, &JoyControlStickEditDialog::updateStickDelaySpinBox);
    connect(ui->stickDelayDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this,
            &JoyControlStickEditDialog::updateStickDelaySlider);

    connect(stick, &JoyControlStick::moved, this, &JoyControlStickEditDialog::refreshStickStats);
    connect(ui->mouseSettingsPushButton, &QPushButton::clicked, this, &JoyControlStickEditDialog::openMouseSettingsDialog);

    connect(ui->stickNameLineEdit, &QLineEdit::textEdited, stick, &JoyControlStick::setStickName);
    connect(stick, &JoyControlStick::stickNameChanged, this, &JoyControlStickEditDialog::updateWindowTitleStickName);
    connect(ui->modifierPushButton, &QPushButton::clicked, this, &JoyControlStickEditDialog::openModifierEditDialog);
    connect(stick->getModifierButton(), &JoyControlStickModifierButton::slotsChanged, this,
            &JoyControlStickEditDialog::changeModifierSummary);
    connect(ui->modifierZoneSlider, &QSlider::valueChanged, ui->modifierZoneSpinBox, &QSpinBox::setValue);
    connect(ui->modifierZoneSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), ui->modifierZoneSlider,
            &QSlider::setValue);
    connect(ui->modifierZoneSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), stick,
            &JoyControlStick::setModifierZone);
    connect(ui->modifierZoneInvertedCheckBox, &QCheckBox::stateChanged, stick,
            [stick](int state) { stick->setModifierZoneInverted(state == Qt::Checked); });
}

// for tests
/*JoyControlStickEditDialog::JoyControlStickEditDialog(QWidget *parent) :
    QDialog(parent, Qt::Window),
    ui(new Ui::JoyControlStickEditDialog)
{
    helper = new JoyControlStick();
}*/

JoyControlStickEditDialog::~JoyControlStickEditDialog() { delete ui; }

void JoyControlStickEditDialog::implementPresets(int index)
{
    JoyButtonSlot *upButtonSlot = nullptr;
    JoyButtonSlot *downButtonSlot = nullptr;
    JoyButtonSlot *leftButtonSlot = nullptr;
    JoyButtonSlot *rightButtonSlot = nullptr;
    JoyButtonSlot *upLeftButtonSlot = nullptr;
    JoyButtonSlot *upRightButtonSlot = nullptr;
    JoyButtonSlot *downLeftButtonSlot = nullptr;
    JoyButtonSlot *downRightButtonSlot = nullptr;

    switch (index)
    {
    case 1: {
        PadderCommon::inputDaemonMutex.lock();

        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);

        PadderCommon::inputDaemonMutex.unlock();

        ui->joyModeComboBox->setCurrentIndex(0);
        ui->diagonalRangeSlider->setValue(65);

        break;
    }
    case 2: {
        PadderCommon::inputDaemonMutex.lock();

        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);

        PadderCommon::inputDaemonMutex.unlock();

        ui->joyModeComboBox->setCurrentIndex(0);
        ui->diagonalRangeSlider->setValue(65);

        break;
    }
    case 3: {
        PadderCommon::inputDaemonMutex.lock();

        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);

        PadderCommon::inputDaemonMutex.unlock();

        ui->joyModeComboBox->setCurrentIndex(0);
        ui->diagonalRangeSlider->setValue(65);

        break;
    }
    case 4: {
        PadderCommon::inputDaemonMutex.lock();

        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);

        PadderCommon::inputDaemonMutex.unlock();

        ui->joyModeComboBox->setCurrentIndex(0);
        ui->diagonalRangeSlider->setValue(65);

        break;
    }
    case 5: {
        PadderCommon::inputDaemonMutex.lock();

        upButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Up), Qt::Key_Up,
                                         JoyButtonSlot::JoyKeyboard, this);
        downButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Down), Qt::Key_Down,
                                           JoyButtonSlot::JoyKeyboard, this);
        leftButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Left), Qt::Key_Left,
                                           JoyButtonSlot::JoyKeyboard, this);
        rightButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Right), Qt::Key_Right,
                                            JoyButtonSlot::JoyKeyboard, this);

        PadderCommon::inputDaemonMutex.unlock();

        ui->joyModeComboBox->setCurrentIndex(0);
        ui->diagonalRangeSlider->setValue(45);

        break;
    }
    case 6: {
        PadderCommon::inputDaemonMutex.lock();

        upButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_W), Qt::Key_W,
                                         JoyButtonSlot::JoyKeyboard, this);
        downButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_S), Qt::Key_S,
                                           JoyButtonSlot::JoyKeyboard, this);
        leftButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_A), Qt::Key_A,
                                           JoyButtonSlot::JoyKeyboard, this);
        rightButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_D), Qt::Key_D,
                                            JoyButtonSlot::JoyKeyboard, this);

        PadderCommon::inputDaemonMutex.unlock();

        ui->joyModeComboBox->setCurrentIndex(0);
        ui->diagonalRangeSlider->setValue(45);

        break;
    }
    case 7: {
        PadderCommon::inputDaemonMutex.lock();

        if ((ui->joyModeComboBox->currentIndex() == 0) || (ui->joyModeComboBox->currentIndex() == 2))
        {
            upButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_8),
                                             QtKeyMapperBase::AntKey_KP_8, JoyButtonSlot::JoyKeyboard, this);
            downButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_2),
                                               QtKeyMapperBase::AntKey_KP_2, JoyButtonSlot::JoyKeyboard, this);
            leftButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_4),
                                               QtKeyMapperBase::AntKey_KP_4, JoyButtonSlot::JoyKeyboard, this);
            rightButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_6),
                                                QtKeyMapperBase::AntKey_KP_6, JoyButtonSlot::JoyKeyboard, this);
        } else if (ui->joyModeComboBox->currentIndex() == 1)
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
        } else if (ui->joyModeComboBox->currentIndex() == 3)
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

        ui->diagonalRangeSlider->setValue(45);

        break;
    }
    case 0:
    case 8: {
        QMetaObject::invokeMethod(&helper, "clearButtonsSlotsEventReset", Qt::BlockingQueuedConnection);

        ui->diagonalRangeSlider->setValue(45);

        stick->getDirectionButton(JoyControlStick::StickUp)->buildActiveZoneSummaryString();
        stick->getDirectionButton(JoyControlStick::StickDown)->buildActiveZoneSummaryString();
        stick->getDirectionButton(JoyControlStick::StickLeft)->buildActiveZoneSummaryString();
        stick->getDirectionButton(JoyControlStick::StickRight)->buildActiveZoneSummaryString();

        break;
    }
    }

    QHash<JoyControlStick::JoyStickDirections, JoyButtonSlot *> tempHash;
    tempHash.insert(JoyControlStick::StickUp, upButtonSlot);
    tempHash.insert(JoyControlStick::StickDown, downButtonSlot);
    tempHash.insert(JoyControlStick::StickLeft, leftButtonSlot);
    tempHash.insert(JoyControlStick::StickRight, rightButtonSlot);
    tempHash.insert(JoyControlStick::StickLeftUp, upLeftButtonSlot);
    tempHash.insert(JoyControlStick::StickRightUp, upRightButtonSlot);
    tempHash.insert(JoyControlStick::StickLeftDown, downLeftButtonSlot);
    tempHash.insert(JoyControlStick::StickRightDown, downRightButtonSlot);

    getHelperLocal().setPendingSlots(&tempHash);
    QMetaObject::invokeMethod(&helper, "setFromPendingSlots", Qt::BlockingQueuedConnection);
}

void JoyControlStickEditDialog::refreshStickStats(int x, int y)
{
    Q_UNUSED(x);
    Q_UNUSED(y);

    PadderCommon::inputDaemonMutex.lock();

    QString xCoorString = QString::number(stick->getXCoordinate());
    if (stick->getCircleAdjust() > 0.0)
    {
        xCoorString.append(QString(" (%1)").arg(stick->getCircleXCoordinate()));
    }
    ui->xCoordinateLabel->setText(xCoorString);

    QString yCoorString = QString::number(stick->getYCoordinate());
    if (stick->getCircleAdjust() > 0.0)
    {
        yCoorString.append(QString(" (%1)").arg(stick->getCircleYCoordinate()));
    }
    ui->yCoordinateLabel->setText(yCoorString);

    ui->distanceLabel->setText(QString::number(stick->getAbsoluteRawDistance()));
    ui->diagonalLabel->setText(QString::number(stick->calculateBearing()));

    double validDistance = stick->getDistanceFromDeadZone() * 100.0;
    ui->fromSafeZoneValueLabel->setText(QString::number(validDistance));

    PadderCommon::inputDaemonMutex.unlock();
}

void JoyControlStickEditDialog::checkMaxZone(int value)
{
    if (value > ui->deadZoneSpinBox->value())
    {
        QMetaObject::invokeMethod(stick, "setMaxZone", Q_ARG(int, value));
    }
}

void JoyControlStickEditDialog::implementModes(int index)
{
    PadderCommon::inputDaemonMutex.lock();

    stick->releaseButtonEvents();

    switch (index)
    {
    case 0: {
        stick->setJoyMode(JoyControlStick::StandardMode);
        ui->diagonalRangeSlider->setEnabled(true);
        ui->diagonalRangeSpinBox->setEnabled(true);

        break;
    }
    case 1: {
        stick->setJoyMode(JoyControlStick::EightWayMode);
        ui->diagonalRangeSlider->setEnabled(true);
        ui->diagonalRangeSpinBox->setEnabled(true);

        break;
    }
    case 2: {
        stick->setJoyMode(JoyControlStick::FourWayCardinal);
        ui->diagonalRangeSlider->setEnabled(false);
        ui->diagonalRangeSpinBox->setEnabled(false);

        break;
    }
    case 3: {
        stick->setJoyMode(JoyControlStick::FourWayDiagonal);
        ui->diagonalRangeSlider->setEnabled(false);
        ui->diagonalRangeSpinBox->setEnabled(false);

        break;
    }
    }

    PadderCommon::inputDaemonMutex.unlock();
}

void JoyControlStickEditDialog::selectCurrentPreset()
{
    JoyControlStickButton *upButton = stick->getDirectionButton(JoyControlStick::StickUp);
    QList<JoyButtonSlot *> *upslots = upButton->getAssignedSlots();
    JoyControlStickButton *downButton = stick->getDirectionButton(JoyControlStick::StickDown);
    QList<JoyButtonSlot *> *downslots = downButton->getAssignedSlots();
    JoyControlStickButton *leftButton = stick->getDirectionButton(JoyControlStick::StickLeft);
    QList<JoyButtonSlot *> *leftslots = leftButton->getAssignedSlots();
    JoyControlStickButton *rightButton = stick->getDirectionButton(JoyControlStick::StickRight);
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
            ui->presetsComboBox->setCurrentIndex(1);
        } else if ((upslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (upslot->getSlotCode() == JoyButtonSlot::MouseUp) &&
                   (downslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (downslot->getSlotCode() == JoyButtonSlot::MouseDown) &&
                   (leftslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (leftslot->getSlotCode() == JoyButtonSlot::MouseRight) &&
                   (rightslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (rightslot->getSlotCode() == JoyButtonSlot::MouseLeft))
        {
            ui->presetsComboBox->setCurrentIndex(2);
        } else if ((upslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (upslot->getSlotCode() == JoyButtonSlot::MouseDown) &&
                   (downslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (downslot->getSlotCode() == JoyButtonSlot::MouseUp) &&
                   (leftslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (leftslot->getSlotCode() == JoyButtonSlot::MouseLeft) &&
                   (rightslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (rightslot->getSlotCode() == JoyButtonSlot::MouseRight))
        {
            ui->presetsComboBox->setCurrentIndex(3);
        } else if ((upslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (upslot->getSlotCode() == JoyButtonSlot::MouseDown) &&
                   (downslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (downslot->getSlotCode() == JoyButtonSlot::MouseUp) &&
                   (leftslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (leftslot->getSlotCode() == JoyButtonSlot::MouseRight) &&
                   (rightslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (rightslot->getSlotCode() == JoyButtonSlot::MouseLeft))
        {
            ui->presetsComboBox->setCurrentIndex(4);
        } else if ((upslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (upslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Up)) &&
                   (downslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (downslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Down)) &&
                   (leftslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (leftslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Left)) &&
                   (rightslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (rightslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Right)))
        {
            ui->presetsComboBox->setCurrentIndex(5);
        } else if ((upslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (upslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_W)) &&
                   (downslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (downslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_S)) &&
                   (leftslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (leftslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_A)) &&
                   (rightslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (rightslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_D)))
        {
            ui->presetsComboBox->setCurrentIndex(6);
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
            ui->presetsComboBox->setCurrentIndex(7);
        }
    } else if ((upslots->length() == 0) && (downslots->length() == 0) && (leftslots->length() == 0) &&
               (rightslots->length() == 0))
    {
        ui->presetsComboBox->setCurrentIndex(8);
    }
}

void JoyControlStickEditDialog::updateMouseMode(int index)
{
    PadderCommon::inputDaemonMutex.lock();

    if (index == 1)
    {
        stick->setButtonsMouseMode(JoyButton::MouseCursor);
    } else if (index == 2)
    {
        stick->setButtonsMouseMode(JoyButton::MouseSpring);
    }

    PadderCommon::inputDaemonMutex.unlock();
}

void JoyControlStickEditDialog::openMouseSettingsDialog()
{
    ui->mouseSettingsPushButton->setEnabled(false);

    MouseControlStickSettingsDialog *dialog = new MouseControlStickSettingsDialog(this->stick, this);
    dialog->show();
    connect(this, SIGNAL(finished(int)), dialog, SLOT(close()));
    connect(dialog, SIGNAL(finished(int)), this, SLOT(enableMouseSettingButton()));
}

void JoyControlStickEditDialog::enableMouseSettingButton() { ui->mouseSettingsPushButton->setEnabled(true); }

void JoyControlStickEditDialog::updateWindowTitleStickName()
{
    QString temp = QString(tr("Set")).append(" ");

    if (!stick->getStickName().isEmpty())
    {
        temp.append(stick->getPartialName(false, true));
    } else
    {
        temp.append(stick->getPartialName());
    }

    if (stick->getParentSet()->getIndex() != 0)
    {
        int setIndex = stick->getParentSet()->getRealIndex();
        temp.append(" [").append(tr("Set %1").arg(setIndex));

        QString setName = stick->getParentSet()->getName();
        if (!setName.isEmpty())
        {
            temp.append(": ").append(setName);
        }

        temp.append("]");
    }

    setWindowTitle(temp);
}

void JoyControlStickEditDialog::changeCircleAdjust(int value)
{
    QMetaObject::invokeMethod(stick, "setCircleAdjust", Q_ARG(double, value * 0.01));
}

/**
 * @brief Update QDoubleSpinBox value based on updated stick delay value.
 * @param Delay value obtained from JoyControlStick.
 */
void JoyControlStickEditDialog::updateStickDelaySpinBox(int value)
{
    double temp = value * 0.001;
    ui->stickDelayDoubleSpinBox->setValue(temp);
}

/**
 * @brief Update QSlider value based on value from QDoubleSpinBox.
 * @param Value from QDoubleSpinBox.
 */
void JoyControlStickEditDialog::updateStickDelaySlider(double value)
{
    int temp = value * 100;

    if (ui->stickDelaySlider->value() != temp)
    {
        ui->stickDelaySlider->setValue(temp);
    }
}

void JoyControlStickEditDialog::openModifierEditDialog()
{
    ButtonEditDialog *dialog =
        new ButtonEditDialog(stick->getModifierButton(), stick->getParentSet()->getInputDevice(), keypadUnlocked, this);
    dialog->show();
}

void JoyControlStickEditDialog::changeModifierSummary()
{
    ui->modifierPushButton->setText(stick->getModifierButton()->getSlotsSummary());
}

JoyControlStickEditDialogHelper &JoyControlStickEditDialog::getHelperLocal() { return helper; }
