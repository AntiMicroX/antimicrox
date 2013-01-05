#include <QDebug>

#include "axiseditdialog.h"
#include "ui_axiseditdialog.h"
#include "event.h"

AxisEditDialog::AxisEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AxisEditDialog)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);
    axis = 0;
}

AxisEditDialog::AxisEditDialog(JoyAxis *axis, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AxisEditDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    this->axis = axis;

    this->setWindowTitle(QString("Set Axis %1").arg(axis->getRealJoyIndex()));

    if (this->axis->getAxisMode() == 0)
    {
        ui->comboBox->setCurrentIndex(0);
    }
    else
    {
        ui->comboBox->setCurrentIndex(axis->getMouseMode()+1);
        ui->spinBox->setEnabled(true);
        ui->pushButton->setEnabled(false);
        ui->pushButton_2->setEnabled(false);
        ui->comboBox_2->setEnabled(false);
    }

    ui->spinBox->setValue(axis->getMouseSpeed());
    updateSpeedConvertLabel(axis->getMouseSpeed());

    ui->horizontalSlider->setValue(axis->getDeadZone());
    ui->lineEdit->setText(QString::number(axis->getDeadZone()));

    ui->horizontalSlider_2->setValue(axis->getMaxZoneValue());
    ui->lineEdit_2->setText(QString::number(axis->getMaxZoneValue()));

    if (axis->getNKey() > 0)
    {
        int keycode = axis->getNKey();
        ui->pushButton->setValue(keycode);
        if (keycode > MOUSE_OFFSET)
        {
            keycode -= MOUSE_OFFSET;
            ui->pushButton->setText(QString("Mouse ").append(QString::number(keycode)));
        }
        else
        {
            ui->pushButton->setText(keycodeToKey(keycode).toUpper());
        }
    }

    if (axis->getPKey() > 0)
    {
        int keycode = axis->getPKey();
        ui->pushButton_2->setValue(keycode);
        if (keycode > MOUSE_OFFSET)
        {
            keycode -= MOUSE_OFFSET;
            ui->pushButton_2->setText(QString("Mouse ").append(QString::number(keycode)));
        }
        else
        {
            ui->pushButton_2->setText(keycodeToKey(keycode).toUpper());
        }
    }

    int currentThrottle = axis->getThrottle();
    ui->comboBox_2->setCurrentIndex(currentThrottle+1);
    if (currentThrottle == -1)
    {
        ui->pushButton->setEnabled(true);
        ui->pushButton_2->setEnabled(false);
    }
    else if (currentThrottle == 1)
    {
        ui->pushButton_2->setEnabled(true);
        ui->pushButton->setEnabled(false);
    }

    ui->axisstatusBox->setDeadZone(axis->getDeadZone());
    ui->axisstatusBox->setMaxZone(axis->getMaxZoneValue());
    ui->axisstatusBox->setThrottle(axis->getThrottle());

    QString currentJoyValueText ("Current Value: ");
    currentJoyValueText = currentJoyValueText.append(QString::number(axis->getCurrentValue()));
    ui->joyValueLabel->setText(currentJoyValueText);

    connect(this, SIGNAL(accepted()), this, SLOT(saveAxisChanges()));
    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateModeUi(int)));
    connect(ui->spinBox, SIGNAL(valueChanged(int)), this, SLOT(updateSpeedConvertLabel(int)));

    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(updateDeadZoneBox(int)));
    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), ui->axisstatusBox, SLOT(setDeadZone(int)));

    connect(ui->horizontalSlider_2, SIGNAL(valueChanged(int)), this, SLOT(updateMaxZoneBox(int)));
    connect(ui->horizontalSlider_2, SIGNAL(valueChanged(int)), ui->axisstatusBox, SLOT(setMaxZone(int)));

    connect(ui->comboBox_2, SIGNAL(currentIndexChanged(int)), this, SLOT(updateThrottleUi(int)));
    connect(axis, SIGNAL(moved(int)), ui->axisstatusBox, SLOT(setValue(int)));
    connect(axis, SIGNAL(moved(int)), this, SLOT(updateJoyValue(int)));

    connect(ui->lineEdit, SIGNAL(textEdited(QString)), this, SLOT(updateDeadZoneSlider(QString)));
    connect(ui->lineEdit_2, SIGNAL(textEdited(QString)), this, SLOT(updateMaxZoneSlider(QString)));
}

