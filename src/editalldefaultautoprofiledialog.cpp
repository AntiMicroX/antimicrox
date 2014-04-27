#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>

#include "editalldefaultautoprofiledialog.h"
#include "ui_editalldefaultautoprofiledialog.h"

EditAllDefaultAutoProfileDialog::EditAllDefaultAutoProfileDialog(AutoProfileInfo *info, QSettings *settings,
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
    QString lookupDir = preferredProfileDir();
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Config"), lookupDir, QString("Config Files (*.xml)"));
    if (!filename.isNull() && !filename.isEmpty())
    {
        ui->profileLineEdit->setText(filename);
    }
}

void EditAllDefaultAutoProfileDialog::saveAutoProfileInformation()
{
    info->setProfileLocation(ui->profileLineEdit->text());
    info->setGUID("all");
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
    else
    {
        validForm = false;
        errorString = tr("No profile selected.");
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

QString EditAllDefaultAutoProfileDialog::preferredProfileDir()
{
    QString lastProfileDir = settings->value("LastProfileDir", "").toString();
    QString defaultProfileDir = settings->value("DefaultProfileDir", "").toString();
    QString lookupDir;

    if (!defaultProfileDir.isEmpty())
    {
        QFileInfo dirinfo(defaultProfileDir);
        if (dirinfo.isDir() && dirinfo.isReadable())
        {
            lookupDir = defaultProfileDir;
        }
    }

    if (lookupDir.isEmpty() && !lastProfileDir.isEmpty())
    {
        QFileInfo dirinfo(lastProfileDir);
        if (dirinfo.isDir() && dirinfo.isReadable())
        {
            lookupDir = lastProfileDir;
        }
    }

    if (lookupDir.isEmpty())
    {
        lookupDir = QDir::homePath();
    }

    return lookupDir;
}
