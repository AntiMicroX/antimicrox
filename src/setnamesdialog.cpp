#include <QTableWidgetItem>

#include "setnamesdialog.h"
#include "ui_setnamesdialog.h"


SetNamesDialog::SetNamesDialog(InputDevice *device, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetNamesDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    this->device = device;

    for (int i=0; i < InputDevice::NUMBER_JOYSETS; i++)
    {
        QString tempSetName = device->getSetJoystick(i)->getName();
        ui->setNamesTableWidget->setItem(i, 0, new QTableWidgetItem(tempSetName));
    }

    connect(this, SIGNAL(accepted()), this, SLOT(saveSetNameChanges()));
}

SetNamesDialog::~SetNamesDialog()
{
    delete ui;
}

void SetNamesDialog::saveSetNameChanges()
{
    for (int i=0; i < ui->setNamesTableWidget->rowCount(); i++)
    {
        QTableWidgetItem *setNameItem = ui->setNamesTableWidget->item(i, 0);
        QString setNameText = setNameItem->text();
        QString oldSetNameText = device->getSetJoystick(i)->getName();
        if (setNameText != oldSetNameText)
        {
            device->getSetJoystick(i)->setName(setNameText);
        }
    }
}
