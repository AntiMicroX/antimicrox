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

#include "mouseaxissettingsdialog.h"

#include "common.h"
#include "inputdevice.h"
#include "joyaxis.h"
#include "setjoystick.h"
#include "springmoderegionpreview.h"
#include "springmousemoveinfo.h"

#include <QComboBox>
#include <QDebug>
#include <QSpinBox>

MouseAxisSettingsDialog::MouseAxisSettingsDialog(JoyAxis *axis, QWidget *parent)
    : MouseSettingsDialog(parent)
    , helper(axis)
{
    setAttribute(Qt::WA_DeleteOnClose);

    this->axis = axis;
    getHelperLocal().moveToThread(axis->thread());

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
        springPreviewWidget = new SpringModeRegionPreview(ui->springWidthSpinBox->value(), ui->springHeightSpinBox->value());
    } else
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

    connect(this, &MouseAxisSettingsDialog::finished, springPreviewWidget, &SpringModeRegionPreview::deleteLater);

    connect(ui->mouseModeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &MouseAxisSettingsDialog::changeMouseMode);
    connect(ui->accelerationComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &MouseAxisSettingsDialog::changeMouseCurve);

    connect(ui->horizontalSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseAxisSettingsDialog::updateConfigHorizontalSpeed);
    connect(ui->verticalSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseAxisSettingsDialog::updateConfigVerticalSpeed);

    connect(ui->springWidthSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseAxisSettingsDialog::updateSpringWidth);
    connect(ui->springWidthSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), springPreviewWidget,
            &SpringModeRegionPreview::setSpringWidth);

    connect(ui->springHeightSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseAxisSettingsDialog::updateSpringHeight);
    connect(ui->springHeightSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), springPreviewWidget,
            &SpringModeRegionPreview::setSpringHeight);

    connect(ui->relativeSpringCheckBox, &QCheckBox::clicked, this, &MouseAxisSettingsDialog::updateSpringRelativeStatus);

    connect(ui->sensitivityDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this,
            &MouseAxisSettingsDialog::updateSensitivity);

    connect(ui->wheelHoriSpeedSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseAxisSettingsDialog::updateWheelSpeedHorizontalSpeed);
    connect(ui->wheelVertSpeedSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseAxisSettingsDialog::updateWheelSpeedVerticalSpeed);

    connect(ui->easingDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), axis,
            &JoyAxis::setButtonsEasingDuration);

    connect(ui->extraAccelerationGroupBox, &QGroupBox::clicked, &helper,
            &MouseAxisSettingsDialogHelper::updateExtraAccelerationStatus);
    connect(ui->extraAccelDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            &helper, &MouseAxisSettingsDialogHelper::updateExtraAccelerationMultiplier);
    connect(ui->minMultiDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), &helper,
            &MouseAxisSettingsDialogHelper::updateStartMultiPercentage);
    connect(ui->minThresholdDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            &helper, &MouseAxisSettingsDialogHelper::updateMinAccelThreshold);
    connect(ui->maxThresholdDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            &helper, &MouseAxisSettingsDialogHelper::updateMaxAccelThreshold);
    connect(ui->accelExtraDurationDoubleSpinBox,
            static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), &helper,
            &MouseAxisSettingsDialogHelper::updateAccelExtraDuration);
    connect(ui->releaseSpringRadiusspinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), &helper,
            &MouseAxisSettingsDialogHelper::updateReleaseSpringRadius);

    connect(ui->extraAccelCurveComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &MouseAxisSettingsDialog::updateExtraAccelerationCurve);
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
    } else if (index == 2)
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

void MouseAxisSettingsDialog::updateSpringWidth(int value) { axis->setButtonsSpringWidth(value); }

void MouseAxisSettingsDialog::updateSpringHeight(int value) { axis->setButtonsSpringHeight(value); }

