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

#include "messagehandler.h"
#include "springmoderegionpreview.h"
#include "joydpad.h"

#include "inputdevice.h"
#include "setjoystick.h"
#include "common.h"

#include <QSpinBox>
#include <QComboBox>
#include <QDebug>


MouseDPadSettingsDialog::MouseDPadSettingsDialog(JoyDPad *dpad, QWidget *parent) :
    MouseSettingsDialog(parent),
    helper(dpad)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    setAttribute(Qt::WA_DeleteOnClose);
    resize(size().width(), 450);

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

    connect(this, &MouseDPadSettingsDialog::finished, springPreviewWidget, &SpringModeRegionPreview::deleteLater);

    connect(ui->mouseModeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MouseDPadSettingsDialog::changeMouseMode);
    connect(ui->accelerationComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MouseDPadSettingsDialog::changeMouseCurve);

    connect(ui->horizontalSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MouseDPadSettingsDialog::updateConfigHorizontalSpeed);
    connect(ui->verticalSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MouseDPadSettingsDialog::updateConfigVerticalSpeed);

    connect(ui->springWidthSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MouseDPadSettingsDialog::updateSpringWidth);
    connect(ui->springWidthSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), springPreviewWidget, &SpringModeRegionPreview::setSpringWidth);

    connect(ui->springHeightSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MouseDPadSettingsDialog::updateSpringHeight);
    connect(ui->springHeightSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), springPreviewWidget, &SpringModeRegionPreview::setSpringHeight);

    connect(ui->relativeSpringCheckBox, &QCheckBox::clicked, this, &MouseDPadSettingsDialog::updateSpringRelativeStatus);

    connect(ui->sensitivityDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &MouseDPadSettingsDialog::updateSensitivity);

    connect(ui->wheelHoriSpeedSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MouseDPadSettingsDialog::updateWheelSpeedHorizontalSpeed);
    connect(ui->wheelVertSpeedSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MouseDPadSettingsDialog::updateWheelSpeedVerticalSpeed);

    connect(ui->easingDoubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), dpad, &JoyDPad::setButtonsEasingDuration);

    connect(ui->releaseSpringRadiusspinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MouseDPadSettingsDialog::updateReleaseSpringRadius);
    connect(ui->extraAccelCurveComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MouseDPadSettingsDialog::updateExtraAccelerationCurve);

    JoyButtonMouseHelper *mouseHelper = JoyButton::getMouseHelper();
    connect(mouseHelper, &JoyButtonMouseHelper::mouseCursorMoved, this, &MouseDPadSettingsDialog::updateMouseCursorStatusLabels);
    connect(mouseHelper, &JoyButtonMouseHelper::mouseSpringMoved, this, &MouseDPadSettingsDialog::updateMouseSpringStatusLabels);
    lastMouseStatUpdate.start();
}

void MouseDPadSettingsDialog::changeMouseMode(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButton::JoyMouseCurve temp = MouseSettingsDialog::getMouseCurveForIndex(index);
    dpad->setButtonsMouseCurve(temp);
}

void MouseDPadSettingsDialog::updateConfigHorizontalSpeed(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<int, JoyDPadButton*> iter(*dpad->getButtons());
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->setMouseSpeedX(value);
    }
}

void MouseDPadSettingsDialog::updateConfigVerticalSpeed(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<int, JoyDPadButton*> iter(*dpad->getButtons());
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->setMouseSpeedY(value);
    }
}

void MouseDPadSettingsDialog::updateSpringWidth(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    dpad->setButtonsSpringWidth(value);
}

void MouseDPadSettingsDialog::updateSpringHeight(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    dpad->setButtonsSpringHeight(value);
}

void MouseDPadSettingsDialog::selectCurrentMouseModePreset()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    dpad->setButtonsSensitivity(value);
}

void MouseDPadSettingsDialog::updateAccelerationCurvePresetComboBox()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButton::JoyMouseCurve temp = dpad->getButtonsPresetMouseCurve();
    MouseSettingsDialog::updateAccelerationCurvePresetComboBox(temp);
}

void MouseDPadSettingsDialog::calculateWheelSpeedPreset()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    dpad->setButtonsWheelSpeedX(value);
}

void MouseDPadSettingsDialog::updateWheelSpeedVerticalSpeed(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    dpad->setButtonsWheelSpeedY(value);
}

void MouseDPadSettingsDialog::updateSpringRelativeStatus(bool value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    dpad->setButtonsSpringRelativeStatus(value);
}

void MouseDPadSettingsDialog::updateWindowTitleDPadName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString(trUtf8("Mouse Settings")).append(" - ");

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
        int setIndex = dpad->getParentSet()->getRealIndex();
        temp.append(" [").append(trUtf8("Set %1").arg(setIndex));

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    dpad->setButtonsSpringDeadCircleMultiplier(value);
}

void MouseDPadSettingsDialog::calculateReleaseSpringRadius()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->releaseSpringRadiusspinBox->setValue(dpad->getButtonsSpringDeadCircleMultiplier());
}

void MouseDPadSettingsDialog::calculateExtraAccelerationCurve()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButton::JoyExtraAccelerationCurve curve = dpad->getButtonsExtraAccelerationCurve();
    updateExtraAccelerationCurvePresetComboBox(curve);
}

void MouseDPadSettingsDialog::updateExtraAccelerationCurve(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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

JoyDPad *MouseDPadSettingsDialog::getDPad() const {

    return dpad;
}

SpringModeRegionPreview *MouseDPadSettingsDialog::getSpringPreviewWidget() const {

    return springPreviewWidget;
}

MouseDpadSettingsDialogHelper const& MouseDPadSettingsDialog::getHelper() {

    return helper;
}

MouseDpadSettingsDialogHelper& MouseDPadSettingsDialog::getHelperLocal() {

    return helper;
}
