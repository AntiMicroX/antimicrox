#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QList>
#include <QListIterator>
#include <QMessageBox>
#include <QThread>

#include "addeditautoprofiledialog.h"
#include "ui_addeditautoprofiledialog.h"

#if defined(Q_OS_UNIX)

    #ifdef WITH_X11
#include "unixcapturewindowutility.h"
#include "x11info.h"

    #endif

    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QApplication>
    #endif

#elif defined(Q_OS_WIN)
#include "winappprofiletimerdialog.h"
#include "wininfo.h"

#endif

#include "common.h"

AddEditAutoProfileDialog::AddEditAutoProfileDialog(AutoProfileInfo *info, AntiMicroSettings *settings,
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

#if defined(Q_OS_UNIX)
    connect(ui->selectWindowPushButton, SIGNAL(clicked()), this, SLOT(showCaptureHelpWindow()));
#elif defined(Q_OS_WIN)
    connect(ui->selectWindowPushButton, SIGNAL(clicked()), this, SLOT(openWinAppProfileDialog()));
#endif

    connect(this, SIGNAL(accepted()), this, SLOT(saveAutoProfileInformation()));
}

AddEditAutoProfileDialog::~AddEditAutoProfileDialog()
{
    delete ui;
}

void AddEditAutoProfileDialog::openProfileBrowseDialog()
{
    QString lookupDir = PadderCommon::preferredProfileDir(settings);
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Config"), lookupDir, QString("Config Files (*.amgp *.xml)"));
    if (!filename.isNull() && !filename.isEmpty())
    {
        ui->profileLineEdit->setText(QDir::toNativeSeparators(filename));
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
/**
 * @brief Display a simple message box and attempt to capture a window using the mouse
 */
void AddEditAutoProfileDialog::showCaptureHelpWindow()
{
#ifdef WITH_X11
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
    #endif
    QMessageBox *box = new QMessageBox(this);
    box->setText(tr("Please select a window by using the mouse. Press Escape if you want to cancel."));
    box->setWindowTitle(tr("Capture Application Window"));
    box->setStandardButtons(QMessageBox::NoButton);
    box->setModal(true);
    box->show();

    UnixCaptureWindowUtility *util = new UnixCaptureWindowUtility();
    QThread *thread = new QThread(this);
    util->moveToThread(thread);

    connect(thread, SIGNAL(started()), util, SLOT(attemptWindowCapture()));
    connect(util, SIGNAL(captureFinished()), thread, SLOT(quit()));
    connect(util, SIGNAL(captureFinished()), box, SLOT(hide()));
    connect(util, SIGNAL(captureFinished()), this, SLOT(checkCapturedPath()), Qt::QueuedConnection);

    connect(thread, SIGNAL(finished()), box, SLOT(deleteLater()));
    connect(util, SIGNAL(destroyed()), thread, SLOT(deleteLater()));
    thread->start();
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    }
    #endif
#endif
}

/**
 * @brief Check if there is a program path saved in an UnixCaptureWindowUtility
 *     object
 */
void AddEditAutoProfileDialog::checkCapturedPath()
{
#ifdef WITH_X11
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
    #endif
    UnixCaptureWindowUtility *util = static_cast<UnixCaptureWindowUtility*>(sender());
    unsigned long targetWindow = util->getTargetWindow();
    bool escaped = !util->hasFailed();
    bool failed = false;
    QString path;

    if (targetWindow != None)
    {
        // Attempt to find the appropriate window below the root window
        // that was clicked.
        targetWindow = X11Info::getInstance()->findClientWindow(targetWindow);
    }

    if (targetWindow != None)
    {
        int pid = X11Info::getInstance()->getApplicationPid(targetWindow);
        if (pid > 0)
        {
            QString exepath = X11Info::getInstance()->getApplicationLocation(pid);

            if (!exepath.isEmpty())
            {
                path = exepath;
            }
            else if (!failed)
            {
                failed = true;
            }
        }
        else if (!failed)
        {
            failed = true;
        }
    }
    else if (!escaped)
    {
        failed = true;
    }

    if (!path.isEmpty())
    {
        ui->applicationLineEdit->setText(path);
    }
    // Ensure that the operation was not cancelled (Escape wasn't pressed).
    else if (failed)
    {
        QMessageBox box;
        box.setText(tr("Could not obtain information for the selected window."));
        box.setWindowTitle(tr("Application Capture Failed"));
        box.setStandardButtons(QMessageBox::Close);
        box.raise();
        box.exec();
    }

    util->deleteLater();
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    }
    #endif
#endif
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

/**
 * @brief Validate the form that is contained in this window
 */
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
