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

#include "addeditautoprofiledialog.h"
#include "ui_addeditautoprofiledialog.h"

#include "messagehandler.h"
#include "autoprofileinfo.h"
#include "inputdevice.h"
#include "antimicrosettings.h"
#include "common.h"

  #ifdef WITH_X11
   #include "unixcapturewindowutility.h"
   #include "capturedwindowinfodialog.h"
   #include "x11extras.h"
 #endif

#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QList>
#include <QListIterator>
#include <QMessageBox>
#include <QThread>



AddEditAutoProfileDialog::AddEditAutoProfileDialog(AutoProfileInfo *info, AntiMicroSettings *settings,
                                                   QList<InputDevice*> *devices,
                                                   QList<QString> &reservedUniques, bool edit, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddEditAutoProfileDialog)
{
    ui->setupUi(this);

    qInstallMessageHandler(MessageHandler::myMessageOutput);
    setAttribute(Qt::WA_DeleteOnClose);

    this->info = info;
    this->settings = settings;
    this->editForm = edit;
    this->devices = devices;
    this->originalUniqueID = info->getUniqueID();
    this->originalExe = info->getExe();
    this->originalWindowClass = info->getWindowClass();
    this->originalWindowName = info->getWindowName();

    if (info->isPartialState()) ui->setPartialCheckBox->setChecked(true);
    else ui->setPartialCheckBox->setChecked(false);

    QListIterator<QString> iterUniques(reservedUniques);

    while (iterUniques.hasNext())
    {
        QString uniqueID = iterUniques.next();

        if (!getReservedUniques().contains(uniqueID)) this->reservedUniques.append(uniqueID);
    }

        ui->devicesComboBox->addItem("all");
        QListIterator<InputDevice*> iter(*devices);
        int found = -1;
        int numItems = 1;

        while (iter.hasNext())
        {
            InputDevice *device = iter.next();
            ui->devicesComboBox->addItem(device->getSDLName(), QVariant::fromValue<InputDevice*>(device));

            if (device->getUniqueIDString() == info->getUniqueID()) found = numItems;

            numItems++;
        }

        if (!info->getUniqueID().isEmpty() && (info->getUniqueID() != "all"))
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

    ui->profileLineEdit->setText(info->getProfileLocation());
    ui->applicationLineEdit->setText(info->getExe());
    ui->winClassLineEdit->setText(info->getWindowClass());
    ui->winNameLineEdit->setText(info->getWindowName());
    ui->selectWindowPushButton->setVisible(false);
    ui->asDefaultCheckBox->setEnabled(info->isCurrentDefault());

    connect(ui->profileBrowsePushButton, &QPushButton::clicked, this, &AddEditAutoProfileDialog::openProfileBrowseDialog);
    connect(ui->applicationPushButton, &QPushButton::clicked, this, &AddEditAutoProfileDialog::openApplicationBrowseDialog);
    connect(ui->devicesComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AddEditAutoProfileDialog::checkForReservedUniques);
    connect(ui->devicesComboBox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged), this, &AddEditAutoProfileDialog::checkDefaultCheckbox);
    connect(ui->applicationLineEdit, &QLineEdit::textChanged, this, &AddEditAutoProfileDialog::checkForDefaultStatus);
    connect(ui->winClassLineEdit, &QLineEdit::textChanged, this, &AddEditAutoProfileDialog::checkForDefaultStatus);
    connect(ui->winNameLineEdit, &QLineEdit::textChanged, this, &AddEditAutoProfileDialog::checkForDefaultStatus);

    connect(ui->detectWinPropsSelectWindowPushButton, &QPushButton::clicked, this, &AddEditAutoProfileDialog::showCaptureHelpWindow);

    connect(this, &AddEditAutoProfileDialog::accepted, this, &AddEditAutoProfileDialog::saveAutoProfileInformation);

    ui->asDefaultCheckBox->setChecked(info->isCurrentDefault());
}


// created for tests
AddEditAutoProfileDialog::AddEditAutoProfileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddEditAutoProfileDialog)
{

}


AddEditAutoProfileDialog::~AddEditAutoProfileDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    delete ui;
}


void AddEditAutoProfileDialog::openProfileBrowseDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString lookupDir = PadderCommon::preferredProfileDir(settings);
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Config"), lookupDir, QString("Config Files (*.amgp *.xml)"));

    if (!filename.isNull() && !filename.isEmpty()) ui->profileLineEdit->setText(QDir::toNativeSeparators(filename));
}


