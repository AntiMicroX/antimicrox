#include "profileimporter.h"
#include "ui_profileimporter.h"

#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;



ProfileImporter::ProfileImporter(AntiMicroXSettings* settings, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProfileImporter)
{
    ui->setupUi(this);

    this->settings = settings;

    rewriteButtonGroup();
}


ProfileImporter::~ProfileImporter()
{
    delete ui;
}


// Author: Lex Fridman
// http://lexfridman.com/convert-string-to-qt-keycode-in-c/
int ProfileImporter::convertStringToQtKey(QString const & keyString)
{
    QKeySequence seq(keyString);
    int keyCode;

    // We should only working with a single key here
    if(seq.count() == 1)
        keyCode = seq[0];
    else {
        // Should be here only if a modifier key (e.g. Ctrl, Alt) is pressed.
        assert(seq.count() == 0);

        // Add a non-modifier key "A" to the picture because QKeySequence
        // seems to need that to acknowledge the modifier. We know that A has
        // a keyCode of 65 (or 0x41 in hex)
        seq = QKeySequence(keyString + "+A");
        assert(seq.count() == 1);
        assert(seq[0] > 65);
        keyCode = seq[0] - 65;
    }

    return keyCode;
}


// http://programmingknowledgeblog.blogspot.com/2013/05/c-program-to-convert-hex-to-ascii-string.html
QChar ProfileImporter::convertHexToString_QChar(QString const & hexValue)
{
   std::istringstream iss (hexValue.toStdString());
   iss.flags(std::ios::hex);
   int i;
   iss >> i;

   return QChar(i);
}


char ProfileImporter::convertHexToString_char(string const & hexValue)
{
   std::istringstream iss (hexValue);
   iss.flags(std::ios::hex);
   int i;
   iss >> i;

   return static_cast<char>(i);
}


void ProfileImporter::putGamecontrMapping()
{
/*  QString mappingString = generateSDLMappingString();

    settings->getLock()->lock();

    settings->setValue(QString("Mappings/").append(device->getGUIDString()), mappingString);
    settings->setValue(QString("Mappings/%1%2").arg(device->getGUIDString()).arg("Disable"), "0");
    settings->sync();

    bool displayMapping = settings->runtimeValue("DisplaySDLMapping", false).toBool();
    settings->getLock()->unlock();

    if (displayMapping)
    {
        QTextStream out(stdout);
        out << generateSDLMappingString();
    }

    emit mappingUpdate(mappingString, device); */
}


void ProfileImporter::putSettingsToApp()
{
    // general data - GUID and full name (if ui->importSameCheckbox was checked before)
    // complex data - app settings as additional settings (if ui->fullSettCheckBox was checked before)
    // data for gamecontroller mapping
    // data for mapping from keyboard/mouse to gamecontroller
}


/*
   // rough draft

    QStringList templist = QStringList();
    templist.append(device->getGUIDString());
    templist.append(device->getSDLName());
    templist.append(QString("platform:").append(device->getSDLPlatform()));

    for (int i=0; i < ui->buttonMappingTableWidget->rowCount(); i++)
    {
        QTableWidgetItem *item = ui->buttonMappingTableWidget->item(i, 0);
        if (item != nullptr)
        {
            QString mapNative = QString();
            QList<QVariant> tempassociation = item->data(Qt::UserRole).toList();
            if (tempassociation.size() == 2)
            {
                int bindingType = tempassociation.value(0).toInt();
                if (bindingType == 0)
                {
                    mapNative.append("b");
                    mapNative.append(QString::number(tempassociation.value(1).toInt()));
                }
                else if (bindingType > 0)
                {
                    mapNative.append("a");
                    mapNative.append(QString::number(tempassociation.value(0).toInt()-1));
                }
                else if (bindingType < 0)
                {
                    mapNative.append("h");
                    mapNative.append(QString::number(tempassociation.value(0).toInt()+1));
                    mapNative.append(".").append(QString::number(tempassociation.value(1).toInt()));
                }
            }

            if (!mapNative.isEmpty())
            {
                QString sdlButtonName = tempaliases.value(i);
                QString temp = QString("%1:%2").arg(sdlButtonName).arg(mapNative);
                templist.append(temp);
            }
        }
    }

    return templist.join(",").append(",");

    */


