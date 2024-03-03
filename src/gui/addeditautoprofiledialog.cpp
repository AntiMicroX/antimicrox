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

#include "addeditautoprofiledialog.h"
#include "ui_addeditautoprofiledialog.h"

#include "antimicrosettings.h"
#include "autoprofileinfo.h"
#include "common.h"
#include "inputdevice.h"

#if defined(Q_OS_UNIX)
    #ifdef WITH_X11
        #include "capturedwindowinfodialog.h"
        #include "unixcapturewindowutility.h"
        #include "x11extras.h"
    #endif
#elif defined(Q_OS_WIN)
    #include "capturedwindowinfodialog.h"
    #include "winappprofiletimerdialog.h"
    #include "winextras.h"
#endif

#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QList>
#include <QListIterator>
#include <QMessageBox>
#include <QThread>

AddEditAutoProfileDialog::AddEditAutoProfileDialog(AutoProfileInfo *info, AntiMicroSettings *settings,
                                                   QList<InputDevice *> *devices, QList<QString> &reservedUniques, bool edit,
                                                   QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddEditAutoProfileDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    this->info = info;
    this->settings = settings;
    this->editForm = edit;
    this->devices = devices;
    this->originalUniqueID = info->getUniqueID();
    this->originalExe = info->getExe();
    this->originalWindowClass = info->getWindowClass();
    this->originalWindowName = info->getWindowName();

    if (info->isPartialState())
        ui->setPartialCheckBox->setChecked(true);
    else
        ui->setPartialCheckBox->setChecked(false);

    QListIterator<QString> iterUniques(reservedUniques);

    while (iterUniques.hasNext())
    {
        QString uniqueID = iterUniques.next();

        if (!getReservedUniques().contains(uniqueID))
            this->reservedUniques.append(uniqueID);
    }

    ui->devicesComboBox->addItem("all");
    QListIterator<InputDevice *> iter(*devices);
    int found = -1;
    int numItems = 1;

    while (iter.hasNext())
    {
        InputDevice *device = iter.next();
        ui->devicesComboBox->addItem(device->getSDLName(), QVariant::fromValue<InputDevice *>(device));

        if (device->getUniqueIDString() == info->getUniqueID())
            found = numItems;

        numItems++;
    }

    if (!info->getUniqueID().isEmpty() && (info->getUniqueID() != "all"))
    {
        if (found >= 0)
        {
            ui->devicesComboBox->setCurrentIndex(found);
        } else
        {
            ui->devicesComboBox->addItem(tr("Current (%1)").arg(info->getDeviceName()));
            ui->devicesComboBox->setCurrentIndex(ui->devicesComboBox->count() - 1);
        }
    }

    ui->profileLineEdit->setText(info->getProfileLocation());
    ui->applicationLineEdit->setText(info->getExe());
    ui->winClassLineEdit->setText(info->getWindowClass());
    ui->winNameLineEdit->setText(info->getWindowName());
#ifdef Q_OS_UNIX
    ui->selectWindowPushButton->setVisible(false);
#elif defined(Q_OS_WIN)
    ui->detectWinPropsSelectWindowPushButton->setVisible(false);

    ui->winClassLineEdit->setVisible(false);
    ui->winClassLabel->setVisible(false);
    // ui->winNameLineEdit->setVisible(false);
    // ui->winNameLabel->setVisible(false);
#endif
    ui->asDefaultCheckBox->setEnabled(info->isCurrentDefault());

    connect(ui->profileBrowsePushButton, &QPushButton::clicked, this, &AddEditAutoProfileDialog::openProfileBrowseDialog);
    connect(ui->applicationPushButton, &QPushButton::clicked, this, &AddEditAutoProfileDialog::openApplicationBrowseDialog);
    connect(ui->devicesComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &AddEditAutoProfileDialog::checkForReservedUniques);
    connect(ui->devicesComboBox, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentTextChanged), this,
            &AddEditAutoProfileDialog::checkDefaultCheckbox);
    connect(ui->applicationLineEdit, &QLineEdit::textChanged, this, &AddEditAutoProfileDialog::checkForDefaultStatus);
    connect(ui->winClassLineEdit, &QLineEdit::textChanged, this, &AddEditAutoProfileDialog::checkForDefaultStatus);
    connect(ui->winNameLineEdit, &QLineEdit::textChanged, this, &AddEditAutoProfileDialog::checkForDefaultStatus);
#if defined(Q_OS_UNIX)
    connect(ui->detectWinPropsSelectWindowPushButton, &QPushButton::clicked, this,
            &AddEditAutoProfileDialog::showCaptureHelpWindow);
