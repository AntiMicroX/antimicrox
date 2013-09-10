#include "mousedpadsettingsdialog.h"
#include "ui_mousesettingsdialog.h"

MouseDPadSettingsDialog::MouseDPadSettingsDialog(JoyDPad *dpad, QWidget *parent) :
    MouseSettingsDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    this->dpad = dpad;

    calculateMouseSpeedPreset();
    selectCurrentMouseModePreset();
    calculateSpringPreset();
    changeSpringSpinBoxStatus(ui->mouseModeComboBox->currentIndex());
    changeSensitivityStatus(ui->accelerationComboBox->currentIndex());
    if (dpad->getButtonsPresetSensitivity() > 0.0)
    {
        ui->sensitivityDoubleSpinBox->setValue(dpad->getButtonsPresetSensitivity());
    }
    updateAccelerationCurvePresetComboBox();

    setWindowTitle(tr("Mouse Settings - ").append(tr("DPad %1").arg(dpad->getRealJoyNumber())));

    connect(ui->mouseModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMouseMode(int)));
    connect(ui->accelerationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMouseCurve(int)));

    connect(ui->horizontalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateConfigHorizontalSpeed(int)));
    connect(ui->verticalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateConfigVerticalSpeed(int)));

    connect(ui->springWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSpringWidth(int)));
    connect(ui->springHeightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSpringHeight(int)));

    connect(ui->sensitivityDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateSensitivity(double)));
}

void MouseDPadSettingsDialog::changeMouseMode(int index)
{
    if (index == 1)
    {
        dpad->setButtonsMouseMode(JoyButton::MouseCursor);
    }
    else if (index == 2)
    {
        dpad->setButtonsMouseMode(JoyButton::MouseSpring);
    }
}

void MouseDPadSettingsDialog::changeMouseCurve(int index)
{
    if (index == 1)
    {
        dpad->setButtonsMouseCurve(JoyButton::LinearCurve);
    }
    else if (index == 2)
    {
        dpad->setButtonsMouseCurve(JoyButton::QuadraticCurve);
    }
    else if (index == 3)
    {
        dpad->setButtonsMouseCurve(JoyButton::CubicCurve);
    }
    else if (index == 4)
    {
        dpad->setButtonsMouseCurve(JoyButton::QuadraticExtremeCurve);
    }
    else if (index == 5)
    {
        dpad->setButtonsMouseCurve(JoyButton::PowerCurve);
    }
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
    if (temp == JoyButton::LinearCurve)
    {
        ui->accelerationComboBox->setCurrentIndex(1);
    }
    else if (temp == JoyButton::QuadraticCurve)
    {
        ui->accelerationComboBox->setCurrentIndex(2);
    }
    else if (temp == JoyButton::CubicCurve)
    {
        ui->accelerationComboBox->setCurrentIndex(3);
    }
    else if (temp == JoyButton::QuadraticExtremeCurve)
    {
        ui->accelerationComboBox->setCurrentIndex(4);
    }
    else if (temp == JoyButton::PowerCurve)
    {
        ui->accelerationComboBox->setCurrentIndex(5);
    }
}
