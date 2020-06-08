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


#ifndef MAINSETTINGSDIALOG_H
#define MAINSETTINGSDIALOG_H

#include "ui_mainsettingsdialog.h"

#include <QDialog>
#include <QSettings>

class AntiMicroSettings;
class InputDevice;
class QWidget;
class AutoProfileInfo;
class QTableWidgetItem;
class EditAllDefaultAutoProfileDialog;
class AddEditAutoProfileDialog;

namespace Ui {
class MainSettingsDialog;
}

class MainSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MainSettingsDialog(AntiMicroSettings *settings, QList<InputDevice*> *devices, QWidget *parent = nullptr);
    ~MainSettingsDialog();

    AntiMicroSettings *getSettings() const;

    QMap<QString, AutoProfileInfo*>* getDefaultAutoProfiles(); // Default profiles assigned to a specific device
    QMap<QString, QList<AutoProfileInfo*> >* getDeviceAutoProfiles(); // Profiles assigned with an association with an application
    QMap<QString, QList<AutoProfileInfo*> >* getExeAutoProfiles();

    QList<AutoProfileInfo*>* getDefaultList();
    QList<AutoProfileInfo*>* getProfileList();

    AutoProfileInfo* getAllDefaultProfile() const;
    QList<InputDevice*>* getConnectedDevices() const;
    QStringList& getChosenQuitComboKeys() const;

protected:
    void fillControllerMappingsTable(); // MainSettingsMapping class
    void insertTempControllerMapping(QHash<QString, QList<QVariant> > &hash, QString newGUID); // MainSettingsMapping class
    void checkLocaleChange(); // MainSettingsLang class
    void populateAutoProfiles(); // MainSettingsProfile class
    void fillAutoProfilesTable(QString guid); // MainSettingsProfile class
    void fillAllAutoProfilesTable(); // MainSettingsProfile class
    void clearAutoProfileData(); // MainSettingsProfile class
    void changePresetLanguage(); // MainSettingsLang class
    void fillSpringScreenPresets(); // MainSettingsMouse class
    void refreshExtraMouseInfo(); // MainSettingsMouse class
    void convToUniqueIDAutoProfGroupSett(QSettings* sett, QString guidAutoProfSett, QString uniqueAutoProfSett);

signals:
    void changeLanguage(QString language); // MainSettingsLang class

protected slots:
    void mappingsTableItemChanged(QTableWidgetItem *item); // MainSettingsMapping class
    void insertMappingRow(); // MainSettingsMapping class
    void deleteMappingRow(); // MainSettingsMapping class
    void syncMappingSettings(); // MainSettingsMapping class
    void saveNewSettings();
    void selectDefaultProfileDir(); // MainSettingsProfile class
    void fillGUIDComboBox(); // MainSettingsProfile class
    void changeDeviceForProfileTable(int index); // MainSettingsProfile class
    void addKeyToQuitCombination(QString key); // MainSettingsProfile class
    void saveAutoProfileSettings(); // MainSettingsProfile class
    void processAutoProfileActiveClick(QTableWidgetItem *item); // MainSettingsProfile class
    void openAddAutoProfileDialog(); // MainSettingsProfile class
    void openEditAutoProfileDialog(); // MainSettingsProfile class
    void openDeleteAutoProfileConfirmDialog(); // MainSettingsProfile class
    void changeAutoProfileButtonsState(); // MainSettingsProfile class
    void transferEditsToCurrentTableRow(AddEditAutoProfileDialog *dialog); // MainSettingsProfile class
    void transferAllProfileEditToCurrentTableRow(EditAllDefaultAutoProfileDialog* dialog); // MainSettingsProfile class
    void addNewAutoProfile(AddEditAutoProfileDialog *dialog); // MainSettingsProfile class
    void autoProfileButtonsActiveState(bool enabled); // MainSettingsProfile class
    void changeKeyRepeatWidgetsStatus(bool enabled); // MainSettingsProfile class
    void checkSmoothingWidgetStatus(bool enabled); // MainSettingsMouse class
    void resetMouseAcceleration(); // MainSettingsMouse class
    void selectLogFile(); // MainSettingsLogs class

private slots:
    void on_resetBtn_clicked();

private:
    Ui::MainSettingsDialog *ui;

    AntiMicroSettings *settings;

    QMap<QString, AutoProfileInfo*> defaultAutoProfiles; // Default profiles assigned to a specific device
    QMap<QString, QList<AutoProfileInfo*> > deviceAutoProfiles; // Profiles assigned with an association with an application
    QMap<QString, QList<AutoProfileInfo*> > exeAutoProfiles;

    QList<AutoProfileInfo*> defaultList;
    QList<AutoProfileInfo*> profileList;

    AutoProfileInfo* allDefaultProfile;
    QList<InputDevice*> *connectedDevices;

    void resetGeneralSett();
    void resetAutoProfSett();
    void resetMouseSett();
    void resetAdvancedSett();
    void initializeKeysList();
};

#endif // MAINSETTINGSDIALOG_H