#elif defined(Q_OS_WIN)
    connect(ui->selectWindowPushButton, &QPushButton::clicked, this, &AddEditAutoProfileDialog::openWinAppProfileDialog);
#endif
    connect(this, &AddEditAutoProfileDialog::accepted, this, &AddEditAutoProfileDialog::saveAutoProfileInformation);

    ui->asDefaultCheckBox->setChecked(info->isCurrentDefault());
}

// created for tests
AddEditAutoProfileDialog::AddEditAutoProfileDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddEditAutoProfileDialog)
{
}

AddEditAutoProfileDialog::~AddEditAutoProfileDialog() { delete ui; }

void AddEditAutoProfileDialog::openProfileBrowseDialog()
{
    QString lookupDir = PadderCommon::preferredProfileDir(settings);
    QString filename =
        QFileDialog::getOpenFileName(this, tr("Open Config"), lookupDir, QString("Config Files (*.amgp *.xml)"));

    if (!filename.isNull() && !filename.isEmpty())
        ui->profileLineEdit->setText(QDir::toNativeSeparators(filename));
}

void AddEditAutoProfileDialog::openApplicationBrowseDialog()
{
#ifdef Q_OS_WIN
    QString filename = QFileDialog::getOpenFileName(this, tr("Select Program"), QDir::homePath(), tr("Programs (*.exe)"));
#else
    QString filename = QFileDialog::getOpenFileName(this, tr("Select Program"), QDir::homePath(), QString());
#endif
    if (!filename.isNull() && !filename.isEmpty())
    {
        QFileInfo exe(filename);

        if (exe.exists() && exe.isExecutable())
            ui->applicationLineEdit->setText(filename);
    }
}

AutoProfileInfo *AddEditAutoProfileDialog::getAutoProfile() const { return info; }

void AddEditAutoProfileDialog::saveAutoProfileInformation()
{
    info->setProfileLocation(ui->profileLineEdit->text());
    int deviceIndex = ui->devicesComboBox->currentIndex();

    if (deviceIndex > 0)
    {
        // Assume that if the following is not true, the GUID should
        // not be changed.
        if (!ui->devicesComboBox->itemData(deviceIndex, Qt::UserRole).isNull())
        {
            InputDevice *device = ui->devicesComboBox->itemData(deviceIndex, Qt::UserRole).value<InputDevice *>();
            info->setUniqueID(device->getUniqueIDString());
            info->setDeviceName(device->getSDLName());
        }
    } else
    {
        info->setUniqueID("all");
        info->setDeviceName("");
    }

    info->setExe(ui->applicationLineEdit->text());
    info->setWindowClass(ui->winClassLineEdit->text());
    info->setWindowName(ui->winNameLineEdit->text());
    info->setDefaultState(ui->asDefaultCheckBox->isChecked());
    info->setPartialState(ui->setPartialCheckBox->isChecked());
}

void AddEditAutoProfileDialog::checkForReservedUniques(int index)
{
    QVariant data = ui->devicesComboBox->itemData(index);

    if (index == 0)
    {
        ui->asDefaultCheckBox->setChecked(false);
        ui->asDefaultCheckBox->setEnabled(false);
        QMessageBox::warning(this, tr("Main Profile"), tr("Please use the main default profile selection."));
    } else if (!data.isNull() && getReservedUniques().contains(data.value<InputDevice *>()->getUniqueIDString()))
    {
        ui->asDefaultCheckBox->setChecked(false);
        ui->asDefaultCheckBox->setEnabled(false);
        QMessageBox::warning(this, tr("Already selected"),
                             tr("A different profile is already selected as the default for this device."));
    } else
    {
        ui->asDefaultCheckBox->setEnabled(true);
        QMessageBox::information(this, tr("Chosen Profile"),
                                 tr("The selection will be used instead\nof the all default profile option."));
    }
}

QString AddEditAutoProfileDialog::getOriginalUniqueID() const { return originalUniqueID; }

QString AddEditAutoProfileDialog::getOriginalExe() const { return originalExe; }

QString AddEditAutoProfileDialog::getOriginalWindowClass() const { return originalWindowClass; }

QString AddEditAutoProfileDialog::getOriginalWindowName() const { return originalWindowName; }

#ifdef Q_OS_UNIX
/**
 * @brief Display a simple message box and attempt to capture a window using the mouse
 */
