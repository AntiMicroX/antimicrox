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

#include "axiseditdialog.h"
#include "ui_axiseditdialog.h"

#include "antkeymapper.h"
#include "axisvaluebox.h"
#include "buttoneditdialog.h"
#include "common.h"
#include "event.h"
#include "haptictriggerps5.h"
#include "inputdevice.h"
#include "joyaxis.h"
#include "joycontrolstick.h"
#include "mousedialog/mouseaxissettingsdialog.h"
#include "setaxisthrottledialog.h"
#include "setjoystick.h"

#include <QDebug>
#include <QList>

AxisEditDialog::AxisEditDialog(JoyAxis *axis, bool keypadUnlocked, QWidget *parent)
    : QDialog(parent, Qt::Window)
    , ui(new Ui::AxisEditDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    setAxisThrottleConfirm = new SetAxisThrottleDialog(axis, this);
    m_axis = axis;
    this->keypadUnlocked = keypadUnlocked;

    updateWindowTitleAxisName();

    initialThrottleState = axis->getThrottle();
    bool actAsTrigger = false;

    if ((initialThrottleState == static_cast<int>(JoyAxis::PositiveThrottle)) ||
        (initialThrottleState == static_cast<int>(JoyAxis::PositiveHalfThrottle)))
    {
        actAsTrigger = true;
    }

    if (actAsTrigger)
        buildTriggerPresetsMenu();

    if (axis->hasHapticTrigger())
    {
        buildHapticTriggerMenu();
        selectHapticTrigger();
    } else
    {
        ui->hapticTriggerLabel->setVisible(false);
        ui->hapticTriggerComboBox->setVisible(false);
    }

    ui->deadZoneSlider->setValue(axis->getDeadZone());
    ui->deadZoneSpinBox->setValue(axis->getDeadZone());

    ui->maxZoneSlider->setValue(axis->getMaxZoneValue());
    ui->maxZoneSpinBox->setValue(axis->getMaxZoneValue());

    JoyAxisButton *nButton = axis->getNAxisButton();

    if (!nButton->getActionName().isEmpty())
    {
        ui->nPushButton->setText(nButton->getActionName());
    } else
    {
        ui->nPushButton->setText(nButton->getSlotsSummary());
    }

    JoyAxisButton *pButton = axis->getPAxisButton();

    if (!pButton->getActionName().isEmpty())
    {
        ui->pPushButton->setText(pButton->getActionName());
    } else
    {
        ui->pPushButton->setText(pButton->getSlotsSummary());
    }

    int currentThrottle = axis->getThrottle();

    if ((currentThrottle == static_cast<int>(JoyAxis::NegativeThrottle)) ||
        (currentThrottle == static_cast<int>(JoyAxis::NegativeHalfThrottle)))
    {
        int tempindex = (currentThrottle == static_cast<int>(JoyAxis::NegativeHalfThrottle)) ? 0 : 1;
        ui->throttleComboBox->setCurrentIndex(tempindex);
        ui->nPushButton->setEnabled(true);
        ui->pPushButton->setEnabled(false);
    } else if ((currentThrottle == static_cast<int>(JoyAxis::PositiveThrottle)) ||
               (currentThrottle == static_cast<int>(JoyAxis::PositiveHalfThrottle)))
    {
        int tempindex = (currentThrottle == static_cast<int>(JoyAxis::PositiveThrottle)) ? 3 : 4;
        ui->throttleComboBox->setCurrentIndex(tempindex);
        ui->pPushButton->setEnabled(true);
        ui->nPushButton->setEnabled(false);
    }

    ui->axisstatusBox->setDeadZone(axis->getDeadZone());
    ui->axisstatusBox->setMaxZone(axis->getMaxZoneValue());
    ui->axisstatusBox->setThrottle(axis->getThrottle());

    ui->joyValueLabel->setText(QString::number(axis->getCurrentRawValue()));
    ui->axisstatusBox->setValue(axis->getCurrentRawValue());

    if (!actAsTrigger)
        selectAxisCurrentPreset();
    else
        selectTriggerPreset();

    ui->axisNameLineEdit->setText(axis->getAxisName());

    connect(ui->presetsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AxisEditDialog::implementPresets);

    connect(ui->deadZoneSlider, &QSlider::valueChanged, this, &AxisEditDialog::updateDeadZoneBox);
    connect(ui->deadZoneSlider, &QSlider::valueChanged, this,
            [this, axis](int deadzone) { ui->axisstatusBox->setDeadZone(axis, deadzone); });

    connect(ui->deadZoneSlider, &QSlider::valueChanged, axis, &JoyAxis::setDeadZone);

    connect(ui->maxZoneSlider, &QSlider::valueChanged, this, &AxisEditDialog::updateMaxZoneBox);
    connect(ui->maxZoneSlider, &QSlider::valueChanged, this,
            [this, axis](int deadzone) { ui->axisstatusBox->setMaxZone(axis, deadzone); });

    connect(ui->maxZoneSlider, &QSlider::valueChanged, axis, &JoyAxis::setMaxZoneValue);

    connect(ui->throttleComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AxisEditDialog::updateThrottleUi);
    connect(ui->throttleComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AxisEditDialog::presetForThrottleChange);

    connect(axis, &JoyAxis::moved, this, [this, axis](int value) { ui->axisstatusBox->setValue(axis, value); });

    connect(axis, &JoyAxis::moved, this, &AxisEditDialog::updateJoyValue);

    connect(ui->deadZoneSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &AxisEditDialog::updateDeadZoneSlider);
    connect(ui->maxZoneSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &AxisEditDialog::updateMaxZoneSlider);

    connect(ui->nPushButton, &QPushButton::clicked, this, &AxisEditDialog::openAdvancedNDialog);
    connect(ui->pPushButton, &QPushButton::clicked, this, &AxisEditDialog::openAdvancedPDialog);

    connect(ui->mouseSettingsPushButton, &QPushButton::clicked, this, &AxisEditDialog::openMouseSettingsDialog);
    connect(ui->axisNameLineEdit, &QLineEdit::textEdited, axis, &JoyAxis::setAxisName);

    connect(axis, &JoyAxis::axisNameChanged, this, &AxisEditDialog::updateWindowTitleAxisName);
    connect(this, &AxisEditDialog::finished, this, &AxisEditDialog::checkFinalSettings);

    connect(ui->hapticTriggerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AxisEditDialog::implementHapticTrigger);
}

