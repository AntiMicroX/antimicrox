/* antimicroX Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
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

#include "mainsettingsdialog.h"

#include "globalvariables.h"
#include "messagehandler.h"
#include "addeditautoprofiledialog.h"
#include "editalldefaultautoprofiledialog.h"
#include "common.h"
#include "autoprofileinfo.h"
#include "inputdevice.h"
#include "antimicrosettings.h"
#include "eventhandlerfactory.h"
#include "event.h"
#include "antkeymapper.h"

#ifdef WITH_X11
  #include "x11extras.h"
  #include <X11/keysymdef.h>
  #include <X11/XF86keysym.h>
  #include <X11/XKBlib.h>
  #include <X11/Xutil.h>
#elif WITH_UINPUT
  #include <linux/input.h>
  #include <linux/uinput.h>
#endif

#include <QTableWidgetItem>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QLocale>
#include <QTranslator>
#include <QListIterator>
#include <QStringList>
#include <QTableWidgetItem>
#include <QMapIterator>
#include <QVariant>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QList>
#include <QListWidget>
#include <QWidget>
#include <QApplication>



static const QString RUNATSTARTUPREGKEY(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run");
static const QString RUNATSTARTUPLOCATION(
        QString("%0\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\antimicroX.lnk")
        .arg(QString::fromUtf8(qgetenv("AppData"))));

MainSettingsDialog::MainSettingsDialog(AntiMicroSettings *settings,
                                       QList<InputDevice *> *devices,
                                       QWidget *parent) :
    QDialog(parent, Qt::Dialog),
    ui(new Ui::MainSettingsDialog)
{
    ui->setupUi(this);

    qInstallMessageHandler(MessageHandler::myMessageOutput);
    setAttribute(Qt::WA_DeleteOnClose);

    #ifndef WITH_X11
        ui->quitCombLineEdit->hide();
        ui->quitComboLabel->hide();
        ui->keysComboBox->hide();
        ui->keysLabel->hide();
        ui->warningComb->hide();
    #endif

    ui->profileOpenDirPushButton->setIcon(QIcon::fromTheme("document_open_folder",
                                                           QIcon(":/icons/hicolor/16x16/actions/document_open_folder.png")));

    ui->logFilePushButton->setIcon(QIcon::fromTheme("document_open_folder",
                            QIcon(":/icons/hicolor/16x16/actions/document_open_folder.png")));

    this->settings = settings;
    this->allDefaultProfile = nullptr;
    this->connectedDevices = devices;

    fillControllerMappingsTable();

    settings->getLock()->lock();

    bool attachedNumKeypad = settings->value("AttachNumKeypad", false).toBool();
    QString quitComboKeys = settings->value("QuitComboKeys", "").toString();
    QString defaultProfileDir = settings->value("DefaultProfileDir", "").toString();
    int numberRecentProfiles = settings->value("NumberRecentProfiles", 5).toInt();
    bool closeToTray = settings->value("CloseToTray", false).toBool();

    if (!defaultProfileDir.isEmpty() && QDir(defaultProfileDir).exists())
    {
        ui->profileDefaultDirLineEdit->setText(defaultProfileDir);
    }
    else
    {
        ui->profileDefaultDirLineEdit->setText(PadderCommon::preferredProfileDir(settings));
    }

    ui->numberRecentProfileSpinBox->setValue(numberRecentProfiles);

    if (closeToTray)
    {
        ui->closeToTrayCheckBox->setChecked(true);
    }

    changePresetLanguage();


    ui->autoProfileTableWidget->hideColumn(7);


    #if defined(WITH_X11)
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
    populateAutoProfiles();
    fillAllAutoProfilesTable();
    fillGUIDComboBox();
    }
    else
    {
        delete ui->categoriesListWidget->item(2);
        ui->stackedWidget->removeWidget(ui->page_2);
    }
    #elif !defined(WITH_X11)
    delete ui->categoriesListWidget->item(2);
    ui->stackedWidget->removeWidget(ui->page_2);
    #endif


    QString autoProfileActive = settings->value("AutoProfiles/AutoProfilesActive", "").toString();
    if (autoProfileActive == "1")
    {
        ui->activeCheckBox->setChecked(true);
        ui->autoProfileTableWidget->setEnabled(true);
        ui->autoProfileAddPushButton->setEnabled(true);
    }


    ui->launchAtWinStartupCheckBox->setVisible(false);
    ui->keyRepeatGroupBox->setVisible(false);

    bool useSingleProfileList = settings->value("TrayProfileList", false).toBool();
    if (useSingleProfileList)
    {
        ui->traySingleProfileListCheckBox->setChecked(true);
    }

    bool minimizeToTaskBar = settings->value("MinimizeToTaskbar", false).toBool();
    if (minimizeToTaskBar)
    {
        ui->minimizeTaskbarCheckBox->setChecked(true);
    }

    bool hideEmpty = settings->value("HideEmptyButtons", false).toBool();
    if (hideEmpty)
    {
        ui->hideEmptyCheckBox->setChecked(true);
    }

    bool autoOpenLastProfile = settings->value("AutoOpenLastProfile", true).toBool();
    if (autoOpenLastProfile)
    {
        ui->autoLoadPreviousCheckBox->setChecked(true);
    }
    else
    {
        ui->autoLoadPreviousCheckBox->setChecked(false);
    }

    bool launchInTray = settings->value("LaunchInTray", false).toBool();
    if (launchInTray)
    {
        ui->launchInTrayCheckBox->setChecked(true);
    }

    ui->associateProfilesCheckBox->setVisible(false);

    ui->disableWindowsEnhancedPointCheckBox->setVisible(false);

    if (attachedNumKeypad)
        ui->attachNumKeypadCheckbox->setChecked(true);

    initializeKeysList();

    ui->quitCombLineEdit->setText(quitComboKeys);


    bool smoothingEnabled = settings->value("Mouse/Smoothing", false).toBool();
    if (smoothingEnabled)
    {
        ui->smoothingEnableCheckBox->setChecked(true);
        ui->historySizeSpinBox->setEnabled(true);
        ui->weightModifierDoubleSpinBox->setEnabled(true);
    }

    int historySize = settings->value("Mouse/HistorySize", 0).toInt();
    if ((historySize > 0) && (historySize <= GlobalVariables::JoyButton::MAXIMUMMOUSEHISTORYSIZE))
    {
        ui->historySizeSpinBox->setValue(historySize);
    }

    double weightModifier = settings->value("Mouse/WeightModifier", 0).toDouble();
    if ((weightModifier > 0.0) && (weightModifier <= GlobalVariables::JoyButton::MAXIMUMWEIGHTMODIFIER))
    {
        ui->weightModifierDoubleSpinBox->setValue(weightModifier);
    }

    for (int i = 1; i <= GlobalVariables::JoyButton::MAXIMUMMOUSEREFRESHRATE; i++)
    {
        ui->mouseRefreshRateComboBox->addItem(QString("%1 ms").arg(i), i);
    }

    int refreshIndex = ui->mouseRefreshRateComboBox->findData(GlobalVariables::JoyButton::mouseRefreshRate);
    if (refreshIndex >= 0)
    {
        ui->mouseRefreshRateComboBox->setCurrentIndex(refreshIndex);
    }

    fillSpringScreenPresets();

    for (int i=1; i <= 16; i++)
    {
        ui->gamepadPollRateComboBox->addItem(QString("%1 ms").arg(i), QVariant(i));
    }

    int gamepadPollIndex = ui->gamepadPollRateComboBox->findData(GlobalVariables::JoyButton::gamepadRefreshRate);
    if (gamepadPollIndex >= 0)
    {
        ui->gamepadPollRateComboBox->setCurrentIndex(gamepadPollIndex);
    }

    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
        refreshExtraMouseInfo();
    }
    else
    {
        ui->extraInfoFrame->hide();
    }

    // Begin Advanced Tab
    QString curLogFile = settings->value("LogFile", "").toString();
    int logLevel = settings->value("LogLevel", Logger::LOG_NONE).toInt();

    if( !curLogFile.isEmpty() ) {
      ui->logFilePathEdit->setText(curLogFile);
    }

    ui->logLevelComboBox->setCurrentIndex( logLevel );
    // End Advanced Tab

    settings->getLock()->unlock();

    connect(ui->categoriesListWidget, &QListWidget::currentRowChanged, ui->stackedWidget, &QStackedWidget::setCurrentIndex);
    connect(ui->controllerMappingsTableWidget, &QTableWidget::itemChanged, this, &MainSettingsDialog::mappingsTableItemChanged);
    connect(ui->mappingDeletePushButton, &QPushButton::clicked, this, &MainSettingsDialog::deleteMappingRow);
    connect(ui->mappngInsertPushButton, &QPushButton::clicked, this, &MainSettingsDialog::insertMappingRow);
    connect(this, &MainSettingsDialog::accepted, this, &MainSettingsDialog::saveNewSettings);
    connect(ui->profileOpenDirPushButton, &QPushButton::clicked, this, &MainSettingsDialog::selectDefaultProfileDir);
    connect(ui->activeCheckBox, &QCheckBox::toggled, ui->autoProfileTableWidget, &QTableWidget::setEnabled);
    connect(ui->activeCheckBox, &QCheckBox::toggled, this, &MainSettingsDialog::autoProfileButtonsActiveState);
    connect(ui->devicesComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &MainSettingsDialog::changeDeviceForProfileTable);
    connect(ui->keysComboBox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged), this, &MainSettingsDialog::addKeyToQuitCombination);
    connect(ui->autoProfileTableWidget, &QTableWidget::itemChanged, this, &MainSettingsDialog::processAutoProfileActiveClick);
    connect(ui->autoProfileAddPushButton, &QPushButton::clicked, this, &MainSettingsDialog::openAddAutoProfileDialog);
    connect(ui->autoProfileDeletePushButton, &QPushButton::clicked, this, &MainSettingsDialog::openDeleteAutoProfileConfirmDialog);
    connect(ui->autoProfileEditPushButton, &QPushButton::clicked, this, &MainSettingsDialog::openEditAutoProfileDialog);
    connect(ui->autoProfileTableWidget, &QTableWidget::itemSelectionChanged, this, &MainSettingsDialog::changeAutoProfileButtonsState);

    connect(ui->keyRepeatEnableCheckBox, &QCheckBox::clicked, this, &MainSettingsDialog::changeKeyRepeatWidgetsStatus);
    connect(ui->keyDelayHorizontalSlider, &QSlider::valueChanged, ui->keyDelaySpinBox, &QSpinBox::setValue);
    connect(ui->keyDelaySpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), ui->keyDelayHorizontalSlider, &QSlider::setValue);
    connect(ui->keyRateHorizontalSlider, &QSlider::valueChanged, ui->keyRateSpinBox, &QSpinBox::setValue);
    connect(ui->keyRateSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), ui->keyRateHorizontalSlider, &QSlider::setValue);

    connect(ui->smoothingEnableCheckBox, &QCheckBox::toggled, this, &MainSettingsDialog::checkSmoothingWidgetStatus);
    connect(ui->resetAccelPushButton, &QPushButton::clicked, this, &MainSettingsDialog::resetMouseAcceleration);

    // Advanced Tab
    connect(ui->logFilePushButton, &QPushButton::clicked, this, &MainSettingsDialog::selectLogFile);
}

MainSettingsDialog::~MainSettingsDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (connectedDevices != nullptr)
    {
        delete connectedDevices;
        connectedDevices = nullptr;
    }

    delete ui;

}

void MainSettingsDialog::fillControllerMappingsTable()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->controllerMappingsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QHash<QString, QList<QVariant> > tempHash;

    settings->getLock()->lock();
    settings->beginGroup("Mappings");

    QStringList mappings = settings->allKeys();
    QStringListIterator iter(mappings);

    while (iter.hasNext())
    {
        QString tempkey = iter.next();
        QString tempGUID = QString();

        if (tempkey.contains("Disable"))
        {
            bool disableGameController = settings->value(tempkey, false).toBool();
            tempGUID = tempkey.remove("Disable");
            insertTempControllerMapping(tempHash, tempGUID);

            if (tempHash.contains(tempGUID))
            {
                QList<QVariant> templist = tempHash.value(tempGUID);
                templist.replace(2, QVariant(disableGameController));
                tempHash.insert(tempGUID, templist); // Overwrite original list
            }
        }
        else
        {
            QString mappingString = settings->value(tempkey, QString()).toString();
            if (!mappingString.isEmpty())
            {
                tempGUID = tempkey;
                insertTempControllerMapping(tempHash, tempGUID);

                if (tempHash.contains(tempGUID))
                {
                    QList<QVariant> templist = tempHash.value(tempGUID);
                    templist.replace(1, mappingString);
                    tempHash.insert(tempGUID, templist); // Overwrite original list
                }
            }
        }
    }

    settings->endGroup();
    settings->getLock()->unlock();

    QHashIterator<QString, QList<QVariant> > iter2(tempHash);
    int i = 0;
    while (iter2.hasNext())
    {
        ui->controllerMappingsTableWidget->insertRow(i);

        QList<QVariant> templist = iter2.next().value();
        QTableWidgetItem* item = new QTableWidgetItem(templist.at(0).toString());
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setData(Qt::UserRole, iter2.key());
        item->setToolTip(templist.at(0).toString());
        ui->controllerMappingsTableWidget->setItem(i, 0, item);

        item = new QTableWidgetItem(templist.at(1).toString());
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setData(Qt::UserRole, iter2.key());

        ui->controllerMappingsTableWidget->setItem(i, 1, item);

        bool disableController = templist.at(2).toBool();
        item = new QTableWidgetItem();
        item->setCheckState(disableController ? Qt::Checked : Qt::Unchecked);
        item->setData(Qt::UserRole, iter2.key());
        ui->controllerMappingsTableWidget->setItem(i, 2, item);

        i++;
    }
}

void MainSettingsDialog::insertTempControllerMapping(QHash<QString, QList<QVariant> > &hash, QString newGUID)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (!newGUID.isEmpty() && !hash.contains(newGUID))
    {
        QList<QVariant> templist;
        templist.append(QVariant(newGUID));
        templist.append(QVariant(""));
        templist.append(QVariant(false));

        hash.insert(newGUID, templist);
    }
}

void MainSettingsDialog::mappingsTableItemChanged(QTableWidgetItem *item)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int column = item->column();
    int row = item->row();

    if ((column == 0) && !item->text().isEmpty())
    {
        QTableWidgetItem *disableitem = ui->controllerMappingsTableWidget->item(row, column);
        if (disableitem != nullptr)
        {
            disableitem->setData(Qt::UserRole, item->text());
        }

        item->setData(Qt::UserRole, item->text());
    }
}

void MainSettingsDialog::insertMappingRow()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int insertRowIndex = ui->controllerMappingsTableWidget->rowCount();
    ui->controllerMappingsTableWidget->insertRow(insertRowIndex);

    QTableWidgetItem* item = new QTableWidgetItem();

    ui->controllerMappingsTableWidget->setItem(insertRowIndex, 0, item);

    item = new QTableWidgetItem();
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);

    ui->controllerMappingsTableWidget->setItem(insertRowIndex, 1, item);

    item = new QTableWidgetItem();
    item->setCheckState(Qt::Unchecked);
    ui->controllerMappingsTableWidget->setItem(insertRowIndex, 2, item);
}

void MainSettingsDialog::deleteMappingRow()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int row = ui->controllerMappingsTableWidget->currentRow();

    if (row >= 0)
    {
        ui->controllerMappingsTableWidget->removeRow(row);
    }
}

void MainSettingsDialog::syncMappingSettings()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    settings->getLock()->lock();

    settings->beginGroup("Mappings");
    settings->remove("");

    for (int i = 0; i < ui->controllerMappingsTableWidget->rowCount(); i++)
    {
        QTableWidgetItem *itemGUID = ui->controllerMappingsTableWidget->item(i, 0);
        QTableWidgetItem *itemMapping = ui->controllerMappingsTableWidget->item(i, 1);
        QTableWidgetItem *itemDisable = ui->controllerMappingsTableWidget->item(i, 2);

        if ((itemGUID != nullptr) && !itemGUID->text().isEmpty() && (itemDisable != nullptr))
        {
            QString disableController = (itemDisable->checkState() == Qt::Checked) ? "1" : "0";
            if ((itemMapping != nullptr) && !itemMapping->text().isEmpty())
            {
                settings->setValue(itemGUID->text(), itemMapping->text());
            }

            settings->setValue(QString("%1Disable").arg(itemGUID->text()), disableController);
        }
    }

    settings->endGroup();
    settings->getLock()->unlock();
}

void MainSettingsDialog::saveNewSettings()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    syncMappingSettings();

    settings->getLock()->lock();
    QString oldProfileDir = settings->value("DefaultProfileDir", "").toString();
    QString possibleProfileDir = ui->profileDefaultDirLineEdit->text();
    bool closeToTray = ui->closeToTrayCheckBox->isChecked();

    bool attachNumKeypad = ui->attachNumKeypadCheckbox->isChecked();

    settings->setValue("AttachNumKeypad", attachNumKeypad ? "1" : "0");

#if defined(WITH_X11)
    settings->setValue("QuitComboKeys", !ui->quitCombLineEdit->text().isEmpty() ? ui->quitCombLineEdit->text() : "");
#endif

    if (oldProfileDir != possibleProfileDir)
    {
        if (QFileInfo(possibleProfileDir).exists())
        {
            settings->setValue("DefaultProfileDir", possibleProfileDir);
        }
        else if (possibleProfileDir.isEmpty())
        {
            settings->remove("DefaultProfileDir");
        }
    }

    int numRecentProfiles = ui->numberRecentProfileSpinBox->value();
    settings->setValue("NumberRecentProfiles", numRecentProfiles);

    if (closeToTray)
    {
        settings->setValue("CloseToTray", closeToTray ? "1" : "0");
    }
    else
    {
        settings->remove("CloseToTray");
    }
    settings->getLock()->unlock();

    checkLocaleChange();

    #if defined(WITH_X11)

    if (QApplication::platformName() == QStringLiteral("xcb"))
    {

    saveAutoProfileSettings();

    }

    #endif


    settings->getLock()->lock();

    if (ui->traySingleProfileListCheckBox->isChecked())
    {
        settings->setValue("TrayProfileList", "1");
    }
    else
    {
        settings->setValue("TrayProfileList", "0");
    }

    bool minimizeToTaskbar = ui->minimizeTaskbarCheckBox->isChecked();
    settings->setValue("MinimizeToTaskbar", minimizeToTaskbar ? "1" : "0");

    bool hideEmpty = ui->hideEmptyCheckBox->isChecked();
    settings->setValue("HideEmptyButtons", hideEmpty ? "1" : "0");

    bool autoOpenLastProfile = ui->autoLoadPreviousCheckBox->isChecked();
    settings->setValue("AutoOpenLastProfile", autoOpenLastProfile ? "1" : "0");

    bool launchInTray = ui->launchInTrayCheckBox->isChecked();
    settings->setValue("LaunchInTray", launchInTray ? "1" : "0");

    PadderCommon::lockInputDevices();

    if (connectedDevices->size() > 0)
    {
        InputDevice *tempDevice = connectedDevices->at(0);
        QMetaObject::invokeMethod(tempDevice, "haltServices", Qt::BlockingQueuedConnection);
    }


    bool smoothingEnabled = ui->smoothingEnableCheckBox->isChecked();
    int historySize = ui->historySizeSpinBox->value();
    double weightModifier = ui->weightModifierDoubleSpinBox->value();

    settings->setValue("Mouse/Smoothing", smoothingEnabled ? "1" : "0");
    if (smoothingEnabled)
    {
        if (historySize > 0)
        {
            JoyButton::setMouseHistorySize(historySize, GlobalVariables::JoyButton::MAXIMUMMOUSEHISTORYSIZE, GlobalVariables::JoyButton::mouseHistorySize, &GlobalVariables::JoyButton::mouseHistoryX, &GlobalVariables::JoyButton::mouseHistoryY);
        }

        if (weightModifier != 0.0)
        {
            JoyButton::setWeightModifier(weightModifier, GlobalVariables::JoyButton::MAXIMUMWEIGHTMODIFIER, GlobalVariables::JoyButton::weightModifier);
        }
    }
    else
    {
        JoyButton::setMouseHistorySize(1, GlobalVariables::JoyButton::MAXIMUMMOUSEHISTORYSIZE, GlobalVariables::JoyButton::mouseHistorySize, &GlobalVariables::JoyButton::mouseHistoryX, &GlobalVariables::JoyButton::mouseHistoryY);
        JoyButton::setWeightModifier(0.0, GlobalVariables::JoyButton::MAXIMUMWEIGHTMODIFIER, GlobalVariables::JoyButton::weightModifier);
    }

    if (historySize > 0)
    {
        settings->setValue("Mouse/HistorySize", historySize);
    }

    if (weightModifier > 0.0)
    {
        settings->setValue("Mouse/WeightModifier", weightModifier);
    }

    int refreshIndex = ui->mouseRefreshRateComboBox->currentIndex();
    int mouseRefreshRate = ui->mouseRefreshRateComboBox->itemData(refreshIndex).toInt();
    if (mouseRefreshRate != GlobalVariables::JoyButton::mouseRefreshRate)
    {
        settings->setValue("Mouse/RefreshRate", mouseRefreshRate);
        JoyButton::setMouseRefreshRate(mouseRefreshRate, GlobalVariables::JoyButton::mouseRefreshRate, GlobalVariables::JoyButton::IDLEMOUSEREFRESHRATE, JoyButton::getMouseHelper(), &GlobalVariables::JoyButton::mouseHistoryX, &GlobalVariables::JoyButton::mouseHistoryY, JoyButton::getTestOldMouseTime(), JoyButton::getStaticMouseEventTimer());
    }

    int springIndex = ui->springScreenComboBox->currentIndex();
    int springScreen = ui->springScreenComboBox->itemData(springIndex).toInt();
    JoyButton::setSpringModeScreen(springScreen, GlobalVariables::JoyButton::springModeScreen);
    settings->setValue("Mouse/SpringScreen", QString::number(springScreen));

    int pollIndex = ui->gamepadPollRateComboBox->currentIndex();
    int gamepadPollRate = ui->gamepadPollRateComboBox->itemData(pollIndex).toInt();
    if (gamepadPollRate != GlobalVariables::JoyButton::gamepadRefreshRate)
    {
        JoyButton::setGamepadRefreshRate(gamepadPollRate, GlobalVariables::JoyButton::gamepadRefreshRate, JoyButton::getMouseHelper());
        settings->setValue("GamepadPollRate", QString::number(gamepadPollRate));
    }

    // Advanced Tab
    settings->setValue("LogFile", ui->logFilePathEdit->text());
    int logLevel = ui->logLevelComboBox->currentIndex();
    if( logLevel < 0 ) {
      logLevel = 0;
    }
    if( Logger::LOG_MAX < logLevel ) {
      logLevel = Logger::LOG_MAX;
    }
    settings->setValue("LogLevel", logLevel);
    // End Advanced Tab

    PadderCommon::unlockInputDevices();

    settings->sync();
    settings->getLock()->unlock();
}

void MainSettingsDialog::selectDefaultProfileDir()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString lookupDir = PadderCommon::preferredProfileDir(settings);
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Default Profile Directory"), lookupDir);
    if (!directory.isEmpty() && QFileInfo(directory).exists())
    {
        ui->profileDefaultDirLineEdit->setText(directory);
    }
}

void MainSettingsDialog::checkLocaleChange()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    settings->getLock()->lock();
    int row = ui->localeListWidget->currentRow();
    if (row == 0)
    {
        if (settings->contains("Language"))
        {
            settings->remove("Language");
        }

        settings->getLock()->unlock();
        emit changeLanguage(QLocale::system().name());
    }
    else
    {
        QString newLocale = "en";

        switch(row)
        {

            case 1:
            {
                newLocale = "br";
                break;
            }
            case 2:
            {
                newLocale = "en";
                break;
            }
            case 3:
            {
                newLocale = "fr";
                break;
            }
            case 4:
            {
                newLocale = "de";
                break;
            }
            case 5:
            {
                newLocale = "it";
                break;
            }
            case 6:
            {
                newLocale = "ja";
                break;
            }
            case 7:
            {
                newLocale = "ru";
                break;
            }
            case 8:
            {
                newLocale = "sr";
                break;
            }
            case 9:
            {
                newLocale = "zh_CN";
                break;
            }
            case 10:
            {
                newLocale = "es";
                break;
            }
            case 11:
            {
                newLocale = "uk";
                break;
            }
           case 12:
            {
                newLocale = "pl";
                break;
            }

        }

        settings->setValue("Language", newLocale);

        settings->getLock()->unlock();
        emit changeLanguage(newLocale);
    }
}

void MainSettingsDialog::populateAutoProfiles()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    exeAutoProfiles.clear();
    defaultAutoProfiles.clear();

    settings->beginGroup("DefaultAutoProfiles");
    QStringList registeredGUIDs = settings->value("Uniques", QStringList()).toStringList();

    settings->endGroup();

    QString allProfile = settings->value(QString("DefaultAutoProfileAll/Profile"), "all").toString();
    QString allActive = settings->value(QString("DefaultAutoProfileAll/Active"), "0").toString();
    QString partialTitle = settings->value(QString("DefaultAutoProfileAll/PartialTitle"), "").toString();

    bool defaultActive = allActive == "1" ? true : false;
    bool partialTitleBool = partialTitle == "1" ? true : false;
    allDefaultProfile = new AutoProfileInfo("all", allProfile, defaultActive, partialTitleBool, this);
    allDefaultProfile->setDefaultState(true);

    QStringListIterator iter(registeredGUIDs);
    while (iter.hasNext())
    {
        QString tempkey = iter.next();
        QString guid = tempkey;
        QString profile = settings->value(QString("DefaultAutoProfile-%1/Profile").arg(guid), "").toString();
        QString active = settings->value(QString("DefaultAutoProfile-%1/Active").arg(guid), "0").toString();
        QString partialTitle = settings->value(QString("DefaultAutoProfile-%1/PartialTitle").arg(guid), "0").toString();
        bool partialTitleBool = partialTitle == "1" ? true : false;
        QString deviceName = settings->value(QString("DefaultAutoProfile-%1/DeviceName").arg(guid), "").toString();
        QString windowClass = settings->value(QString("DefaultAutoProfile-%1/WindowClass").arg(guid), "").toString();
        QString windowName = settings->value(QString("DefaultAutoProfile-%1/WindowName").arg(guid), "").toString();
        QString exe = settings->value(QString("DefaultAutoProfile-%1/Exe").arg(guid), "").toString();

        if (!guid.isEmpty() && !profile.isEmpty() && !deviceName.isEmpty())
        {
            bool profileActive = active == "1" ? true : false;
            if (!defaultAutoProfiles.contains(guid) && guid != "all")
            {
                AutoProfileInfo *info = new AutoProfileInfo(guid, profile, profileActive, partialTitleBool, this);
                info->setDefaultState(true);
                info->setDeviceName(deviceName);
                info->setExe(exe);
                info->setWindowName(windowName);
                info->setWindowClass(windowClass);
                defaultAutoProfiles.insert(guid, info);
                defaultList.append(info);
                QList<AutoProfileInfo*> templist;
                templist.append(info);
                deviceAutoProfiles.insert(guid, templist);
            }
        }
    }

    settings->beginGroup("AutoProfiles");
    bool quitSearch = false;

    for (int i = 1; !quitSearch; i++)
    {
        QString exe = settings->value(QString("AutoProfile%1Exe").arg(i), "").toString();
        QString windowName = settings->value(QString("AutoProfile%1WindowName").arg(i), "").toString();

        QString windowClass = settings->value(QString("AutoProfile%1WindowClass").arg(i), "").toString();

        convToUniqueIDAutoProfGroupSett(settings, QString("AutoProfile%1GUID").arg(i), QString("AutoProfile%1UniqueID").arg(i));

        QString guid = settings->value(QString("AutoProfile%1UniqueID").arg(i), "").toString();
        QString profile = settings->value(QString("AutoProfile%1Profile").arg(i), "").toString();
        QString active = settings->value(QString("AutoProfile%1Active").arg(i), 0).toString();
        QString partialTitle = settings->value(QString("AutoProfile%1PartialTitle").arg(i), 0).toString();
        bool partialTitleBool = partialTitle == "1" ? true : false;
        QString deviceName = settings->value(QString("AutoProfile%1DeviceName").arg(i), "").toString();

        // Check if all required elements exist. If not, assume that the end of the
        // list has been reached.
        if ((!exe.isEmpty() || !windowClass.isEmpty() || !windowName.isEmpty()) &&
            !guid.isEmpty())
        {
            bool profileActive = active == "1" ? true : false;
            AutoProfileInfo *info = new AutoProfileInfo(guid, profile, exe, profileActive, partialTitleBool, this);
            if (!deviceName.isEmpty())
            {
                info->setDeviceName(deviceName);
            }

            info->setWindowName(windowName);

            info->setWindowClass(windowClass);


            profileList.append(info);
            QList<AutoProfileInfo*> templist;
            if (guid != "all")
            {
                if (deviceAutoProfiles.contains(guid))
                {
                    templist = deviceAutoProfiles.value(guid);
                }
                templist.append(info);
                deviceAutoProfiles.insert(guid, templist);
            }
        }
        else
        {
            quitSearch = true;
        }
    }

    settings->endGroup();
}

void MainSettingsDialog::fillAutoProfilesTable(QString guid)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    for (int i = ui->autoProfileTableWidget->rowCount() - 1; i >= 0; i--)
    {
        ui->autoProfileTableWidget->removeRow(i);
    }

    ui->autoProfileTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    if (defaultAutoProfiles.contains(guid) ||
        deviceAutoProfiles.contains(guid))
    {
        int i = 0;

        AutoProfileInfo *defaultForGUID = nullptr;
        if (defaultAutoProfiles.contains(guid))
        {
            AutoProfileInfo *info = defaultAutoProfiles.value(guid);
            defaultForGUID = info;
            ui->autoProfileTableWidget->insertRow(i);

            QTableWidgetItem *item = new QTableWidgetItem();
            item->setCheckState(info->isActive() ? Qt::Checked : Qt::Unchecked);

            ui->autoProfileTableWidget->setItem(i, 0, item);

            QString deviceName = info->getDeviceName();
            QString guidDisplay = info->getUniqueID();

            if (!deviceName.isEmpty())
            {
                guidDisplay = QString("%1 ").arg(info->getDeviceName());
                guidDisplay.append(QString("(%1)").arg(info->getUniqueID()));
            }

            item = new QTableWidgetItem(guidDisplay);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            item->setData(Qt::UserRole, info->getUniqueID());
            item->setToolTip(info->getUniqueID());
            ui->autoProfileTableWidget->setItem(i, 1, item);

            QFileInfo profilePath(info->getProfileLocation());
            item = new QTableWidgetItem(profilePath.fileName());
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            item->setData(Qt::UserRole, info->getProfileLocation());
            item->setToolTip(info->getProfileLocation());
            ui->autoProfileTableWidget->setItem(i, 2, item);

            item = new QTableWidgetItem(info->getWindowClass());
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            item->setData(Qt::UserRole, info->getWindowClass());
            item->setToolTip(info->getWindowClass());
            ui->autoProfileTableWidget->setItem(i, 3, item);

            item = new QTableWidgetItem(info->getWindowName());
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            item->setData(Qt::UserRole, info->getWindowName());
            item->setToolTip(info->getWindowName());
            ui->autoProfileTableWidget->setItem(i, 4, item);

            QFileInfo exeInfo(info->getExe());
            item = new QTableWidgetItem(exeInfo.fileName());
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            item->setData(Qt::UserRole, info->getExe());
            item->setToolTip(info->getExe());
            ui->autoProfileTableWidget->setItem(i, 5, item);

            item = new QTableWidgetItem("Default");
            item->setData(Qt::UserRole, "default");
            ui->autoProfileTableWidget->setItem(i, 6, item);

            item = new QTableWidgetItem("Instance");
            item->setData(Qt::UserRole, QVariant::fromValue<AutoProfileInfo*>(info));
            ui->autoProfileTableWidget->setItem(i, 7, item);

            i++;
        }

        QListIterator<AutoProfileInfo*> iter(deviceAutoProfiles.value(guid));
        while (iter.hasNext())
        {
            AutoProfileInfo *info = iter.next();
            if (!defaultForGUID || info != defaultForGUID)
            {
                ui->autoProfileTableWidget->insertRow(i);

                QTableWidgetItem *item = new QTableWidgetItem();
                item->setCheckState(info->isActive() ? Qt::Checked : Qt::Unchecked);
                ui->autoProfileTableWidget->setItem(i, 0, item);

                QString deviceName = info->getDeviceName();
                QString guidDisplay = info->getUniqueID();

                if (!deviceName.isEmpty())
                {
                    guidDisplay = QString("%1 ").arg(info->getDeviceName());
                    guidDisplay.append(QString("(%1)").arg(info->getUniqueID()));
                }

                item = new QTableWidgetItem(guidDisplay);
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                item->setData(Qt::UserRole, info->getUniqueID());
                item->setToolTip(info->getUniqueID());
                ui->autoProfileTableWidget->setItem(i, 1, item);

                QFileInfo profilePath(info->getProfileLocation());
                item = new QTableWidgetItem(profilePath.fileName());
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                item->setData(Qt::UserRole, info->getProfileLocation());
                item->setToolTip(info->getProfileLocation());
                ui->autoProfileTableWidget->setItem(i, 2, item);

                // nie są wstawiane wartości jeśli !deviceName.isEmpty(), więc jest inne niż all i jeśli jest default
                item = new QTableWidgetItem(info->getWindowClass());
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                item->setData(Qt::UserRole, info->getWindowClass());
                item->setToolTip(info->getWindowClass());
                ui->autoProfileTableWidget->setItem(i, 3, item);

                // nie są wstawiane wartości jeśli !deviceName.isEmpty(), więc jest inne niż all i jeśli jest default
                item = new QTableWidgetItem(info->getWindowName());
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                item->setData(Qt::UserRole, info->getWindowName());
                item->setToolTip(info->getWindowName());
                ui->autoProfileTableWidget->setItem(i, 4, item);

                // nie są wstawiane wartości jeśli !deviceName.isEmpty(), więc jest inne niż all i jeśli jest default
                QFileInfo exeInfo(info->getExe());
                item = new QTableWidgetItem(exeInfo.fileName());
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                item->setData(Qt::UserRole, info->getExe());
                item->setToolTip(info->getExe());
                ui->autoProfileTableWidget->setItem(i, 5, item);

                item = new QTableWidgetItem("Instance");
                item->setData(Qt::UserRole, QVariant::fromValue<AutoProfileInfo*>(info));
                ui->autoProfileTableWidget->setItem(i, 7, item);

                i++;
            }
        }
    }
}

void MainSettingsDialog::clearAutoProfileData()
{
qInstallMessageHandler(MessageHandler::myMessageOutput);
}

void MainSettingsDialog::fillGUIDComboBox()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->devicesComboBox->clear();
    ui->devicesComboBox->addItem(tr("All"), QVariant("all"));
    QList<QString> guids = deviceAutoProfiles.keys();
    QListIterator<QString> iter(guids);
    while (iter.hasNext())
    {
        QString guid = iter.next();
        QList<AutoProfileInfo*> temp = deviceAutoProfiles.value(guid);

        if (temp.count() > 0)
        {
            QString deviceName = temp.first()->getDeviceName();

            if (!deviceName.isEmpty())
            {
                ui->devicesComboBox->addItem(deviceName, QVariant(guid));
            }
            else
            {
                ui->devicesComboBox->addItem(guid, QVariant(guid));
            }
        }
        else
        {
            ui->devicesComboBox->addItem(guid, QVariant(guid));
        }
    }
}

void MainSettingsDialog::changeDeviceForProfileTable(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    disconnect(ui->autoProfileTableWidget, &QTableWidget::itemChanged, this, &MainSettingsDialog::processAutoProfileActiveClick);

    if (index == 0)
    {
        fillAllAutoProfilesTable();
    }
    else
    {
        QString guid = ui->devicesComboBox->itemData(index).toString();
        fillAutoProfilesTable(guid);
    }

    connect(ui->autoProfileTableWidget, &QTableWidget::itemChanged, this, &MainSettingsDialog::processAutoProfileActiveClick);
}

void MainSettingsDialog::addKeyToQuitCombination(QString key)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->quitCombLineEdit->setText(key);
}

void MainSettingsDialog::saveAutoProfileSettings()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    settings->getLock()->lock();
    settings->beginGroup("DefaultAutoProfiles");
    QStringList defaultkeys = settings->allKeys();
    settings->endGroup();

    QStringListIterator iterDefaults(defaultkeys);
    while (iterDefaults.hasNext())
    {
        QString tempkey = iterDefaults.next();
        QString guid = QString(tempkey).replace("UniqueID", "");

        QString testkey = QString("DefaultAutoProfile-%1").arg(guid);
        settings->beginGroup(testkey);
        settings->remove("");
        settings->endGroup();
    }

    settings->beginGroup("DefaultAutoProfiles");
    settings->remove("");
    settings->endGroup();

    settings->beginGroup("DefaultAutoProfileAll");
    settings->remove("");
    settings->endGroup();

    settings->beginGroup("AutoProfiles");
    settings->remove("");
    settings->endGroup();

    if (allDefaultProfile)
    {
        QString profile = allDefaultProfile->getProfileLocation();
        QString defaultActive = allDefaultProfile->isActive() ? "1" : "0";
        settings->setValue(QString("DefaultAutoProfileAll/Profile"), profile);
        settings->setValue(QString("DefaultAutoProfileAll/Active"), defaultActive);
    }

    QMapIterator<QString, AutoProfileInfo*> iter(defaultAutoProfiles);
    QStringList registeredGUIDs = QStringList();
    while (iter.hasNext())
    {
        iter.next();
        QString guid = iter.key();
        registeredGUIDs.append(guid);
        AutoProfileInfo *info = iter.value();
        QString profileActive = info->isActive() ? "1" : "0";
        QString deviceName = info->getDeviceName();
        settings->setValue(QString("DefaultAutoProfiles/UniqueID%1").arg(guid), guid);
        settings->setValue(QString("DefaultAutoProfile-%1/Profile").arg(guid), info->getProfileLocation());
        settings->setValue(QString("DefaultAutoProfile-%1/Active").arg(guid), profileActive);
        settings->setValue(QString("DefaultAutoProfile-%1/DeviceName").arg(guid), deviceName);
        settings->setValue(QString("DefaultAutoProfile-%1/WindowName").arg(guid), info->getWindowName());
        settings->setValue(QString("DefaultAutoProfile-%1/WindowClass").arg(guid), info->getWindowClass());
        settings->setValue(QString("DefaultAutoProfile-%1/Exe").arg(guid), info->getExe());
        settings->setValue(QString("DefaultAutoProfile-%1/PartialTitle").arg(guid), 0);
    }

    if (!registeredGUIDs.isEmpty())
        settings->setValue("DefaultAutoProfiles/Uniques", registeredGUIDs);

    settings->beginGroup("AutoProfiles");
    QString autoActive = ui->activeCheckBox->isChecked() ? "1" : "0";
    settings->setValue("AutoProfilesActive", autoActive);

    QListIterator<AutoProfileInfo*> iterProfiles(profileList);
    int i = 1;
    while (iterProfiles.hasNext())
    {
        AutoProfileInfo *info = iterProfiles.next();
        QString defaultActive = info->isActive() ? "1" : "0";
        QString partialTitle = info->isPartialState() ? "1" : "0";
        if (!info->getExe().isEmpty())
        {
            settings->setValue(QString("AutoProfile%1Exe").arg(i), info->getExe());
        }

        if (!info->getWindowClass().isEmpty())
        {
            settings->setValue(QString("AutoProfile%1WindowClass").arg(i), info->getWindowClass());
        }

        if (!info->getWindowName().isEmpty())
        {
            settings->setValue(QString("AutoProfile%1WindowName").arg(i), info->getWindowName());
        }

        convToUniqueIDAutoProfGroupSett(settings, QString("AutoProfile%1GUID").arg(i), QString("AutoProfile%1UniqueID").arg(i));

        settings->setValue(QString("AutoProfile%1UniqueID").arg(i), info->getUniqueID());
        settings->setValue(QString("AutoProfile%1Profile").arg(i), info->getProfileLocation());
        settings->setValue(QString("AutoProfile%1Active").arg(i), defaultActive);
        settings->setValue(QString("AutoProfile%1PartialTitle").arg(i), partialTitle);
        settings->setValue(QString("AutoProfile%1DeviceName").arg(i), info->getDeviceName());
        i++;
    }
    settings->endGroup();
    settings->getLock()->unlock();
}

void MainSettingsDialog::fillAllAutoProfilesTable()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    for (int i = ui->autoProfileTableWidget->rowCount()-1; i >= 0; i--)
    {
        ui->autoProfileTableWidget->removeRow(i);
    }

    ui->autoProfileTableWidget->horizontalHeader()->setVisible(true);
    ui->autoProfileTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->autoProfileTableWidget->hideColumn(7);

    int i = 0;

    AutoProfileInfo *info = allDefaultProfile;

    ui->autoProfileTableWidget->insertRow(i);
    QTableWidgetItem *item = new QTableWidgetItem();
    item->setCheckState(info->isActive() ? Qt::Checked : Qt::Unchecked);
    ui->autoProfileTableWidget->setItem(i, 0, item);

    QString deviceName = info->getDeviceName();
    QString guidDisplay = info->getUniqueID();

    if (!deviceName.isEmpty())
    {
        guidDisplay = QString("%1 ").arg(info->getDeviceName());
        guidDisplay.append(QString("(%1)").arg(info->getUniqueID()));
    }

    item = new QTableWidgetItem(guidDisplay);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    item->setData(Qt::UserRole, info->getUniqueID());
    item->setToolTip(info->getUniqueID());
    ui->autoProfileTableWidget->setItem(i, 1, item);

    QFileInfo profilePath(info->getProfileLocation());
    item = new QTableWidgetItem(profilePath.fileName());
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    item->setData(Qt::UserRole, info->getProfileLocation());
    item->setToolTip(info->getProfileLocation());
    ui->autoProfileTableWidget->setItem(i, 2, item);

    QFileInfo exeInfo(info->getExe());
    item = new QTableWidgetItem(exeInfo.fileName());
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    item->setData(Qt::UserRole, info->getExe());
    item->setToolTip(info->getExe());
    ui->autoProfileTableWidget->setItem(i, 5, item);

    item = new QTableWidgetItem("Default");
    item->setData(Qt::UserRole, "default");
    ui->autoProfileTableWidget->setItem(i, 6, item);

    item = new QTableWidgetItem("Instance");
    item->setData(Qt::UserRole, QVariant::fromValue<AutoProfileInfo*>(info));
    ui->autoProfileTableWidget->setItem(i, 7, item);

    i++;

    QListIterator<AutoProfileInfo*> iterDefaults(defaultList);
    while (iterDefaults.hasNext())
    {
        AutoProfileInfo *info = iterDefaults.next();
        ui->autoProfileTableWidget->insertRow(i);

        QTableWidgetItem *item = new QTableWidgetItem();
        item->setCheckState(info->isActive() ? Qt::Checked : Qt::Unchecked);
        ui->autoProfileTableWidget->setItem(i, 0, item);

        QString deviceName = info->getDeviceName();
        QString guidDisplay = info->getUniqueID();

        if (!deviceName.isEmpty())
        {
            guidDisplay = QString("%1 ").arg(info->getDeviceName());
            guidDisplay.append(QString("(%1)").arg(info->getUniqueID()));
        }

        item = new QTableWidgetItem(guidDisplay);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setData(Qt::UserRole, info->getUniqueID());
        item->setToolTip(info->getUniqueID());
        ui->autoProfileTableWidget->setItem(i, 1, item);

        QFileInfo profilePath(info->getProfileLocation());
        item = new QTableWidgetItem(profilePath.fileName());
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setData(Qt::UserRole, info->getProfileLocation());
        item->setToolTip(info->getProfileLocation());
        ui->autoProfileTableWidget->setItem(i, 2, item);

        item = new QTableWidgetItem(info->getWindowClass());
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setData(Qt::UserRole, info->getWindowClass());
        item->setToolTip(info->getWindowClass());
        ui->autoProfileTableWidget->setItem(i, 3, item);

        item = new QTableWidgetItem(info->getWindowName());
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setData(Qt::UserRole, info->getWindowName());
        item->setToolTip(info->getWindowName());
        ui->autoProfileTableWidget->setItem(i, 4, item);

        QFileInfo exeInfo(info->getExe());
        item = new QTableWidgetItem(exeInfo.fileName());
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setData(Qt::UserRole, info->getExe());
        item->setToolTip(info->getExe());
        ui->autoProfileTableWidget->setItem(i, 5, item);

        item = new QTableWidgetItem("Default");
        item->setData(Qt::UserRole, "default");
        ui->autoProfileTableWidget->setItem(i, 6, item);

        item = new QTableWidgetItem("Instance");
        item->setData(Qt::UserRole, QVariant::fromValue<AutoProfileInfo*>(info));
        ui->autoProfileTableWidget->setItem(i, 7, item);

        i++;
    }

    QListIterator<AutoProfileInfo*> iter(profileList);
    while (iter.hasNext())
    {
        AutoProfileInfo *info = iter.next();
        ui->autoProfileTableWidget->insertRow(i);

        QTableWidgetItem *item = new QTableWidgetItem();
        item->setCheckState(info->isActive() ? Qt::Checked : Qt::Unchecked);
        ui->autoProfileTableWidget->setItem(i, 0, item);

        QString deviceName = info->getDeviceName();
        QString guidDisplay = info->getUniqueID();

        if (!deviceName.isEmpty())
        {
            guidDisplay = QString("%1 ").arg(info->getDeviceName());
            guidDisplay.append(QString("(%1)").arg(info->getUniqueID()));
        }

        item = new QTableWidgetItem(guidDisplay);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setData(Qt::UserRole, info->getUniqueID());
        item->setToolTip(info->getUniqueID());
        ui->autoProfileTableWidget->setItem(i, 1, item);

        QFileInfo profilePath(info->getProfileLocation());
        item = new QTableWidgetItem(profilePath.fileName());
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setData(Qt::UserRole, info->getProfileLocation());
        item->setToolTip(info->getProfileLocation());
        ui->autoProfileTableWidget->setItem(i, 2, item);

        item = new QTableWidgetItem(info->getWindowClass());
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setData(Qt::UserRole, info->getWindowClass());
        item->setToolTip(info->getWindowClass());
        ui->autoProfileTableWidget->setItem(i, 3, item);

        item = new QTableWidgetItem(info->getWindowName());
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setData(Qt::UserRole, info->getWindowName());
        item->setToolTip(info->getWindowName());
        ui->autoProfileTableWidget->setItem(i, 4, item);

        QFileInfo exeInfo(info->getExe());
        item = new QTableWidgetItem(exeInfo.fileName());
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setData(Qt::UserRole, info->getExe());
        item->setToolTip(info->getExe());
        ui->autoProfileTableWidget->setItem(i, 5, item);

        item = new QTableWidgetItem();
        item->setData(Qt::UserRole, "");
        ui->autoProfileTableWidget->setItem(i, 6, item);

        item = new QTableWidgetItem("Instance");
        item->setData(Qt::UserRole, QVariant::fromValue<AutoProfileInfo*>(info));
        ui->autoProfileTableWidget->setItem(i, 7, item);

        i++;
    }
}

void MainSettingsDialog::processAutoProfileActiveClick(QTableWidgetItem *item)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (item && (item->column() == 0))
    {
        QTableWidgetItem *infoitem = ui->autoProfileTableWidget->item(item->row(), 7);
        AutoProfileInfo *info = infoitem->data(Qt::UserRole).value<AutoProfileInfo*>();
        Qt::CheckState active = item->checkState();
        if (active == Qt::Unchecked)
        {
            info->setActive(false);
        }
        else if (active == Qt::Checked)
        {
            info->setActive(true);
        }
    }
}

void MainSettingsDialog::openAddAutoProfileDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QList<QString> reservedGUIDs = defaultAutoProfiles.keys();
    AutoProfileInfo *info = new AutoProfileInfo(this);
    AddEditAutoProfileDialog *dialog = new AddEditAutoProfileDialog(info, settings, connectedDevices, reservedGUIDs, false, this);

    connect(dialog, &AddEditAutoProfileDialog::accepted, this, [this, dialog]
    {
        addNewAutoProfile(dialog);
    });

    connect(dialog, &AddEditAutoProfileDialog::rejected, info, &AutoProfileInfo::deleteLater);
    dialog->show();
}

void MainSettingsDialog::openEditAutoProfileDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int selectedRow = ui->autoProfileTableWidget->currentRow();
    if (selectedRow >= 0)
    {
        QTableWidgetItem *item = ui->autoProfileTableWidget->item(selectedRow, 7);

        AutoProfileInfo *info = item->data(Qt::UserRole).value<AutoProfileInfo*>();
        if (info != allDefaultProfile)
        {
            QList<QString> reservedGUIDs = defaultAutoProfiles.keys();

            if (info->getUniqueID() != "all")
            {
                AutoProfileInfo *temp = defaultAutoProfiles.value(info->getUniqueID());

                if (info == temp)
                {
                    reservedGUIDs.removeAll(info->getUniqueID());
                }
            }

            QTableWidgetItem *itemDefault = ui->autoProfileTableWidget->item(selectedRow, 6);
            if (itemDefault->data(Qt::UserRole) == "default" || itemDefault->text() == "Default") info->setDefaultState(true);


            AddEditAutoProfileDialog *dialog = new AddEditAutoProfileDialog(info, settings, connectedDevices, reservedGUIDs, true, this);
            connect(dialog, &AddEditAutoProfileDialog::accepted, this, [this, dialog] {
                transferEditsToCurrentTableRow(dialog);
            });

            dialog->show();
        }
        else
        {
            info->setDefaultState(true);
            EditAllDefaultAutoProfileDialog *dialog = new EditAllDefaultAutoProfileDialog(info, settings, this);
            dialog->show();
            connect(dialog, &EditAllDefaultAutoProfileDialog::accepted, this, [this, dialog] {
                transferAllProfileEditToCurrentTableRow(dialog);
            });
        }
    }
}

void MainSettingsDialog::openDeleteAutoProfileConfirmDialog()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QMessageBox msgBox;
    msgBox.setText(tr("Are you sure you want to delete the profile?"));
    msgBox.setStandardButtons(QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();

    if (ret == QMessageBox::Discard)
    {
        int selectedRow = ui->autoProfileTableWidget->currentRow();

        if (selectedRow >= 0)
        {
            QTableWidgetItem *item = ui->autoProfileTableWidget->item(selectedRow, 7);
            AutoProfileInfo *info = item->data(Qt::UserRole).value<AutoProfileInfo*>();

            if (info->isCurrentDefault())
            {
                if (info->getUniqueID() == "all")
                {
                    delete allDefaultProfile;
                    allDefaultProfile = nullptr;
                }
                else if (defaultAutoProfiles.contains(info->getUniqueID()))
                {
                    defaultAutoProfiles.remove(info->getUniqueID());
                    defaultList.removeAll(info);
                    delete info;
                    info = nullptr;
                }
            }
            else
            {
                if (deviceAutoProfiles.contains(info->getUniqueID()))
                {
                    QList<AutoProfileInfo*> temp = deviceAutoProfiles.value(info->getUniqueID());
                    temp.removeAll(info);
                    deviceAutoProfiles.insert(info->getUniqueID(), temp);
                }

                profileList.removeAll(info);

                delete info;
                info = nullptr;
            }
        }

        ui->autoProfileTableWidget->removeRow(selectedRow);
    }
}

void MainSettingsDialog::changeAutoProfileButtonsState()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int selectedRow = ui->autoProfileTableWidget->currentRow();

    if (selectedRow >= 0)
    {
        QTableWidgetItem *item = ui->autoProfileTableWidget->item(selectedRow, 7);

        AutoProfileInfo *info = item->data(Qt::UserRole).value<AutoProfileInfo*>();

        if (info == allDefaultProfile)
        {
            ui->autoProfileAddPushButton->setEnabled(true);
            ui->autoProfileEditPushButton->setEnabled(true);
            ui->autoProfileDeletePushButton->setEnabled(false);
        }
        else
        {
            ui->autoProfileAddPushButton->setEnabled(true);
            ui->autoProfileEditPushButton->setEnabled(true);
            ui->autoProfileDeletePushButton->setEnabled(true);
        }
    }
    else
    {
        ui->autoProfileAddPushButton->setEnabled(true);
        ui->autoProfileDeletePushButton->setEnabled(false);
        ui->autoProfileEditPushButton->setEnabled(false);
    }
}

void MainSettingsDialog::transferAllProfileEditToCurrentTableRow(EditAllDefaultAutoProfileDialog* dialog)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    AutoProfileInfo *info = dialog->getAutoProfile();
    allDefaultProfile = info;
    changeDeviceForProfileTable(0);
}

void MainSettingsDialog::transferEditsToCurrentTableRow(AddEditAutoProfileDialog *dialog)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    AutoProfileInfo *info = dialog->getAutoProfile();

    // Delete pointers to object that might be misplaced
    // due to an association change.
    QString oldGUID = dialog->getOriginalUniqueID();

    if (oldGUID != info->getUniqueID())
    {
        if (defaultAutoProfiles.value(oldGUID) == info)
        {
            defaultAutoProfiles.remove(oldGUID);
        }

        if (info->isCurrentDefault())
        {
            defaultAutoProfiles.insert(info->getUniqueID(), info);
        }
    }

    if ((oldGUID != info->getUniqueID()) && deviceAutoProfiles.contains(oldGUID))
    {
        QList<AutoProfileInfo*> temp = deviceAutoProfiles.value(oldGUID);
        temp.removeAll(info);

        if (temp.count() > 0)
        {
            deviceAutoProfiles.insert(oldGUID, temp);
        }
        else
        {
            deviceAutoProfiles.remove(oldGUID);
        }

        if (deviceAutoProfiles.contains(info->getUniqueID()))
        {
            QList<AutoProfileInfo*> temp2 = deviceAutoProfiles.value(oldGUID);

            if (!temp2.contains(info))
            {
                temp2.append(info);
                deviceAutoProfiles.insert(info->getUniqueID(), temp2);
            }
        }
        else if (info->getUniqueID().toLower() != "all")
        {
            QList<AutoProfileInfo*> temp2;
            temp2.append(info);
            deviceAutoProfiles.insert(info->getUniqueID(), temp2);
        }
    }
    else if ((oldGUID != info->getUniqueID()) && (info->getUniqueID().toLower() != "all"))
    {
        QList<AutoProfileInfo*> temp;
        temp.append(info);
        deviceAutoProfiles.insert(info->getUniqueID(), temp);
    }

    if (!info->isCurrentDefault())
    {
        defaultList.removeAll(info);

        if (!profileList.contains(info))
        {
            profileList.append(info);
        }
    }
    else
    {
        profileList.removeAll(info);

        if (!defaultList.contains(info))
        {
            defaultList.append(info);
        }
    }

    if (deviceAutoProfiles.contains(info->getUniqueID()))
    {
        QList<AutoProfileInfo*> temp2 = deviceAutoProfiles.value(info->getUniqueID());

        if (!temp2.contains(info))
        {
            temp2.append(info);
            deviceAutoProfiles.insert(info->getUniqueID(), temp2);
        }
    }
    else
    {
        QList<AutoProfileInfo*> temp2;
        temp2.append(info);
        deviceAutoProfiles.insert(info->getUniqueID(), temp2);
    }

    fillGUIDComboBox();
    int currentIndex = ui->devicesComboBox->currentIndex();
    changeDeviceForProfileTable(currentIndex);
}

void MainSettingsDialog::addNewAutoProfile(AddEditAutoProfileDialog *dialog)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

   // AddEditAutoProfileDialog *dialog = static_cast<AddEditAutoProfileDialog*>(sender());
    AutoProfileInfo *info = dialog->getAutoProfile();
    bool found = false;

    if (info->isCurrentDefault() && defaultAutoProfiles.contains(info->getUniqueID()))
    {
        found = true;
    }

    if (!found)
    {
        if (info->isCurrentDefault())
        {
            if (!info->getUniqueID().isEmpty() && !info->getExe().isEmpty())
            {
                defaultAutoProfiles.insert(info->getUniqueID(), info);
                defaultList.append(info);
            }
        }
        else
        {
            if (!info->getUniqueID().isEmpty() &&
                !info->getExe().isEmpty())
            {
                profileList.append(info);

                if (info->getUniqueID() != "all")
                {
                    QList<AutoProfileInfo*> tempDevProfileList;

                    if (deviceAutoProfiles.contains(info->getUniqueID()))
                        tempDevProfileList = deviceAutoProfiles.value(info->getUniqueID());

                    tempDevProfileList.append(info);
                    deviceAutoProfiles.insert(info->getUniqueID(), tempDevProfileList);
                }
            }
        }

        fillGUIDComboBox();
        changeDeviceForProfileTable(ui->devicesComboBox->currentIndex());
    }
}

void MainSettingsDialog::autoProfileButtonsActiveState(bool enabled)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (enabled)
    {
        changeAutoProfileButtonsState();
    }
    else
    {
        ui->autoProfileAddPushButton->setEnabled(false);
        ui->autoProfileEditPushButton->setEnabled(false);
        ui->autoProfileDeletePushButton->setEnabled(false);
    }
}

void MainSettingsDialog::changeKeyRepeatWidgetsStatus(bool enabled)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->keyDelayHorizontalSlider->setEnabled(enabled);
    ui->keyDelaySpinBox->setEnabled(enabled);
    ui->keyRateHorizontalSlider->setEnabled(enabled);
    ui->keyRateSpinBox->setEnabled(enabled);
}

void MainSettingsDialog::checkSmoothingWidgetStatus(bool enabled)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (enabled)
    {
        ui->historySizeSpinBox->setEnabled(true);
        ui->weightModifierDoubleSpinBox->setEnabled(true);
    }
    else
    {
        ui->historySizeSpinBox->setEnabled(false);
        ui->weightModifierDoubleSpinBox->setEnabled(false);
    }
}


void MainSettingsDialog::changePresetLanguage()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (settings->contains("Language"))
    {
        QString targetLang = settings->value("Language").toString();
        if (targetLang == "br")
        {
            ui->localeListWidget->setCurrentRow(1);
        }
        else if (targetLang == "en")
        {
            ui->localeListWidget->setCurrentRow(2);
        }
        else if (targetLang == "fr")
        {
            ui->localeListWidget->setCurrentRow(3);
        }
        else if (targetLang == "de")
        {
            ui->localeListWidget->setCurrentRow(4);
        }
        else if (targetLang == "it")
        {
            ui->localeListWidget->setCurrentRow(5);
        }
        else if (targetLang == "ja")
        {
            ui->localeListWidget->setCurrentRow(6);
        }
        else if (targetLang == "ru")
        {
            ui->localeListWidget->setCurrentRow(7);
        }
        else if (targetLang == "sr")
        {
            ui->localeListWidget->setCurrentRow(8);
        }
        else if (targetLang == "zh_CN")
        {
            ui->localeListWidget->setCurrentRow(9);
        }
        else if (targetLang == "es")
        {
            ui->localeListWidget->setCurrentRow(10);
        }
        else if (targetLang == "uk")
        {
            ui->localeListWidget->setCurrentRow(11);
        }
        else if (targetLang == "pl")
        {
            ui->localeListWidget->setCurrentRow(12);
        }
        else
        {
            ui->localeListWidget->setCurrentRow(0);
        }
    }
    else
    {
        ui->localeListWidget->setCurrentRow(0);
    }
}

void MainSettingsDialog::fillSpringScreenPresets()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    ui->springScreenComboBox->clear();
    ui->springScreenComboBox->addItem(tr("Default"),
                                      QVariant(GlobalVariables::AntimicroSettings::defaultSpringScreen));

    QDesktopWidget deskWid;

    for (int i = 0; i < deskWid.screenCount(); i++)
    {
        ui->springScreenComboBox->addItem(QString(":%1").arg(i), QVariant(i));
    }

    int screenIndex = ui->springScreenComboBox->findData(GlobalVariables::JoyButton::springModeScreen);

    if (screenIndex > -1)
    {
        ui->springScreenComboBox->setCurrentIndex(screenIndex);
    }
}

void MainSettingsDialog::refreshExtraMouseInfo()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

#if defined(WITH_X11)
    QString handler = EventHandlerFactory::getInstance()->handler()->getIdentifier();

    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
        struct X11Extras::ptrInformation temp;
        if (handler == "uinput")
        {
            temp = X11Extras::getInstance()->getPointInformation();
        }
        else if (handler == "xtest")
        {
            temp = X11Extras::getInstance()->getPointInformation(GlobalVariables::X11Extras::xtestMouseDeviceName);
        }

        if (temp.id >= 0)
        {
            ui->accelNumLabel->setText(QString::number(temp.accelNum));
            ui->accelDenomLabel->setText(QString::number(temp.accelDenom));
            ui->accelThresLabel->setText(QString::number(temp.threshold));
        }
    }

#endif
}

void MainSettingsDialog::resetMouseAcceleration()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

#if defined(WITH_X11)
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
        X11Extras::getInstance()->x11ResetMouseAccelerationChange();
        refreshExtraMouseInfo();
    }
#endif
}

void MainSettingsDialog::selectLogFile() {

    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString oldLogFile = settings->value("LogFile", "").toString();
    QString newLogFile = QFileDialog::getSaveFileName(this, tr("Save Log File As"), oldLogFile, tr("Log Files (*.log)"));

    if (!newLogFile.isEmpty()) ui->logFilePathEdit->setText(newLogFile);

}

AntiMicroSettings *MainSettingsDialog::getSettings() const {

    return settings;
}

QMap<QString, AutoProfileInfo*>* MainSettingsDialog::getDefaultAutoProfiles() {

    return &defaultAutoProfiles;
}

QMap<QString, QList<AutoProfileInfo*> >* MainSettingsDialog::getDeviceAutoProfiles() {

    return &deviceAutoProfiles;
}

QMap<QString, QList<AutoProfileInfo*> >* MainSettingsDialog::getExeAutoProfiles() {

    return &exeAutoProfiles;
}

QList<AutoProfileInfo*>* MainSettingsDialog::getDefaultList() {

    return &defaultList;
}

QList<AutoProfileInfo*>* MainSettingsDialog::getProfileList() {

    return &profileList;
}

AutoProfileInfo* MainSettingsDialog::getAllDefaultProfile() const {

    return allDefaultProfile;
}

QList<InputDevice*>* MainSettingsDialog::getConnectedDevices() const {

    return connectedDevices;
}

void MainSettingsDialog::on_resetBtn_clicked()
{
    QMessageBox msgBox;
    msgBox.setText(tr("Do you really want to reset setting?"));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

    switch(msgBox.exec())
    {
        case QMessageBox::Ok:
            resetGeneralSett();
            resetAutoProfSett();
            resetMouseSett();
            resetAdvancedSett();
            ui->localeListWidget->setCurrentRow(0);

        break;

        case QMessageBox::Cancel:

        break;

        default:

        break;
    }

    QMessageBox msgBox2;
    msgBox2.setText(tr("Would you like to reset mappings too?"));
    msgBox2.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

    switch(msgBox2.exec())
    {
        case QMessageBox::Ok:

            ui->controllerMappingsTableWidget->setRowCount(0);

        break;

        case QMessageBox::Cancel:

        break;

        default:

        break;
    }
}


void MainSettingsDialog::resetGeneralSett()
{
    ui->profileDefaultDirLineEdit->setText(PadderCommon::preferredProfileDir(settings));
    ui->numberRecentProfileSpinBox->setValue(5);
    int gamepadPollIndex = ui->gamepadPollRateComboBox->findData(GlobalVariables::JoyButton::gamepadRefreshRate);

    if (gamepadPollIndex >= 0)
    {
        ui->gamepadPollRateComboBox->setCurrentIndex(gamepadPollIndex);
    }

    ui->closeToTrayCheckBox->setChecked(false);
    ui->attachNumKeypadCheckbox->setChecked(false);
    ui->launchAtWinStartupCheckBox->setChecked(false);
    ui->traySingleProfileListCheckBox->setChecked(false);
    ui->minimizeTaskbarCheckBox->setChecked(false);
    ui->hideEmptyCheckBox->setChecked(false);
    ui->autoLoadPreviousCheckBox->setChecked(true);
    ui->launchInTrayCheckBox->setChecked(false);
    ui->associateProfilesCheckBox->setChecked(true);
    ui->keyRepeatEnableCheckBox->setChecked(false);

    ui->keyDelayHorizontalSlider->setValue(660);
    ui->keyRateHorizontalSlider->setValue(25);
}


void MainSettingsDialog::resetAutoProfSett()
{

    disconnect(ui->autoProfileTableWidget, &QTableWidget::itemChanged, this, &MainSettingsDialog::processAutoProfileActiveClick);

    ui->activeCheckBox->setChecked(false);
    ui->devicesComboBox->setCurrentIndex(0);

    ui->autoProfileTableWidget->setRowCount(0);

    AutoProfileInfo *info = allDefaultProfile;

    ui->autoProfileTableWidget->insertRow(0);
    QTableWidgetItem *item = new QTableWidgetItem();
    item->setCheckState(info->isActive() ? Qt::Checked : Qt::Unchecked);
    ui->autoProfileTableWidget->setItem(0, 0, item);

    QString deviceName = info->getDeviceName();
    QString guidDisplay = info->getUniqueID();

    if (!deviceName.isEmpty())
    {
        guidDisplay = QString("%1 ").arg(info->getDeviceName());
        guidDisplay.append(QString("(%1)").arg(info->getUniqueID()));
    }

    item = new QTableWidgetItem(guidDisplay);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    item->setData(Qt::UserRole, info->getUniqueID());
    item->setToolTip(info->getUniqueID());
    ui->autoProfileTableWidget->setItem(0, 1, item);

    item = new QTableWidgetItem("Default");
    item->setData(Qt::UserRole, "default");
    ui->autoProfileTableWidget->setItem(0, 6, item);

    item = new QTableWidgetItem("Instance");
    item->setData(Qt::UserRole, QVariant::fromValue<AutoProfileInfo*>(info));
    ui->autoProfileTableWidget->setItem(0, 7, item);

    if (ui->autoProfileTableWidget->rowCount() == 1) profileList.clear();

    connect(ui->autoProfileTableWidget, &QTableWidget::itemChanged, this, &MainSettingsDialog::processAutoProfileActiveClick);
}


void MainSettingsDialog::resetMouseSett()
{
    ui->disableWindowsEnhancedPointCheckBox->setChecked(false);
    ui->smoothingEnableCheckBox->setChecked(false);
    ui->smoothingEnableCheckBox->setEnabled(true);
    ui->historySizeSpinBox->setValue(10);
    ui->historySizeSpinBox->setEnabled(false);
    ui->weightModifierDoubleSpinBox->setValue(0.20);
    ui->weightModifierDoubleSpinBox->setEnabled(false);

#if defined(WITH_X11)
    ui->quitCombLineEdit->clear();
#endif

    int refreshIndex = ui->mouseRefreshRateComboBox->findData(GlobalVariables::JoyButton::mouseRefreshRate);

    if (refreshIndex >= 0)
    {
        ui->mouseRefreshRateComboBox->setCurrentIndex(refreshIndex);
    }

    int screenIndex = ui->springScreenComboBox->findData(GlobalVariables::JoyButton::springModeScreen);

    if (screenIndex > -1)
    {
        ui->springScreenComboBox->setCurrentIndex(screenIndex);
    }
}


void MainSettingsDialog::resetAdvancedSett()
{
    ui->logFilePathEdit->setText("");
    ui->logLevelComboBox->setCurrentIndex(0);
}

void MainSettingsDialog::initializeKeysList()
{

    QStringList keySysNames = QStringList();

    // standard shortcuts for Linux
    keySysNames.append("F1");
    keySysNames.append("Shift+F1");
    keySysNames.append("Ctrl+O");
    keySysNames.append("Ctrl+W");
    keySysNames.append("Ctrl+S");
    keySysNames.append("Ctrl+Q");
    keySysNames.append("Ctrl+N");
    keySysNames.append("Del");
    keySysNames.append("Ctrl+D");
    keySysNames.append("Ctrl+X");
    keySysNames.append("Shift+Del");
    keySysNames.append("Ctrl+C");
    keySysNames.append("Ctrl+Ins");
    keySysNames.append("Ctrl+V");
    keySysNames.append("Shift+Ins");
    keySysNames.append("Ctrl+Z");
    keySysNames.append("Ctrl+Shift+Z");
    keySysNames.append("Alt+Left");
    keySysNames.append("Alt+Right");
    keySysNames.append("F5");
    keySysNames.append("Ctrl+Plus");
    keySysNames.append("Ctrl+Minus");
    keySysNames.append("Ctrl+Shift+F");
    keySysNames.append("Ctrl+P");
    keySysNames.append("Ctrl+T");
    keySysNames.append("Ctrl+Tab");
    keySysNames.append("Forward");
    keySysNames.append("Ctrl+Shift+Tab");
    keySysNames.append("Back");
    keySysNames.append("Ctrl+F");
    keySysNames.append("F3");
    keySysNames.append("Shift+F3");
    keySysNames.append("Ctrl+A");
    keySysNames.append("Ctrl+Shift+A");
    keySysNames.append("Ctrl+B");
    keySysNames.append("Ctrl+I");
    keySysNames.append("Ctrl+U");
    keySysNames.append("Right");
    keySysNames.append("Left");
    keySysNames.append("Ctrl+Right");
    keySysNames.append("Ctrl+Left");
    keySysNames.append("Down");
    keySysNames.append("Up");
    keySysNames.append("PgDown");
    keySysNames.append("PgUp");
    keySysNames.append("Home");
    keySysNames.append("End");
    keySysNames.append("Ctrl+E");
    keySysNames.append("Ctrl+Home");
    keySysNames.append("Ctrl+End");
    keySysNames.append("Shift+Right");
    keySysNames.append("Shift+Left");
    keySysNames.append("Ctrl+Shift+Right");
    keySysNames.append("Ctrl+Shift+Left");
    keySysNames.append("Shift+Down");
    keySysNames.append("Shift+Up");
    keySysNames.append("Shift+PgDown");
    keySysNames.append("Shift+PgUp");
    keySysNames.append("Shift+Home");
    keySysNames.append("Shift+End");
    keySysNames.append("Ctrl+Shift+Home");
    keySysNames.append("Ctrl+Shift+End");
    keySysNames.append("Ctrl+Backspace");
    keySysNames.append("Ctrl+Del");
    keySysNames.append("Ctrl+K");
    keySysNames.append("Ctrl+U");
    keySysNames.append("Enter");
    keySysNames.append("Shift+Enter");
    keySysNames.append("Escape");

    if (keysymToKeyString(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_F20)) != tr("[NO KEY]"))
        keySysNames.append("F20");

    if (keysymToKeyString(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_F16)) != tr("[NO KEY]"))
        keySysNames.append("F16");

    if (keysymToKeyString(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_F18)) != tr("[NO KEY]"))
        keySysNames.append("F18");

    if (keysymToKeyString(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_F18)) != tr("[NO KEY]"))
        keySysNames.append("F14");

    if (ui->keysComboBox->count() > 0)
        ui->keysComboBox->clear();

    ui->keysComboBox->addItems(keySysNames);

    update();
}


void MainSettingsDialog::convToUniqueIDAutoProfGroupSett(QSettings* sett, QString guidAutoProfSett, QString uniqueAutoProfSett)
{
    if (sett->contains(guidAutoProfSett))
    {
        sett->setValue(uniqueAutoProfSett, sett->value(guidAutoProfSett));
        sett->remove(guidAutoProfSett);
    }
}