void AxisEditDialog::saveAxisChanges()
{
    if (ui->pushButton->getValue() >= 0)
    {
        int keycode = ui->pushButton->getValue();
        if (keycode == 0)
        {
            axis->setNKey(keycode);
        }
        else if (keycode <= MOUSE_OFFSET)
        {
            axis->setNKey(keycode);
        }
        else
        {
            axis->setNKey(keycode);
        }
    }

    if (ui->pushButton_2->getValue() >= 0)
    {
        int keycode = ui->pushButton_2->getValue();
        if (keycode == 0)
        {
            axis->setPKey(keycode);
        }
        else if (keycode <= MOUSE_OFFSET)
        {
            axis->setPKey(keycode);
        }
        else
        {
            axis->setPKey(keycode);
        }
    }

    axis->setDeadZone(ui->horizontalSlider->value());
    axis->setMaxZoneValue(ui->horizontalSlider_2->value());

    if (ui->comboBox->currentIndex() == 0)
    {
        axis->setAxisMode(0);
    }
    else
    {
        axis->setAxisMode(1);
        axis->setMouseMode(ui->comboBox->currentIndex() - 1);
        axis->setMouseSpeed(ui->spinBox->value());
    }

    int currentThrottle = 0;
    if (ui->comboBox_2->isEnabled())
    {
        currentThrottle = ui->comboBox_2->currentIndex() - 1;
    }
    axis->setThrottle(currentThrottle);
}

void AxisEditDialog::updateModeUi(int index)
{
    if (index > 0)
    {
        ui->spinBox->setEnabled(true);
        ui->pushButton->setEnabled(false);
        ui->pushButton_2->setEnabled(false);
        ui->comboBox_2->setEnabled(false);
    }
    else
    {
        ui->spinBox->setEnabled(false);
        ui->pushButton->setEnabled(true);
        ui->pushButton_2->setEnabled(true);
        ui->comboBox_2->setEnabled(true);
    }
}

void AxisEditDialog::updateDeadZoneBox(int value)
{
    ui->lineEdit->setText(QString::number(value));
}

void AxisEditDialog::updateMaxZoneBox(int value)
{
    ui->lineEdit_2->setText(QString::number(value));
}

void AxisEditDialog::updateSpeedConvertLabel(int value)
{
    QString label = QString (QString::number(value));
    label = label.append(" = ").append(QString::number(JoyAxis::JOYSPEED * value)).append(" pps");
    ui->label_5->setText(label);
}

void AxisEditDialog::updateThrottleUi(int index)
{
    if (index == 0)
    {
        ui->pushButton->setEnabled(true);
        ui->pushButton_2->setEnabled(false);
    }
    else if (index == 1)
    {
        ui->pushButton->setEnabled(true);
        ui->pushButton_2->setEnabled(true);
    }
    else if (index == 2)
    {
        ui->pushButton_2->setEnabled(true);
        ui->pushButton->setEnabled(false);
    }
    ui->axisstatusBox->setThrottle(index - 1);
}

void AxisEditDialog::updateJoyValue(int index)
{
    QString currentJoyValueText ("Current Value: ");
    currentJoyValueText = currentJoyValueText.append(QString::number(index));
    ui->joyValueLabel->setText(currentJoyValueText);
}

void AxisEditDialog::updateDeadZoneSlider(QString value)
{
    int temp = value.toInt();
    if (temp >= JoyAxis::AXISMIN && temp <= JoyAxis::AXISMAX)
    {
        ui->horizontalSlider->setValue(temp);
    }
}

void AxisEditDialog::updateMaxZoneSlider(QString value)
{
    int temp = value.toInt();
    if (temp >= JoyAxis::AXISMIN && temp <= JoyAxis::AXISMAX)
    {
        ui->horizontalSlider_2->setValue(temp);
    }
}

AxisEditDialog::~AxisEditDialog()
{
    delete ui;
}
