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

#include "mousesettingsdialog.h"
#include "ui_mousesettingsdialog.h"

#include "globalvariables.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QSpinBox>
#include <QString>

MouseSettingsDialog::MouseSettingsDialog(QWidget *parent)
    : QDialog(parent, Qt::Window)
    , ui(new Ui::MouseSettingsDialog)
{
    ui->setupUi(this);
    ui->horizontalSpinBox->setMaximum(GlobalVariables::JoyButton::MAXMOUSESPEED);
    ui->verticalSpinBox->setMaximum(GlobalVariables::JoyButton::MAXMOUSESPEED);
    setAttribute(Qt::WA_DeleteOnClose);

    JoyButtonMouseHelper *mouseHelper = JoyButton::getMouseHelper();
    connect(mouseHelper, &JoyButtonMouseHelper::mouseCursorMoved, this, &MouseSettingsDialog::updateMouseCursorStatusLabels);
    connect(mouseHelper, &JoyButtonMouseHelper::mouseSpringMoved, this, &MouseSettingsDialog::updateMouseSpringStatusLabels);
    lastMouseStatUpdate.start();

    connect(ui->accelerationComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &MouseSettingsDialog::changeSettingsWidgetStatus);
    connect(ui->accelerationComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &MouseSettingsDialog::refreshMouseCursorSpeedValues);
    connect(ui->mouseModeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &MouseSettingsDialog::changeSpringSectionStatus);
    connect(ui->mouseModeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &MouseSettingsDialog::changeMouseSpeedBoxStatus);
    connect(ui->mouseModeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &MouseSettingsDialog::changeWheelSpeedBoxStatus);
    connect(ui->mouseModeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &MouseSettingsDialog::changeSensitivityStatusForMouseMode);

    connect(ui->horizontalSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseSettingsDialog::updateHorizontalSpeedConvertLabel);
    connect(ui->horizontalSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseSettingsDialog::moveSpeedsTogether);

    connect(ui->verticalSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseSettingsDialog::updateVerticalSpeedConvertLabel);
    connect(ui->verticalSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseSettingsDialog::moveSpeedsTogether);

    connect(ui->wheelVertSpeedSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseSettingsDialog::updateWheelVerticalSpeedLabel);
    connect(ui->wheelHoriSpeedSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
            &MouseSettingsDialog::updateWheelHorizontalSpeedLabel);

    connect(ui->relativeSpringCheckBox, &QCheckBox::clicked, this, &MouseSettingsDialog::disableReleaseSpringBox);
    connect(ui->relativeSpringCheckBox, &QCheckBox::clicked, this, &MouseSettingsDialog::resetReleaseRadius);
}

MouseSettingsDialog::~MouseSettingsDialog() { delete ui; }

void MouseSettingsDialog::changeSettingsWidgetStatus(int index)
{
    JoyButton::JoyMouseCurve temp = getMouseCurveForIndex(index);
    int currentMouseMode = ui->mouseModeComboBox->currentIndex();

    if ((currentMouseMode == 1) && (temp == JoyButton::PowerCurve))
    {
        ui->sensitivityDoubleSpinBox->setEnabled(true);
    } else
    {
        ui->sensitivityDoubleSpinBox->setEnabled(false);
    }

    if ((currentMouseMode == 1) && ((temp == JoyButton::EasingQuadraticCurve) || (temp == JoyButton::EasingCubicCurve)))
    {
        ui->easingDoubleSpinBox->setEnabled(true);
    } else
    {
        ui->easingDoubleSpinBox->setEnabled(false);
    }
}

void MouseSettingsDialog::changeSpringSectionStatus(int index)
{
    if (index == 2)
    {
        ui->springWidthSpinBox->setEnabled(true);
        ui->springHeightSpinBox->setEnabled(true);
        ui->relativeSpringCheckBox->setEnabled(true);
        bool enableSpringRadiusBox = !ui->relativeSpringCheckBox->isChecked();
        ui->releaseSpringRadiusspinBox->setEnabled(enableSpringRadiusBox);
    } else
    {
        ui->springWidthSpinBox->setEnabled(false);
        ui->springHeightSpinBox->setEnabled(false);
        ui->relativeSpringCheckBox->setEnabled(false);
        ui->releaseSpringRadiusspinBox->setEnabled(false);
    }
}

void MouseSettingsDialog::updateHorizontalSpeedConvertLabel(int value)
{
    QString label = QString(QString::number(value));

    int currentCurveIndex = ui->accelerationComboBox->currentIndex();
    JoyButton::JoyMouseCurve tempCurve = getMouseCurveForIndex(currentCurveIndex);
    int finalSpeed = JoyButton::calculateFinalMouseSpeed(tempCurve, value, GlobalVariables::JoyAxis::JOYSPEED);

    label = label.append(" = ").append(QString::number(finalSpeed)).append(" pps");
    ui->horizontalSpeedLabel->setText(label);
}

