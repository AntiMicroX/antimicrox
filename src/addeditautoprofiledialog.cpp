#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QList>
#include <QListIterator>
#include <QEventLoop>
#include <QMessageBox>

#include "addeditautoprofiledialog.h"
#include "ui_addeditautoprofiledialog.h"

#if defined(Q_OS_UNIX)
#include <X11/Xlib.h>
#include <X11/cursorfont.h> // for XGrabPointer
#include "x11info.h"
#include "antkeymapper.h"

#elif defined(Q_OS_WIN)
#include "winappprofiletimerdialog.h"
#include "wininfo.h"

#endif

AddEditAutoProfileDialog::AddEditAutoProfileDialog(AutoProfileInfo *info, QSettings *settings,
                                                   QList<InputDevice*> *devices,
                                                   QList<QString> &reservedGUIDS, bool edit, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddEditAutoProfileDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    this->info = info;
    this->settings = settings;
    this->editForm = edit;
    this->devices = devices;
    this->originalGUID = info->getGUID();
    this->originalExe = info->getExe();

    QListIterator<QString> iterGUIDs(reservedGUIDS);
    while (iterGUIDs.hasNext())
    {
        QString guid = iterGUIDs.next();
        if (!this->reservedGUIDs.contains(guid))
        {
            this->reservedGUIDs.append(guid);
        }
    }

    bool allowDefault = false;
    if (info->getGUID() != "all" &&
        info->getGUID() != "" &&
        !this->reservedGUIDs.contains(info->getGUID()))
    {
        allowDefault = true;
    }

    if (allowDefault && info->getExe().isEmpty())
    {
        ui->asDefaultCheckBox->setEnabled(true);
        if (info->isCurrentDefault())
        {
            ui->asDefaultCheckBox->setChecked(true);
        }
    }
    else
    {
        ui->asDefaultCheckBox->setToolTip(tr("A different profile is already selected as the default for this device."));
    }

    //if (!edit)
    //{

        ui->devicesComboBox->addItem("all");
        QListIterator<InputDevice*> iter(*devices);
        int found = -1;
        int numItems = 1;
        while (iter.hasNext())
        {
            InputDevice *device = iter.next();
            ui->devicesComboBox->addItem(device->getSDLName(), QVariant::fromValue<InputDevice*>(device));
            if (device->getGUIDString() == info->getGUID())
            {
                found = numItems;
            }
            numItems++;
        }

        if (!info->getGUID().isEmpty() && info->getGUID() != "all")
        {
            if (found >= 0)
            {
                ui->devicesComboBox->setCurrentIndex(found);
            }
            else
            {
                ui->devicesComboBox->addItem(tr("Current (%1)").arg(info->getDeviceName()));
                ui->devicesComboBox->setCurrentIndex(ui->devicesComboBox->count()-1);
            }
        }
    //}

    if (!info->getProfileLocation().isEmpty())
    {
        ui->profileLineEdit->setText(info->getProfileLocation());
    }

    if (!info->getExe().isEmpty())
    {
        ui->applicationLineEdit->setText(info->getExe());
    }

    connect(ui->profileBrowsePushButton, SIGNAL(clicked()), this, SLOT(openProfileBrowseDialog()));
    connect(ui->applicationPushButton, SIGNAL(clicked()), this, SLOT(openApplicationBrowseDialog()));
    connect(ui->devicesComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(checkForReservedGUIDs(int)));
    connect(ui->applicationLineEdit, SIGNAL(textChanged(QString)), this, SLOT(checkForDefaultStatus(QString)));

#if defined(Q_OS_LINUX)
    connect(ui->selectWindowPushButton, SIGNAL(clicked()), this, SLOT(selectWindowWithCursor()));
#elif defined(Q_OS_WIN)
    connect(ui->selectWindowPushButton, SIGNAL(clicked()), this, SLOT(openWinAppProfileDialog()));
#endif
    //connect(ui->devicesComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT()
    //connect(ui->asDefaultCheckBox, SIGNAL(clicked(bool)), this, SLOT());
    //connect(ui->devicesComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT());
    connect(this, SIGNAL(accepted()), this, SLOT(saveAutoProfileInformation()));
}

