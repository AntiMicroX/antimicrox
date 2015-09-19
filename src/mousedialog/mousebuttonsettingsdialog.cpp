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

#include "mousebuttonsettingsdialog.h"
#include "ui_mousesettingsdialog.h"
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>

#include <inputdevice.h>
#include <setjoystick.h>

MouseButtonSettingsDialog::MouseButtonSettingsDialog(JoyButton *button, QWidget *parent) :
    MouseSettingsDialog(parent),
    helper(button)
{
    setAttribute(Qt::WA_DeleteOnClose);
    resize(size().width(), 450);
    //setGeometry(geometry().x(), geometry().y(), size().width(), 450);

    this->button = button;
    helper.moveToThread(button->thread());

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
        springPreviewWidget = new SpringModeRegionPreview(ui->springWidthSpinBox->value(),
                                                          ui->springHeightSpinBox->value());
    }
    else
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
    }
    else
    {
        ui->extraAccelerationGroupBox->setVisible(false);
    }

    ui->releaseSpringRadiusspinBox->setValue(button->getSpringDeadCircleMultiplier());
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

    connect(ui->relativeSpringCheckBox, SIGNAL(clicked(bool)), &helper, SLOT(updateSpringRelativeStatus(bool)));

    connect(ui->sensitivityDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateSensitivity(double)));

    connect(ui->wheelHoriSpeedSpinBox, SIGNAL(valueChanged(int)), button, SLOT(setWheelSpeedX(int)));
    connect(ui->wheelVertSpeedSpinBox, SIGNAL(valueChanged(int)), button, SLOT(setWheelSpeedY(int)));

    connect(ui->easingDoubleSpinBox, SIGNAL(valueChanged(double)), button, SLOT(setEasingDuration(double)));

    connect(ui->extraAccelerationGroupBox, SIGNAL(clicked(bool)), &helper, SLOT(updateExtraAccelerationStatus(bool)));
    connect(ui->extraAccelDoubleSpinBox, SIGNAL(valueChanged(double)), &helper, SLOT(updateExtraAccelerationMultiplier(double)));
    connect(ui->minMultiDoubleSpinBox, SIGNAL(valueChanged(double)), &helper, SLOT(updateStartMultiPercentage(double)));
    connect(ui->minThresholdDoubleSpinBox, SIGNAL(valueChanged(double)), &helper, SLOT(updateMinAccelThreshold(double)));
    connect(ui->maxThresholdDoubleSpinBox, SIGNAL(valueChanged(double)), &helper, SLOT(updateMaxAccelThreshold(double)));
    connect(ui->accelExtraDurationDoubleSpinBox, SIGNAL(valueChanged(double)), &helper, SLOT(updateAccelExtraDuration(double)));
    connect(ui->releaseSpringRadiusspinBox, SIGNAL(valueChanged(int)), &helper, SLOT(updateReleaseSpringRadius(int)));

    connect(ui->extraAccelCurveComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateExtraAccelerationCurve(int)));
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
    }
    else if (index == 2)
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
    }
    else if (mode == JoyButton::MouseSpring)
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

void MouseButtonSettingsDialog::updateSensitivity(double value)
{
    button->setSensitivity(value);
}

void MouseButtonSettingsDialog::updateAccelerationCurvePresetComboBox()
{
    JoyButton::JoyMouseCurve temp = button->getMouseCurve();
    MouseSettingsDialog::updateAccelerationCurvePresetComboBox(temp);
}

/*void MouseButtonSettingsDialog::updateSpringRelativeStatus(bool value)
{
    button->setSpringRelativeStatus(value);
}
*/

void MouseButtonSettingsDialog::updateWindowTitleButtonName()
{
    QString temp;
    temp.append(tr("Mouse Settings - ")).append(button->getPartialName(false, true));


    if (button->getParentSet()->getIndex() != 0)
    {
        unsigned int setIndex = button->getParentSet()->getRealIndex();
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