void MouseSettingsDialog::updateVerticalSpeedConvertLabel(int value)
{
    QString label = QString(QString::number(value));

    int currentCurveIndex = ui->accelerationComboBox->currentIndex();
    JoyButton::JoyMouseCurve tempCurve = getMouseCurveForIndex(currentCurveIndex);
    int finalSpeed = JoyButton::calculateFinalMouseSpeed(tempCurve, value, GlobalVariables::JoyAxis::JOYSPEED);

    label = label.append(" = ").append(QString::number(finalSpeed)).append(" pps");
    ui->verticalSpeedLabel->setText(label);
}

void MouseSettingsDialog::moveSpeedsTogether(int value)
{
    if (ui->changeMouseSpeedsTogetherCheckBox->isChecked())
    {
        ui->horizontalSpinBox->setValue(value);
        ui->verticalSpinBox->setValue(value);
    }
}

void MouseSettingsDialog::changeMouseSpeedBoxStatus(int index)
{
    if (index == 2)
    {
        ui->horizontalSpinBox->setEnabled(false);
        ui->verticalSpinBox->setEnabled(false);
        ui->changeMouseSpeedsTogetherCheckBox->setEnabled(false);
        ui->extraAccelerationGroupBox->setChecked(false);
        ui->extraAccelerationGroupBox->setEnabled(false);
    } else
    {
        ui->horizontalSpinBox->setEnabled(true);
        ui->verticalSpinBox->setEnabled(true);
        ui->changeMouseSpeedsTogetherCheckBox->setEnabled(true);
        ui->extraAccelerationGroupBox->setEnabled(true);

        if (ui->extraAccelerationGroupBox->isChecked())
            ui->extraAccelerationGroupBox->setEnabled(true);
    }
}

void MouseSettingsDialog::changeWheelSpeedBoxStatus(int index)
{
    if (index == 2)
    {
        ui->wheelHoriSpeedSpinBox->setEnabled(false);
        ui->wheelVertSpeedSpinBox->setEnabled(false);
    } else
    {
        ui->wheelHoriSpeedSpinBox->setEnabled(true);
        ui->wheelVertSpeedSpinBox->setEnabled(true);
    }
}

void MouseSettingsDialog::updateWheelVerticalSpeedLabel(int value)
{
    QString label = QString(QString::number(value));
    label.append(" = ");
    label.append(tr("%n notch(es)/s", "", value));
    ui->wheelVertSpeedUnitsLabel->setText(label);
}

void MouseSettingsDialog::updateWheelHorizontalSpeedLabel(int value)
{
    QString label = QString(QString::number(value));
    label.append(" = ");
    label.append(tr("%n notch(es)/s", "", value));
    ui->wheelHoriSpeedUnitsLabel->setText(label);
}

void MouseSettingsDialog::updateAccelerationCurvePresetComboBox(JoyButton::JoyMouseCurve mouseCurve)
{
    switch (mouseCurve)
    {
    case JoyButton::EnhancedPrecisionCurve: {
        ui->accelerationComboBox->setCurrentIndex(1);
        break;
    }
    case JoyButton::LinearCurve: {
        ui->accelerationComboBox->setCurrentIndex(2);
        break;
    }
    case JoyButton::QuadraticCurve: {
        ui->accelerationComboBox->setCurrentIndex(3);
        break;
    }
    case JoyButton::CubicCurve: {
        ui->accelerationComboBox->setCurrentIndex(4);
        break;
    }
    case JoyButton::QuadraticExtremeCurve: {
        ui->accelerationComboBox->setCurrentIndex(5);
        break;
    }
    case JoyButton::PowerCurve: {
        ui->accelerationComboBox->setCurrentIndex(6);
        break;
    }
    case JoyButton::EasingQuadraticCurve: {
        ui->accelerationComboBox->setCurrentIndex(7);
        break;
    }
    case JoyButton::EasingCubicCurve: {
        ui->accelerationComboBox->setCurrentIndex(8);
        break;
    }
    }
}