AddEditAutoProfileDialog::~AddEditAutoProfileDialog()
{
    delete ui;
}

void AddEditAutoProfileDialog::openProfileBrowseDialog()
{
    QString lookupDir = preferredProfileDir();
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Config"), lookupDir, QString("Config Files (*.xml)"));
    if (!filename.isNull() && !filename.isEmpty())
    {
        ui->profileLineEdit->setText(filename);
    }
}

void AddEditAutoProfileDialog::openApplicationBrowseDialog()
{
    /*QString filename;
    QFileDialog dialog(this, tr("Select Program"), QDir::homePath());
    dialog.setFilter(QDir::Files | QDir::Executable);
    if (dialog.exec())
    {
        filename = dialog.selectedFiles().first();
    }*/
    QString filename = QFileDialog::getOpenFileName(this, tr("Select Program"), QDir::homePath(), QString());
    if (!filename.isNull() && !filename.isEmpty())
    {
        QFileInfo exe(filename);
        if (exe.exists() && exe.isExecutable())
        {
            ui->applicationLineEdit->setText(filename);
        }
    }
}

AutoProfileInfo* AddEditAutoProfileDialog::getAutoProfile()
{
    return info;
}

void AddEditAutoProfileDialog::saveAutoProfileInformation()
{
    info->setProfileLocation(ui->profileLineEdit->text());
    int deviceIndex = ui->devicesComboBox->currentIndex();
    if (deviceIndex > 0)
    {
        QVariant temp = ui->devicesComboBox->itemData(deviceIndex, Qt::UserRole);
        // Assume that if the following is not true, the GUID should
        // not be changed.
        if (!temp.isNull())
        {
            InputDevice *device = ui->devicesComboBox->itemData(deviceIndex, Qt::UserRole).value<InputDevice*>();
            info->setGUID(device->getGUIDString());
            info->setDeviceName(device->getSDLName());
        }
    }
    else
    {
        info->setGUID("all");
        info->setDeviceName("");
    }

    info->setExe(ui->applicationLineEdit->text());
    info->setDefaultState(ui->asDefaultCheckBox->isChecked());
    //info->setActive(true);
}

QString AddEditAutoProfileDialog::preferredProfileDir()
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

void AddEditAutoProfileDialog::checkForReservedGUIDs(int index)
{
    QVariant data = ui->devicesComboBox->itemData(index);
    if (index == 0)
    {
        ui->asDefaultCheckBox->setChecked(false);
        ui->asDefaultCheckBox->setEnabled(false);
        ui->asDefaultCheckBox->setToolTip(tr("Please use the main default profile selection."));
    }
    else if (!data.isNull())
    {
        InputDevice *device = data.value<InputDevice*>();
        if (reservedGUIDs.contains(device->getGUIDString()))
        {
            ui->asDefaultCheckBox->setChecked(false);
            ui->asDefaultCheckBox->setEnabled(false);
            ui->asDefaultCheckBox->setToolTip(tr("A different profile is already selected as the default for this device."));
        }
        else
        {
            ui->asDefaultCheckBox->setEnabled(true);
            ui->asDefaultCheckBox->setToolTip(tr("Select this profile to be the default loaded for\nthe specified device. The selection will be used instead\nof the all default profile option."));
        }
    }
}

QString AddEditAutoProfileDialog::getOriginalGUID()
{
    return originalGUID;
}

QString AddEditAutoProfileDialog::getOriginalExe()
{
    return originalExe;
}

