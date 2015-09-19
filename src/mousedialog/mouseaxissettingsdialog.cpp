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

#include "mouseaxissettingsdialog.h"
#include "ui_mousesettingsdialog.h"

#include <QSpinBox>
#include <QComboBox>

#include <inputdevice.h>
#include <setjoystick.h>

MouseAxisSettingsDialog::MouseAxisSettingsDialog(JoyAxis *axis, QWidget *parent) :
    MouseSettingsDialog(parent),
    helper(axis)
{
    setAttribute(Qt::WA_DeleteOnClose);

    this->axis = axis;
    helper.moveToThread(axis->thread());

    calculateMouseSpeedPreset();
    selectCurrentMouseModePreset();
    calculateSpringPreset();

    if (axis->getButtonsPresetSensitivity() > 0.0)
    {
        ui->sensitivityDoubleSpinBox->setValue(axis->getButtonsPresetSensitivity());
    }
    updateAccelerationCurvePresetComboBox();

    updateWindowTitleAxisName();

    if (ui->mouseModeComboBox->currentIndex() == 2)
    {
        springPreviewWidget = new SpringModeRegionPreview(ui->springWidthSpinBox->value(),
                                                          ui->springHeightSpinBox->value());
    }
    else
    {
        springPreviewWidget = new SpringModeRegionPreview(0, 0);
    }

    calculateWheelSpeedPreset();

    if (axis->isRelativeSpring())
    {
        ui->relativeSpringCheckBox->setChecked(true);
    }

    double easingDuration = axis->getButtonsEasingDuration();
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

    connect(this, SIGNAL(finished(int)), springPreviewWidget, SLOT(deleteLater()));

    connect(ui->mouseModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMouseMode(int)));
    connect(ui->accelerationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMouseCurve(int)));

    connect(ui->horizontalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateConfigHorizontalSpeed(int)));
    connect(ui->verticalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateConfigVerticalSpeed(int)));

    connect(ui->springWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSpringWidth(int)));
    connect(ui->springWidthSpinBox, SIGNAL(valueChanged(int)), springPreviewWidget, SLOT(setSpringWidth(int)));

    connect(ui->springHeightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSpringHeight(int)));
    connect(ui->springHeightSpinBox, SIGNAL(valueChanged(int)), springPreviewWidget, SLOT(setSpringHeight(int)));

    connect(ui->relativeSpringCheckBox, SIGNAL(clicked(bool)), this, SLOT(updateSpringRelativeStatus(bool)));

    connect(ui->sensitivityDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateSensitivity(double)));

    connect(ui->wheelHoriSpeedSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateWheelSpeedHorizontalSpeed(int)));
    connect(ui->wheelVertSpeedSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateWheelSpeedVerticalSpeed(int)));

    connect(ui->easingDoubleSpinBox, SIGNAL(valueChanged(double)), axis, SLOT(setButtonsEasingDuration(double)));

    connect(ui->extraAccelerationGroupBox, SIGNAL(clicked(bool)), &helper, SLOT(updateExtraAccelerationStatus(bool)));
    connect(ui->extraAccelDoubleSpinBox, SIGNAL(valueChanged(double)), &helper, SLOT(updateExtraAccelerationMultiplier(double)));
    connect(ui->minMultiDoubleSpinBox, SIGNAL(valueChanged(double)), &helper, SLOT(updateStartMultiPercentage(double)));
    connect(ui->minThresholdDoubleSpinBox, SIGNAL(valueChanged(double)), &helper, SLOT(updateMinAccelThreshold(double)));
    connect(ui->maxThresholdDoubleSpinBox, SIGNAL(valueChanged(double)), &helper, SLOT(updateMaxAccelThreshold(double)));
    connect(ui->accelExtraDurationDoubleSpinBox, SIGNAL(valueChanged(double)), &helper, SLOT(updateAccelExtraDuration(double)));
    connect(ui->releaseSpringRadiusspinBox, SIGNAL(valueChanged(int)), &helper, SLOT(updateReleaseSpringRadius(int)));

    connect(ui->extraAccelCurveComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateExtraAccelerationCurve(int)));
}

void MouseAxisSettingsDialog::changeMouseMode(int index)
{
    if (index == 1)
    {
        axis->setButtonsMouseMode(JoyButton::MouseCursor);
        if (springPreviewWidget->isVisible())
        {
            springPreviewWidget->hide();
        }
    }
    else if (index == 2)
    {
        axis->setButtonsMouseMode(JoyButton::MouseSpring);
        if (!springPreviewWidget->isVisible())
        {
            springPreviewWidget->setSpringWidth(ui->springWidthSpinBox->value());
            springPreviewWidget->setSpringHeight(ui->springHeightSpinBox->value());
        }
        axis->getPAxisButton()->setExtraAccelerationStatus(false);
        axis->getNAxisButton()->setExtraAccelerationStatus(false);
    }
}

