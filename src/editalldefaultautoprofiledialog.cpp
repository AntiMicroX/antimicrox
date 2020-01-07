/* antimicroX Gamepad to KB+M event mapper
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

#include "editalldefaultautoprofiledialog.h"
#include "ui_editalldefaultautoprofiledialog.h"

#include "messagehandler.h"
#include "autoprofileinfo.h"
#include "antimicrosettings.h"
#include "common.h"

#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QWidget>
#include <QDebug>


EditAllDefaultAutoProfileDialog::EditAllDefaultAutoProfileDialog(AutoProfileInfo *info, AntiMicroSettings *settings,
                                                                 QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditAllDefaultAutoProfileDialog)
{
    ui->setupUi(this);

    qInstallMessageHandler(MessageHandler::myMessageOutput);

    setAttribute(Qt::WA_DeleteOnClose);

    this->info = info;
    this->settings = settings;

    if (!info->getProfileLocation().isEmpty())
        ui->profileLineEdit->setText(info->getProfileLocation());

    connect(ui->profileBrowsePushButton, &QPushButton::clicked, this, &EditAllDefaultAutoProfileDialog::openProfileBrowseDialog);
    connect(this, &EditAllDefaultAutoProfileDialog::accepted, this, &EditAllDefaultAutoProfileDialog::saveAutoProfileInformation);
}

EditAllDefaultAutoProfileDialog::~EditAllDefaultAutoProfileDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    delete ui;
}

void EditAllDefaultAutoProfileDialog::openProfileBrowseDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString preferredProfileDir = PadderCommon::preferredProfileDir(settings);
    QString profileFilename = QFileDialog::getOpenFileName(this, tr("Open Config"), preferredProfileDir, QString("Config Files (*.amgp *.xml)"));

    if (!profileFilename.isNull() && !profileFilename.isEmpty()) ui->profileLineEdit->setText(profileFilename);
}

void EditAllDefaultAutoProfileDialog::saveAutoProfileInformation()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    //info->setGUID("all");
    info->setUniqueID("all");
    info->setProfileLocation(ui->profileLineEdit->text());
    info->setActive(true);
}

AutoProfileInfo* EditAllDefaultAutoProfileDialog::getAutoProfile() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return info;
}

void EditAllDefaultAutoProfileDialog::accept()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool validForm = true;
    QString errorString = QString();

    if (ui->profileLineEdit->text().length() > 0)
    {
        QFileInfo profileInfo(ui->profileLineEdit->text());

        if (!profileInfo.exists())
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

AntiMicroSettings* EditAllDefaultAutoProfileDialog::getSettings() const {

    return settings;
}
