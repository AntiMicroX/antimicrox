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

#include <QString>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>

#include "mousesettingsdialog.h"
#include "ui_mousesettingsdialog.h"

MouseSettingsDialog::MouseSettingsDialog(QWidget *parent) :
    QDialog(parent, Qt::Window),
    ui(new Ui::MouseSettingsDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    JoyButtonMouseHelper *mouseHelper = JoyButton::getMouseHelper();
    connect(mouseHelper, SIGNAL(mouseCursorMoved(int,int,int)), this, SLOT(updateMouseCursorStatusLabels(int,int,int)));
    connect(mouseHelper, SIGNAL(mouseSpringMoved(int,int)), this, SLOT(updateMouseSpringStatusLabels(int,int)));
    lastMouseStatUpdate.start();

    connect(ui->accelerationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeSettingsWidgetStatus(int)));
    connect(ui->accelerationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(refreshMouseCursorSpeedValues(int)));
    connect(ui->mouseModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeSpringSectionStatus(int)));
    connect(ui->mouseModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMouseSpeedBoxStatus(int)));
    connect(ui->mouseModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeWheelSpeedBoxStatus(int)));
    connect(ui->mouseModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeSensitivityStatusForMouseMode(int)));

    connect(ui->horizontalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateHorizontalSpeedConvertLabel(int)));
    connect(ui->horizontalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(moveSpeedsTogether(int)));

    connect(ui->verticalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateVerticalSpeedConvertLabel(int)));
    connect(ui->verticalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(moveSpeedsTogether(int)));

    connect(ui->wheelVertSpeedSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateWheelVerticalSpeedLabel(int)));
    connect(ui->wheelHoriSpeedSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateWheelHorizontalSpeedLabel(int)));

    connect(ui->relativeSpringCheckBox, SIGNAL(clicked(bool)), this, SLOT(disableReleaseSpringBox(bool)));
    connect(ui->relativeSpringCheckBox, SIGNAL(clicked(bool)), this, SLOT(resetReleaseRadius(bool)));
}

MouseSettingsDialog::~MouseSettingsDialog()
{
    delete ui;
}

void MouseSettingsDialog::changeSettingsWidgetStatus(int index)
{
    JoyButton::JoyMouseCurve temp = getMouseCurveForIndex(index);
    int currentMouseMode = ui->mouseModeComboBox->currentIndex();

    if (currentMouseMode == 1 && temp == JoyButton::PowerCurve)
    {
        ui->sensitivityDoubleSpinBox->setEnabled(true);
    }
    else
    {
        ui->sensitivityDoubleSpinBox->setEnabled(false);
    }

    if (currentMouseMode == 1 && (temp == JoyButton::EasingQuadraticCurve ||
                                  temp == JoyButton::EasingCubicCurve))
    {
        ui->easingDoubleSpinBox->setEnabled(true);
    }
    else
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
    }
    else
    {
        ui->springWidthSpinBox->setEnabled(false);
        ui->springHeightSpinBox->setEnabled(false);
        ui->relativeSpringCheckBox->setEnabled(false);
        ui->releaseSpringRadiusspinBox->setEnabled(false);
    }
}

void MouseSettingsDialog::updateHorizontalSpeedConvertLabel(int value)
{
    QString label = QString (QString::number(value));

    int currentCurveIndex = ui->accelerationComboBox->currentIndex();
    JoyButton::JoyMouseCurve tempCurve = getMouseCurveForIndex(currentCurveIndex);
    int finalSpeed = JoyButton::calculateFinalMouseSpeed(tempCurve, value);

    label = label.append(" = ").append(QString::number(finalSpeed)).append(" pps");
    ui->horizontalSpeedLabel->setText(label);
}

void MouseSettingsDialog::updateVerticalSpeedConvertLabel(int value)
{
    QString label = QString (QString::number(value));

    int currentCurveIndex = ui->accelerationComboBox->currentIndex();
    JoyButton::JoyMouseCurve tempCurve = getMouseCurveForIndex(currentCurveIndex);
    int finalSpeed = JoyButton::calculateFinalMouseSpeed(tempCurve, value);

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
    }
    else
    {
        ui->horizontalSpinBox->setEnabled(true);
        ui->verticalSpinBox->setEnabled(true);
        ui->changeMouseSpeedsTogetherCheckBox->setEnabled(true);

        ui->extraAccelerationGroupBox->setEnabled(true);
        if (ui->extraAccelerationGroupBox->isChecked())
        {
            ui->extraAccelerationGroupBox->setEnabled(true);
        }
    }
}

