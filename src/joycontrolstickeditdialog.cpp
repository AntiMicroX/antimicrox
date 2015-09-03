/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
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

//#include <QDebug>
#include <QHashIterator>
#include <QList>

#include "joycontrolstickeditdialog.h"
#include "ui_joycontrolstickeditdialog.h"
#include "mousedialog/mousecontrolsticksettingsdialog.h"
#include "event.h"
#include "antkeymapper.h"
#include "setjoystick.h"
#include "buttoneditdialog.h"
#include "inputdevice.h"
#include "common.h"

JoyControlStickEditDialog::JoyControlStickEditDialog(JoyControlStick *stick, QWidget *parent) :
    QDialog(parent, Qt::Window),
    ui(new Ui::JoyControlStickEditDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    this->stick = stick;

    updateWindowTitleStickName();

    ui->deadZoneSlider->setValue(stick->getDeadZone());
    ui->deadZoneSpinBox->setValue(stick->getDeadZone());

    ui->maxZoneSlider->setValue(stick->getMaxZone());
    ui->maxZoneSpinBox->setValue(stick->getMaxZone());

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

    if (stick->getJoyMode() == JoyControlStick::StandardMode)
    {
        ui->joyModeComboBox->setCurrentIndex(0);
    }
    else if (stick->getJoyMode() == JoyControlStick::EightWayMode)
    {
        ui->joyModeComboBox->setCurrentIndex(1);
    }
    else if (stick->getJoyMode() == JoyControlStick::FourWayCardinal)
    {
        ui->joyModeComboBox->setCurrentIndex(2);
        ui->diagonalRangeSlider->setEnabled(false);
        ui->diagonalRangeSpinBox->setEnabled(false);
    }
    else if (stick->getJoyMode() == JoyControlStick::FourWayDiagonal)
    {
        ui->joyModeComboBox->setCurrentIndex(3);
        ui->diagonalRangeSlider->setEnabled(false);
        ui->diagonalRangeSpinBox->setEnabled(false);
    }

    ui->stickStatusBoxWidget->setStick(stick);

    selectCurrentPreset();

    ui->stickNameLineEdit->setText(stick->getStickName());
    double validDistance = stick->getDistanceFromDeadZone() * 100.0;
    ui->fromSafeZoneValueLabel->setText(QString::number(validDistance));

    double circleValue = stick->getCircleAdjust();
    ui->squareStickSlider->setValue(circleValue * 100);
    ui->squareStickSpinBox->setValue(circleValue * 100);

    unsigned int stickDelay = stick->getStickDelay();
    ui->stickDelaySlider->setValue(stickDelay * .1);
    ui->stickDelayDoubleSpinBox->setValue(stickDelay * .001);

    ui->modifierPushButton->setText(stick->getModifierButton()->getSlotsSummary());
    stick->getModifierButton()->establishPropertyUpdatedConnections();

    connect(ui->presetsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(implementPresets(int)));
    connect(ui->joyModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(implementModes(int)));

    connect(ui->deadZoneSlider, SIGNAL(valueChanged(int)), ui->deadZoneSpinBox, SLOT(setValue(int)));
    connect(ui->maxZoneSlider, SIGNAL(valueChanged(int)), ui->maxZoneSpinBox, SLOT(setValue(int)));
    connect(ui->diagonalRangeSlider, SIGNAL(valueChanged(int)), ui->diagonalRangeSpinBox, SLOT(setValue(int)));
    connect(ui->squareStickSlider, SIGNAL(valueChanged(int)), ui->squareStickSpinBox, SLOT(setValue(int)));

    connect(ui->deadZoneSpinBox, SIGNAL(valueChanged(int)), ui->deadZoneSlider, SLOT(setValue(int)));
    connect(ui->maxZoneSpinBox, SIGNAL(valueChanged(int)), ui->maxZoneSlider, SLOT(setValue(int)));
    connect(ui->maxZoneSpinBox, SIGNAL(valueChanged(int)), this, SLOT(checkMaxZone(int)));
    connect(ui->diagonalRangeSpinBox, SIGNAL(valueChanged(int)), ui->diagonalRangeSlider, SLOT(setValue(int)));
    connect(ui->squareStickSpinBox, SIGNAL(valueChanged(int)), ui->squareStickSlider, SLOT(setValue(int)));
    connect(ui->stickDelaySlider, SIGNAL(valueChanged(int)), this, SLOT(updateControlStickDelay(int)));

    connect(ui->deadZoneSpinBox, SIGNAL(valueChanged(int)), stick, SLOT(setDeadZone(int)));
    connect(ui->diagonalRangeSpinBox, SIGNAL(valueChanged(int)), stick, SLOT(setDiagonalRange(int)));
    connect(ui->squareStickSpinBox, SIGNAL(valueChanged(int)), this, SLOT(changeCircleAdjust(int)));
    connect(stick, SIGNAL(stickDelayChanged(int)), this, SLOT(updateStickDelaySpinBox(int)));
    connect(ui->stickDelayDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateStickDelaySlider(double)));

    connect(stick, SIGNAL(moved(int,int)), this, SLOT(refreshStickStats(int,int)));
    connect(ui->mouseSettingsPushButton, SIGNAL(clicked()), this, SLOT(openMouseSettingsDialog()));

    connect(ui->stickNameLineEdit, SIGNAL(textEdited(QString)), stick, SLOT(setStickName(QString)));
    connect(stick, SIGNAL(stickNameChanged()), this, SLOT(updateWindowTitleStickName()));
    connect(ui->modifierPushButton, SIGNAL(clicked()), this, SLOT(openModifierEditDialog()));
    connect(stick->getModifierButton(), SIGNAL(slotsChanged()), this, SLOT(changeModifierSummary()));
}

