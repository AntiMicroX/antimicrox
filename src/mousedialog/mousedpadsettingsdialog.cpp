#include "mousedpadsettingsdialog.h"
#include "ui_mousesettingsdialog.h"

#include <setjoystick.h>

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
    selectSmoothingPreset();

    updateWindowTitleDPadName();

    if (ui->mouseModeComboBox->currentIndex() == 2)
    {
        springPreviewWidget = new SpringModeRegionPreview(ui->springWidthSpinBox->value(), ui->springHeightSpinBox->value());
    }
    else
    {
        springPreviewWidget = new SpringModeRegionPreview(0, 0);
    }

    calculateWheelSpeedPreset();

    connect(this, SIGNAL(finished(int)), springPreviewWidget, SLOT(deleteLater()));

    connect(ui->mouseModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMouseMode(int)));
    connect(ui->accelerationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMouseCurve(int)));

    connect(ui->horizontalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateConfigHorizontalSpeed(int)));
    connect(ui->verticalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateConfigVerticalSpeed(int)));

    connect(ui->springWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSpringWidth(int)));
    connect(ui->springWidthSpinBox, SIGNAL(valueChanged(int)), springPreviewWidget, SLOT(setSpringWidth(int)));

    connect(ui->springHeightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSpringHeight(int)));
    connect(ui->springHeightSpinBox, SIGNAL(valueChanged(int)), springPreviewWidget, SLOT(setSpringHeight(int)));

    connect(ui->sensitivityDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateSensitivity(double)));
    connect(ui->smoothingCheckBox, SIGNAL(clicked(bool)), this, SLOT(updateSmoothingSetting(bool)));

    connect(ui->wheelHoriSpeedSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateWheelSpeedHorizontalSpeed(int)));
    connect(ui->wheelVertSpeedSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateWheelSpeedVerticalSpeed(int)));
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

void MouseDPadSettingsDialog::updateSmoothingSetting(bool clicked)
{
    dpad->setButtonsSmoothing(clicked);
}

void MouseDPadSettingsDialog::selectSmoothingPreset()
{
    bool smoothing = dpad->getButtonsPresetSmoothing();
    if (smoothing)
    {
        ui->smoothingCheckBox->setChecked(true);
    }
    else
    {
        ui->smoothingCheckBox->setChecked(false);
    }
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