// for tests
AxisEditDialog::AxisEditDialog(QWidget *parent)
    : QDialog(parent, Qt::Window)
    , ui(new Ui::AxisEditDialog)
{
}

AxisEditDialog::~AxisEditDialog() { delete ui; }

void AxisEditDialog::implementPresets(int index)
{
    bool actAsTrigger = false;
    int currentThrottle = m_axis->getThrottle();

    if ((currentThrottle == static_cast<int>(JoyAxis::PositiveThrottle)) ||
        (currentThrottle == static_cast<int>(JoyAxis::PositiveHalfThrottle)))
    {
        actAsTrigger = true;
    }

    if (actAsTrigger)
        implementTriggerPresets(index);
    else
        implementAxisPresets(index);
}

void AxisEditDialog::implementAxisPresets(int index)
{
    JoyButtonSlot *nbuttonslot = nullptr;
    JoyButtonSlot *pbuttonslot = nullptr;

    PadderCommon::lockInputDevices();
    InputDevice *tempDevice = m_axis->getParentSet()->getInputDevice();
    QMetaObject::invokeMethod(tempDevice, "haltServices", Qt::BlockingQueuedConnection);

    switch (index)
    {
    case 1:
        nbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        break;

    case 2:
        nbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement, this);
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement, this);
        break;

    case 3:
        nbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        break;

    case 4:
        nbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement, this);
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement, this);
        break;

    case 5:
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Up), Qt::Key_Up,
                                        JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Down), Qt::Key_Down,
                                        JoyButtonSlot::JoyKeyboard, this);
        break;

    case 6:
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Left), Qt::Key_Left,
                                        JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Right), Qt::Key_Right,
                                        JoyButtonSlot::JoyKeyboard, this);
        break;

    case 7:
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_W), Qt::Key_W,
                                        JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_S), Qt::Key_S,
                                        JoyButtonSlot::JoyKeyboard, this);
        break;

    case 8:
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_A), Qt::Key_A,
                                        JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_D), Qt::Key_D,
                                        JoyButtonSlot::JoyKeyboard, this);
        break;

    case 9:
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_8),
                                        QtKeyMapperBase::AntKey_KP_8, JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_2),
                                        QtKeyMapperBase::AntKey_KP_2, JoyButtonSlot::JoyKeyboard, this);
        break;

    case 10:
        nbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_4),
                                        QtKeyMapperBase::AntKey_KP_4, JoyButtonSlot::JoyKeyboard, this);
        pbuttonslot = new JoyButtonSlot(AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_6),
                                        QtKeyMapperBase::AntKey_KP_6, JoyButtonSlot::JoyKeyboard, this);
        break;

    case 0:
    case 11:
        JoyAxisButton *nbutton = m_axis->getNAxisButton();
        JoyAxisButton *pbutton = m_axis->getPAxisButton();

        QMetaObject::invokeMethod(nbutton, "clearSlotsEventReset");
        QMetaObject::invokeMethod(pbutton, "clearSlotsEventReset", Qt::BlockingQueuedConnection);

        refreshNButtonLabel();
        refreshPButtonLabel();

        nbutton->buildActiveZoneSummaryString();
        pbutton->buildActiveZoneSummaryString();
        break;
    }

    if (nbuttonslot != nullptr)
    {
        JoyAxisButton *button = m_axis->getNAxisButton();
        QMetaObject::invokeMethod(button, "clearSlotsEventReset", Q_ARG(bool, false));

        QMetaObject::invokeMethod(button, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, nbuttonslot->getSlotCode()), Q_ARG(int, nbuttonslot->getSlotCodeAlias()),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, nbuttonslot->getSlotMode()));

        refreshNButtonLabel();
        nbuttonslot->deleteLater();
    }

    if (pbuttonslot != nullptr)
    {
        JoyAxisButton *button = m_axis->getPAxisButton();
        QMetaObject::invokeMethod(button, "clearSlotsEventReset", Q_ARG(bool, false));

        QMetaObject::invokeMethod(button, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, pbuttonslot->getSlotCode()), Q_ARG(int, pbuttonslot->getSlotCodeAlias()),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, pbuttonslot->getSlotMode()));

        refreshPButtonLabel();
        pbuttonslot->deleteLater();
    }

    PadderCommon::unlockInputDevices();
}

