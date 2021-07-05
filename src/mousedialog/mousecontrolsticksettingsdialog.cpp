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

#include "mousecontrolsticksettingsdialog.h"

#include "common.h"
#include "inputdevice.h"
#include "joybuttontypes/joycontrolstickbutton.h"
#include "joycontrolstick.h"
#include "messagehandler.h"
#include "setjoystick.h"
#include "springmoderegionpreview.h"

#include <QComboBox>
#include <QDebug>
#include <QHashIterator>
#include <QSpinBox>

MouseControlStickSettingsDialog::MouseControlStickSettingsDialog(JoyControlStick *stick, QWidget *parent)
    : MouseSettingsDialog(parent)
    , helper(stick)
{
    setAttribute(Qt::WA_DeleteOnClose);

    this->stick = stick;
    getHelperLocal().moveToThread(stick->thread());

    calculateMouseSpeedPreset();
    selectCurrentMouseModePreset();
    calculateSpringPreset();

    if (stick->getButtonsPresetSensitivity() > 0.0)
    {
        ui->sensitivityDoubleSpinBox->setValue(stick->getButtonsPresetSensitivity());
    }
    updateAccelerationCurvePresetComboBox();

    updateWindowTitleStickName();

    if (ui->mouseModeComboBox->currentIndex() == 2)
    {
        springPreviewWidget = new SpringModeRegionPreview(ui->springWidthSpinBox->value(), ui->springHeightSpinBox->value());
    } else
    {
        springPreviewWidget = new SpringModeRegionPreview(0, 0);
    }

    calculateWheelSpeedPreset();

    if (stick->isRelativeSpring())
    {
        ui->relativeSpringCheckBox->setChecked(true);
    }

    double easingDuration = stick->getButtonsEasingDuration();
    ui->easingDoubleSpinBox->setValue(easingDuration);

    calculateExtraAccelrationStatus();
    calculateExtraAccelerationMultiplier();
    calculateStartAccelerationMultiplier();
    calculateMinAccelerationThreshold();
    calculateMaxAccelerationThreshold();
    calculateAccelExtraDuration();
    calculateReleaseSpringRadius();
    calculateExtraAccelerationCurve();

    changeSpringSectionStatus(ui->mouseModeComboBox->currentIndex());
    changeSettingsWidgetStatus(ui->accelerationComboBox->currentIndex());

    connect(this, &MouseControlStickSettingsDialog::finished, springPreviewWidget, &SpringModeRegionPreview::deleteLater);

    connect(ui->mouseModeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &MouseControlStickSettingsDialog::changeMouseMode);
    connect(ui->accelerationComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &MouseControlStickSettingsDialog::changeMouseCurve);

    connect(ui->horizontalSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseControlStickSettingsDialog::updateConfigHorizontalSpeed);
    connect(ui->verticalSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseControlStickSettingsDialog::updateConfigVerticalSpeed);

    connect(ui->springWidthSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseControlStickSettingsDialog::updateSpringWidth);
    connect(ui->springWidthSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), springPreviewWidget,
            &SpringModeRegionPreview::setSpringWidth);

    connect(ui->springHeightSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseControlStickSettingsDialog::updateSpringHeight);
    connect(ui->springHeightSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), springPreviewWidget,
            &SpringModeRegionPreview::setSpringHeight);

    connect(ui->relativeSpringCheckBox, &QCheckBox::clicked, this,
            &MouseControlStickSettingsDialog::updateSpringRelativeStatus);

    connect(ui->sensitivityDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this,
            &MouseControlStickSettingsDialog::updateSensitivity);

    connect(ui->wheelHoriSpeedSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseControlStickSettingsDialog::updateWheelSpeedHorizontalSpeed);
    connect(ui->wheelVertSpeedSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseControlStickSettingsDialog::updateWheelSpeedVerticalSpeed);

    connect(ui->easingDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), stick,
            &JoyControlStick::setButtonsEasingDuration);

    connect(ui->extraAccelerationGroupBox, &QGroupBox::clicked, &helper,
            &MouseControlStickSettingsDialogHelper::updateExtraAccelerationStatus);
    connect(ui->extraAccelDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            &helper, &MouseControlStickSettingsDialogHelper::updateExtraAccelerationMultiplier);
    connect(ui->minMultiDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), &helper,
            &MouseControlStickSettingsDialogHelper::updateStartMultiPercentage);
    connect(ui->minThresholdDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            &helper, &MouseControlStickSettingsDialogHelper::updateMinAccelThreshold);
    connect(ui->maxThresholdDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            &helper, &MouseControlStickSettingsDialogHelper::updateMaxAccelThreshold);
    connect(ui->accelExtraDurationDoubleSpinBox,
            static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), &helper,
            &MouseControlStickSettingsDialogHelper::updateAccelExtraDuration);
    connect(ui->releaseSpringRadiusspinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), &helper,
            &MouseControlStickSettingsDialogHelper::updateReleaseSpringRadius);

    connect(ui->extraAccelCurveComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &MouseControlStickSettingsDialog::updateExtraAccelerationCurve);
}

