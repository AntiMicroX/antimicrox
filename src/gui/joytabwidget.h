/* antimicrox Gamepad to KB+M event mapper
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

#ifndef JOYTABWIDGET_H
#define JOYTABWIDGET_H

#include <QLabel>
#include <QWidget>

#include <SDL_joystick.h>

#include "uihelpers/joytabwidgethelper.h"

class InputDevice;
class AntiMicroSettings;
class QEvent;
class SetJoystick;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QPushButton;
class QComboBox;
class QSpacerItem;
class AxisEditDialog;
class QAction;
class QMenu;
class QStackedWidget;
class QSettings;

/**
 * @brief Widget containing tab representing selected joystick.
 *
 * It is visible in main window of application.
 */
class JoyTabWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit JoyTabWidget(InputDevice *joystick, AntiMicroSettings *settings, QWidget *parent = nullptr);

    void saveSettings();                          // JoyTabSettings class
    void loadSettings(bool forceRefresh = false); // JoyTabSettings class
    void setCurrentConfig(int index);             // JoyTabSettings class
    void unloadConfig();                          // JoyTabSettings class
    void checkHideEmptyOption();
    void refreshHelperThread();
    void convToUniqueIDControllerGroupSett(QSettings *sett, QString guidControllerSett, QString uniqueControllerSett);

    bool isDisplayingNames();
    bool discardUnsavedProfileChanges();
    bool static changesNotSaved();

    int getCurrentConfigIndex(); // JoyTabSettings class

    QHash<int, QString> *recentConfigs(); // JoyTabSettings class

    QString getCurrentConfigName(); // JoyTabSettings class
    QString getConfigName(int index);

    InputDevice *getJoystick();

  protected:
    virtual void changeEvent(QEvent *event);
    void removeCurrentButtons();
    void retranslateUi();
    void disconnectMainComboBoxEvents();
    void reconnectMainComboBoxEvents();
    void disconnectCheckUnsavedEvent();
    void reconnectCheckUnsavedEvent();
    void fillSetButtons(SetJoystick *set);   // JoyTabWidgetSets class
    void removeSetButtons(SetJoystick *set); // JoyTabWidgetSets class
    bool isKeypadUnlocked();

    static const int DEFAULTNUMBERPROFILES = 5;

  signals:
    void joystickConfigChanged(int index); // JoyTabSettings class
    void joystickAxisRefreshLabels(int axisIndex);
    void namesDisplayChanged(bool status);
    void forceTabUnflash(JoyTabWidget *tabWidget);
    void mappingUpdated(QString mapping, InputDevice *device);

  public slots:
    void openConfigFileDialog();                // JoyTabSettings class
    void fillButtons();                         // JoyTabWidgetSets class
    void saveDeviceSettings(bool sync = false); // JoyTabSettings class
    void loadDeviceSettings();                  // JoyTabSettings class
    void changeNameDisplay(bool displayNames);
    void changeCurrentSet(int index);          // JoyTabWidgetSets class
    void loadConfigFile(QString fileLocation); // JoyTabSettings class
    void refreshButtons();

  private slots:
    void saveConfigFile(); // JoyTabSettings class
    void resetJoystick();
    void saveAsConfig();             // JoyTabSettings class
    void removeConfig();             // JoyTabSettings class
    void changeJoyConfig(int index); // JoyTabSettings class
    void showAxisDialog();
    void showButtonDialog();
    void showStickAssignmentDialog();
    void showQuickSetDialog();
    void showKeyDelayDialog();
    void showSetNamesDialog(); // JoyTabWidgetSets class
    void toggleNames();
    void updateBatteryIcon();

    void changeSetOne();   // JoyTabWidgetSets class
    void changeSetTwo();   // JoyTabWidgetSets class
    void changeSetThree(); // JoyTabWidgetSets class
    void changeSetFour();  // JoyTabWidgetSets class
    void changeSetFive();  // JoyTabWidgetSets class
    void changeSetSix();   // JoyTabWidgetSets class
    void changeSetSeven(); // JoyTabWidgetSets class
    void changeSetEight(); // JoyTabWidgetSets class
    void displayProfileEditNotification();
    void removeProfileEditNotification();
    void checkForUnsavedProfile(int newindex = -1);

    void checkStickDisplay();
    void checkSensorDisplay();
    void checkDPadButtonDisplay();
    void checkAxisButtonDisplay();
    void checkButtonDisplay();

    void checkStickEmptyDisplay();
    void checkSensorEmptyDisplay();
    void checkDPadButtonEmptyDisplay();
    void checkAxisButtonEmptyDisplay();
    void checkButtonEmptyDisplay();
    void editCurrentProfileItemText(QString text);
    void refreshCopySetActions(); // JoyTabWidgetSets class
    void performSetCopy();        // JoyTabWidgetSets class
    void disableCopyCurrentSet(); // JoyTabWidgetSets class
    void refreshSetButtons();     // JoyTabWidgetSets class
    void openGameControllerMappingWindow();
    void propogateMappingUpdate(QString mapping, InputDevice *device);

  private:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *configHorizontalLayout;
    QLabel *batteryIcon;
    QPushButton *removeButton;
    QPushButton *loadButton;
    QPushButton *saveButton;
    QPushButton *resetButton;
    QPushButton *namesPushButton;
    QPushButton *saveAsButton;
    QPushButton *delayButton;
    QComboBox *configBox;
    QGridLayout *gridLayout;
    QGridLayout *gridLayout2;
    QGridLayout *gridLayout3;
    QGridLayout *gridLayout4;
    QGridLayout *gridLayout5;
    QGridLayout *gridLayout6;
    QGridLayout *gridLayout7;
    QGridLayout *gridLayout8;

    QSpacerItem *spacer1;
    QSpacerItem *spacer2;
    QSpacerItem *spacer3;
    AxisEditDialog *axisDialog;

    QPushButton *setPushButton1;
    QPushButton *setPushButton2;
    QPushButton *setPushButton3;
    QPushButton *setPushButton4;
    QPushButton *setPushButton5;
    QPushButton *setPushButton6;
    QPushButton *setPushButton7;
    QPushButton *setPushButton8;

    QPushButton *setsMenuButton;
    QAction *setAction1;
    QAction *setAction2;
    QAction *setAction3;
    QAction *setAction4;
    QAction *setAction5;
    QAction *setAction6;
    QAction *setAction7;
    QAction *setAction8;
    QMenu *copySetMenu;

    QHBoxLayout *horizontalLayout_2;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *stickAssignPushButton;
    QPushButton *quickSetPushButton;
    QPushButton *gameControllerMappingPushButton;
    QSpacerItem *verticalSpacer_2;
    QStackedWidget *stackedWidget_2;
    QWidget *page;
    QWidget *page_2;
    QWidget *page_3;
    QWidget *page_4;
    QWidget *page_5;
    QWidget *page_6;
    QWidget *page_7;
    QWidget *page_8;
    QPushButton *pushButton;
    QSpacerItem *verticalSpacer_3;

    InputDevice *m_joystick;
    bool displayingNames;
    static bool changedNotSaved;
    AntiMicroSettings *m_settings;
    int comboBoxIndex = 0;
    bool hideEmptyButtons = false;
    QString oldProfileName;

    JoyTabWidgetHelper tabHelper;

    SDL_JoystickPowerLevel m_old_power_level = SDL_JOYSTICK_POWER_UNKNOWN;
    QTimer *m_battery_updater;
};

#endif // JOYTABWIDGET_H
