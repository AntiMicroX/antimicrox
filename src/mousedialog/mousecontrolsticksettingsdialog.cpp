#include <QHashIterator>

#include "mousecontrolsticksettingsdialog.h"
#include "ui_mousesettingsdialog.h"

MouseControlStickSettingsDialog::MouseControlStickSettingsDialog(JoyControlStick *stick, QWidget *parent) :
    MouseSettingsDialog(parent)
{
    this->stick = stick;

    calculateMouseSpeedPreset();
    selectCurrentMouseModePreset();
    calculateSpringPreset();
    changeSensitivityStatus(ui->accelerationComboBox->currentIndex());
    if (stick->getButtonsPresetSensitivity() > 0.0)
    {
        ui->sensitivityDoubleSpinBox->setValue(stick->getButtonsPresetSensitivity());
    }

    setWindowTitle(tr("Mouse Settings - ").append(tr("Stick %1").arg(stick->getRealJoyIndex())));

    connect(ui->mouseModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMouseMode(int)));
    connect(ui->accelerationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMouseCurve(int)));

    connect(ui->horizontalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateConfigHorizontalSpeed(int)));
    connect(ui->verticalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateConfigVerticalSpeed(int)));

    connect(ui->springWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSpringWidth(int)));
    connect(ui->springHeightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSpringHeight(int)));

    connect(ui->sensitivityDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateSensitivity(double)));
}

void MouseControlStickSettingsDialog::changeMouseMode(int index)
{
    if (index == 1)
    {
        stick->setButtonsMouseMode(JoyButton::MouseCursor);
    }
    else if (index == 2)
    {
        stick->setButtonsMouseMode(JoyButton::MouseSpring);
    }
}

void MouseControlStickSettingsDialog::changeMouseCurve(int index)
{
    if (index == 1)
    {
        stick->setButtonsMouseCurve(JoyButton::LinearCurve);
    }
    else if (index == 2)
    {
        stick->setButtonsMouseCurve(JoyButton::QuadraticCurve);
    }
    else if (index == 3)
    {
        stick->setButtonsMouseCurve(JoyButton::CubicCurve);
    }
    else if (index == 4)
    {
        stick->setButtonsMouseCurve(JoyButton::QuadraticExtremeCurve);
    }
    else if (index == 5)
    {
        stick->setButtonsMouseCurve(JoyButton::PowerCurve);
    }
}

void MouseControlStickSettingsDialog::updateConfigHorizontalSpeed(int value)
{
    if (ui->changeMouseSpeedsCheckBox->isChecked())
    {
        QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> iter(*stick->getButtons());
        while (iter.hasNext())
        {
            JoyControlStickButton *button = iter.next().value();
            button->setMouseSpeedX(value);
        }
    }
}

void MouseControlStickSettingsDialog::updateConfigVerticalSpeed(int value)
{
    if (ui->changeMouseSpeedsCheckBox->isChecked())
    {
        QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> iter(*stick->getButtons());
        while (iter.hasNext())
        {
            JoyControlStickButton *button = iter.next().value();
            button->setMouseSpeedY(value);
        }
    }
}

void MouseControlStickSettingsDialog::updateSpringWidth(int value)
{
    stick->setButtonsSpringWidth(value);
}

void MouseControlStickSettingsDialog::updateSpringHeight(int value)
{
    stick->setButtonsSpringHeight(value);
}

void MouseControlStickSettingsDialog::selectCurrentMouseModePreset()
{
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
    stick->setButtonsSensitivity(value);
}