JoyControlStickEditDialog::~JoyControlStickEditDialog()
{
    delete ui;
}

void JoyControlStickEditDialog::implementPresets(int index)
{
    PadderCommon::lockInputDevices();

    InputDevice *tempDevice = stick->getParentSet()->getInputDevice();
    QMetaObject::invokeMethod(tempDevice, "haltServices", Qt::BlockingQueuedConnection);

    JoyButtonSlot *upButtonSlot = 0;
    JoyButtonSlot *downButtonSlot = 0;
    JoyButtonSlot *leftButtonSlot = 0;
    JoyButtonSlot *rightButtonSlot = 0;
    JoyButtonSlot *upLeftButtonSlot = 0;
    JoyButtonSlot *upRightButtonSlot = 0;
    JoyButtonSlot *downLeftButtonSlot = 0;
    JoyButtonSlot *downRightButtonSlot = 0;

    if (index == 1)
    {
        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        ui->joyModeComboBox->setCurrentIndex(0);
        ui->diagonalRangeSlider->setValue(65);
    }
    else if (index == 2)
    {
        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        ui->joyModeComboBox->setCurrentIndex(0);
        ui->diagonalRangeSlider->setValue(65);
    }
    else if (index == 3)
    {
        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        ui->joyModeComboBox->setCurrentIndex(0);
        ui->diagonalRangeSlider->setValue(65);
    }
    else if (index == 4)
    {
        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        ui->joyModeComboBox->setCurrentIndex(0);
        ui->diagonalRangeSlider->setValue(65);
    }
    else if (index == 5)
    {
        upButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Up), Qt::Key_Up, JoyButtonSlot::JoyKeyboard, this);
        downButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Down), Qt::Key_Down, JoyButtonSlot::JoyKeyboard, this);
        leftButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Left), Qt::Key_Left, JoyButtonSlot::JoyKeyboard, this);
        rightButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Right), Qt::Key_Right, JoyButtonSlot::JoyKeyboard, this);
        ui->joyModeComboBox->setCurrentIndex(0);
        ui->diagonalRangeSlider->setValue(45);
    }
    else if (index == 6)
    {
        upButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_W), Qt::Key_W, JoyButtonSlot::JoyKeyboard, this);
        downButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_S), Qt::Key_S, JoyButtonSlot::JoyKeyboard, this);
        leftButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_A), Qt::Key_A, JoyButtonSlot::JoyKeyboard, this);
        rightButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_D), Qt::Key_D, JoyButtonSlot::JoyKeyboard, this);
        ui->joyModeComboBox->setCurrentIndex(0);
        ui->diagonalRangeSlider->setValue(45);
    }
    else if (index == 7)
    {
        if (ui->joyModeComboBox->currentIndex() == 0 ||
            ui->joyModeComboBox->currentIndex() == 2)
        {
            upButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_8), QtKeyMapperBase::AntKey_KP_8, JoyButtonSlot::JoyKeyboard, this);
            downButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_2), QtKeyMapperBase::AntKey_KP_2, JoyButtonSlot::JoyKeyboard, this);
            leftButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_4), QtKeyMapperBase::AntKey_KP_4, JoyButtonSlot::JoyKeyboard, this);
            rightButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_6), QtKeyMapperBase::AntKey_KP_6, JoyButtonSlot::JoyKeyboard, this);
        }
        else if (ui->joyModeComboBox->currentIndex() == 1)
        {
            upButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_8), QtKeyMapperBase::AntKey_KP_8, JoyButtonSlot::JoyKeyboard, this);
            downButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_2), QtKeyMapperBase::AntKey_KP_2, JoyButtonSlot::JoyKeyboard, this);
            leftButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_4), QtKeyMapperBase::AntKey_KP_4, JoyButtonSlot::JoyKeyboard, this);
            rightButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_6), QtKeyMapperBase::AntKey_KP_6, JoyButtonSlot::JoyKeyboard, this);

            upLeftButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_7), QtKeyMapperBase::AntKey_KP_7, JoyButtonSlot::JoyKeyboard, this);
            upRightButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_9), QtKeyMapperBase::AntKey_KP_9, JoyButtonSlot::JoyKeyboard, this);
            downLeftButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_1), QtKeyMapperBase::AntKey_KP_1, JoyButtonSlot::JoyKeyboard, this);
            downRightButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_3), QtKeyMapperBase::AntKey_KP_3, JoyButtonSlot::JoyKeyboard, this);
        }
        else if (ui->joyModeComboBox->currentIndex() == 3)
        {
            upLeftButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_7), QtKeyMapperBase::AntKey_KP_7, JoyButtonSlot::JoyKeyboard, this);
            upRightButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_9), QtKeyMapperBase::AntKey_KP_9, JoyButtonSlot::JoyKeyboard, this);
            downLeftButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_1), QtKeyMapperBase::AntKey_KP_1, JoyButtonSlot::JoyKeyboard, this);
            downRightButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_3), QtKeyMapperBase::AntKey_KP_3, JoyButtonSlot::JoyKeyboard, this);
        }

        ui->diagonalRangeSlider->setValue(45);
    }
    else if (index == 8)
    {
        QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton*> *buttons = stick->getButtons();
        QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> iter(*buttons);
        while (iter.hasNext())
        {
            JoyControlStickButton *button = iter.next().value();
            QMetaObject::invokeMethod(button, "clearSlotsEventReset", Qt::BlockingQueuedConnection);
            //button->clearSlotsEventReset();
        }

        ui->diagonalRangeSlider->setValue(45);
    }

    if (upButtonSlot)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::StickUp);
        QMetaObject::invokeMethod(button, "clearSlotsEventReset",
                                  Q_ARG(bool, false));
        //button->clearSlotsEventReset(false);
        QMetaObject::invokeMethod(button, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, upButtonSlot->getSlotCode()),
                                  Q_ARG(unsigned int, upButtonSlot->getSlotCodeAlias()),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, upButtonSlot->getSlotMode()));
        //button->setAssignedSlot(upButtonSlot->getSlotCode(), upButtonSlot->getSlotCodeAlias(), upButtonSlot->getSlotMode());
        upButtonSlot->deleteLater();
    }

    if (downButtonSlot)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::StickDown);
        QMetaObject::invokeMethod(button, "clearSlotsEventReset",
                                  Q_ARG(bool, false));
        //button->clearSlotsEventReset(false);
        QMetaObject::invokeMethod(button, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, downButtonSlot->getSlotCode()),
                                  Q_ARG(unsigned int, downButtonSlot->getSlotCodeAlias()),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, downButtonSlot->getSlotMode()));
        //button->setAssignedSlot(downButtonSlot->getSlotCode(), downButtonSlot->getSlotCodeAlias(), downButtonSlot->getSlotMode());
        downButtonSlot->deleteLater();
    }

    if (leftButtonSlot)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::StickLeft);
        QMetaObject::invokeMethod(button, "clearSlotsEventReset",
                                  Q_ARG(bool, false));
        //button->clearSlotsEventReset(false);
        QMetaObject::invokeMethod(button, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, leftButtonSlot->getSlotCode()),
                                  Q_ARG(unsigned int, leftButtonSlot->getSlotCodeAlias()),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, leftButtonSlot->getSlotMode()));
        //button->setAssignedSlot(leftButtonSlot->getSlotCode(), leftButtonSlot->getSlotCodeAlias(), leftButtonSlot->getSlotMode());
        leftButtonSlot->deleteLater();
    }

    if (rightButtonSlot)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::StickRight);
        QMetaObject::invokeMethod(button, "clearSlotsEventReset",
                                  Q_ARG(bool, false));
        //button->clearSlotsEventReset(false);
        QMetaObject::invokeMethod(button, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, rightButtonSlot->getSlotCode()),
                                  Q_ARG(unsigned int, rightButtonSlot->getSlotCodeAlias()),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, rightButtonSlot->getSlotMode()));
        //button->setAssignedSlot(rightButtonSlot->getSlotCode(), rightButtonSlot->getSlotCodeAlias(), rightButtonSlot->getSlotMode());
        rightButtonSlot->deleteLater();
    }

    if (upLeftButtonSlot)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::StickLeftUp);
        QMetaObject::invokeMethod(button, "clearSlotsEventReset",
                                  Q_ARG(bool, false));
        //button->clearSlotsEventReset(false);
        QMetaObject::invokeMethod(button, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, upLeftButtonSlot->getSlotCode()),
                                  Q_ARG(unsigned int, upLeftButtonSlot->getSlotCodeAlias()),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, upLeftButtonSlot->getSlotMode()));
        //button->setAssignedSlot(upLeftButtonSlot->getSlotCode(), upLeftButtonSlot->getSlotCodeAlias(), upLeftButtonSlot->getSlotMode());
        upLeftButtonSlot->deleteLater();
    }

    if (upRightButtonSlot)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::StickRightUp);
        QMetaObject::invokeMethod(button, "clearSlotsEventReset",
                                  Q_ARG(bool, false));
        //button->clearSlotsEventReset(false);
        QMetaObject::invokeMethod(button, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, upRightButtonSlot->getSlotCode()),
                                  Q_ARG(unsigned int, upRightButtonSlot->getSlotCodeAlias()),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, upRightButtonSlot->getSlotMode()));
        //button->setAssignedSlot(upRightButtonSlot->getSlotCode(), upRightButtonSlot->getSlotCodeAlias(), upRightButtonSlot->getSlotMode());
        upRightButtonSlot->deleteLater();
    }

    if (downLeftButtonSlot)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::StickLeftDown);
        QMetaObject::invokeMethod(button, "clearSlotsEventReset",
                                  Q_ARG(bool, false));
        //button->clearSlotsEventReset(false);
        QMetaObject::invokeMethod(button, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, downLeftButtonSlot->getSlotCode()),
                                  Q_ARG(unsigned int, downLeftButtonSlot->getSlotCodeAlias()),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, downLeftButtonSlot->getSlotMode()));
        //button->setAssignedSlot(downLeftButtonSlot->getSlotCode(), downLeftButtonSlot->getSlotCodeAlias(), downLeftButtonSlot->getSlotMode());
        downLeftButtonSlot->deleteLater();
    }

    if (downRightButtonSlot)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::StickRightDown);
        QMetaObject::invokeMethod(button, "clearSlotsEventReset",
                                  Q_ARG(bool, false));
        //button->clearSlotsEventReset(false);
        QMetaObject::invokeMethod(button, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, downRightButtonSlot->getSlotCode()),
                                  Q_ARG(unsigned int, downRightButtonSlot->getSlotCodeAlias()),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, downRightButtonSlot->getSlotMode()));
        //button->setAssignedSlot(downRightButtonSlot->getSlotCode(), downRightButtonSlot->getSlotCodeAlias(), downRightButtonSlot->getSlotMode());
        downRightButtonSlot->deleteLater();
    }

    PadderCommon::unlockInputDevices();
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
        stick->setMaxZone(value);
    }
}