void MouseAxisSettingsDialog::changeMouseCurve(int index)
{
    JoyButton::JoyMouseCurve temp = MouseSettingsDialog::getMouseCurveForIndex(index);
    axis->setButtonsMouseCurve(temp);
}

void MouseAxisSettingsDialog::updateConfigHorizontalSpeed(int value)
{
    axis->getPAxisButton()->setMouseSpeedX(value);
    axis->getNAxisButton()->setMouseSpeedX(value);
}

void MouseAxisSettingsDialog::updateConfigVerticalSpeed(int value)
{
    axis->getPAxisButton()->setMouseSpeedY(value);
    axis->getNAxisButton()->setMouseSpeedY(value);
}

void MouseAxisSettingsDialog::updateSpringWidth(int value)
{
    axis->setButtonsSpringWidth(value);
}

void MouseAxisSettingsDialog::updateSpringHeight(int value)
{
    axis->setButtonsSpringHeight(value);
}

void MouseAxisSettingsDialog::selectCurrentMouseModePreset()
{
    bool presetDefined = axis->hasSameButtonsMouseMode();
    if (presetDefined)
    {
        JoyButton::JoyMouseMovementMode mode = axis->getButtonsPresetMouseMode();
        if (mode == JoyButton::MouseCursor)
        {
            ui->mouseModeComboBox->setCurrentIndex(1);
        }
        else if (mode == JoyButton::MouseSpring)
        {
            ui->mouseModeComboBox->setCurrentIndex(2);
        }
    }
    else
    {
        ui->mouseModeComboBox->setCurrentIndex(0);
    }
}

void MouseAxisSettingsDialog::calculateSpringPreset()
{
    int tempWidth = axis->getButtonsPresetSpringWidth();
    int tempHeight = axis->getButtonsPresetSpringHeight();

    if (tempWidth > 0)
    {
        ui->springWidthSpinBox->setValue(tempWidth);
    }

    if (tempHeight > 0)
    {
        ui->springHeightSpinBox->setValue(tempHeight);
    }
}

void MouseAxisSettingsDialog::calculateMouseSpeedPreset()
{
    int tempMouseSpeedX = 0;
    tempMouseSpeedX = qMax(axis->getPAxisButton()->getMouseSpeedX(), axis->getNAxisButton()->getMouseSpeedX());

    int tempMouseSpeedY = 0;
    tempMouseSpeedY = qMax(axis->getPAxisButton()->getMouseSpeedY(), axis->getNAxisButton()->getMouseSpeedY());

    ui->horizontalSpinBox->setValue(tempMouseSpeedX);
    ui->verticalSpinBox->setValue(tempMouseSpeedY);
}

void MouseAxisSettingsDialog::updateSensitivity(double value)
{
    axis->setButtonsSensitivity(value);
}

void MouseAxisSettingsDialog::updateAccelerationCurvePresetComboBox()
{
    JoyButton::JoyMouseCurve temp = axis->getButtonsPresetMouseCurve();
    MouseSettingsDialog::updateAccelerationCurvePresetComboBox(temp);
}

void MouseAxisSettingsDialog::calculateWheelSpeedPreset()
{
    JoyAxisButton *paxisbutton = axis->getPAxisButton();
    JoyAxisButton *naxisbutton = axis->getNAxisButton();

    int tempWheelSpeedX = qMax(paxisbutton->getWheelSpeedX(), naxisbutton->getWheelSpeedX());
    int tempWheelSpeedY = qMax(paxisbutton->getWheelSpeedY(), naxisbutton->getWheelSpeedY());

    ui->wheelHoriSpeedSpinBox->setValue(tempWheelSpeedX);
    ui->wheelVertSpeedSpinBox->setValue(tempWheelSpeedY);
}

void MouseAxisSettingsDialog::updateWheelSpeedHorizontalSpeed(int value)
{
    axis->setButtonsWheelSpeedX(value);
}

void MouseAxisSettingsDialog::updateWheelSpeedVerticalSpeed(int value)
{
    axis->setButtonsWheelSpeedY(value);
}

void MouseAxisSettingsDialog::updateSpringRelativeStatus(bool value)
{
    axis->setButtonsSpringRelativeStatus(value);
}

