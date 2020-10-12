/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "setnamesdialog.h"
#include "ui_setnamesdialog.h"

#include "globalvariables.h"
#include "inputdevice.h"
#include "messagehandler.h"

#include <QDebug>
#include <QTableWidgetItem>
#include <QWidget>

SetNamesDialog::SetNamesDialog(InputDevice *device, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SetNamesDialog)
{
    ui->setupUi(this);

    qInstallMessageHandler(MessageHandler::myMessageOutput);
    setAttribute(Qt::WA_DeleteOnClose);
    this->device = device;

    auto joysList = device->getJoystick_sets();

    for (int i = 0; i < joysList.size(); i++)
    {
        auto name = joysList[i]->getName();
        ui->setNamesTableWidget->setItem(i, 0, new QTableWidgetItem(name));
    }

    connect(this, &SetNamesDialog::accepted, this, &SetNamesDialog::saveSetNameChanges);
}

SetNamesDialog::~SetNamesDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    delete ui;
}

void SetNamesDialog::saveSetNameChanges()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    for (int i = 0; i < ui->setNamesTableWidget->rowCount(); i++)
    {
        QTableWidgetItem *setNameItem = ui->setNamesTableWidget->item(i, 0);
        QString setNameText = setNameItem->text();
        QString oldSetNameText = device->getSetJoystick(i)->getName();

        if (setNameText != oldSetNameText)
            qDebug() << "Set number: " << i << "  Renamed to: " << setNameText;
        device->getSetJoystick(i)->setName(setNameText);
    }
}

InputDevice *SetNamesDialog::getDevice() const { return device; }