void MouseControlStickSettingsDialog::changeMouseMode(int index)
{
    if (index == 1)
    {
        stick->setButtonsMouseMode(JoyButton::MouseCursor);
        if (springPreviewWidget->isVisible())
        {
            springPreviewWidget->hide();
        }
    } else if (index == 2)
    {
        stick->setButtonsMouseMode(JoyButton::MouseSpring);
        if (!springPreviewWidget->isVisible())
        {
            springPreviewWidget->setSpringWidth(ui->springWidthSpinBox->value());
            springPreviewWidget->setSpringHeight(ui->springHeightSpinBox->value());
        }
        stick->setButtonsExtraAccelerationStatus(false);
    }
}

void MouseControlStickSettingsDialog::changeMouseCurve(int index)
{
    JoyButton::JoyMouseCurve temp = MouseSettingsDialog::getMouseCurveForIndex(index);
    stick->setButtonsMouseCurve(temp);
}

void MouseControlStickSettingsDialog::updateConfigHorizontalSpeed(int value)
{
    QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton *> iter(*stick->getButtons());
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setMouseSpeedX(value);
    }
}

void MouseControlStickSettingsDialog::updateConfigVerticalSpeed(int value)
{
    QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton *> iter(*stick->getButtons());
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setMouseSpeedY(value);
    }
}

void MouseControlStickSettingsDialog::updateSpringWidth(int value) { stick->setButtonsSpringWidth(value); }

void MouseControlStickSettingsDialog::updateSpringHeight(int value) { stick->setButtonsSpringHeight(value); }

void MouseControlStickSettingsDialog::selectCurrentMouseModePreset()
{
    bool presetDefined = stick->hasSameButtonsMouseMode();
    if (presetDefined)
    {
        JoyButton::JoyMouseMovementMode mode = stick->getButtonsPresetMouseMode();
        if (mode == JoyButton::MouseCursor)
        {
            ui->mouseModeComboBox->setCurrentIndex(1);
        } else if (mode == JoyButton::MouseSpring)
        {
            ui->mouseModeComboBox->setCurrentIndex(2);
        }
    } else
    {
        ui->mouseModeComboBox->setCurrentIndex(0);
    }
}

void MouseControlStickSettingsDialog::calculateSpringPreset()
{
    int tempWidth = stick->getButtonsPresetSpringWidth();
    int tempHeight = stick->getButtonsPresetSpringHeight();

    if (tempWidth > 0)
    {
        ui->springWidthSpinBox->setValue(tempWidth);
    }

    if (tempHeight > 0)
    {
        ui->springHeightSpinBox->setValue(tempHeight);
    }
}

void MouseControlStickSettingsDialog::calculateMouseSpeedPreset()
{
    QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton *> iter(*stick->getButtons());
    int tempMouseSpeedX = 0;
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        tempMouseSpeedX = qMax(tempMouseSpeedX, button->getMouseSpeedX());
    }

    iter.toFront();
    int tempMouseSpeedY = 0;
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        tempMouseSpeedY = qMax(tempMouseSpeedY, button->getMouseSpeedY());
    }

    ui->horizontalSpinBox->setValue(tempMouseSpeedX);
    ui->verticalSpinBox->setValue(tempMouseSpeedY);
}

