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

#include <QPushButton>

#include "unixwindowinfodialog.h"
#include "ui_unixwindowinfodialog.h"

#include "x11info.h"

UnixWindowInfoDialog::UnixWindowInfoDialog(unsigned long window, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UnixWindowInfoDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    X11Info *info = X11Info::getInstance();
    bool setRadioDefault = false;

    winClass = info->getWindowClass(window);
    ui->winClassLabel->setText(winClass);
    if (winClass.isEmpty())
    {
        ui->winClassCheckBox->setEnabled(false);
        ui->winClassCheckBox->setChecked(false);
    }
    else
    {
        ui->winClassCheckBox->setChecked(true);
        setRadioDefault = true;
    }

    winName = info->getWindowTitle(window);
    ui->winTitleLabel->setText(winName);
    if (winName.isEmpty())
    {
        ui->winTitleCheckBox->setEnabled(false);
        ui->winTitleCheckBox->setChecked(false);
    }
    else if (!setRadioDefault)
    {
        ui->winTitleCheckBox->setChecked(true);
        setRadioDefault = true;
    }

    ui->winPathLabel->clear();
    int pid = info->getApplicationPid(window);
    if (pid > 0)
    {
        QString exepath = X11Info::getInstance()->getApplicationLocation(pid);
        if (!exepath.isEmpty())
        {
            ui->winPathLabel->setText(exepath);
            winPath = exepath;
            if (!setRadioDefault)
            {
                ui->winTitleCheckBox->setChecked(true);
                setRadioDefault = true;
            }
        }
        else
        {
            ui->winPathCheckBox->setEnabled(false);
            ui->winPathCheckBox->setChecked(false);
        }
    }
    else
    {
        ui->winPathCheckBox->setEnabled(false);
        ui->winPathCheckBox->setChecked(false);
    }

    if (winClass.isEmpty() && winName.isEmpty() &&
        winPath.isEmpty())
    {
        QPushButton *button = ui->buttonBox->button(QDialogButtonBox::Ok);
        button->setEnabled(false);
    }

    connect(this, SIGNAL(accepted()), this, SLOT(populateOption()));
}

void UnixWindowInfoDialog::populateOption()
{
    if (ui->winClassCheckBox->isChecked())
    {
        selectedMatch = selectedMatch | WindowClass;
    }

    if (ui->winTitleCheckBox->isChecked())
    {
        selectedMatch = selectedMatch | WindowName;
    }

    if (ui->winPathCheckBox->isChecked())
    {
        selectedMatch = selectedMatch | WindowPath;
    }
}

UnixWindowInfoDialog::DialogWindowOption UnixWindowInfoDialog::getSelectedOptions()
{
    return selectedMatch;
}

QString UnixWindowInfoDialog::getWindowClass()
{
    return winClass;
}

QString UnixWindowInfoDialog::getWindowName()
{
    return winName;
}

QString UnixWindowInfoDialog::getWindowPath()
{
    return winPath;
}

UnixWindowInfoDialog::~UnixWindowInfoDialog()
{
    delete ui;
}
