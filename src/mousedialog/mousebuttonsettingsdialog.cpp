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

#include "mousebuttonsettingsdialog.h"

#include "common.h"
#include "inputdevice.h"
#include "joybuttontypes/joybutton.h"
#include "setjoystick.h"
#include "springmoderegionpreview.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QMetaObject>
#include <QSpinBox>

MouseButtonSettingsDialog::MouseButtonSettingsDialog(JoyButton *button, QWidget *parent)
    : MouseSettingsDialog(parent)
    , helper(button)
{
    setAttribute(Qt::WA_DeleteOnClose);
    resize(size().width(), 450);

    this->button = button;
    getHelperLocal().moveToThread(button->thread());

    calculateMouseSpeedPreset();
    selectCurrentMouseModePreset();
    calculateSpringPreset();

    if (button->getSensitivity() > 0.0)
    {
        ui->sensitivityDoubleSpinBox->setValue(button->getSensitivity());
    }

    updateAccelerationCurvePresetComboBox();
    updateWindowTitleButtonName();

    if (ui->mouseModeComboBox->currentIndex() == 2)
    {
        springPreviewWidget = new SpringModeRegionPreview(ui->springWidthSpinBox->value(), ui->springHeightSpinBox->value());
    } else
    {
        springPreviewWidget = new SpringModeRegionPreview(0, 0);
    }

    ui->wheelHoriSpeedSpinBox->setValue(button->getWheelSpeedX());
    ui->wheelVertSpeedSpinBox->setValue(button->getWheelSpeedY());

    if (button->isRelativeSpring())
    {
        ui->relativeSpringCheckBox->setChecked(true);
    }

    double easingDuration = button->getEasingDuration();
    ui->easingDoubleSpinBox->setValue(easingDuration);

    if (button->isPartRealAxis())
    {
        ui->extraAccelerationGroupBox->setChecked(button->isExtraAccelerationEnabled());
        ui->extraAccelDoubleSpinBox->setValue(button->getExtraAccelerationMultiplier());
        ui->minMultiDoubleSpinBox->setValue(button->getStartAccelMultiplier());
        ui->minThresholdDoubleSpinBox->setValue(button->getMinAccelThreshold());
        ui->maxThresholdDoubleSpinBox->setValue(button->getMaxAccelThreshold());
        ui->accelExtraDurationDoubleSpinBox->setValue(button->getAccelExtraDuration());
    } else
    {
        ui->extraAccelerationGroupBox->setVisible(false);
    }

    ui->releaseSpringRadiusspinBox->setValue(button->getSpringDeadCircleMultiplier());
    calculateExtraAccelerationCurve();

    changeSpringSectionStatus(ui->mouseModeComboBox->currentIndex());
    changeSettingsWidgetStatus(ui->accelerationComboBox->currentIndex());

    connect(this, &MouseButtonSettingsDialog::finished, springPreviewWidget, &SpringModeRegionPreview::deleteLater);

    connect(ui->mouseModeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &MouseButtonSettingsDialog::changeMouseMode);
    connect(ui->accelerationComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &MouseButtonSettingsDialog::changeMouseCurve);

    connect(ui->horizontalSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseButtonSettingsDialog::updateConfigHorizontalSpeed);
    connect(ui->verticalSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseButtonSettingsDialog::updateConfigVerticalSpeed);

    connect(ui->springWidthSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseButtonSettingsDialog::updateSpringWidth);
    connect(ui->springWidthSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), springPreviewWidget,
            &SpringModeRegionPreview::setSpringWidth);

    connect(ui->springHeightSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseButtonSettingsDialog::updateSpringHeight);
    connect(ui->springHeightSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), springPreviewWidget,
            &SpringModeRegionPreview::setSpringHeight);

    connect(ui->relativeSpringCheckBox, &QCheckBox::clicked, &helper,
            &MouseButtonSettingsDialogHelper::updateSpringRelativeStatus);

    connect(ui->sensitivityDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this,
            &MouseButtonSettingsDialog::updateSensitivity);

    QChar x = 'X';
    QChar y = 'Y';

    connect(ui->wheelHoriSpeedSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), button,
            [button, x](int value) { button->setWheelSpeed(value, x); });
    connect(ui->wheelVertSpeedSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), button,
            [button, y](int value) { button->setWheelSpeed(value, y); });

    connect(ui->easingDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), button,
            &JoyButton::setEasingDuration);
    connect(ui->extraAccelerationGroupBox, &QGroupBox::clicked, &helper,
            &MouseButtonSettingsDialogHelper::updateExtraAccelerationStatus);
    connect(ui->extraAccelDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            &helper, &MouseButtonSettingsDialogHelper::updateExtraAccelerationMultiplier);
    connect(ui->minMultiDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), &helper,
            &MouseButtonSettingsDialogHelper::updateStartMultiPercentage);
    connect(ui->minThresholdDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            &helper, &MouseButtonSettingsDialogHelper::updateMinAccelThreshold);
    connect(ui->maxThresholdDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            &helper, &MouseButtonSettingsDialogHelper::updateMaxAccelThreshold);
    connect(ui->accelExtraDurationDoubleSpinBox,
            static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), &helper,
            &MouseButtonSettingsDialogHelper::updateAccelExtraDuration);
    connect(ui->releaseSpringRadiusspinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), &helper,
            &MouseButtonSettingsDialogHelper::updateReleaseSpringRadius);

    connect(ui->extraAccelCurveComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &MouseButtonSettingsDialog::updateExtraAccelerationCurve);
}

