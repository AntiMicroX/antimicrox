/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
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

#include <QDialog>
#include <QHash>
#include <QList>
#include <QSettings>
#include <QMap>

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

protected:
    void fillControllerMappingsTable();
    void insertTempControllerMapping(QHash<QString, QList<QVariant> > &hash, QString newGUID);
    void checkLocaleChange();
    void populateAutoProfiles();
    void fillAutoProfilesTable(QString guid);
    void fillAllAutoProfilesTable();
    void clearAutoProfileData();
    void changePresetLanguage();
    void fillSpringScreenPresets();
    void refreshExtraMouseInfo();

signals:
    void changeLanguage(QString language);

protected slots:
    void mappingsTableItemChanged(QTableWidgetItem *item);
    void insertMappingRow();
    void deleteMappingRow();
    void syncMappingSettings();
    void saveNewSettings();
    void selectDefaultProfileDir();
    void fillGUIDComboBox();
    void changeDeviceForProfileTable(int index);
    void saveAutoProfileSettings();
    void processAutoProfileActiveClick(QTableWidgetItem *item);
    void openAddAutoProfileDialog();
    void openEditAutoProfileDialog();
    void openDeleteAutoProfileConfirmDialog();
    void changeAutoProfileButtonsState();
    void transferEditsToCurrentTableRow(AddEditAutoProfileDialog *dialog);
    void transferAllProfileEditToCurrentTableRow(EditAllDefaultAutoProfileDialog* dialog);
    void addNewAutoProfile();
    void autoProfileButtonsActiveState(bool enabled);
    void changeKeyRepeatWidgetsStatus(bool enabled);
    void checkSmoothingWidgetStatus(bool enabled);
    void resetMouseAcceleration();
    void selectLogFile();

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
};

#endif // MAINSETTINGSDIALOG_H
