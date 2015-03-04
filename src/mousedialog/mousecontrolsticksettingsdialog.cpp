#include <QHashIterator>

#include "mousecontrolsticksettingsdialog.h"
#include "ui_mousesettingsdialog.h"

#include <inputdevice.h>
#include <setjoystick.h>

MouseControlStickSettingsDialog::MouseControlStickSettingsDialog(JoyControlStick *stick, QWidget *parent) :
    MouseSettingsDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    this->stick = stick;

    calculateMouseSpeedPreset();
    selectCurrentMouseModePreset();
    calculateSpringPreset();
    changeSpringSectionStatus(ui->mouseModeComboBox->currentIndex());
    changeSettingsWidgetStatus(ui->accelerationComboBox->currentIndex());
    if (stick->getButtonsPresetSensitivity() > 0.0)
    {
        ui->sensitivityDoubleSpinBox->setValue(stick->getButtonsPresetSensitivity());
    }
    updateAccelerationCurvePresetComboBox();
    //selectSmoothingPreset();

    updateWindowTitleStickName();

    if (ui->mouseModeComboBox->currentIndex() == 2)
    {
        springPreviewWidget = new SpringModeRegionPreview(ui->springWidthSpinBox->value(), ui->springHeightSpinBox->value());
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
    //connect(ui->smoothingCheckBox, SIGNAL(clicked(bool)), this, SLOT(updateSmoothingSetting(bool)));

    connect(ui->wheelHoriSpeedSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateWheelSpeedHorizontalSpeed(int)));
    connect(ui->wheelVertSpeedSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateWheelSpeedVerticalSpeed(int)));

    connect(ui->easingDoubleSpinBox, SIGNAL(valueChanged(double)), stick, SLOT(setButtonsEasingDuration(double)));

    connect(ui->extraAccelCheckBox, SIGNAL(clicked(bool)), this, SLOT(updateExtraAccelerationStatus(bool)));
    connect(ui->extraAccelDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateExtraAccelerationMultiplier(double)));
}

void MouseControlStickSettingsDialog::changeMouseMode(int index)
{
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
    }
}

void MouseControlStickSettingsDialog::changeMouseCurve(int index)
{
    JoyButton::JoyMouseCurve temp = MouseSettingsDialog::getMouseCurveForIndex(index);
    stick->setButtonsMouseCurve(temp);
}

void MouseControlStickSettingsDialog::updateConfigHorizontalSpeed(int value)
{
    QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> iter(*stick->getButtons());
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setMouseSpeedX(value);
    }
}

void MouseControlStickSettingsDialog::updateConfigVerticalSpeed(int value)
{
    QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> iter(*stick->getButtons());
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setMouseSpeedY(value);
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

void MouseControlStickSettingsDialog::updateAccelerationCurvePresetComboBox()
{
    JoyButton::JoyMouseCurve temp = stick->getButtonsPresetMouseCurve();
    MouseSettingsDialog::updateAccelerationCurvePresetComboBox(temp);
}

/*void MouseControlStickSettingsDialog::updateSmoothingSetting(bool clicked)
{
    stick->setButtonsSmoothing(clicked);
}
*/

/*void MouseControlStickSettingsDialog::selectSmoothingPreset()
{
    bool smoothing = stick->getButtonsPresetSmoothing();
    if (smoothing)
    {
        ui->smoothingCheckBox->setChecked(true);
    }
    else
    {
        ui->smoothingCheckBox->setChecked(false);
    }
}
*/

void MouseControlStickSettingsDialog::calculateWheelSpeedPreset()
{
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
    stick->setButtonsWheelSpeedX(value);
}

void MouseControlStickSettingsDialog::updateWheelSpeedVerticalSpeed(int value)
{
    stick->setButtonsWheelSpeedY(value);
}

void MouseControlStickSettingsDialog::updateSpringRelativeStatus(bool value)
{
    stick->setButtonsSpringRelativeStatus(value);
}

void MouseControlStickSettingsDialog::updateWindowTitleStickName()
{
    QString temp = QString(tr("Mouse Settings")).append(" - ");

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
        unsigned int setIndex = stick->getParentSet()->getRealIndex();
        temp.append(" [").append(tr("Set %1").arg(setIndex));

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
    if (stick->getButtonsExtraAccelerationStatus())
    {
        ui->extraAccelCheckBox->setChecked(true);
        ui->extraAccelDoubleSpinBox->setEnabled(true);
    }
}

void MouseControlStickSettingsDialog::calculateExtraAccelerationMultiplier()
{
    ui->extraAccelDoubleSpinBox->setValue(stick->getButtonsExtraAccelerationMultiplier());
}

void MouseControlStickSettingsDialog::updateExtraAccelerationStatus(bool checked)
{
    stick->setButtonsExtraAccelerationStatus(checked);
}

void MouseControlStickSettingsDialog::updateExtraAccelerationMultiplier(double value)
{
    stick->setButtonsExtraAccelerationMultiplier(value);
}