void AddEditAutoProfileDialog::showCaptureHelpWindow()
{
    #ifdef WITH_X11

    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
        QMessageBox *box = new QMessageBox(this);
        box->setText(tr("Please select a window by using the mouse. Press Escape if you want to cancel."));
        box->setWindowTitle(tr("Capture Application Window"));
        box->setStandardButtons(QMessageBox::NoButton);
        box->setModal(true);
        box->show();

        UnixCaptureWindowUtility *util = new UnixCaptureWindowUtility();
        QThread *capture_window_thr = new QThread; // QTHREAD(this)
        capture_window_thr->setObjectName("capture_window_thr");

        util->moveToThread(capture_window_thr);

        connect(capture_window_thr, &QThread::started, util, &UnixCaptureWindowUtility::attemptWindowCapture);
        connect(util, &UnixCaptureWindowUtility::captureFinished, capture_window_thr, &QThread::quit);
        connect(util, &UnixCaptureWindowUtility::captureFinished, box, &QMessageBox::hide);
        connect(
            util, &UnixCaptureWindowUtility::captureFinished, this, [this, util]() { checkForGrabbedWindow(util); },
            Qt::QueuedConnection);

        connect(capture_window_thr, &QThread::finished, box, &QMessageBox::deleteLater);
        connect(util, &UnixCaptureWindowUtility::destroyed, capture_window_thr, &QThread::deleteLater);
        capture_window_thr->start();
    }

    #endif
}

/**
 * @brief Check if there is a program path saved in an UnixCaptureWindowUtility
 *     object
 */
void AddEditAutoProfileDialog::checkForGrabbedWindow(UnixCaptureWindowUtility *util)
{
    #ifdef WITH_X11
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
        long targetWindow = util->getTargetWindow();
        bool escaped = !util->hasFailed();
        bool failed = false;

        if (targetWindow != None)
        {
            // Attempt to find the appropriate window below the root window
            // that was clicked.

            qDebug() << "ORIGINAL: " << QString::number(targetWindow, 16);

            long tempWindow = X11Extras::getInstance()->findClientWindow(targetWindow);
            if (tempWindow > 0)
            {
                targetWindow = tempWindow;
            }

            qDebug() << "ADJUSTED: " << QString::number(targetWindow, 16);
        }

        if (targetWindow != None)
        {
            CapturedWindowInfoDialog *dialog = new CapturedWindowInfoDialog(targetWindow, this);
            connect(dialog, &CapturedWindowInfoDialog::accepted, [this, dialog]() { windowPropAssignment(dialog); });

            dialog->show();
        } else if (!escaped)
        {
            failed = true;
        }

        // Ensure that the operation was not cancelled (Escape wasn't pressed).
        if (failed)
        {
            QMessageBox box;
            box.setText(tr("Could not obtain information for the selected window."));
            box.setWindowTitle(tr("Application Capture Failed"));
            box.setStandardButtons(QMessageBox::Close);
            box.raise();
            box.exec();
        }

        util->deleteLater();
    }
    #endif
}
#endif

void AddEditAutoProfileDialog::windowPropAssignment(CapturedWindowInfoDialog *dialog)
{
    disconnect(ui->applicationLineEdit, &QLineEdit::textChanged, this, &AddEditAutoProfileDialog::checkForDefaultStatus);
    disconnect(ui->winClassLineEdit, &QLineEdit::textChanged, this, &AddEditAutoProfileDialog::checkForDefaultStatus);
    disconnect(ui->winNameLineEdit, &QLineEdit::textChanged, this, &AddEditAutoProfileDialog::checkForDefaultStatus);

    ui->applicationLineEdit->clear();
    ui->winClassLineEdit->clear();
    ui->winNameLineEdit->clear();

#ifdef WITH_X11
    if (dialog->useFullWindowPath() && dialog->getSelectedOptions() & CapturedWindowInfoDialog::WindowPath)
    {
        ui->applicationLineEdit->setText(dialog->getWindowPath());
    } else if (!dialog->useFullWindowPath() && dialog->getSelectedOptions() & CapturedWindowInfoDialog::WindowPath)
    {
        ui->applicationLineEdit->setText(QFileInfo(dialog->getWindowPath()).fileName());
    }

    if (dialog->getSelectedOptions() & CapturedWindowInfoDialog::WindowClass)
    {
        ui->winClassLineEdit->setText(dialog->getWindowClass());
    }

    if (dialog->getSelectedOptions() & CapturedWindowInfoDialog::WindowName)
    {
        ui->winNameLineEdit->setText(dialog->getWindowName());
    }
#endif

    checkForDefaultStatus();

    connect(ui->applicationLineEdit, &QLineEdit::textChanged, this, &AddEditAutoProfileDialog::checkForDefaultStatus);
    connect(ui->winClassLineEdit, &QLineEdit::textChanged, this, &AddEditAutoProfileDialog::checkForDefaultStatus);
    connect(ui->winNameLineEdit, &QLineEdit::textChanged, this, &AddEditAutoProfileDialog::checkForDefaultStatus);
}