void AxisEditDialog::updateDeadZoneBox(int value) { ui->deadZoneSpinBox->setValue(value); }

void AxisEditDialog::updateMaxZoneBox(int value) { ui->maxZoneSpinBox->setValue(value); }

void AxisEditDialog::updateThrottleUi(int index)
{
    int tempthrottle = 0;

    if ((index == 0) || (index == 1))
    {
        ui->nPushButton->setEnabled(true);
        ui->pPushButton->setEnabled(false);
        tempthrottle =
            (index == 0) ? static_cast<int>(JoyAxis::NegativeHalfThrottle) : static_cast<int>(JoyAxis::NegativeThrottle);
    } else if (index == 2)
    {
        ui->nPushButton->setEnabled(true);
        ui->pPushButton->setEnabled(true);
        tempthrottle = static_cast<int>(JoyAxis::NormalThrottle);
    } else if ((index == 3) || (index == 4))
    {
        ui->pPushButton->setEnabled(true);
        ui->nPushButton->setEnabled(false);
        tempthrottle =
            (index == 3) ? static_cast<int>(JoyAxis::PositiveThrottle) : static_cast<int>(JoyAxis::PositiveHalfThrottle);
    }

    m_axis->setThrottle(tempthrottle);
    ui->axisstatusBox->setThrottle(tempthrottle);
}