void AddEditAutoProfileDialog::openApplicationBrowseDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString filename = QFileDialog::getOpenFileName(this, tr("Select Program"), QDir::homePath(), QString());

    if (!filename.isNull() && !filename.isEmpty())
    {
        QFileInfo exe(filename);

        if (exe.exists() && exe.isExecutable()) ui->applicationLineEdit->setText(filename);
    }
}


AutoProfileInfo* AddEditAutoProfileDialog::getAutoProfile() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return info;
}


void AddEditAutoProfileDialog::saveAutoProfileInformation()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    info->setProfileLocation(ui->profileLineEdit->text());
    int deviceIndex = ui->devicesComboBox->currentIndex();


    if (deviceIndex > 0)
    {

        // Assume that if the following is not true, the GUID should
        // not be changed.
        if (!ui->devicesComboBox->itemData(deviceIndex, Qt::UserRole).isNull())
        {
            InputDevice *device = ui->devicesComboBox->itemData(deviceIndex, Qt::UserRole).value<InputDevice*>();
            info->setUniqueID(device->getUniqueIDString());
            info->setDeviceName(device->getSDLName());
        }
    }
    else
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QVariant data = ui->devicesComboBox->itemData(index);

    if (index == 0)
    {
        ui->asDefaultCheckBox->setChecked(false);
        ui->asDefaultCheckBox->setEnabled(false);
        QMessageBox::warning(this, tr("Main Profile"), tr("Please use the main default profile selection."));
    }
    else if (!data.isNull() && getReservedUniques().contains(data.value<InputDevice*>()->getUniqueIDString()))
    {
        ui->asDefaultCheckBox->setChecked(false);
        ui->asDefaultCheckBox->setEnabled(false);
        QMessageBox::warning(this, tr("Already selected"), tr("A different profile is already selected as the default for this device."));
    }
    else
    {
        ui->asDefaultCheckBox->setEnabled(true);
        QMessageBox::information(this, tr("Chosen Profile"), tr("The selection will be used instead\nof the all default profile option."));
    }
}


QString AddEditAutoProfileDialog::getOriginalUniqueID() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return originalUniqueID;
}


QString AddEditAutoProfileDialog::getOriginalExe() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return originalExe;
}

QString AddEditAutoProfileDialog::getOriginalWindowClass() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return originalWindowClass;
}

QString AddEditAutoProfileDialog::getOriginalWindowName() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return originalWindowName;
}


/**
 * @brief Display a simple message box and attempt to capture a window using the mouse
 */
void AddEditAutoProfileDialog::showCaptureHelpWindow()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);
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
        QThread *thread = new QThread; // QTHREAD(this)
        util->moveToThread(thread);

        connect(thread, &QThread::started, util, &UnixCaptureWindowUtility::attemptWindowCapture);
        connect(util, &UnixCaptureWindowUtility::captureFinished, thread, &QThread::quit);
        connect(util, &UnixCaptureWindowUtility::captureFinished, box, &QMessageBox::hide);
        connect(util, &UnixCaptureWindowUtility::captureFinished, this, [this, util]() {
            checkForGrabbedWindow(util);
        }, Qt::QueuedConnection);

        connect(thread, &QThread::finished, box, &QMessageBox::deleteLater);
        connect(util, &UnixCaptureWindowUtility::destroyed, thread, &QThread::deleteLater);
        thread->start();
    }

    #endif
}

/**
 * @brief Check if there is a program path saved in an UnixCaptureWindowUtility
 *     object
 */
void AddEditAutoProfileDialog::checkForGrabbedWindow(UnixCaptureWindowUtility* util)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
            connect(dialog, &CapturedWindowInfoDialog::accepted, [this, dialog]() {
                windowPropAssignment(dialog);
            });

            dialog->show();
        }
        else if (!escaped)
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


