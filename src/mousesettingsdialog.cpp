#include <QString>

#include "mousesettingsdialog.h"
#include "ui_mousesettingsdialog.h"
#include "joyaxis.h"

MouseSettingsDialog::MouseSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MouseSettingsDialog)
{
    ui->setupUi(this);

    connect(ui->accelerationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeSensitivityStatus(int)));
    connect(ui->mouseModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeSpringSpinBoxStatus(int)));
    connect(ui->mouseModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMouseSpeedBoxStatus(int)));

    connect(ui->horizontalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateHorizontalSpeedConvertLabel(int)));
    connect(ui->horizontalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(moveSpeedsTogether(int)));

    connect(ui->verticalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateVerticalSpeedConvertLabel(int)));
    connect(ui->verticalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(moveSpeedsTogether(int)));
}

MouseSettingsDialog::~MouseSettingsDialog()
{
    delete ui;
}

void MouseSettingsDialog::changeSensitivityStatus(int index)
{
    if (index == 5)
    {
        ui->sensitivityDoubleSpinBox->setEnabled(true);
    }
    else
    {
        ui->sensitivityDoubleSpinBox->setEnabled(false);
    }
}

void MouseSettingsDialog::changeSpringSpinBoxStatus(int index)
{
    if (index == 2)
    {
        ui->springWidthSpinBox->setEnabled(true);
        ui->springHeightSpinBox->setEnabled(true);
    }
    else
    {
        ui->springWidthSpinBox->setEnabled(false);
        ui->springHeightSpinBox->setEnabled(false);
    }
}

void MouseSettingsDialog::updateHorizontalSpeedConvertLabel(int value)
{
    QString label = QString (QString::number(value));
    label = label.append(" = ").append(QString::number(JoyAxis::JOYSPEED * value)).append(" pps");
    ui->horizontalSpeedLabel->setText(label);
}

void MouseSettingsDialog::updateVerticalSpeedConvertLabel(int value)
{
    QString label = QString (QString::number(value));
    label = label.append(" = ").append(QString::number(JoyAxis::JOYSPEED * value)).append(" pps");
    ui->verticalSpeedLabel->setText(label);
}

void MouseSettingsDialog::moveSpeedsTogether(int value)
{
    if (ui->changeMouseSpeedsCheckBox->isChecked())
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
    }
    else
    {
        ui->horizontalSpinBox->setEnabled(true);
        ui->verticalSpinBox->setEnabled(true);
    }
}