void AxisEditDialog::updateJoyValue(int value) { ui->joyValueLabel->setText(QString::number(value)); }

void AxisEditDialog::updateDeadZoneSlider(int value)
{
    if ((value >= GlobalVariables::JoyAxis::AXISMIN) && (value <= GlobalVariables::JoyAxis::AXISMAX))
    {
        ui->deadZoneSlider->setValue(value);
    }
}

void AxisEditDialog::updateMaxZoneSlider(int value)
{
    if ((value >= GlobalVariables::JoyAxis::AXISMIN) && (value <= GlobalVariables::JoyAxis::AXISMAX))
    {
        ui->maxZoneSlider->setValue(value);
    }
}

void AxisEditDialog::openAdvancedPDialog()
{
    ButtonEditDialog *dialog = new ButtonEditDialog(
        m_axis->getPAxisButton(), m_axis->getPAxisButton()->getParentSet()->getInputDevice(), keypadUnlocked, this);
    dialog->show();

    connect(dialog, &ButtonEditDialog::finished, this, &AxisEditDialog::refreshPButtonLabel);
    connect(dialog, &ButtonEditDialog::finished, this, &AxisEditDialog::refreshPreset);
}

void AxisEditDialog::openAdvancedNDialog()
{
    ButtonEditDialog *dialog = new ButtonEditDialog(
        m_axis->getNAxisButton(), m_axis->getNAxisButton()->getParentSet()->getInputDevice(), keypadUnlocked, this);
    dialog->show();

    connect(dialog, &ButtonEditDialog::finished, this, &AxisEditDialog::refreshNButtonLabel);
    connect(dialog, &ButtonEditDialog::finished, this, &AxisEditDialog::refreshPreset);
}

void AxisEditDialog::refreshNButtonLabel() { ui->nPushButton->setText(m_axis->getNAxisButton()->getSlotsSummary()); }

void AxisEditDialog::refreshPButtonLabel() { ui->pPushButton->setText(m_axis->getPAxisButton()->getSlotsSummary()); }

void AxisEditDialog::checkFinalSettings()
{
    if (m_axis->getThrottle() != initialThrottleState)
        setAxisThrottleConfirm->exec();
}