void MouseButtonSettingsDialog::changeMouseMode(int index)
{
    if (index == 1)
    {
        button->setMouseMode(JoyButton::MouseCursor);
        if (springPreviewWidget->isVisible())
        {
            springPreviewWidget->hide();
        }
    } else if (index == 2)
    {
        button->setMouseMode(JoyButton::MouseSpring);
        if (!springPreviewWidget->isVisible())
        {
            springPreviewWidget->setSpringWidth(ui->springWidthSpinBox->value());
            springPreviewWidget->setSpringHeight(ui->springHeightSpinBox->value());
        }

        if (button->isPartRealAxis())
        {
            button->setExtraAccelerationStatus(false);
        }
    }
}

void MouseButtonSettingsDialog::changeMouseCurve(int index)
{
    JoyButton::JoyMouseCurve temp = MouseSettingsDialog::getMouseCurveForIndex(index);
    button->setMouseCurve(temp);
}

void MouseButtonSettingsDialog::updateConfigHorizontalSpeed(int value)
{
    QMetaObject::invokeMethod(button, "setMouseSpeedX", Q_ARG(int, value));
}

void MouseButtonSettingsDialog::updateConfigVerticalSpeed(int value)
{
    QMetaObject::invokeMethod(button, "setMouseSpeedY", Q_ARG(int, value));
}

void MouseButtonSettingsDialog::updateSpringWidth(int value)
{
    QMetaObject::invokeMethod(button, "setSpringWidth", Q_ARG(int, value));
}

void MouseButtonSettingsDialog::updateSpringHeight(int value)
{
    QMetaObject::invokeMethod(button, "setSpringHeight", Q_ARG(int, value));
}

void MouseButtonSettingsDialog::selectCurrentMouseModePreset()
{
    JoyButton::JoyMouseMovementMode mode = button->getMouseMode();
    if (mode == JoyButton::MouseCursor)
    {
        ui->mouseModeComboBox->setCurrentIndex(1);
    } else if (mode == JoyButton::MouseSpring)
    {
        ui->mouseModeComboBox->setCurrentIndex(2);
    }
}

void MouseButtonSettingsDialog::calculateSpringPreset()
{
    int tempWidth = button->getSpringWidth();
    int tempHeight = button->getSpringHeight();

    if (tempWidth > 0)
    {
        ui->springWidthSpinBox->setValue(tempWidth);
    }

    if (tempHeight > 0)
    {
        ui->springHeightSpinBox->setValue(tempHeight);
    }
}

void MouseButtonSettingsDialog::calculateMouseSpeedPreset()
{
    int tempMouseSpeedX = button->getMouseSpeedX();
    int tempMouseSpeedY = button->getMouseSpeedY();

    ui->horizontalSpinBox->setValue(tempMouseSpeedX);
    ui->verticalSpinBox->setValue(tempMouseSpeedY);
}

void MouseButtonSettingsDialog::updateSensitivity(double value) { button->setSensitivity(value); }

void MouseButtonSettingsDialog::updateAccelerationCurvePresetComboBox()
{
    JoyButton::JoyMouseCurve temp = button->getMouseCurve();
    MouseSettingsDialog::updateAccelerationCurvePresetComboBox(temp);
}

void MouseButtonSettingsDialog::updateWindowTitleButtonName()
{
    QString temp = QString();
    temp.append(tr("Mouse Settings - ")).append(button->getPartialName(false, true));

    if (button->getParentSet()->getIndex() != 0)
    {
        int setIndex = button->getParentSet()->getRealIndex();
        temp.append(" [").append(tr("Set %1").arg(setIndex));

        QString setName = button->getParentSet()->getName();
        if (!setName.isEmpty())
        {
            temp.append(": ").append(setName);
        }

        temp.append("]");
    }

    setWindowTitle(temp);
}

void MouseButtonSettingsDialog::calculateExtraAccelerationCurve()
{
    JoyButton::JoyExtraAccelerationCurve temp = button->getExtraAccelerationCurve();
    updateExtraAccelerationCurvePresetComboBox(temp);
}

void MouseButtonSettingsDialog::updateExtraAccelerationCurve(int index)
{
    JoyButton::JoyExtraAccelerationCurve temp = getExtraAccelCurveForIndex(index);

    if (index > 0)
    {
        PadderCommon::inputDaemonMutex.lock();

        button->setExtraAccelerationCurve(temp);
        button->setExtraAccelerationCurve(temp);

        PadderCommon::inputDaemonMutex.unlock();
    }
}

JoyButton *MouseButtonSettingsDialog::getButton() const { return button; }

SpringModeRegionPreview *MouseButtonSettingsDialog::getSpringPreviewWidget() const { return springPreviewWidget; }

MouseButtonSettingsDialogHelper const &MouseButtonSettingsDialog::getHelper() { return helper; }

MouseButtonSettingsDialogHelper &MouseButtonSettingsDialog::getHelperLocal() { return helper; }
