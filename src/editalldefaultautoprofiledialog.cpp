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

#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>

#include "editalldefaultautoprofiledialog.h"
#include "ui_editalldefaultautoprofiledialog.h"
#include "common.h"

EditAllDefaultAutoProfileDialog::EditAllDefaultAutoProfileDialog(AutoProfileInfo *info, AntiMicroSettings *settings,
                                                                 QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditAllDefaultAutoProfileDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    this->info = info;
    this->settings = settings;

    if (!info->getProfileLocation().isEmpty())
    {
        ui->profileLineEdit->setText(info->getProfileLocation());
    }

    connect(ui->profileBrowsePushButton, SIGNAL(clicked()), this, SLOT(openProfileBrowseDialog()));
    connect(this, SIGNAL(accepted()), this, SLOT(saveAutoProfileInformation()));
}

EditAllDefaultAutoProfileDialog::~EditAllDefaultAutoProfileDialog()
{
    delete ui;
}

void EditAllDefaultAutoProfileDialog::openProfileBrowseDialog()
{
    QString lookupDir = PadderCommon::preferredProfileDir(settings);
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Config"), lookupDir, QString("Config Files (*.amgp *.xml)"));
    if (!filename.isNull() && !filename.isEmpty())
    {
        ui->profileLineEdit->setText(filename);
    }
}

void EditAllDefaultAutoProfileDialog::saveAutoProfileInformation()
{
    info->setGUID("all");
    info->setProfileLocation(ui->profileLineEdit->text());
    info->setActive(true);
}

AutoProfileInfo* EditAllDefaultAutoProfileDialog::getAutoProfile()
{
    return info;
}

void EditAllDefaultAutoProfileDialog::accept()
{
    bool validForm = true;
    QString errorString;
    if (ui->profileLineEdit->text().length() > 0)
    {
        QString profileFilename = ui->profileLineEdit->text();
        QFileInfo info(profileFilename);
        if (!info.exists())
        {
            validForm = false;
            errorString = tr("Profile file path is invalid.");
        }
    }

    if (validForm)
    {
        QDialog::accept();
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText(errorString);
        msgBox.setStandardButtons(QMessageBox::Close);
        msgBox.exec();
    }
}
