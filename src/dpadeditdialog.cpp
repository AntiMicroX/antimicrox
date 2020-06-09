/* antimicroX Gamepad to KB+M event mapper
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

#include "dpadeditdialog.h"
#include "ui_dpadeditdialog.h"

#include "messagehandler.h"
#include "joydpad.h"
#include "mousedialog/mousedpadsettingsdialog.h"
#include "event.h"
#include "antkeymapper.h"
#include "setjoystick.h"
#include "inputdevice.h"
#include "common.h"

#include <QDebug>
#include <QHashIterator>
#include <QList>



DPadEditDialog::DPadEditDialog(JoyDPad *dpad, QWidget *parent) :
    QDialog(parent, Qt::Window),
    ui(new Ui::DPadEditDialog),
    helper(dpad)
{
    ui->setupUi(this);

    qInstallMessageHandler(MessageHandler::myMessageOutput);
    setAttribute(Qt::WA_DeleteOnClose);

    this->dpad = dpad;
    getHelperLocal().moveToThread(dpad->thread());

    PadderCommon::inputDaemonMutex.lock();

    updateWindowTitleDPadName();


    switch(dpad->getJoyMode())
    {
        case JoyDPad::StandardMode:
            ui->joyModeComboBox->setCurrentIndex(0);
        break;

        case JoyDPad::EightWayMode:
            ui->joyModeComboBox->setCurrentIndex(1);
        break;

        case JoyDPad::FourWayCardinal:
            ui->joyModeComboBox->setCurrentIndex(2);
        break;

        case JoyDPad::FourWayDiagonal:
            ui->joyModeComboBox->setCurrentIndex(3);
        break;

    }

    selectCurrentPreset();

    ui->dpadNameLineEdit->setText(dpad->getDpadName());

    int dpadDelay = dpad->getDPadDelay();
    ui->dpadDelaySlider->setValue(dpadDelay * .1);
    ui->dpadDelayDoubleSpinBox->setValue(dpadDelay * .001);

    PadderCommon::inputDaemonMutex.unlock();

    connect(ui->presetsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DPadEditDialog::implementPresets);
    connect(ui->joyModeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DPadEditDialog::implementModes);
    connect(ui->mouseSettingsPushButton, &QPushButton::clicked, this, &DPadEditDialog::openMouseSettingsDialog);
    connect(ui->dpadNameLineEdit, &QLineEdit::textEdited, dpad, &JoyDPad::setDPadName);

    connect(ui->dpadDelaySlider, static_cast<void (QSlider::*)(int)>(&QSlider::valueChanged), &helper, &DPadEditDialogHelper::updateJoyDPadDelay);
    connect(dpad, &JoyDPad::dpadDelayChanged, this, &DPadEditDialog::updateDPadDelaySpinBox);
    connect(ui->dpadDelayDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &DPadEditDialog::updateDPadDelaySlider);

    connect(dpad, &JoyDPad::dpadNameChanged, this, &DPadEditDialog::updateWindowTitleDPadName);
}

DPadEditDialog::~DPadEditDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    delete ui;
}

void DPadEditDialog::implementPresets(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButtonSlot *upButtonSlot = nullptr;
    JoyButtonSlot *downButtonSlot = nullptr;
    JoyButtonSlot *leftButtonSlot = nullptr;
    JoyButtonSlot *rightButtonSlot = nullptr;
    JoyButtonSlot *upLeftButtonSlot = nullptr;
    JoyButtonSlot *upRightButtonSlot = nullptr;
    JoyButtonSlot *downLeftButtonSlot = nullptr;
    JoyButtonSlot *downRightButtonSlot = nullptr;

    switch(index)
    {
        case 1:
                PadderCommon::inputDaemonMutex.lock();

                upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
                downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
                leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
                rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);

                PadderCommon::inputDaemonMutex.unlock();

                ui->joyModeComboBox->setCurrentIndex(0);
        break;

        case 2:

                PadderCommon::inputDaemonMutex.lock();

                upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
                downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
                leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
                rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);

                PadderCommon::inputDaemonMutex.unlock();

                ui->joyModeComboBox->setCurrentIndex(0);

        break;

        case 3:

                PadderCommon::inputDaemonMutex.lock();

                upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
                downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
                leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
                rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);

                PadderCommon::inputDaemonMutex.unlock();

                ui->joyModeComboBox->setCurrentIndex(0);

        break;

        case 4:

                PadderCommon::inputDaemonMutex.lock();

                upButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
                downButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
                leftButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
                rightButtonSlot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);

                PadderCommon::inputDaemonMutex.unlock();

                ui->joyModeComboBox->setCurrentIndex(0);

        break;

        case 5:

                PadderCommon::inputDaemonMutex.lock();

                upButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Up), Qt::Key_Up, JoyButtonSlot::JoyKeyboard, this);
                downButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Down), Qt::Key_Down, JoyButtonSlot::JoyKeyboard, this);
                leftButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Left), Qt::Key_Left, JoyButtonSlot::JoyKeyboard, this);
                rightButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Right), Qt::Key_Right, JoyButtonSlot::JoyKeyboard, this);

                PadderCommon::inputDaemonMutex.unlock();

                ui->joyModeComboBox->setCurrentIndex(0);

        break;

        case 6:

                PadderCommon::inputDaemonMutex.lock();

                upButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_W), Qt::Key_W, JoyButtonSlot::JoyKeyboard, this);
                downButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_S), Qt::Key_S, JoyButtonSlot::JoyKeyboard, this);
                leftButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_A), Qt::Key_A, JoyButtonSlot::JoyKeyboard, this);
                rightButtonSlot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_D), Qt::Key_D, JoyButtonSlot::JoyKeyboard, this);

                PadderCommon::inputDaemonMutex.unlock();

                ui->joyModeComboBox->setCurrentIndex(0);

        break;

        case 7:

                PadderCommon::inputDaemonMutex.lock();

                if ((ui->joyModeComboBox->currentIndex() == 0) ||
                    (ui->joyModeComboBox->currentIndex() == 2))
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

        break;

        case 0:
        case 8:

                QMetaObject::invokeMethod(&helper, "clearButtonsSlotsEventReset", Qt::BlockingQueuedConnection);

                dpad->getJoyButton(JoyDPadButton::DpadUp)->buildActiveZoneSummaryString();
                dpad->getJoyButton(JoyDPadButton::DpadDown)->buildActiveZoneSummaryString();
                dpad->getJoyButton(JoyDPadButton::DpadLeft)->buildActiveZoneSummaryString();
                dpad->getJoyButton(JoyDPadButton::DpadRight)->buildActiveZoneSummaryString();

        break;
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

    getHelperLocal().setPendingSlots(&tempHash);
    QMetaObject::invokeMethod(&helper, "setFromPendingSlots", Qt::BlockingQueuedConnection);
}


void DPadEditDialog::implementModes(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    PadderCommon::inputDaemonMutex.lock();

    dpad->releaseButtonEvents();

    switch(index)
    {
        case 0:
            dpad->setJoyMode(JoyDPad::StandardMode);
            break;

        case 1:
            dpad->setJoyMode(JoyDPad::EightWayMode);
            break;

        case 2:
            dpad->setJoyMode(JoyDPad::FourWayCardinal);
            break;

        case 3:
            dpad->setJoyMode(JoyDPad::FourWayDiagonal);
            break;
    }

    PadderCommon::inputDaemonMutex.unlock();
}

void DPadEditDialog::selectCurrentPreset()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyDPadButton *upButton = dpad->getJoyButton(JoyDPadButton::DpadUp);
    QList<JoyButtonSlot*> *upslots = upButton->getAssignedSlots();

    JoyDPadButton *downButton = dpad->getJoyButton(JoyDPadButton::DpadDown);
    QList<JoyButtonSlot*> *downslots = downButton->getAssignedSlots();

    JoyDPadButton *leftButton = dpad->getJoyButton(JoyDPadButton::DpadLeft);
    QList<JoyButtonSlot*> *leftslots = leftButton->getAssignedSlots();

    JoyDPadButton *rightButton = dpad->getJoyButton(JoyDPadButton::DpadRight);
    QList<JoyButtonSlot*> *rightslots = rightButton->getAssignedSlots();

    if ((upslots->length() == 1) && (downslots->length() == 1) && (leftslots->length() == 1) && (rightslots->length() == 1))
    {
        JoyButtonSlot *upslot = upslots->at(0);
        JoyButtonSlot *downslot = downslots->at(0);
        JoyButtonSlot *leftslot = leftslots->at(0);
        JoyButtonSlot *rightslot = rightslots->at(0);

        if ((upslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) && (upslot->getSlotCode() == JoyButtonSlot::MouseUp) &&
            (downslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) && (downslot->getSlotCode() == JoyButtonSlot::MouseDown) &&
            (leftslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) && (leftslot->getSlotCode() == JoyButtonSlot::MouseLeft) &&
            (rightslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) && (rightslot->getSlotCode() == JoyButtonSlot::MouseRight))
        {
            ui->presetsComboBox->setCurrentIndex(1);
        }
        else if ((upslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) && (upslot->getSlotCode() == JoyButtonSlot::MouseUp) &&
                 (downslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) && (downslot->getSlotCode() == JoyButtonSlot::MouseDown) &&
                 (leftslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) && (leftslot->getSlotCode() == JoyButtonSlot::MouseRight) &&
                 (rightslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) && (rightslot->getSlotCode() == JoyButtonSlot::MouseLeft))
        {
            ui->presetsComboBox->setCurrentIndex(2);
        }
        else if ((upslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) && (upslot->getSlotCode() == JoyButtonSlot::MouseDown) &&
                 (downslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) && (downslot->getSlotCode() == JoyButtonSlot::MouseUp) &&
                 (leftslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) && (leftslot->getSlotCode() == JoyButtonSlot::MouseLeft) &&
                 (rightslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) && (rightslot->getSlotCode() == JoyButtonSlot::MouseRight))
        {
            ui->presetsComboBox->setCurrentIndex(3);
        }
        else if ((upslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) && (upslot->getSlotCode() == JoyButtonSlot::MouseDown) &&
                 (downslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) && (downslot->getSlotCode() == JoyButtonSlot::MouseUp) &&
                 (leftslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) && (leftslot->getSlotCode() == JoyButtonSlot::MouseRight) &&
                 (rightslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) && (rightslot->getSlotCode() == JoyButtonSlot::MouseLeft))
        {
            ui->presetsComboBox->setCurrentIndex(4);
        }
        else if ((upslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) && (upslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Up)) &&
                 (downslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) && (downslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Down)) &&
                 (leftslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) && (leftslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Left)) &&
                 (rightslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) && (rightslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Right)))
        {
            ui->presetsComboBox->setCurrentIndex(5);
        }
        else if ((upslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) && (upslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_W)) &&
                 (downslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) && (downslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_S)) &&
                 (leftslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) && (leftslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_A)) &&
                 (rightslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) && (rightslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_D)))
        {
            ui->presetsComboBox->setCurrentIndex(6);
        }
        else if ((upslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) && (upslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_8)) &&
                 (downslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) && (downslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_2)) &&
                 (leftslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) && (leftslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_4)) &&
                 (rightslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) && (rightslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_6)))
        {
            ui->presetsComboBox->setCurrentIndex(7);
        }
    }
    else if ((upslots->length() == 0) && (downslots->length() == 0) &&
             (leftslots->length() == 0) && (rightslots->length() == 0))
    {
        ui->presetsComboBox->setCurrentIndex(8);
    }
}

void DPadEditDialog::openMouseSettingsDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->mouseSettingsPushButton->setEnabled(false);

    MouseDPadSettingsDialog *dialog = new MouseDPadSettingsDialog(this->dpad, this);
    dialog->show();
    connect(this, &DPadEditDialog::finished, dialog, &MouseDPadSettingsDialog::close);
    connect(dialog, &MouseDPadSettingsDialog::finished, this, &DPadEditDialog::enableMouseSettingButton);
}

void DPadEditDialog::enableMouseSettingButton()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->mouseSettingsPushButton->setEnabled(true);
}

/**
 * @brief Update QDoubleSpinBox value based on updated dpad delay value.
 * @param Delay value obtained from JoyDPad.
 */
void DPadEditDialog::updateDPadDelaySpinBox(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    double temp = value * 0.001;
    ui->dpadDelayDoubleSpinBox->setValue(temp);
}

/**
 * @brief Update QSlider value based on value from QDoubleSpinBox.
 * @param Value from QDoubleSpinBox.
 */
void DPadEditDialog::updateDPadDelaySlider(double value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int currentDpadDelay = value * 100;

    if (ui->dpadDelaySlider->value() != currentDpadDelay)
    {
        ui->dpadDelaySlider->setValue(currentDpadDelay);
    }
}

void DPadEditDialog::updateWindowTitleDPadName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString(tr("Set")).append(" ");

    if (!dpad->getDpadName().isEmpty()) temp.append(dpad->getName(false, true));
    else temp.append(dpad->getName());

    if (dpad->getParentSet()->getIndex() != 0)
    {
        int setIndex = dpad->getParentSet()->getRealIndex();
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

JoyDPad* DPadEditDialog::getDPad() const {

    return dpad;
}

DPadEditDialogHelper const& DPadEditDialog::getHelper() {

    return helper;
}

DPadEditDialogHelper& DPadEditDialog::getHelperLocal() {

    return helper;
}
