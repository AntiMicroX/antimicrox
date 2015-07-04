/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
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

//#include <QDebug>

#include "setaxisthrottledialog.h"
#include "ui_setaxisthrottledialog.h"

SetAxisThrottleDialog::SetAxisThrottleDialog(JoyAxis *axis, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetAxisThrottleDialog)
{
    ui->setupUi(this);

    this->axis = axis;

    QString currentText = ui->label->text();
    currentText = currentText.arg(QString::number(axis->getRealJoyIndex()));
    ui->label->setText(currentText);

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(propogateThrottleChange()));
    connect(this, SIGNAL(initiateSetAxisThrottleChange()), axis, SLOT(propogateThrottleChange()));
}

SetAxisThrottleDialog::~SetAxisThrottleDialog()
{
    delete ui;
}

void SetAxisThrottleDialog::propogateThrottleChange()
{
    emit initiateSetAxisThrottleChange();
}