void AddEditAutoProfileDialog::checkForDefaultStatus()
{
    bool status = ui->applicationLineEdit->text().length() > 0;
    status = status ? status : (ui->winClassLineEdit->text().length() > 0);
    status = status ? status : (ui->winNameLineEdit->text().length() > 0);

    if (status)
    {
        ui->asDefaultCheckBox->setChecked(false);
        ui->asDefaultCheckBox->setEnabled(false);
    } else
    {
        ui->asDefaultCheckBox->setEnabled(true);
    }
}

/**
 * @throw std::runtime_error
 */
void AddEditAutoProfileDialog::check_profile_file()
{
    if (ui->profileLineEdit->text().length() > 0)
    {
        QFileInfo profileFileName(ui->profileLineEdit->text());

        if (!profileFileName.exists())
        {
            throw std::runtime_error(tr("Profile file path is invalid.").toStdString());
        }
    }
}

/**
 * @throw std::runtime_error
 */
void AddEditAutoProfileDialog::check_executable_file()
{
    if (!ui->applicationLineEdit->text().isEmpty())
    {
        QString exeFileName = ui->applicationLineEdit->text();
        QFileInfo info(exeFileName);

        if (info.isAbsolute() && (!info.exists() || !info.isExecutable()))
        {
            throw std::runtime_error(tr("Program path is invalid or not executable.").toStdString());
        }
#ifdef Q_OS_WIN
        else if (!info.isAbsolute() && (info.fileName() != exeFileName || info.suffix() != "exe"))
        {
            throw std::runtime_error(tr("File is not an .exe file.").toStdString());
        }
#endif
    }
}

/**
 * @brief Validate the form that is contained in this window
 */
void AddEditAutoProfileDialog::accept()
{
    QString errorString = QString();
    try
    {
        check_profile_file();

        bool is_window_specified = !(ui->applicationLineEdit->text().isEmpty() && ui->winClassLineEdit->text().isEmpty() &&
                                     ui->winNameLineEdit->text().isEmpty());
        if (!is_window_specified && !ui->asDefaultCheckBox->isChecked())
            throw std::runtime_error(tr("No window matching property was specified.").toStdString());
        check_executable_file();

    } catch (const std::runtime_error &e)
    {
        QMessageBox msgBox;
        msgBox.setText(e.what());
        msgBox.setStandardButtons(QMessageBox::Close);
        msgBox.exec();
    }

    QDialog::accept();
}

QList<InputDevice *> *AddEditAutoProfileDialog::getDevices() const { return devices; }

AntiMicroSettings *AddEditAutoProfileDialog::getSettings() const { return settings; }

bool AddEditAutoProfileDialog::getEditForm() const { return editForm; }

bool AddEditAutoProfileDialog::getDefaultInfo() const { return defaultInfo; }

QList<QString> const &AddEditAutoProfileDialog::getReservedUniques() { return reservedUniques; }

void AddEditAutoProfileDialog::checkDefaultCheckbox(const QString &text)
{
    if (text == "all")
    {
        ui->asDefaultCheckBox->setChecked(false);
        ui->asDefaultCheckBox->setDisabled(true);
    } else
    {
        ui->asDefaultCheckBox->setDisabled(false);
    }
}

#ifdef Q_OS_WIN
void AddEditAutoProfileDialog::openWinAppProfileDialog()
{
    WinAppProfileTimerDialog *dialog = new WinAppProfileTimerDialog(this);
    connect(dialog, &WinAppProfileTimerDialog::accepted, this, &AddEditAutoProfileDialog::captureWindowsApplicationPath);
    dialog->show();
}

void AddEditAutoProfileDialog::callWindowPropAssignment() { windowPropAssignment(m_capture_window_info_dialog); }

void AddEditAutoProfileDialog::captureWindowsApplicationPath()
{
    m_capture_window_info_dialog = new CapturedWindowInfoDialog(this);
    connect(m_capture_window_info_dialog, &CapturedWindowInfoDialog::accepted, this,
            &AddEditAutoProfileDialog::callWindowPropAssignment);
    m_capture_window_info_dialog->show();
}
#endif