void AxisEditDialog::selectAxisCurrentPreset()
{
    JoyAxisButton *naxisbutton = m_axis->getNAxisButton();
    QList<JoyButtonSlot *> *naxisslots = naxisbutton->getAssignedSlots();
    JoyAxisButton *paxisbutton = m_axis->getPAxisButton();
    QList<JoyButtonSlot *> *paxisslots = paxisbutton->getAssignedSlots();

    if ((naxisslots->length() == 1) && (paxisslots->length() == 1))
    {
        JoyButtonSlot *nslot = naxisslots->at(0);
        JoyButtonSlot *pslot = paxisslots->at(0);

        if ((nslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
            (nslot->getSlotCode() == JoyButtonSlot::MouseLeft) &&
            (pslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) && (pslot->getSlotCode() == JoyButtonSlot::MouseRight))
        {
            ui->presetsComboBox->setCurrentIndex(1);
        } else if ((nslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (nslot->getSlotCode() == JoyButtonSlot::MouseRight) &&
                   (pslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (pslot->getSlotCode() == JoyButtonSlot::MouseLeft))
        {
            ui->presetsComboBox->setCurrentIndex(2);
        } else if ((nslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (nslot->getSlotCode() == JoyButtonSlot::MouseUp) &&
                   (pslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (pslot->getSlotCode() == JoyButtonSlot::MouseDown))
        {
            ui->presetsComboBox->setCurrentIndex(3);
        } else if ((nslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (nslot->getSlotCode() == JoyButtonSlot::MouseDown) &&
                   (pslot->getSlotMode() == JoyButtonSlot::JoyMouseMovement) &&
                   (pslot->getSlotCode() == JoyButtonSlot::MouseUp))
        {
            ui->presetsComboBox->setCurrentIndex(4);
        } else if ((nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (nslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Up)) &&
                   (pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (pslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Down)))
        {
            ui->presetsComboBox->setCurrentIndex(5);
        } else if ((nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (nslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Left)) &&
                   (pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (pslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Right)))
        {
            ui->presetsComboBox->setCurrentIndex(6);
        } else if ((nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (nslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_W)) &&
                   (pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (pslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_S)))
        {
            ui->presetsComboBox->setCurrentIndex(7);
        } else if ((nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (nslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_A)) &&
                   (pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (pslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_D)))
        {
            ui->presetsComboBox->setCurrentIndex(8);
        } else if ((nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (nslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_8)) &&
                   (pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (pslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_2)))
        {
            ui->presetsComboBox->setCurrentIndex(9);
        } else if ((nslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (nslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_4)) &&
                   (pslot->getSlotMode() == JoyButtonSlot::JoyKeyboard) &&
                   (pslot->getSlotCode() == AntKeyMapper::getInstance()->returnVirtualKey(QtKeyMapperBase::AntKey_KP_6)))
        {
            ui->presetsComboBox->setCurrentIndex(10);
        } else
        {
            ui->presetsComboBox->setCurrentIndex(0);
        }
    } else if ((naxisslots->length() == 0) && (paxisslots->length() == 0))
    {
        ui->presetsComboBox->setCurrentIndex(11);
    } else
    {
        ui->presetsComboBox->setCurrentIndex(0);
    }
}

void AxisEditDialog::selectTriggerPreset()
{
    JoyAxisButton *paxisbutton = m_axis->getPAxisButton();
    QList<JoyButtonSlot *> *paxisslots = paxisbutton->getAssignedSlots();

    if (paxisslots->length() == 1)
    {
        JoyButtonSlot *pslot = paxisslots->at(0);

        if ((pslot->getSlotMode() == JoyButtonSlot::JoyMouseButton) && (pslot->getSlotCode() == JoyButtonSlot::MouseLB))
        {
            ui->presetsComboBox->setCurrentIndex(1);
        } else if ((pslot->getSlotMode() == JoyButtonSlot::JoyMouseButton) &&
                   (pslot->getSlotCode() == JoyButtonSlot::MouseRB))
        {
            ui->presetsComboBox->setCurrentIndex(2);
        } else
        {
            ui->presetsComboBox->setCurrentIndex(0);
        }
    } else if (paxisslots->length() == 0)
    {
        ui->presetsComboBox->setCurrentIndex(3);
    } else
    {
        ui->presetsComboBox->setCurrentIndex(0);
    }
}

/**
 * @brief Converts HapticTriggerMode from the current axis to combo box
 *   index and selects to element.
 */
void AxisEditDialog::selectHapticTrigger()
{
    HapticTriggerModePs5 mode = m_axis->getHapticTrigger()->get_mode();

    switch (mode)
    {
    case HAPTIC_TRIGGER_NONE:
        ui->hapticTriggerComboBox->setCurrentIndex(HAPTIC_TRIGGER_NONE_INDEX);
        return;
    case HAPTIC_TRIGGER_CLICK:
        ui->hapticTriggerComboBox->setCurrentIndex(HAPTIC_TRIGGER_CLICK_INDEX);
        return;
    case HAPTIC_TRIGGER_RIGID:
        ui->hapticTriggerComboBox->setCurrentIndex(HAPTIC_TRIGGER_RIGID_INDEX);
        return;
    case HAPTIC_TRIGGER_RIGID_GRADIENT:
        ui->hapticTriggerComboBox->setCurrentIndex(HAPTIC_TRIGGER_RIGID_GRADIENT_INDEX);
        return;
    case HAPTIC_TRIGGER_VIBRATION:
        ui->hapticTriggerComboBox->setCurrentIndex(HAPTIC_TRIGGER_VIBRATION_INDEX);
        return;
    }
}

void AxisEditDialog::implementTriggerPresets(int index)
{
    JoyButtonSlot *pbuttonslot = nullptr;

    if (index == 1)
    {
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseLB, JoyButtonSlot::JoyMouseButton, this);
    } else if (index == 2)
    {
        pbuttonslot = new JoyButtonSlot(JoyButtonSlot::MouseRB, JoyButtonSlot::JoyMouseButton, this);
    } else if (index == 3)
    {
        JoyAxisButton *nbutton = m_axis->getNAxisButton();
        JoyAxisButton *pbutton = m_axis->getPAxisButton();

        QMetaObject::invokeMethod(nbutton, "clearSlotsEventReset");
        QMetaObject::invokeMethod(pbutton, "clearSlotsEventReset", Qt::BlockingQueuedConnection);

        refreshNButtonLabel();
        refreshPButtonLabel();
    }

    if (pbuttonslot != nullptr)
    {
        JoyAxisButton *nbutton = m_axis->getNAxisButton();
        JoyAxisButton *pbutton = m_axis->getPAxisButton();

        if (nbutton->getAssignedSlots()->length() > 0)
        {
            QMetaObject::invokeMethod(nbutton, "clearSlotsEventReset", Qt::BlockingQueuedConnection, Q_ARG(bool, false));
            refreshNButtonLabel();
        }

        QMetaObject::invokeMethod(pbutton, "clearSlotsEventReset", Q_ARG(bool, false));

        QMetaObject::invokeMethod(pbutton, "setAssignedSlot", Qt::BlockingQueuedConnection,
                                  Q_ARG(int, pbuttonslot->getSlotCode()), Q_ARG(int, pbuttonslot->getSlotCodeAlias()),
                                  Q_ARG(JoyButtonSlot::JoySlotInputAction, pbuttonslot->getSlotMode()));

        refreshPButtonLabel();
        pbuttonslot->deleteLater();
    }
}

void AxisEditDialog::refreshPreset()
{ // Disconnect event associated with presetsComboBox so a change in the index does not
    // alter the axis buttons
    disconnect(ui->presetsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
               &AxisEditDialog::implementPresets);
    selectAxisCurrentPreset();
    // Reconnect the event
    connect(ui->presetsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AxisEditDialog::implementPresets);
}

void AxisEditDialog::openMouseSettingsDialog()
{
    ui->mouseSettingsPushButton->setEnabled(false);

    MouseAxisSettingsDialog *dialog = new MouseAxisSettingsDialog(m_axis, this);
    dialog->show();
    connect(this, &AxisEditDialog::finished, dialog, &MouseAxisSettingsDialog::close);
    connect(dialog, &MouseAxisSettingsDialog::finished, this, &AxisEditDialog::enableMouseSettingButton);
}

void AxisEditDialog::enableMouseSettingButton() { ui->mouseSettingsPushButton->setEnabled(true); }

void AxisEditDialog::updateWindowTitleAxisName()
{
    QString temp = QString(tr("Set")).append(" ");

    if (!m_axis->getAxisName().isEmpty())
    {
        temp.append(m_axis->getPartialName(false, true));
    } else
    {
        temp.append(m_axis->getPartialName());
    }

    if (m_axis->getParentSet()->getIndex() != 0)
    {
        int setIndex = m_axis->getParentSet()->getRealIndex();
        temp.append(" [").append(tr("Set %1").arg(setIndex));

        QString setName = m_axis->getParentSet()->getName();
        if (!setName.isEmpty())
            temp.append(": ").append(setName);

        temp.append("]");
    }

    setWindowTitle(temp);
}

void AxisEditDialog::buildAxisPresetsMenu()
{
    ui->presetsComboBox->clear();
    ui->presetsComboBox->addItem(tr(""));
    ui->presetsComboBox->addItem(tr("Mouse (Horizontal)"));
    ui->presetsComboBox->addItem(tr("Mouse (Inverted Horizontal)"));
    ui->presetsComboBox->addItem(tr("Mouse (Vertical)"));
    ui->presetsComboBox->addItem(tr("Mouse (Inverted Vertical)"));
    ui->presetsComboBox->addItem(tr("Arrows: Up | Down"));
    ui->presetsComboBox->addItem(tr("Arrows: Left | Right"));
    ui->presetsComboBox->addItem(tr("Keys: W | S"));
    ui->presetsComboBox->addItem(tr("Keys: A | D"));
    ui->presetsComboBox->addItem(tr("NumPad: KP_8 | KP_2"));
    ui->presetsComboBox->addItem(tr("NumPad: KP_4 | KP_6"));
    ui->presetsComboBox->addItem(tr("None"));
}

void AxisEditDialog::buildTriggerPresetsMenu()
{
    ui->presetsComboBox->clear();
    ui->presetsComboBox->addItem(tr(""));
    ui->presetsComboBox->addItem(tr("Left Mouse Button"));
    ui->presetsComboBox->addItem(tr("Right Mouse Button"));
    ui->presetsComboBox->addItem(tr("None"));
}

void AxisEditDialog::buildHapticTriggerMenu()
{
    ui->hapticTriggerComboBox->clear();
    ui->hapticTriggerComboBox->addItem(tr("None"));
    ui->hapticTriggerComboBox->addItem(tr("Click"));
    ui->hapticTriggerComboBox->addItem(tr("Rigid"));
    ui->hapticTriggerComboBox->addItem(tr("Rigid Gradient"));
    ui->hapticTriggerComboBox->addItem(tr("Vibration"));
}

void AxisEditDialog::presetForThrottleChange(int index)
{
    Q_UNUSED(index);

    bool actAsTrigger = false;
    int currentThrottle = m_axis->getThrottle();

    if ((currentThrottle == static_cast<int>(JoyAxis::PositiveThrottle)) ||
        (currentThrottle == static_cast<int>(JoyAxis::PositiveHalfThrottle)))
    {
        actAsTrigger = true;
    }

    disconnect(ui->presetsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
               &AxisEditDialog::implementPresets);

    if (actAsTrigger)
    {
        buildTriggerPresetsMenu();
        selectTriggerPreset();
    } else
    {
        buildAxisPresetsMenu();
        selectAxisCurrentPreset();
    }

    connect(ui->presetsComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AxisEditDialog::implementPresets);
}

/**
 * @brief Converts the haptic trigger combo box index to a
 *   HapticTriggerMode and applies it to the current axis.
 */
void AxisEditDialog::implementHapticTrigger(int index)
{
    HapticTriggerModePs5 mode;
    switch (static_cast<HapticTriggerIndex>(index))
    {
    case HAPTIC_TRIGGER_NONE_INDEX:
        mode = HAPTIC_TRIGGER_NONE;
        break;
    case HAPTIC_TRIGGER_CLICK_INDEX:
        mode = HAPTIC_TRIGGER_CLICK;
        break;
    case HAPTIC_TRIGGER_RIGID_INDEX:
        mode = HAPTIC_TRIGGER_RIGID;
        break;
    case HAPTIC_TRIGGER_RIGID_GRADIENT_INDEX:
        mode = HAPTIC_TRIGGER_RIGID_GRADIENT;
        break;
    case HAPTIC_TRIGGER_VIBRATION_INDEX:
        mode = HAPTIC_TRIGGER_VIBRATION;
        break;
    }
    m_axis->setHapticTriggerMode(mode);
}