JoyButton::JoyMouseCurve MouseSettingsDialog::getMouseCurveForIndex(int index)
{
    JoyButton::JoyMouseCurve temp = JoyButton::DEFAULTMOUSECURVE;

    switch (index)
    {
    case 1: {
        temp = JoyButton::EnhancedPrecisionCurve;
        break;
    }
    case 2: {
        temp = JoyButton::LinearCurve;
        break;
    }
    case 3: {
        temp = JoyButton::QuadraticCurve;
        break;
    }
    case 4: {
        temp = JoyButton::CubicCurve;
        break;
    }
    case 5: {
        temp = JoyButton::QuadraticExtremeCurve;
        break;
    }
    case 6: {
        temp = JoyButton::PowerCurve;
        break;
    }
    case 7: {
        temp = JoyButton::EasingQuadraticCurve;
        break;
    }
    case 8: {
        temp = JoyButton::EasingCubicCurve;
        break;
    }
    }

    return temp;
}

void MouseSettingsDialog::changeSensitivityStatusForMouseMode(int index)
{
    if (index == 2)
    {
        ui->sensitivityDoubleSpinBox->setEnabled(false);
    } else if (index == 1)
    {
        int currentCurveIndex = ui->accelerationComboBox->currentIndex();
        JoyButton::JoyMouseCurve temp = getMouseCurveForIndex(currentCurveIndex);

        if (temp == JoyButton::PowerCurve)
            ui->sensitivityDoubleSpinBox->setEnabled(true);
    } else
    {
        ui->sensitivityDoubleSpinBox->setEnabled(false);
    }
}

/**
 * @brief Update mouse status labels with cursor mouse information provided by
 *      an InputDevice.
 * @param X distance in pixels
 * @param Y distance in pixels
 * @param Time elapsed for generated event
 */
void MouseSettingsDialog::updateMouseCursorStatusLabels(int mouseX, int mouseY, int elapsed)
{
    if ((lastMouseStatUpdate.elapsed() >= 100) && (elapsed > 0))
    {
        QString tempX("%1 (%2 pps) (%3 ms)");
        QString tempY("%1 (%2 pps) (%3 ms)");

        ui->mouseStatusXLabel->setText(tempX.arg(mouseX).arg(mouseX * (1000 / elapsed)).arg(elapsed));
        ui->mouseStatusYLabel->setText(tempY.arg(mouseY).arg(mouseY * (1000 / elapsed)).arg(elapsed));

        lastMouseStatUpdate.start();
    }
}

/**
 * @brief Update mouse status labels with spring mouse information
 *     provided by an InputDevice.
 * @param X coordinate of cursor
 * @param Y coordinate of cursor
 */
void MouseSettingsDialog::updateMouseSpringStatusLabels(int coordX, int coordY)
{
    if (lastMouseStatUpdate.elapsed() >= 100)
    {
        QString tempX("%1");
        QString tempY("%1");

        ui->mouseStatusXLabel->setText(tempX.arg(coordX));
        ui->mouseStatusYLabel->setText(tempY.arg(coordY));
        lastMouseStatUpdate.start();
    }
}

void MouseSettingsDialog::refreshMouseCursorSpeedValues(int index)
{
    Q_UNUSED(index);

    updateHorizontalSpeedConvertLabel(ui->horizontalSpinBox->value());
    updateVerticalSpeedConvertLabel(ui->verticalSpinBox->value());
}

void MouseSettingsDialog::disableReleaseSpringBox(bool enable) { ui->releaseSpringRadiusspinBox->setEnabled(!enable); }

void MouseSettingsDialog::resetReleaseRadius(bool enabled)
{
    if (enabled && (ui->releaseSpringRadiusspinBox->value() > 0))
    {
        ui->releaseSpringRadiusspinBox->setValue(0);
    }
}

JoyButton::JoyExtraAccelerationCurve MouseSettingsDialog::getExtraAccelCurveForIndex(int index)
{
    JoyButton::JoyExtraAccelerationCurve temp = JoyButton::LinearAccelCurve;

    switch (index)
    {
    case 1: {
        temp = JoyButton::LinearAccelCurve;
        break;
    }
    case 2: {
        temp = JoyButton::EaseOutSineCurve;
        break;
    }
    case 3: {
        temp = JoyButton::EaseOutQuadAccelCurve;
        break;
    }
    case 4: {
        temp = JoyButton::EaseOutCubicAccelCurve;
        break;
    }
    }

    return temp;
}

void MouseSettingsDialog::updateExtraAccelerationCurvePresetComboBox(JoyButton::JoyExtraAccelerationCurve curve)
{
    int temp = 0;

    switch (curve)
    {
    case JoyButton::LinearAccelCurve: {
        temp = 1;
        break;
    }
    case JoyButton::EaseOutSineCurve: {
        temp = 2;
        break;
    }
    case JoyButton::EaseOutQuadAccelCurve: {
        temp = 3;
        break;
    }
    case JoyButton::EaseOutCubicAccelCurve: {
        temp = 4;
        break;
    }
    }

    ui->extraAccelCurveComboBox->setCurrentIndex(temp);
}
