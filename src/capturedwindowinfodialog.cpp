/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail.com>
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

#include "capturedwindowinfodialog.h"
#include "ui_capturedwindowinfodialog.h"

#include "messagehandler.h"

#include <QDebug>
#include <QPushButton>
#include <QWidget>

#include "x11extras.h"

CapturedWindowInfoDialog::CapturedWindowInfoDialog(long window, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CapturedWindowInfoDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    selectedMatch = WindowNone;

    X11Extras *info = X11Extras::getInstance();
    ui->winPathChoiceComboBox->setVisible(false);

    bool setRadioDefault = false;
    fullWinPath = false;

    winClass = info->getWindowClass(static_cast<Window>(window));
    ui->winClassLabel->setText(winClass);

    if (winClass.isEmpty())
    {
        ui->winClassCheckBox->setEnabled(false);
        ui->winClassCheckBox->setChecked(false);
    } else
    {
        ui->winClassCheckBox->setChecked(true);
        setRadioDefault = true;
    }

    ui->winPathChoiceComboBox->setVisible(false);

    winName = info->getWindowTitle(static_cast<Window>(window));

    ui->winTitleLabel->setText(winName);

    if (winName.isEmpty())
    {
        ui->winTitleCheckBox->setEnabled(false);
        ui->winTitleCheckBox->setChecked(false);
    } else if (!setRadioDefault)
    {
        ui->winTitleCheckBox->setChecked(true);
        setRadioDefault = true;
    }

    ui->winPathLabel->clear();

    int pid = info->getApplicationPid(static_cast<Window>(window));

    if (pid > 0)
    {
        QString exepath = X11Extras::getInstance()->getApplicationLocation(pid);

        if (!exepath.isEmpty())
        {
            ui->winPathLabel->setText(exepath);
            winPath = exepath;

            if (!setRadioDefault)
                ui->winTitleCheckBox->setChecked(true);

        } else
        {
            ui->winPathCheckBox->setEnabled(false);
            ui->winPathCheckBox->setChecked(false);
        }
    } else
    {
        ui->winPathCheckBox->setEnabled(false);
        ui->winPathCheckBox->setChecked(false);
    }

    if (winClass.isEmpty() && winName.isEmpty() && winPath.isEmpty())
    {
        QPushButton *button = ui->buttonBox->button(QDialogButtonBox::Ok);
        button->setEnabled(false);
    }

    connect(this, &CapturedWindowInfoDialog::accepted, this, &CapturedWindowInfoDialog::populateOption);
}

CapturedWindowInfoDialog::~CapturedWindowInfoDialog() { delete ui; }

void CapturedWindowInfoDialog::populateOption()
{
    if (ui->winClassCheckBox->isChecked())
        selectedMatch = selectedMatch | WindowClass;

    if (ui->winTitleCheckBox->isChecked())
        selectedMatch = selectedMatch | WindowName;

    if (ui->winPathCheckBox->isChecked())
    {
        selectedMatch = selectedMatch | WindowPath;

        if (ui->winPathChoiceComboBox->currentIndex() == 0)
            fullWinPath = true;
        else
            fullWinPath = false;
    }
}

CapturedWindowInfoDialog::CapturedWindowOption CapturedWindowInfoDialog::getSelectedOptions() { return selectedMatch; }

QString CapturedWindowInfoDialog::getWindowClass() { return winClass; }

QString CapturedWindowInfoDialog::getWindowName() { return winName; }

QString CapturedWindowInfoDialog::getWindowPath() { return winPath; }

bool CapturedWindowInfoDialog::useFullWindowPath() { return fullWinPath; }
