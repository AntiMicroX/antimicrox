#include <QDebug>

#include "keydelaydialog.h"
#include "ui_keydelaydialog.h"

KeyDelayDialog::KeyDelayDialog(InputDevice *device, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KeyDelayDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    this->device = device;

    ui->delayValueLabel->setText(QString::number(0.10, 'g', 3).append("").append(tr("ms")));
    if (device->getDeviceKeyPressTime() > 0)
    {
        int tempdelay = device->getDeviceKeyPressTime();
        ui->keyDelayHorizontalSlider->setValue(device->getDeviceKeyPressTime() / 10);
        if (tempdelay >= 1000)
        {
            ui->delayValueLabel->setText(QString::number(tempdelay / 1000.0, 'g', 3).append("").append(tr("s")));
        }
        else
        {
            ui->delayValueLabel->setText(QString::number(tempdelay / 1000.0, 'g', 3).append("").append(tr("ms")));
        }

    }

    connect(ui->keyDelayHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeDeviceKeyDelay(int)));
}

KeyDelayDialog::~KeyDelayDialog()
{
    delete ui;
}

void KeyDelayDialog::changeDeviceKeyDelay(int value)
{
    int tempdelay = value * 10;
    device->setDeviceKeyPressTime(tempdelay);
    if (tempdelay >= 1000)
    {
        ui->delayValueLabel->setText(QString::number(tempdelay / 1000.0, 'g', 3).append("").append(tr("s")));
    }
    else
    {
        ui->delayValueLabel->setText(QString::number(tempdelay / 1000.0, 'g', 3).append("").append(tr("ms")));
    }
}
