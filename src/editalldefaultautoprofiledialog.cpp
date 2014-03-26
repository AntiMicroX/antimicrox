#include <QFileInfo>
#include <QFileDialog>

#include "editalldefaultautoprofiledialog.h"
#include "ui_editalldefaultautoprofiledialog.h"

EditAllDefaultAutoProfileDialog::EditAllDefaultAutoProfileDialog(AutoProfileInfo *info, QSettings *settings,
                                                                 QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditAllDefaultAutoProfileDialog)
{
    ui->setupUi(this);

    this->info = info;
    this->settings = settings;

    if (!info->getProfileLocation().isEmpty())
    {
        ui->profileLineEdit->setText(info->getProfileLocation());
    }

    /*if (!info->getExe().isEmpty())
    {
        ui->applicationLineEdit->setText(info->getExe());
    }*/

    connect(ui->profileBrowsePushButton, SIGNAL(clicked()), this, SLOT(openProfileBrowseDialog()));
    //connect(ui->applicationPushButton, SIGNAL(clicked()), this, SLOT(openApplicationBrowseDialog()));
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

void EditAllDefaultAutoProfileDialog::openApplicationBrowseDialog()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select Program"), QDir::homePath(), QString());
    if (!filename.isNull() && !filename.isEmpty())
    {
        QFileInfo exe(filename);
        if (exe.exists() && exe.isExecutable())
        {
            //ui->applicationLineEdit->setText(filename);
        }
    }
}

void EditAllDefaultAutoProfileDialog::saveAutoProfileInformation()
{
    info->setProfileLocation(ui->profileLineEdit->text());
    info->setGUID("all");
    //info->setExe(ui->applicationLineEdit->text());
    info->setActive(true);
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
