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
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Config"), lookupDir, QString("Config Files (*.xml)"));
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