void MouseControlStickSettingsDialog::updateSensitivity(double value) { stick->setButtonsSensitivity(value); }

void MouseControlStickSettingsDialog::updateAccelerationCurvePresetComboBox()
{
    JoyButton::JoyMouseCurve temp = stick->getButtonsPresetMouseCurve();
    MouseSettingsDialog::updateAccelerationCurvePresetComboBox(temp);
}

void MouseControlStickSettingsDialog::calculateWheelSpeedPreset()
{
    QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton *> iter(*stick->getButtons());
    int tempWheelSpeedX = 0;
    int tempWheelSpeedY = 0;
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        tempWheelSpeedX = qMax(tempWheelSpeedX, button->getWheelSpeedX());
        tempWheelSpeedY = qMax(tempWheelSpeedY, button->getWheelSpeedY());
    }

    ui->wheelHoriSpeedSpinBox->setValue(tempWheelSpeedX);
    ui->wheelVertSpeedSpinBox->setValue(tempWheelSpeedY);
}

void MouseControlStickSettingsDialog::updateWheelSpeedHorizontalSpeed(int value) { stick->setButtonsWheelSpeedX(value); }

void MouseControlStickSettingsDialog::updateWheelSpeedVerticalSpeed(int value) { stick->setButtonsWheelSpeedY(value); }

void MouseControlStickSettingsDialog::updateSpringRelativeStatus(bool value)
{
    stick->setButtonsSpringRelativeStatus(value);
}

void MouseControlStickSettingsDialog::updateWindowTitleStickName()
{
    QString temp = QString(tr("Mouse Settings")).append(" - ");

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

void MouseControlStickSettingsDialog::calculateExtraAccelrationStatus()
{
    if (stick->getButtonsExtraAccelerationStatus())
    {
        ui->extraAccelerationGroupBox->setChecked(true);
    }
}

void MouseControlStickSettingsDialog::calculateExtraAccelerationMultiplier()
{
    ui->extraAccelDoubleSpinBox->setValue(stick->getButtonsExtraAccelerationMultiplier());
}

void MouseControlStickSettingsDialog::calculateStartAccelerationMultiplier()
{
    ui->minMultiDoubleSpinBox->setValue(stick->getButtonsStartAccelerationMultiplier());
}

void MouseControlStickSettingsDialog::calculateMinAccelerationThreshold()
{
    ui->minThresholdDoubleSpinBox->setValue(stick->getButtonsMinAccelerationThreshold());
}

void MouseControlStickSettingsDialog::calculateMaxAccelerationThreshold()
{
    ui->maxThresholdDoubleSpinBox->setValue(stick->getButtonsMaxAccelerationThreshold());
}

void MouseControlStickSettingsDialog::calculateAccelExtraDuration()
{
    ui->accelExtraDurationDoubleSpinBox->setValue(stick->getButtonsAccelerationEasingDuration());
}

void MouseControlStickSettingsDialog::calculateReleaseSpringRadius()
{
    ui->releaseSpringRadiusspinBox->setValue(stick->getButtonsSpringDeadCircleMultiplier());
}

void MouseControlStickSettingsDialog::calculateExtraAccelerationCurve()
{
    JoyButton::JoyExtraAccelerationCurve curve = stick->getButtonsExtraAccelerationCurve();
    updateExtraAccelerationCurvePresetComboBox(curve);
}

void MouseControlStickSettingsDialog::updateExtraAccelerationCurve(int index)
{
    JoyButton::JoyExtraAccelerationCurve temp = getExtraAccelCurveForIndex(index);
    if (index > 0)
    {
        PadderCommon::inputDaemonMutex.lock();
        stick->setButtonsExtraAccelCurve(temp);
        PadderCommon::inputDaemonMutex.unlock();
    }
}

JoyControlStick *MouseControlStickSettingsDialog::getStick() const { return stick; }

SpringModeRegionPreview *MouseControlStickSettingsDialog::getSpringPreviewWidget() const { return springPreviewWidget; }

MouseControlStickSettingsDialogHelper const &MouseControlStickSettingsDialog::getHelper() { return helper; }

MouseControlStickSettingsDialogHelper &MouseControlStickSettingsDialog::getHelperLocal() { return helper; }