void JoyControlStickEditDialog::implementModes(int index)
{
    stick->releaseButtonEvents();

    if (index == 0)
    {
        stick->setJoyMode(JoyControlStick::StandardMode);
        ui->diagonalRangeSlider->setEnabled(true);
        ui->diagonalRangeSpinBox->setEnabled(true);
    }
    else if (index == 1)
    {
        stick->setJoyMode(JoyControlStick::EightWayMode);
        ui->diagonalRangeSlider->setEnabled(true);
        ui->diagonalRangeSpinBox->setEnabled(true);
    }
    else if (index == 2)
    {
        stick->setJoyMode(JoyControlStick::FourWayCardinal);
        ui->diagonalRangeSlider->setEnabled(false);
        ui->diagonalRangeSpinBox->setEnabled(false);
    }
    else if (index == 3)
    {
        stick->setJoyMode(JoyControlStick::FourWayDiagonal);
        ui->diagonalRangeSlider->setEnabled(false);
        ui->diagonalRangeSpinBox->setEnabled(false);
    }
}

void JoyControlStickEditDialog::selectCurrentPreset()
{
    JoyControlStickButton *upButton = stick->getDirectionButton(JoyControlStick::StickUp);
    QList<JoyButtonSlot*> *upslots = upButton->getAssignedSlots();
    JoyControlStickButton *downButton = stick->getDirectionButton(JoyControlStick::StickDown);
    QList<JoyButtonSlot*> *downslots = downButton->getAssignedSlots();
    JoyControlStickButton *leftButton = stick->getDirectionButton(JoyControlStick::StickLeft);
    QList<JoyButtonSlot*> *leftslots = leftButton->getAssignedSlots();
    JoyControlStickButton *rightButton = stick->getDirectionButton(JoyControlStick::StickRight);
    QList<JoyButtonSlot*> *rightslots = rightButton->getAssignedSlots();

    if (upslots->length() == 1 && downslots->length() == 1 && leftslots->length() == 1 && rightslots->length() == 1)
    {
        JoyButtonSlot *upslot = upslots->at(0);
        JoyButtonSlot *downslot = downslots->at(0);
        JoyButtonSlot *leftslot = leftslots->at(0);
        JoyButtonSlot *rightslot = rightslots->at(0);

        if (upslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && upslot->getSlotCode() == JoyButtonSlot::MouseUp &&
            downslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && downslot->getSlotCode() == JoyButtonSlot::MouseDown &&
            leftslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && leftslot->getSlotCode() == JoyButtonSlot::MouseLeft &&
            rightslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && rightslot->getSlotCode() == JoyButtonSlot::MouseRight)
        {
            ui->presetsComboBox->setCurrentIndex(1);
        }
        else if (upslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && upslot->getSlotCode() == JoyButtonSlot::MouseUp &&
                downslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && downslot->getSlotCode() == JoyButtonSlot::MouseDown &&
                leftslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && leftslot->getSlotCode() == JoyButtonSlot::MouseRight &&
                rightslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && rightslot->getSlotCode() == JoyButtonSlot::MouseLeft)
        {
            ui->presetsComboBox->setCurrentIndex(2);
        }
        else if (upslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && upslot->getSlotCode() == JoyButtonSlot::MouseDown &&
                downslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && downslot->getSlotCode() == JoyButtonSlot::MouseUp &&
                leftslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && leftslot->getSlotCode() == JoyButtonSlot::MouseLeft &&
                rightslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && rightslot->getSlotCode() == JoyButtonSlot::MouseRight)
        {
            ui->presetsComboBox->setCurrentIndex(3);
        }
        else if (upslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && upslot->getSlotCode() == JoyButtonSlot::MouseDown &&
                downslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && downslot->getSlotCode() == JoyButtonSlot::MouseUp &&
                leftslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && leftslot->getSlotCode() == JoyButtonSlot::MouseRight &&
                rightslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement && rightslot->getSlotCode() == JoyButtonSlot::MouseLeft)
        {
            ui->presetsComboBox->setCurrentIndex(4);
        }
        else if (upslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)upslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Up) &&
                 downslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)downslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Down) &&
                 leftslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)leftslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Left) &&
                 rightslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)rightslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Right))
        {
            ui->presetsComboBox->setCurrentIndex(5);
        }
        else if (upslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)upslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_W) &&
                 downslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)downslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_S) &&
                 leftslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)leftslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_A) &&
                 rightslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)rightslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_D))
        {
            ui->presetsComboBox->setCurrentIndex(6);
        }
        else if (upslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)upslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_8) &&
                 downslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)downslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_2) &&
                 leftslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)leftslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_4) &&
                 rightslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && (unsigned int)rightslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_6))
        {
            ui->presetsComboBox->setCurrentIndex(7);
        }
    }
    else if (upslots->length() == 0 && downslots->length() == 0 && leftslots->length() == 0 && rightslots->length() == 0)
    {
        ui->presetsComboBox->setCurrentIndex(8);
    }
}

