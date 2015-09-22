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

#include "dpadeditdialog.h"
#include "ui_dpadeditdialog.h"
#include "mousedialog/mousedpadsettingsdialog.h"
#include "event.h"
#include "antkeymapper.h"
#include "setjoystick.h"
#include "inputdevice.h"
#include "common.h"

DPadEditDialog::DPadEditDialog(JoyDPad *dpad, QWidget *parent) :
    QDialog(parent, Qt::Window),
    ui(new Ui::DPadEditDialog),
    helper(dpad)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    this->dpad = dpad;
    helper.moveToThread(dpad->thread());

    PadderCommon::inputDaemonMutex.lock();

    updateWindowTitleDPadName();

    if (dpad->getJoyMode() == JoyDPad::StandardMode)
    {
        ui->joyModeComboBox->setCurrentIndex(0);
    }
    else if (dpad->getJoyMode() == JoyDPad::EightWayMode)
    {
        ui->joyModeComboBox->setCurrentIndex(1);
    }
    else if (dpad->getJoyMode() == JoyDPad::FourWayCardinal)
    {
        ui->joyModeComboBox->setCurrentIndex(2);
    }
    else if (dpad->getJoyMode() == JoyDPad::FourWayDiagonal)
    {
        ui->joyModeComboBox->setCurrentIndex(3);
    }

    selectCurrentPreset();

    ui->dpadNameLineEdit->setText(dpad->getDpadName());

    unsigned int dpadDelay = dpad->getDPadDelay();
    ui->dpadDelaySlider->setValue(dpadDelay * .1);
    ui->dpadDelayDoubleSpinBox->setValue(dpadDelay * .001);

    PadderCommon::inputDaemonMutex.unlock();

    connect(ui->presetsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(implementPresets(int)));
    connect(ui->joyModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(implementModes(int)));
    connect(ui->mouseSettingsPushButton, SIGNAL(clicked()), this, SLOT(openMouseSettingsDialog()));
    connect(ui->dpadNameLineEdit, SIGNAL(textEdited(QString)), dpad, SLOT(setDPadName(QString)));

    connect(ui->dpadDelaySlider, SIGNAL(valueChanged(int)), &helper, SLOT(updateJoyDPadDelay(int)));
    connect(dpad, SIGNAL(dpadDelayChanged(int)), this, SLOT(updateDPadDelaySpinBox(int)));
    connect(ui->dpadDelayDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateDPadDelaySlider(double)));

    connect(dpad, SIGNAL(dpadNameChanged()), this, SLOT(updateWindowTitleDPadName()));
}

DPadEditDialog::~DPadEditDialog()
{
    delete ui;
}

void DPadEditDialog::implementPresets(int index)
{
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
        PadderCommon::inputDaemonMutex.lock();

        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);

        PadderCommon::inputDaemonMutex.unlock();

        ui->joyModeComboBox->setCurrentIndex(0);
    }
    else if (index == 2)
    {
        PadderCommon::inputDaemonMutex.lock();

        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);

        PadderCommon::inputDaemonMutex.unlock();

        ui->joyModeComboBox->setCurrentIndex(0);
    }
    else if (index == 3)
    {
        PadderCommon::inputDaemonMutex.lock();

        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);

        PadderCommon::inputDaemonMutex.unlock();

        ui->joyModeComboBox->setCurrentIndex(0);
    }
    else if (index == 4)
    {
        PadderCommon::inputDaemonMutex.lock();

        upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);

        PadderCommon::inputDaemonMutex.unlock();

        ui->joyModeComboBox->setCurrentIndex(0);
    }
    else if (index == 5)
    {
        PadderCommon::inputDaemonMutex.lock();

        upButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Up), Qt::Key_Up, JoyButtonSlot::JoyKeyboard, this);
        downButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Down), Qt::Key_Down, JoyButtonSlot::JoyKeyboard, this);
        leftButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Left), Qt::Key_Left, JoyButtonSlot::JoyKeyboard, this);
        rightButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Right), Qt::Key_Right, JoyButtonSlot::JoyKeyboard, this);

        PadderCommon::inputDaemonMutex.unlock();

        ui->joyModeComboBox->setCurrentIndex(0);
    }
    else if (index == 6)
    {
        PadderCommon::inputDaemonMutex.lock();

        upButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_W), Qt::Key_W, JoyButtonSlot::JoyKeyboard, this);
        downButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_S), Qt::Key_S, JoyButtonSlot::JoyKeyboard, this);
        leftButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_A), Qt::Key_A, JoyButtonSlot::JoyKeyboard, this);
        rightButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_D), Qt::Key_D, JoyButtonSlot::JoyKeyboard, this);

        PadderCommon::inputDaemonMutex.unlock();

        ui->joyModeComboBox->setCurrentIndex(0);
    }
    else if (index == 7)
    {
        PadderCommon::inputDaemonMutex.lock();

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

        PadderCommon::inputDaemonMutex.unlock();
    }
    else if (index == 8)
    {
        QMetaObject::invokeMethod(&helper, "clearButtonsSlotsEventReset", Qt::BlockingQueuedConnection);
    }


    QHash<JoyDPadButton::JoyDPadDirections, JoyButtonSlot*> tempHash;
    tempHash.insert(JoyDPadButton::DpadUp, upButtonSlot);
    tempHash.insert(JoyDPadButton::DpadDown, downButtonSlot);
    tempHash.insert(JoyDPadButton::DpadLeft, leftButtonSlot);
    tempHash.insert(JoyDPadButton::DpadRight, rightButtonSlot);
    tempHash.insert(JoyDPadButton::DpadLeftUp, upLeftButtonSlot);
    tempHash.insert(JoyDPadButton::DpadRightUp, upRightButtonSlot);
    tempHash.insert(JoyDPadButton::DpadLeftDown, downLeftButtonSlot);
    tempHash.insert(JoyDPadButton::DpadRightDown, downRightButtonSlot);

    helper.setPendingSlots(&tempHash);
    QMetaObject::invokeMethod(&helper, "setFromPendingSlots", Qt::BlockingQueuedConnection);
}

