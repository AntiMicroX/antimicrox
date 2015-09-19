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

#include "mousedpadsettingsdialog.h"
#include "ui_mousesettingsdialog.h"

#include <QSpinBox>
#include <QComboBox>

#include <inputdevice.h>
#include <setjoystick.h>

MouseDPadSettingsDialog::MouseDPadSettingsDialog(JoyDPad *dpad, QWidget *parent) :
    MouseSettingsDialog(parent),
    helper(dpad)
{
    setAttribute(Qt::WA_DeleteOnClose);
    resize(size().width(), 450);
    //setGeometry(geometry().x(), geometry().y(), size().width(), 450);

    this->dpad = dpad;
    helper.moveToThread(dpad->thread());

    calculateMouseSpeedPreset();
    selectCurrentMouseModePreset();
    calculateSpringPreset();

    if (dpad->getButtonsPresetSensitivity() > 0.0)
    {
        ui->sensitivityDoubleSpinBox->setValue(dpad->getButtonsPresetSensitivity());
    }
    updateAccelerationCurvePresetComboBox();

    updateWindowTitleDPadName();

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

    if (dpad->isRelativeSpring())
    {
        ui->relativeSpringCheckBox->setChecked(true);
    }

    double easingDuration = dpad->getButtonsEasingDuration();
    ui->easingDoubleSpinBox->setValue(easingDuration);

    ui->extraAccelerationGroupBox->setVisible(false);

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

    connect(ui->easingDoubleSpinBox, SIGNAL(valueChanged(double)), dpad, SLOT(setButtonsEasingDuration(double)));

    connect(ui->releaseSpringRadiusspinBox, SIGNAL(valueChanged(int)), this, SLOT(updateReleaseSpringRadius(int)));
    connect(ui->extraAccelCurveComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateExtraAccelerationCurve(int)));

    JoyButtonMouseHelper *mouseHelper = JoyButton::getMouseHelper();
    connect(mouseHelper, SIGNAL(mouseCursorMoved(int,int,int)), this, SLOT(updateMouseCursorStatusLabels(int,int,int)));
    connect(mouseHelper, SIGNAL(mouseSpringMoved(int,int)), this, SLOT(updateMouseSpringStatusLabels(int,int)));
    lastMouseStatUpdate.start();
}

void MouseDPadSettingsDialog::changeMouseMode(int index)
{
    if (index == 1)
    {
        dpad->setButtonsMouseMode(JoyButton::MouseCursor);
        if (springPreviewWidget->isVisible())
        {
            springPreviewWidget->hide();
        }
    }
    else if (index == 2)
    {
        dpad->setButtonsMouseMode(JoyButton::MouseSpring);
        if (!springPreviewWidget->isVisible())
        {
            springPreviewWidget->setSpringWidth(ui->springWidthSpinBox->value());
            springPreviewWidget->setSpringHeight(ui->springHeightSpinBox->value());
        }
    }
}

void MouseDPadSettingsDialog::changeMouseCurve(int index)
{
    JoyButton::JoyMouseCurve temp = MouseSettingsDialog::getMouseCurveForIndex(index);
    dpad->setButtonsMouseCurve(temp);
}

void MouseDPadSettingsDialog::updateConfigHorizontalSpeed(int value)
{
    QHashIterator<int, JoyDPadButton*> iter(*dpad->getButtons());
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->setMouseSpeedX(value);
    }
}

void MouseDPadSettingsDialog::updateConfigVerticalSpeed(int value)
{
    QHashIterator<int, JoyDPadButton*> iter(*dpad->getButtons());
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->setMouseSpeedY(value);
    }
}

void MouseDPadSettingsDialog::updateSpringWidth(int value)
{
    dpad->setButtonsSpringWidth(value);
}

void MouseDPadSettingsDialog::updateSpringHeight(int value)
{
    dpad->setButtonsSpringHeight(value);
}