void AddEditAutoProfileDialog::windowPropAssignment(CapturedWindowInfoDialog *dialog)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    disconnect(ui->applicationLineEdit, &QLineEdit::textChanged, this, &AddEditAutoProfileDialog::checkForDefaultStatus);
    disconnect(ui->winClassLineEdit, &QLineEdit::textChanged, this, &AddEditAutoProfileDialog::checkForDefaultStatus);
    disconnect(ui->winNameLineEdit, &QLineEdit::textChanged, this, &AddEditAutoProfileDialog::checkForDefaultStatus);

    #ifdef WITH_X11
    if (dialog->useFullWindowPath() && dialog->getSelectedOptions() & CapturedWindowInfoDialog::WindowPath)
    {
       ui->applicationLineEdit->setText(dialog->getWindowPath());
    }
    else if (!dialog->useFullWindowPath() && dialog->getSelectedOptions() & CapturedWindowInfoDialog::WindowPath)
    {
        ui->applicationLineEdit->setText(QFileInfo(dialog->getWindowPath()).fileName());
    }
    else
    {
#endif
        ui->applicationLineEdit->clear();
#ifdef WITH_X11
    }
#endif

    #ifdef WITH_X11
    if (dialog->getSelectedOptions() & CapturedWindowInfoDialog::WindowClass)
    {
        ui->winClassLineEdit->setText(dialog->getWindowClass());
    }
    else
    {
    #endif
        ui->winClassLineEdit->clear();
#ifdef WITH_X11
    }
#endif

    #ifdef WITH_X11
    if (dialog->getSelectedOptions() & CapturedWindowInfoDialog::WindowName)
    {
        ui->winNameLineEdit->setText(dialog->getWindowName());
    }
    else
    {
    #endif
        ui->winNameLineEdit->clear();
#ifdef WITH_X11
    }
#endif

    checkForDefaultStatus();

    connect(ui->applicationLineEdit, &QLineEdit::textChanged, this, &AddEditAutoProfileDialog::checkForDefaultStatus);
    connect(ui->winClassLineEdit, &QLineEdit::textChanged, this, &AddEditAutoProfileDialog::checkForDefaultStatus);
    connect(ui->winNameLineEdit, &QLineEdit::textChanged, this, &AddEditAutoProfileDialog::checkForDefaultStatus);
}


void AddEditAutoProfileDialog::checkForDefaultStatus()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool status = ui->applicationLineEdit->text().length() > 0;
    status = status ? status : (ui->winClassLineEdit->text().length() > 0);
    status = status ? status : (ui->winNameLineEdit->text().length() > 0);

    if (status)
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool validForm = true;
    bool propertyFound = false;
    QString errorString = QString();

    if (ui->profileLineEdit->text().length() > 0)
    {
        QFileInfo profileFileName(ui->profileLineEdit->text());

        if (!profileFileName.exists())
        {
            validForm = false;
            errorString = tr("Profile file path is invalid.");
        }
    }

    if (validForm &&
        (ui->applicationLineEdit->text().isEmpty() &&
         ui->winClassLineEdit->text().isEmpty() &&
         ui->winNameLineEdit->text().isEmpty()))
    {
        validForm = false;
        errorString = tr("No window matching property was specified.");
    }
    else
    {
        propertyFound = true;
    }

    if (validForm && !ui->applicationLineEdit->text().isEmpty())
    {
        QString exeFileName = ui->applicationLineEdit->text();
        QFileInfo info(exeFileName);

        if (info.isAbsolute() && (!info.exists() || !info.isExecutable()))
        {
            validForm = false;
            errorString = tr("Program path is invalid or not executable.");
        }
    }

    if (validForm && !propertyFound && !ui->asDefaultCheckBox->isChecked())
    {
        validForm = false;
        errorString = tr("No window matching property was selected.");
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


QList<InputDevice*> *AddEditAutoProfileDialog::getDevices() const {

    return devices;
}

AntiMicroSettings *AddEditAutoProfileDialog::getSettings() const {

    return settings;
}

bool AddEditAutoProfileDialog::getEditForm() const {

    return editForm;
}

bool AddEditAutoProfileDialog::getDefaultInfo() const {

    return defaultInfo;
}

QList<QString> const& AddEditAutoProfileDialog::getReservedUniques() {

    return reservedUniques;
}

void AddEditAutoProfileDialog::on_setPartialCheckBox_stateChanged(int arg1)
{
    if (arg1 == 0) ui->winNameLineEdit->setEnabled(false);
    else ui->winNameLineEdit->setEnabled(true);
}

void AddEditAutoProfileDialog::checkDefaultCheckbox(const QString& text)
{
    if (text == "all")
    {
        ui->asDefaultCheckBox->setChecked(false);
        ui->asDefaultCheckBox->setDisabled(true);
    }
    else
    {
        ui->asDefaultCheckBox->setDisabled(false);
    }
}
