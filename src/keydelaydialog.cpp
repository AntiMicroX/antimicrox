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

    if (device->getDeviceKeyDelay() > 10)
    {
        int tempdelay = device->getDeviceKeyDelay();
        ui->keyDelayHorizontalSlider->setValue(device->getDeviceKeyDelay() / 10);
        ui->delayValueLabel->setText(QString::number(tempdelay / 1000.0, 'g', 3).append("").append(tr("ms")));
    }
    else
    {
        ui->delayValueLabel->setText(QString::number(0.10, 'g', 3).append("").append(tr("ms")));
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
    device->setDeviceKeyDelay(tempdelay);
    ui->delayValueLabel->setText(QString::number(tempdelay / 1000.0, 'g', 3).append("").append(tr("ms")));
}