void MouseAxisSettingsDialog::selectCurrentMouseModePreset()
{
    bool presetDefined = axis->hasSameButtonsMouseMode();
    if (presetDefined)
    {
        JoyButton::JoyMouseMovementMode mode = axis->getButtonsPresetMouseMode();
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

void MouseAxisSettingsDialog::updateSensitivity(double value) { axis->setButtonsSensitivity(value); }

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

void MouseAxisSettingsDialog::updateWheelSpeedHorizontalSpeed(int value) { axis->setButtonsWheelSpeedX(value); }

void MouseAxisSettingsDialog::updateWheelSpeedVerticalSpeed(int value) { axis->setButtonsWheelSpeedY(value); }

void MouseAxisSettingsDialog::updateSpringRelativeStatus(bool value) { axis->setButtonsSpringRelativeStatus(value); }

void MouseAxisSettingsDialog::updateWindowTitleAxisName()
{
    QString temp = QString();
    temp.append(tr("Mouse Settings - "));

    if (!axis->getAxisName().isEmpty())
    {
        temp.append(axis->getPartialName(false, true));
    } else
    {
        temp.append(axis->getPartialName());
    }

    if (axis->getParentSet()->getIndex() != 0)
    {
        int setIndex = axis->getParentSet()->getRealIndex();
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
    if ((axis->getPAxisButton()->isExtraAccelerationEnabled()) && (axis->getNAxisButton()->isExtraAccelerationEnabled()))
    {
        ui->extraAccelerationGroupBox->setChecked(true);
    } else
    {
        ui->extraAccelerationGroupBox->setChecked(false);
    }
}

void MouseAxisSettingsDialog::calculateExtraAccelerationMultiplier()
{
    if (qFuzzyCompare(axis->getPAxisButton()->getExtraAccelerationMultiplier(),
                      axis->getNAxisButton()->getExtraAccelerationMultiplier()))
    {
        double temp = axis->getPAxisButton()->getExtraAccelerationMultiplier();
        ui->extraAccelDoubleSpinBox->setValue(temp);
    }
}

void MouseAxisSettingsDialog::calculateStartAccelerationMultiplier()
{
    if (qFuzzyCompare(axis->getPAxisButton()->getStartAccelMultiplier(), axis->getNAxisButton()->getStartAccelMultiplier()))
    {
        double temp = axis->getPAxisButton()->getStartAccelMultiplier();
        ui->minMultiDoubleSpinBox->setValue(temp);
    }
}

void MouseAxisSettingsDialog::calculateMinAccelerationThreshold()
{
    if (qFuzzyCompare(axis->getPAxisButton()->getMinAccelThreshold(), axis->getNAxisButton()->getMinAccelThreshold()))
    {
        double temp = axis->getPAxisButton()->getMinAccelThreshold();
        ui->minThresholdDoubleSpinBox->setValue(temp);
    }
}

void MouseAxisSettingsDialog::calculateMaxAccelerationThreshold()
{
    if (qFuzzyCompare(axis->getPAxisButton()->getMaxAccelThreshold(), axis->getNAxisButton()->getMaxAccelThreshold()))
    {
        double temp = axis->getPAxisButton()->getMaxAccelThreshold();
        ui->maxThresholdDoubleSpinBox->setValue(temp);
    }
}

void MouseAxisSettingsDialog::calculateAccelExtraDuration()
{
    if (qFuzzyCompare(axis->getPAxisButton()->getAccelExtraDuration(), axis->getNAxisButton()->getAccelExtraDuration()))
    {
        double temp = axis->getPAxisButton()->getAccelExtraDuration();
        ui->accelExtraDurationDoubleSpinBox->setValue(temp);
    }
}

void MouseAxisSettingsDialog::calculateReleaseSpringRadius()
{
    int result = 0;
    if ((axis->getPAxisButton()->getSpringDeadCircleMultiplier()) ==
        (axis->getNAxisButton()->getSpringDeadCircleMultiplier()))
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
        PadderCommon::inputDaemonMutex.lock();

        axis->getPAxisButton()->setExtraAccelerationCurve(temp);
        axis->getNAxisButton()->setExtraAccelerationCurve(temp);

        PadderCommon::inputDaemonMutex.unlock();
    }
}

void MouseAxisSettingsDialog::calculateExtraAccelerationCurve()
{
    if (axis->getPAxisButton()->getExtraAccelerationCurve() == axis->getNAxisButton()->getExtraAccelerationCurve())
    {
        JoyButton::JoyExtraAccelerationCurve temp = axis->getPAxisButton()->getExtraAccelerationCurve();
        updateExtraAccelerationCurvePresetComboBox(temp);
    }
}

JoyAxis *MouseAxisSettingsDialog::getAxis() const { return axis; }

SpringModeRegionPreview *MouseAxisSettingsDialog::getSpringPreviewWidget() const { return springPreviewWidget; }

MouseAxisSettingsDialogHelper const &MouseAxisSettingsDialog::getHelper() { return helper; }

MouseAxisSettingsDialogHelper &MouseAxisSettingsDialog::getHelperLocal() { return helper; }
