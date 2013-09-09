#include "mouseaxissettingsdialog.h"
#include "ui_mousesettingsdialog.h"

MouseAxisSettingsDialog::MouseAxisSettingsDialog(JoyAxis *axis, QWidget *parent) :
    MouseSettingsDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    this->axis = axis;

    calculateMouseSpeedPreset();
    selectCurrentMouseModePreset();
    calculateSpringPreset();
    changeSpringSpinBoxStatus(ui->mouseModeComboBox->currentIndex());
    changeSensitivityStatus(ui->accelerationComboBox->currentIndex());
    if (axis->getButtonsPresetSensitivity() > 0.0)
    {
        ui->sensitivityDoubleSpinBox->setValue(axis->getButtonsPresetSensitivity());
    }
    updateAccelerationCurvePresetComboBox();

    setWindowTitle(tr("Mouse Settings - ").append(tr("Axis %1").arg(axis->getRealJoyIndex())));

    connect(ui->mouseModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMouseMode(int)));
    connect(ui->accelerationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMouseCurve(int)));

    connect(ui->horizontalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateConfigHorizontalSpeed(int)));
    connect(ui->verticalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateConfigVerticalSpeed(int)));

    connect(ui->springWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSpringWidth(int)));
    connect(ui->springHeightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSpringHeight(int)));

    connect(ui->sensitivityDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateSensitivity(double)));
}

void MouseAxisSettingsDialog::changeMouseMode(int index)
{
    if (index == 1)
    {
        axis->setButtonsMouseMode(JoyButton::MouseCursor);
    }
    else if (index == 2)
    {
        axis->setButtonsMouseMode(JoyButton::MouseSpring);
    }
}

void MouseAxisSettingsDialog::changeMouseCurve(int index)
{
    if (index == 1)
    {
        axis->setButtonsMouseCurve(JoyButton::LinearCurve);
    }
    else if (index == 2)
    {
        axis->setButtonsMouseCurve(JoyButton::QuadraticCurve);
    }
    else if (index == 3)
    {
        axis->setButtonsMouseCurve(JoyButton::CubicCurve);
    }
    else if (index == 4)
    {
        axis->setButtonsMouseCurve(JoyButton::QuadraticExtremeCurve);
    }
    else if (index == 5)
    {
        axis->setButtonsMouseCurve(JoyButton::PowerCurve);
    }
}

void MouseAxisSettingsDialog::updateConfigHorizontalSpeed(int value)
{
    if (ui->changeMouseSpeedsCheckBox->isChecked())
    {
        axis->getPAxisButton()->setMouseSpeedX(value);
        axis->getNAxisButton()->setMouseSpeedX(value);
    }
}

void MouseAxisSettingsDialog::updateConfigVerticalSpeed(int value)
{
    if (ui->changeMouseSpeedsCheckBox->isChecked())
    {
        axis->getPAxisButton()->setMouseSpeedY(value);
        axis->getNAxisButton()->setMouseSpeedY(value);
    }
}

void MouseAxisSettingsDialog::updateSpringWidth(int value)
{
    axis->setButtonsSpringWidth(value);
}

void MouseAxisSettingsDialog::updateSpringHeight(int value)
{
    axis->setButtonsSpringHeight(value);
}

void MouseAxisSettingsDialog::selectCurrentMouseModePreset()
{
    bool presetDefined = axis->hasSameButtonsMouseMode();
    if (presetDefined)
    {
        JoyButton::JoyMouseMovementMode mode = axis->getButtonsPresetMouseMode();
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

void MouseAxisSettingsDialog::calculateSpringPreset()
{
    int tempWidth = axis->getButtonsPresetSpringWidth();
    int tempHeight = axis->getButtonsPresetSpringHeight();

    if (tempWidth > 0)
    {
        ui->springWidthSpinBox->setValue(tempWidth);
    }

    if (tempHeight > 0)
    {
        ui->springHeightSpinBox->setValue(tempHeight);
    }
}

void MouseAxisSettingsDialog::calculateMouseSpeedPreset()
{
    int tempMouseSpeedX = 0;
    tempMouseSpeedX = qMax(axis->getPAxisButton()->getMouseSpeedX(), axis->getNAxisButton()->getMouseSpeedX());

    int tempMouseSpeedY = 0;
    tempMouseSpeedY = qMax(axis->getPAxisButton()->getMouseSpeedY(), axis->getNAxisButton()->getMouseSpeedY());

    ui->horizontalSpinBox->setValue(tempMouseSpeedX);
    ui->verticalSpinBox->setValue(tempMouseSpeedY);
}

void MouseAxisSettingsDialog::updateSensitivity(double value)
{
    axis->setButtonsSensitivity(value);
}

void MouseAxisSettingsDialog::updateAccelerationCurvePresetComboBox()
{
    JoyButton::JoyMouseCurve temp = axis->getButtonsPresetMouseCurve();
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