void MouseAxisSettingsDialog::updateWindowTitleAxisName()
{
    QString temp;
    temp.append(tr("Mouse Settings - "));

    if (!axis->getAxisName().isEmpty())
    {
        temp.append(axis->getPartialName(false, true));
    }
    else
    {
        temp.append(axis->getPartialName());
    }

    if (axis->getParentSet()->getIndex() != 0)
    {
        unsigned int setIndex = axis->getParentSet()->getRealIndex();
        temp.append(" [").append(tr("Set %1").arg(setIndex));

        QString setName = axis->getParentSet()->getName();
        if (!setName.isEmpty())
        {
            temp.append(": ").append(setName);
        }

        temp.append("]");
    }

    setWindowTitle(temp);
}

void MouseAxisSettingsDialog::calculateExtraAccelrationStatus()
{
    if (axis->getPAxisButton()->isExtraAccelerationEnabled() &&
        axis->getNAxisButton()->isExtraAccelerationEnabled())
    {
        ui->extraAccelerationGroupBox->setChecked(true);
        //ui->extraAccelCheckBox->setChecked(true);
        //ui->extraAccelDoubleSpinBox->setEnabled(true);
    }
    else
    {
        ui->extraAccelerationGroupBox->setChecked(false);
    }
}

void MouseAxisSettingsDialog::calculateExtraAccelerationMultiplier()
{
    if (axis->getPAxisButton()->getExtraAccelerationMultiplier() ==
        axis->getNAxisButton()->getExtraAccelerationMultiplier())
    {
        double temp = axis->getPAxisButton()->getExtraAccelerationMultiplier();
        ui->extraAccelDoubleSpinBox->setValue(temp);
    }
}

void MouseAxisSettingsDialog::calculateStartAccelerationMultiplier()
{
    if (axis->getPAxisButton()->getStartAccelMultiplier() ==
        axis->getNAxisButton()->getStartAccelMultiplier())
    {
        double temp = axis->getPAxisButton()->getStartAccelMultiplier();
        ui->minMultiDoubleSpinBox->setValue(temp);
    }
}

void MouseAxisSettingsDialog::calculateMinAccelerationThreshold()
{
    if (axis->getPAxisButton()->getMinAccelThreshold() ==
        axis->getNAxisButton()->getMinAccelThreshold())
    {
        double temp = axis->getPAxisButton()->getMinAccelThreshold();
        ui->minThresholdDoubleSpinBox->setValue(temp);
    }
}

void MouseAxisSettingsDialog::calculateMaxAccelerationThreshold()
{
    if (axis->getPAxisButton()->getMaxAccelThreshold() ==
        axis->getNAxisButton()->getMaxAccelThreshold())
    {
        double temp = axis->getPAxisButton()->getMaxAccelThreshold();
        ui->maxThresholdDoubleSpinBox->setValue(temp);
    }
}

void MouseAxisSettingsDialog::calculateAccelExtraDuration()
{
    if (axis->getPAxisButton()->getAccelExtraDuration() ==
        axis->getNAxisButton()->getAccelExtraDuration())
    {
        double temp = axis->getPAxisButton()->getAccelExtraDuration();
        ui->accelExtraDurationDoubleSpinBox->setValue(temp);
    }
}

void MouseAxisSettingsDialog::calculateReleaseSpringRadius()
{
    int result = 0;
    if (axis->getPAxisButton()->getSpringDeadCircleMultiplier() ==
        axis->getNAxisButton()->getSpringDeadCircleMultiplier())
    {
        result = axis->getPAxisButton()->getSpringDeadCircleMultiplier();
    }

    ui->releaseSpringRadiusspinBox->setValue(result);
}

void MouseAxisSettingsDialog::updateExtraAccelerationCurve(int index)
{
    JoyButton::JoyExtraAccelerationCurve temp = getExtraAccelCurveForIndex(index);

    if (index > 0)
    {
        InputDevice *device = axis->getParentSet()->getInputDevice();

        //PadderCommon::lockInputDevices();
        //QMetaObject::invokeMethod(device, "haltServices", Qt::BlockingQueuedConnection);

        PadderCommon::inputDaemonMutex.lock();

        axis->getPAxisButton()->setExtraAccelerationCurve(temp);
        axis->getNAxisButton()->setExtraAccelerationCurve(temp);

        PadderCommon::inputDaemonMutex.unlock();

        //PadderCommon::unlockInputDevices();
    }
}

void MouseAxisSettingsDialog::calculateExtraAccelerationCurve()
{
    if (axis->getPAxisButton()->getExtraAccelerationCurve() ==
        axis->getNAxisButton()->getExtraAccelerationCurve())
    {
        JoyButton::JoyExtraAccelerationCurve temp = axis->getPAxisButton()->getExtraAccelerationCurve();
        updateExtraAccelerationCurvePresetComboBox(temp);
    }
}
