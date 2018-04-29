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


#include "mousecontrolsticksettingsdialog.h"

#include "messagehandler.h"
#include "springmoderegionpreview.h"
#include "joycontrolstick.h"
#include "inputdevice.h"
#include "setjoystick.h"
#include "joybuttontypes/joycontrolstickbutton.h"
#include "common.h"

#include <QSpinBox>
#include <QComboBox>
#include <QHashIterator>
#include <QDebug>


MouseControlStickSettingsDialog::MouseControlStickSettingsDialog(JoyControlStick *stick, QWidget *parent) :
    MouseSettingsDialog(parent),
    helper(stick)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
        springPreviewWidget = new SpringModeRegionPreview(ui->springWidthSpinBox->value(),
                                                          ui->springHeightSpinBox->value());
    }
    else
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

    connect(ui->easingDoubleSpinBox, SIGNAL(valueChanged(double)), stick, SLOT(setButtonsEasingDuration(double)));

    connect(ui->extraAccelerationGroupBox, SIGNAL(clicked(bool)), &helper, SLOT(updateExtraAccelerationStatus(bool)));
    connect(ui->extraAccelDoubleSpinBox, SIGNAL(valueChanged(double)), &helper, SLOT(updateExtraAccelerationMultiplier(double)));
    connect(ui->minMultiDoubleSpinBox, SIGNAL(valueChanged(double)), &helper, SLOT(updateStartMultiPercentage(double)));
    connect(ui->minThresholdDoubleSpinBox, SIGNAL(valueChanged(double)), &helper, SLOT(updateMinAccelThreshold(double)));
    connect(ui->maxThresholdDoubleSpinBox, SIGNAL(valueChanged(double)), &helper, SLOT(updateMaxAccelThreshold(double)));
    connect(ui->accelExtraDurationDoubleSpinBox, SIGNAL(valueChanged(double)), &helper, SLOT(updateAccelExtraDuration(double)));
    connect(ui->releaseSpringRadiusspinBox, SIGNAL(valueChanged(int)), &helper, SLOT(updateReleaseSpringRadius(int)));

    connect(ui->extraAccelCurveComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateExtraAccelerationCurve(int)));
}

void MouseControlStickSettingsDialog::changeMouseMode(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (index == 1)
    {
        stick->setButtonsMouseMode(JoyButton::MouseCursor);
        if (springPreviewWidget->isVisible())
        {
            springPreviewWidget->hide();
        }
    }
    else if (index == 2)
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButton::JoyMouseCurve temp = MouseSettingsDialog::getMouseCurveForIndex(index);
    stick->setButtonsMouseCurve(temp);
}

void MouseControlStickSettingsDialog::updateConfigHorizontalSpeed(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> iter(*stick->getButtons());
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setMouseSpeedX(value);
    }
}

void MouseControlStickSettingsDialog::updateConfigVerticalSpeed(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> iter(*stick->getButtons());
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setMouseSpeedY(value);
    }
}

void MouseControlStickSettingsDialog::updateSpringWidth(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    stick->setButtonsSpringWidth(value);
}

void MouseControlStickSettingsDialog::updateSpringHeight(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    stick->setButtonsSpringHeight(value);
}

void MouseControlStickSettingsDialog::selectCurrentMouseModePreset()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool presetDefined = stick->hasSameButtonsMouseMode();
    if (presetDefined)
    {
        JoyButton::JoyMouseMovementMode mode = stick->getButtonsPresetMouseMode();
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

void MouseControlStickSettingsDialog::calculateSpringPreset()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> iter(*stick->getButtons());
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

void MouseControlStickSettingsDialog::updateSensitivity(double value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    stick->setButtonsSensitivity(value);
}

void MouseControlStickSettingsDialog::updateAccelerationCurvePresetComboBox()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButton::JoyMouseCurve temp = stick->getButtonsPresetMouseCurve();
    MouseSettingsDialog::updateAccelerationCurvePresetComboBox(temp);
}

void MouseControlStickSettingsDialog::calculateWheelSpeedPreset()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> iter(*stick->getButtons());
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

void MouseControlStickSettingsDialog::updateWheelSpeedHorizontalSpeed(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    stick->setButtonsWheelSpeedX(value);
}

void MouseControlStickSettingsDialog::updateWheelSpeedVerticalSpeed(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    stick->setButtonsWheelSpeedY(value);
}

void MouseControlStickSettingsDialog::updateSpringRelativeStatus(bool value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    stick->setButtonsSpringRelativeStatus(value);
}

void MouseControlStickSettingsDialog::updateWindowTitleStickName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString(trUtf8("Mouse Settings")).append(" - ");

    if (!stick->getStickName().isEmpty())
    {
        temp.append(stick->getPartialName(false, true));
    }
    else
    {
        temp.append(stick->getPartialName());
    }

    if (stick->getParentSet()->getIndex() != 0)
    {
        int setIndex = stick->getParentSet()->getRealIndex();
        temp.append(" [").append(trUtf8("Set %1").arg(setIndex));

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (stick->getButtonsExtraAccelerationStatus())
    {
        ui->extraAccelerationGroupBox->setChecked(true);
    }
}

void MouseControlStickSettingsDialog::calculateExtraAccelerationMultiplier()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->extraAccelDoubleSpinBox->setValue(stick->getButtonsExtraAccelerationMultiplier());
}

void MouseControlStickSettingsDialog::calculateStartAccelerationMultiplier()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->minMultiDoubleSpinBox->setValue(stick->getButtonsStartAccelerationMultiplier());
}

void MouseControlStickSettingsDialog::calculateMinAccelerationThreshold()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->minThresholdDoubleSpinBox->setValue(stick->getButtonsMinAccelerationThreshold());
}

void MouseControlStickSettingsDialog::calculateMaxAccelerationThreshold()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->maxThresholdDoubleSpinBox->setValue(stick->getButtonsMaxAccelerationThreshold());
}

void MouseControlStickSettingsDialog::calculateAccelExtraDuration()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->accelExtraDurationDoubleSpinBox->setValue(stick->getButtonsAccelerationEasingDuration());
}

void MouseControlStickSettingsDialog::calculateReleaseSpringRadius()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->releaseSpringRadiusspinBox->setValue(stick->getButtonsSpringDeadCircleMultiplier());
}

void MouseControlStickSettingsDialog::calculateExtraAccelerationCurve()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButton::JoyExtraAccelerationCurve curve = stick->getButtonsExtraAccelerationCurve();
    updateExtraAccelerationCurvePresetComboBox(curve);
}

void MouseControlStickSettingsDialog::updateExtraAccelerationCurve(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButton::JoyExtraAccelerationCurve temp = getExtraAccelCurveForIndex(index);
    if (index > 0)
    {
        PadderCommon::inputDaemonMutex.lock();
        stick->setButtonsExtraAccelCurve(temp);
        PadderCommon::inputDaemonMutex.unlock();
    }
}


JoyControlStick* MouseControlStickSettingsDialog::getStick() const {

    return stick;
}

SpringModeRegionPreview* MouseControlStickSettingsDialog::getSpringPreviewWidget() const {

    return springPreviewWidget;
}

MouseControlStickSettingsDialogHelper const& MouseControlStickSettingsDialog::getHelper() {

    return helper;
}

MouseControlStickSettingsDialogHelper& MouseControlStickSettingsDialog::getHelperLocal() {

    return helper;
}