void MouseSettingsDialog::changeWheelSpeedBoxStatus(int index)
{
    if (index == 2)
    {
        ui->wheelHoriSpeedSpinBox->setEnabled(false);
        ui->wheelVertSpeedSpinBox->setEnabled(false);
    }
    else
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
    if (mouseCurve == JoyButton::EnhancedPrecisionCurve)
    {
        ui->accelerationComboBox->setCurrentIndex(1);
    }
    else if (mouseCurve == JoyButton::LinearCurve)
    {
        ui->accelerationComboBox->setCurrentIndex(2);
    }
    else if (mouseCurve == JoyButton::QuadraticCurve)
    {
        ui->accelerationComboBox->setCurrentIndex(3);
    }
    else if (mouseCurve == JoyButton::CubicCurve)
    {
        ui->accelerationComboBox->setCurrentIndex(4);
    }
    else if (mouseCurve == JoyButton::QuadraticExtremeCurve)
    {
        ui->accelerationComboBox->setCurrentIndex(5);
    }
    else if (mouseCurve == JoyButton::PowerCurve)
    {
        ui->accelerationComboBox->setCurrentIndex(6);
    }
    else if (mouseCurve == JoyButton::EasingQuadraticCurve)
    {
        ui->accelerationComboBox->setCurrentIndex(7);
    }
    else if (mouseCurve == JoyButton::EasingCubicCurve)
    {
        ui->accelerationComboBox->setCurrentIndex(8);
    }
}

JoyButton::JoyMouseCurve MouseSettingsDialog::getMouseCurveForIndex(int index)
{
    JoyButton::JoyMouseCurve temp = JoyButton::DEFAULTMOUSECURVE;

    if (index == 1)
    {
        temp = JoyButton::EnhancedPrecisionCurve;
    }
    else if (index == 2)
    {
        temp = JoyButton::LinearCurve;
    }
    else if (index == 3)
    {
        temp = JoyButton::QuadraticCurve;
    }
    else if (index == 4)
    {
        temp = JoyButton::CubicCurve;
    }
    else if (index == 5)
    {
        temp = JoyButton::QuadraticExtremeCurve;
    }
    else if (index == 6)
    {
        temp = JoyButton::PowerCurve;
    }
    else if (index == 7)
    {
        temp = JoyButton::EasingQuadraticCurve;
    }
    else if (index == 8)
    {
        temp = JoyButton::EasingCubicCurve;
    }

    return temp;
}

void MouseSettingsDialog::changeSensitivityStatusForMouseMode(int index)
{
    if (index == 2)
    {
        ui->sensitivityDoubleSpinBox->setEnabled(false);
    }
    else if (index == 1)
    {
        int currentCurveIndex = ui->accelerationComboBox->currentIndex();
        JoyButton::JoyMouseCurve temp = getMouseCurveForIndex(currentCurveIndex);
        if (temp == JoyButton::PowerCurve)
        {
            ui->sensitivityDoubleSpinBox->setEnabled(true);
        }
    }
    else
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
    if (lastMouseStatUpdate.elapsed() >= 100 && elapsed > 0)
    {
        QString tempX("%1 (%2 pps) (%3 ms)");
        QString tempY("%1 (%2 pps) (%3 ms)");
        //QString tempPoll("%1 Hz");

        ui->mouseStatusXLabel->setText(tempX.arg(mouseX).arg(mouseX * (1000/elapsed)).arg(elapsed));
        ui->mouseStatusYLabel->setText(tempY.arg(mouseY).arg(mouseY * (1000/elapsed)).arg(elapsed));
        //ui->mouseStatusPollLabel->setText(tempPoll.arg(1000/elapsed));
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

void MouseSettingsDialog::disableReleaseSpringBox(bool enable)
{
    ui->releaseSpringRadiusspinBox->setEnabled(!enable);
}

void MouseSettingsDialog::resetReleaseRadius(bool enabled)
{
    if (enabled && ui->releaseSpringRadiusspinBox->value() > 0)
    {
        ui->releaseSpringRadiusspinBox->setValue(0);
    }
}

JoyButton::JoyExtraAccelerationCurve MouseSettingsDialog::getExtraAccelCurveForIndex(int index)
{
    JoyButton::JoyExtraAccelerationCurve temp = JoyButton::LinearAccelCurve;

    if (index == 1)
    {
        temp = JoyButton::LinearAccelCurve;
    }
    else if (index == 2)
    {
        temp = JoyButton::EaseOutSineCurve;
    }
    else if (index == 3)
    {
        temp = JoyButton::EaseOutQuadAccelCurve;
    }
    else if (index == 4)
    {
        temp = JoyButton::EaseOutCubicAccelCurve;
    }

    return temp;
}

void
MouseSettingsDialog::updateExtraAccelerationCurvePresetComboBox
(JoyButton::JoyExtraAccelerationCurve curve)
{
    int temp = 0;

    if (curve == JoyButton::LinearAccelCurve)
    {
        temp = 1;
    }
    else if (curve == JoyButton::EaseOutSineCurve)
    {
        temp = 2;
    }
    else if (curve == JoyButton::EaseOutQuadAccelCurve)
    {
        temp = 3;
    }
    else if (curve == JoyButton::EaseOutCubicAccelCurve)
    {
        temp = 4;
    }

    ui->extraAccelCurveComboBox->setCurrentIndex(temp);
}