void DPadEditDialog::implementModes(int index)
{
    PadderCommon::inputDaemonMutex.lock();

    dpad->releaseButtonEvents();

    if (index == 0)
    {
        dpad->setJoyMode(JoyDPad::StandardMode);
    }
    else if (index == 1)
    {
        dpad->setJoyMode(JoyDPad::EightWayMode);
    }
    else if (index == 2)
    {
        dpad->setJoyMode(JoyDPad::FourWayCardinal);
    }
    else if (index == 3)
    {
        dpad->setJoyMode(JoyDPad::FourWayDiagonal);
    }

    PadderCommon::inputDaemonMutex.unlock();
}

void DPadEditDialog::selectCurrentPreset()
{
    JoyDPadButton *upButton = dpad->getJoyButton(JoyDPadButton::DpadUp);
    QList<JoyButtonSlot*> *upslots = upButton->getAssignedSlots();
    JoyDPadButton *downButton = dpad->getJoyButton(JoyDPadButton::DpadDown);
    QList<JoyButtonSlot*> *downslots = downButton->getAssignedSlots();
    JoyDPadButton *leftButton = dpad->getJoyButton(JoyDPadButton::DpadLeft);
    QList<JoyButtonSlot*> *leftslots = leftButton->getAssignedSlots();
    JoyDPadButton *rightButton = dpad->getJoyButton(JoyDPadButton::DpadRight);
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
        else if (upslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && static_cast<unsigned int>(upslot->getSlotCode()) == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Up) &&
                 downslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && static_cast<unsigned int>(downslot->getSlotCode()) == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Down) &&
                 leftslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && static_cast<unsigned int>(leftslot->getSlotCode()) == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Left) &&
                 rightslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && static_cast<unsigned int>(rightslot->getSlotCode()) == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Right))
        {
            ui->presetsComboBox->setCurrentIndex(5);
        }
        else if (upslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && static_cast<unsigned int>(upslot->getSlotCode()) == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_W) &&
                 downslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && static_cast<unsigned int>(downslot->getSlotCode()) == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_S) &&
                 leftslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && static_cast<unsigned int>(leftslot->getSlotCode()) == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_A) &&
                 rightslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && static_cast<unsigned int>(rightslot->getSlotCode()) == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_D))
        {
            ui->presetsComboBox->setCurrentIndex(6);
        }
        else if (upslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && static_cast<unsigned int>(upslot->getSlotCode()) == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_8) &&
                 downslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && static_cast<unsigned int>(downslot->getSlotCode()) == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_2) &&
                 leftslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && static_cast<unsigned int>(leftslot->getSlotCode()) == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_4) &&
                 rightslot->getSlotMode() == JoyButtonSlot::JoyKeyboard && static_cast<unsigned int>(rightslot->getSlotCode()) == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_6))
        {
            ui->presetsComboBox->setCurrentIndex(7);
        }
    }
    else if (upslots->length() == 0 && downslots->length() == 0 &&
             leftslots->length() == 0 && rightslots->length() == 0)
    {
        ui->presetsComboBox->setCurrentIndex(8);
    }
}

void DPadEditDialog::openMouseSettingsDialog()
{
    ui->mouseSettingsPushButton->setEnabled(false);

    MouseDPadSettingsDialog *dialog = new MouseDPadSettingsDialog(this->dpad, this);
    dialog->show();
    connect(this, SIGNAL(finished(int)), dialog, SLOT(close()));
    connect(dialog, SIGNAL(finished(int)), this, SLOT(enableMouseSettingButton()));
}

void DPadEditDialog::enableMouseSettingButton()
{
    ui->mouseSettingsPushButton->setEnabled(true);
}

/**
 * @brief Update QDoubleSpinBox value based on updated dpad delay value.
 * @param Delay value obtained from JoyDPad.
 */
void DPadEditDialog::updateDPadDelaySpinBox(int value)
{
    double temp = static_cast<double>(value * 0.001);
    ui->dpadDelayDoubleSpinBox->setValue(temp);
}

/**
 * @brief Update QSlider value based on value from QDoubleSpinBox.
 * @param Value from QDoubleSpinBox.
 */
void DPadEditDialog::updateDPadDelaySlider(double value)
{
    int temp = static_cast<int>(value * 100);
    if (ui->dpadDelaySlider->value() != temp)
    {
        ui->dpadDelaySlider->setValue(temp);
    }
}

void DPadEditDialog::updateWindowTitleDPadName()
{
    QString temp = QString(tr("Set")).append(" ");

    if (!dpad->getDpadName().isEmpty())
    {
        temp.append(dpad->getName(false, true));
    }
    else
    {
        temp.append(dpad->getName());
    }

    if (dpad->getParentSet()->getIndex() != 0)
    {
        unsigned int setIndex = dpad->getParentSet()->getRealIndex();
        temp.append(" [").append(tr("Set %1").arg(setIndex));

        QString setName = dpad->getParentSet()->getName();
        if (!setName.isEmpty())
        {
            temp.append(": ").append(setName);
        }

        temp.append("]");
    }

    setWindowTitle(temp);
}
