#include "mousebuttonsettingsdialog.h"
#include "ui_mousesettingsdialog.h"

MouseButtonSettingsDialog::MouseButtonSettingsDialog(JoyButton *button, QWidget *parent) :
    MouseSettingsDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setModal(true);

    this->button = button;

    calculateMouseSpeedPreset();
    selectCurrentMouseModePreset();
    calculateSpringPreset();
    changeSpringSpinBoxStatus(ui->mouseModeComboBox->currentIndex());
    changeSensitivityStatus(ui->accelerationComboBox->currentIndex());
    if (button->getSensitivity() > 0.0)
    {
        ui->sensitivityDoubleSpinBox->setValue(button->getSensitivity());
    }
    updateAccelerationCurvePresetComboBox();
    selectSmoothingPreset();

    setWindowTitle(tr("Mouse Settings - ").append(tr("Button %1").arg(button->getRealJoyNumber())));

    connect(ui->mouseModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMouseMode(int)));
    connect(ui->accelerationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMouseCurve(int)));

    connect(ui->horizontalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateConfigHorizontalSpeed(int)));
    connect(ui->verticalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateConfigVerticalSpeed(int)));

    connect(ui->springWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSpringWidth(int)));
    connect(ui->springHeightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSpringHeight(int)));

    connect(ui->sensitivityDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateSensitivity(double)));
    connect(ui->smoothingCheckBox, SIGNAL(clicked(bool)), this, SLOT(updateSmoothingSetting(bool)));
}

void MouseButtonSettingsDialog::changeMouseMode(int index)
{
    if (index == 1)
    {
        button->setMouseMode(JoyButton::MouseCursor);
    }
    else if (index == 2)
    {
        button->setMouseMode(JoyButton::MouseSpring);
    }
}

void MouseButtonSettingsDialog::changeMouseCurve(int index)
{
    if (index == 1)
    {
        button->setMouseCurve(JoyButton::LinearCurve);
    }
    else if (index == 2)
    {
        button->setMouseCurve(JoyButton::QuadraticCurve);
    }
    else if (index == 3)
    {
        button->setMouseCurve(JoyButton::CubicCurve);
    }
    else if (index == 4)
    {
        button->setMouseCurve(JoyButton::QuadraticExtremeCurve);
    }
    else if (index == 5)
    {
        button->setMouseCurve(JoyButton::PowerCurve);
    }
}

void MouseButtonSettingsDialog::updateConfigHorizontalSpeed(int value)
{
    button->setMouseSpeedX(value);
}

void MouseButtonSettingsDialog::updateConfigVerticalSpeed(int value)
{
    button->setMouseSpeedY(value);
}

void MouseButtonSettingsDialog::updateSpringWidth(int value)
{
    button->setSpringWidth(value);
}

void MouseButtonSettingsDialog::updateSpringHeight(int value)
{
    button->setSpringHeight(value);
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

void MouseButtonSettingsDialog::updateSmoothingSetting(bool clicked)
{
    button->setSmoothing(clicked);
}

void MouseButtonSettingsDialog::selectSmoothingPreset()
{
    bool smoothing = button->isSmoothingEnabled();
    if (smoothing)
    {
        ui->smoothingCheckBox->setChecked(true);
    }
    else
    {
        ui->smoothingCheckBox->setChecked(false);
    }
}