void JoyControlStickEditDialog::updateMouseMode(int index)
{
    if (index == 1)
    {
        stick->setButtonsMouseMode(JoyButton::MouseCursor);
    }
    else if (index == 2)
    {
        stick->setButtonsMouseMode(JoyButton::MouseSpring);
    }
}

void JoyControlStickEditDialog::openMouseSettingsDialog()
{
    ui->mouseSettingsPushButton->setEnabled(false);

    MouseControlStickSettingsDialog *dialog = new MouseControlStickSettingsDialog(this->stick, this);
    dialog->show();
    connect(this, SIGNAL(finished(int)), dialog, SLOT(close()));
    connect(dialog, SIGNAL(finished(int)), this, SLOT(enableMouseSettingButton()));
}

void JoyControlStickEditDialog::enableMouseSettingButton()
{
    ui->mouseSettingsPushButton->setEnabled(true);
}

void JoyControlStickEditDialog::updateWindowTitleStickName()
{
    QString temp = QString(tr("Set")).append(" ");

    if (!stick->getStickName().isEmpty())
    {
        temp.append(stick->getPartialName(false, true));
    }
    else
    {
        temp.append(stick->getPartialName());
    }

    if (stick->getParentSet()->getIndex() != 0)
    {
        unsigned int setIndex = stick->getParentSet()->getRealIndex();
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
    stick->setCircleAdjust(value * .01);
}

/**
 * @brief Update QDoubleSpinBox value based on updated stick delay value.
 * @param Delay value obtained from JoyControlStick.
 */
void JoyControlStickEditDialog::updateStickDelaySpinBox(int value)
{
    double temp = static_cast<double>(value * 0.001);
    ui->stickDelayDoubleSpinBox->setValue(temp);
}

/**
 * @brief Update QSlider value based on value from QDoubleSpinBox.
 * @param Value from QDoubleSpinBox.
 */
void JoyControlStickEditDialog::updateStickDelaySlider(double value)
{
    int temp = static_cast<int>(value * 100);
    if (ui->stickDelaySlider->value() != temp)
    {
        ui->stickDelaySlider->setValue(temp);
    }
}

/**
 * @brief Update JoyControlStick delay value based on QSlider value.
 * @param Value from QSlider.
 */
void JoyControlStickEditDialog::updateControlStickDelay(int value)
{
    int temp = value * 10;
    if (stick->getStickDelay() != temp)
    {
        stick->setStickDelay(temp);
    }
}

void JoyControlStickEditDialog::openModifierEditDialog()
{
    ButtonEditDialog *dialog = new ButtonEditDialog(stick->getModifierButton(), this);
    dialog->show();
}

void JoyControlStickEditDialog::changeModifierSummary()
{
    ui->modifierPushButton->setText(stick->getModifierButton()->getSlotsSummary());
}