#ifdef Q_OS_UNIX
void AddEditAutoProfileDialog::selectWindowWithCursor()
{
    Cursor cursor;
    Window target_window = None;
    int status = 0;
    cursor = XCreateFontCursor(X11Info::display(), XC_crosshair);

    status = XGrabPointer(X11Info::display(), X11Info::appRootWindow(), False, ButtonPressMask,
                 GrabModeSync, GrabModeAsync, None,
                 cursor, CurrentTime);
    if (status == Success)
    {
        XGrabKey(X11Info::display(), XKeysymToKeycode(X11Info::display(), AntKeyMapper::returnVirtualKey(Qt::Key_Escape)), 0, X11Info::appRootWindow(),
                 true, GrabModeAsync, GrabModeAsync);

        XEvent event;
        XAllowEvents(X11Info::display(), SyncPointer, CurrentTime);
        XWindowEvent(X11Info::display(), X11Info::appRootWindow(), ButtonPressMask|KeyPressMask, &event);
        switch (event.type)
        {
            case (ButtonPress):
                target_window = event.xbutton.subwindow;
                if (target_window == None) {
                    target_window = event.xbutton.window;
                }

                target_window = X11Info::findClientInChildren(target_window);
                break;

            case (KeyPress):
                break;
        }

        XUngrabKey(X11Info::display(), XKeysymToKeycode(X11Info::display(), AntKeyMapper::returnVirtualKey(Qt::Key_Escape)),
                   0, X11Info::appRootWindow());
        XUngrabPointer(X11Info::display(), CurrentTime);
        XFlush(X11Info::display());
    }

    if (target_window != None)
    {
        int pid = X11Info::getApplicationPid(target_window);
        if (pid > 0)
        {
            QString exepath = X11Info::getApplicationLocation(pid);

            if (!exepath.isEmpty())
            {
                ui->applicationLineEdit->setText(exepath);
            }
        }
    }
}
#endif

void AddEditAutoProfileDialog::checkForDefaultStatus(QString text)
{
    if (text.length() > 0)
    {
        ui->asDefaultCheckBox->setChecked(false);
        ui->asDefaultCheckBox->setEnabled(false);
    }
    else
    {
        ui->asDefaultCheckBox->setEnabled(true);
    }
}

void AddEditAutoProfileDialog::accept()
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

    if (validForm && ui->applicationLineEdit->text().length() > 0)
    {
        QString exeFileName = ui->applicationLineEdit->text();
        QFileInfo info(exeFileName);
        if (!info.exists() || !info.isExecutable())
        {
            validForm = false;
            errorString = tr("Program path is invalid or not executable.");
        }
    }
    else if (validForm && !ui->asDefaultCheckBox->isChecked())
    {
        validForm = false;
        errorString = tr("No program selected.");
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

bool AddEditAutoProfileDialog::validateForm()
{
    info->setProfileLocation(ui->profileLineEdit->text());
    int deviceIndex = ui->devicesComboBox->currentIndex();
    if (deviceIndex > 0)
    {
        QVariant temp = ui->devicesComboBox->itemData(deviceIndex, Qt::UserRole);
        // Assume that if the following is not true, the GUID should
        // not be changed.
        if (!temp.isNull())
        {
            InputDevice *device = ui->devicesComboBox->itemData(deviceIndex, Qt::UserRole).value<InputDevice*>();
            info->setGUID(device->getGUIDString());
            info->setDeviceName(device->getSDLName());
        }
    }
    else
    {
        info->setGUID("all");
        info->setDeviceName("");
    }

    info->setExe(ui->applicationLineEdit->text());
    info->setDefaultState(ui->asDefaultCheckBox->isChecked());

    return true;
}

#ifdef Q_OS_WIN
void AddEditAutoProfileDialog::openWinAppProfileDialog()
{
    WinAppProfileTimerDialog *dialog = new WinAppProfileTimerDialog(this);
    connect(dialog, SIGNAL(accepted()), this, SLOT(captureWindowsApplicationPath()));
    dialog->show();
}

void AddEditAutoProfileDialog::captureWindowsApplicationPath()
{
    QString temp = WinInfo::getForegroundWindowExePath();
    if (!temp.isEmpty())
    {
        ui->applicationLineEdit->setText(temp);
    }
}

#endif
