#include <QDebug>

#include "extraprofilesettingsdialog.h"
#include "ui_extraprofilesettingsdialog.h"

ExtraProfileSettingsDialog::ExtraProfileSettingsDialog(InputDevice *device, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExtraProfileSettingsDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    this->device = device;

    ui->pressValueLabel->setText(QString::number(0.10, 'g', 3).append("").append(tr("s")));
    if (device->getDeviceKeyPressTime() > 0)
    {
        int temppress = device->getDeviceKeyPressTime();
        ui->keyPressHorizontalSlider->setValue(device->getDeviceKeyPressTime() / 10);
        ui->pressValueLabel->setText(QString::number(temppress / 1000.0, 'g', 3).append("").append(tr("s")));
    }

    if (!device->getProfileName().isEmpty())
    {
        ui->profileNameLineEdit->setText(device->getProfileName());
    }

    connect(ui->keyPressHorizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(changeDeviceKeyPress(int)));
    connect(ui->profileNameLineEdit, SIGNAL(textChanged(QString)), device, SLOT(setProfileName(QString)));
}

ExtraProfileSettingsDialog::~ExtraProfileSettingsDialog()
{
    delete ui;
}

void ExtraProfileSettingsDialog::changeDeviceKeyPress(int value)
{
    int temppress = value * 10;
    device->setDeviceKeyPressTime(temppress);
    ui->pressValueLabel->setText(QString::number(temppress / 1000.0, 'g', 3).append("").append(tr("s")));
}