void MouseDPadSettingsDialog::selectCurrentMouseModePreset()
{
    bool presetDefined = dpad->hasSameButtonsMouseMode();
    if (presetDefined)
    {
        JoyButton::JoyMouseMovementMode mode = dpad->getButtonsPresetMouseMode();
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

void MouseDPadSettingsDialog::calculateSpringPreset()
{
    int tempWidth = dpad->getButtonsPresetSpringWidth();
    int tempHeight = dpad->getButtonsPresetSpringHeight();

    if (tempWidth > 0)
    {
        ui->springWidthSpinBox->setValue(tempWidth);
    }

    if (tempHeight > 0)
    {
        ui->springHeightSpinBox->setValue(tempHeight);
    }
}

void MouseDPadSettingsDialog::calculateMouseSpeedPreset()
{
    QHashIterator<int, JoyDPadButton*> iter(*dpad->getButtons());
    int tempMouseSpeedX = 0;
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        tempMouseSpeedX = qMax(tempMouseSpeedX, button->getMouseSpeedX());
    }

    iter.toFront();
    int tempMouseSpeedY = 0;
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        tempMouseSpeedY = qMax(tempMouseSpeedY, button->getMouseSpeedY());
    }

    ui->horizontalSpinBox->setValue(tempMouseSpeedX);
    ui->verticalSpinBox->setValue(tempMouseSpeedY);
}

void MouseDPadSettingsDialog::updateSensitivity(double value)
{
    dpad->setButtonsSensitivity(value);
}

void MouseDPadSettingsDialog::updateAccelerationCurvePresetComboBox()
{
    JoyButton::JoyMouseCurve temp = dpad->getButtonsPresetMouseCurve();
    MouseSettingsDialog::updateAccelerationCurvePresetComboBox(temp);
}

void MouseDPadSettingsDialog::calculateWheelSpeedPreset()
{
    QHashIterator<int, JoyDPadButton*> iter(*dpad->getButtons());
    int tempWheelSpeedX = 0;
    int tempWheelSpeedY = 0;
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        tempWheelSpeedX = qMax(tempWheelSpeedX, button->getWheelSpeedX());
        tempWheelSpeedY = qMax(tempWheelSpeedY, button->getWheelSpeedY());
    }

    ui->wheelHoriSpeedSpinBox->setValue(tempWheelSpeedX);
    ui->wheelVertSpeedSpinBox->setValue(tempWheelSpeedY);
}

void MouseDPadSettingsDialog::updateWheelSpeedHorizontalSpeed(int value)
{
    dpad->setButtonsWheelSpeedX(value);
}

void MouseDPadSettingsDialog::updateWheelSpeedVerticalSpeed(int value)
{
    dpad->setButtonsWheelSpeedY(value);
}

void MouseDPadSettingsDialog::updateSpringRelativeStatus(bool value)
{
    dpad->setButtonsSpringRelativeStatus(value);
}

void MouseDPadSettingsDialog::updateWindowTitleDPadName()
{
    QString temp = QString(tr("Mouse Settings")).append(" - ");

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

void MouseDPadSettingsDialog::updateReleaseSpringRadius(int value)
{
    dpad->setButtonsSpringDeadCircleMultiplier(value);
}

void MouseDPadSettingsDialog::calculateReleaseSpringRadius()
{
    ui->releaseSpringRadiusspinBox->setValue(dpad->getButtonsSpringDeadCircleMultiplier());
}

void MouseDPadSettingsDialog::calculateExtraAccelerationCurve()
{
    JoyButton::JoyExtraAccelerationCurve curve = dpad->getButtonsExtraAccelerationCurve();
    updateExtraAccelerationCurvePresetComboBox(curve);
}

void MouseDPadSettingsDialog::updateExtraAccelerationCurve(int index)
{
    JoyButton::JoyExtraAccelerationCurve temp = JoyButton::LinearAccelCurve;

    if (index > 0)
    {
        InputDevice *device = dpad->getParentSet()->getInputDevice();

        PadderCommon::lockInputDevices();
        QMetaObject::invokeMethod(device, "haltServices", Qt::BlockingQueuedConnection);

        temp = getExtraAccelCurveForIndex(index);
        dpad->setButtonsExtraAccelerationCurve(temp);

        PadderCommon::unlockInputDevices();
    }
}