bool ProfileImporter::allFilled()
{
    if ((!ui->profileLineEdit->text().isEmpty()) && (radioBtnProfiles.checkedButton() != nullptr))
    {
        if (ui->fullSettCheckBox->isChecked())
        {
            if (ui->configLineEdit->text().isEmpty()) return false;
            else return true;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return false;
    }
}


void ProfileImporter::changeExtensionFile(QString filePath)
{
    QFileInfo info(filePath);
    if (properExtension(filePath))
    {
        if ((radioBtnProfiles.checkedButton()->text().remove('&') == "JoyToKey"))
        {
            QString strNewName = info.path() + "/" + info.completeBaseName() + ".ini";
            QFile renamed(filePath);
            renamed.rename(filePath, strNewName);
        }
    }
    else
    {
        QMessageBox box;
        box.setText(trUtf8("Extension of file is incorrect. Choose one type of profile and define full path of file."));
        box.setWindowTitle(trUtf8("Incorrect extension"));
        box.setStandardButtons(QMessageBox::Close);
        box.raise();
        box.exec();
    }
}


void ProfileImporter::backExtensionFile(QString filePath)
{
    QFileInfo info(filePath);

    QString strNewName = info.path() + "/" + info.completeBaseName() + "." + extensionProfile();
    QFile renamed(filePath);
    renamed.rename(filePath, strNewName);
}


QString ProfileImporter::extensionProfile()
{
    if ((radioBtnProfiles.checkedButton()->text().remove('&') == "JoyToKey")) return QString("cfg");
    else if ((radioBtnProfiles.checkedButton()->text().remove('&') == "XPadder")) return QString("xpaddercontroller");
    else if ((radioBtnProfiles.checkedButton()->text().remove('&') == "Pinnacle Game Profiler")) return QString("pin");

    return QString("cfg");
}


bool ProfileImporter::properExtension(const QString& profilePath)
{
    QFileInfo info(profilePath);

    if ((radioBtnProfiles.checkedButton()->text().remove('&') == "JoyToKey") && (info.completeSuffix() == "cfg"))
    {
        return true;
    }
    else if ((radioBtnProfiles.checkedButton()->text().remove('&') == "XPadder") && (info.completeSuffix() == "xpaddercontroller"))
    {
        return true;
    }
    else if ((radioBtnProfiles.checkedButton()->text().remove('&') == "Pinnacle Game Profiler") && (info.completeSuffix() == "pin"))
    {
        return true;
    }

    return false;
}


bool ProfileImporter::properExtensionSett(const QString & settfilePath)
{
    QFileInfo info(settfilePath);

    if (info.completeSuffix() == "ini") return true;

    return false;
}


void ProfileImporter::openFile(QString importedFilePath)
{
    QSettings settings(importedFilePath, QSettings::NativeFormat);



}


void ProfileImporter::readSettGroups()
{


}


const QString ProfileImporter::displayModeJoyToKey()
{
    return displayedModeJoyToKey;
}


void ProfileImporter::setDisplayModeJoyToKey(QString const & displayMode)
{
    displayedModeJoyToKey = displayMode;
}


QString ProfileImporter::filedialogDescExt()
{
    if (radioBtnProfiles.checkedButton()->text().remove('&') == "JoyToKey") return trUtf8("JoyToKey profiles (*.cfg)");
    else if (radioBtnProfiles.checkedButton()->text().remove('&') == "XPadder") return trUtf8("XPadder profiles (*.xpaddercontroller)");
    else if (radioBtnProfiles.checkedButton()->text().remove('&') == "Pinnacle Game Profiler") return trUtf8("Pinnacle Game profiles (*.pin)");

    return "";
}


const QString ProfileImporter::importedFilePath(QString title, QString extensionFile)
{
    QString fileName = QFileDialog::getOpenFileName(
            this, title,
            QFileDialog::getExistingDirectory(this, trUtf8("Find"),
                                              QDir::currentPath()),
            extensionFile);

    return fileName;
}


void ProfileImporter::rewriteButtonGroup()
{
    QList<QRadioButton *> allButtons = ui->groupBox->findChildren<QRadioButton *>();

    qDebug() << allButtons.size();

        for(int i = 0; i < allButtons.size(); ++i)
        {
            radioBtnProfiles.addButton(allButtons[i],i);
        }

    qDebug() << radioBtnProfiles.checkedId();
    qDebug() << radioBtnProfiles.checkedButton();
}


void ProfileImporter::putSettingsFromJoyToKey()
{

}


void ProfileImporter::putSettingsFromXPadder()
{

}


void ProfileImporter::putSettingsFromPinnacle()
{

}


// ----------------------------- SLOTS ------------------------------------- //


// 0 - unchecked
// 2 - checked
void ProfileImporter::on_fullSettCheckBox_stateChanged(int state)
{
    switch (state)
    {
        case 0:

            ui->configLineEdit->clear();
            ui->configLabel->setDisabled(true);
            ui->configLineEdit->setDisabled(true);
            ui->findConfigBtn->setDisabled(true);

        break;

        case 2:

            ui->configLabel->setDisabled(false);
            ui->configLineEdit->setDisabled(false);
            ui->findConfigBtn->setDisabled(false);

        break;

        default:

            ui->configLineEdit->clear();
            ui->configLabel->setDisabled(true);
            ui->configLineEdit->setDisabled(true);
            ui->findConfigBtn->setDisabled(true);

        break;
    }
}


void ProfileImporter::on_AcceptBtn_clicked()
{
    if (!allFilled())
    {
        QMessageBox box;
        box.setText(trUtf8("Could not import profile. Choose profile type and profile's file."));
        box.setWindowTitle(trUtf8("Insufficient data"));
        box.setStandardButtons(QMessageBox::Close);
        box.raise();
        box.exec();
    }
    else
    {


    }
}


void ProfileImporter::on_cancelBtn_clicked()
{
    close();
}


void ProfileImporter::on_findProfileBtn_clicked()
{
    if (radioBtnProfiles.checkedButton() == nullptr) {

        QMessageBox box;
        box.setText(trUtf8("Could not define file's extension. Choose profile's type first above."));
        box.setWindowTitle(trUtf8("Insufficient data"));
        box.setStandardButtons(QMessageBox::Close);
        box.raise();
        box.exec();

    } else {

        QString profile = importedFilePath(trUtf8("Choose game profile"), filedialogDescExt());

        if (!properExtension(profile))
        {
            QMessageBox box;
            box.setText(trUtf8("Could not choose a file. Find a file with proper extension."));
            box.setWindowTitle(trUtf8("Incorrect filename"));
            box.setStandardButtons(QMessageBox::Close);
            box.raise();
            box.exec();

        } else {

            ui->profileLineEdit->setText(profile);

            QFileInfo info(profile);
            chosenFile = info.completeBaseName();
        }
    }
}


void ProfileImporter::on_findConfigBtn_clicked()
{
    if (radioBtnProfiles.checkedButton() == nullptr)
    {
        QMessageBox box;
        box.setText(trUtf8("Could not define file's extension. Choose profile's type first above."));
        box.setWindowTitle(trUtf8("Insufficient data"));
        box.setStandardButtons(QMessageBox::Close);
        box.raise();
        box.exec();

    } else {

        QString settingsFile = importedFilePath(trUtf8("Choose app settings file"), trUtf8("Settings file (*.ini)"));

        if (!properExtensionSett(settingsFile))
        {
            QMessageBox box;
            box.setText(trUtf8("Could not choose a file. Find a file with proper extension."));
            box.setWindowTitle(trUtf8("Incorrect filename"));
            box.setStandardButtons(QMessageBox::Close);
            box.raise();
            box.exec();

        } else {

            ui->configLineEdit->setText(settingsFile);

            QFileInfo info(settingsFile);
            chosenFileSett = info.completeBaseName();
        }
    }
}
