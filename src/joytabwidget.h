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

#ifndef JOYTABWIDGET_H
#define JOYTABWIDGET_H


#include "uihelpers/joytabwidgethelper.h"

#include <QFileDialog>
#include <QHash>
#include <QScrollArea>
#include <QIcon>


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

class JoyTabWidget : public QWidget
{
    Q_OBJECT

public:
    explicit JoyTabWidget(InputDevice *joystick, AntiMicroSettings *settings, QWidget *parent = nullptr);

    void saveSettings();
    void loadSettings(bool forceRefresh=false);
    void setCurrentConfig(int index);
    void unloadConfig();
    void checkHideEmptyOption();
    void refreshHelperThread();

    bool isDisplayingNames();
    bool discardUnsavedProfileChanges();

    int getCurrentConfigIndex();

    QHash<int, QString>* recentConfigs();

    QString getCurrentConfigName();
    QString getConfigName(int index);

    InputDevice *getJoystick();


#ifdef Q_OS_WIN
    void deviceKeyRepeatSettings();
#endif

protected:
    virtual void changeEvent(QEvent *event);
    void removeCurrentButtons();
    void retranslateUi();
    void disconnectMainComboBoxEvents();
    void reconnectMainComboBoxEvents();
    void disconnectCheckUnsavedEvent();
    void reconnectCheckUnsavedEvent();
    void fillSetButtons(SetJoystick *set);
    void removeSetButtons(SetJoystick *set);

    static const int DEFAULTNUMBERPROFILES = 5;

signals:
    void joystickConfigChanged(int index);
    void joystickAxisRefreshLabels(int axisIndex);
    void namesDisplayChanged(bool status);
    void forceTabUnflash(JoyTabWidget *tabWidget);
    void mappingUpdated(QString mapping, InputDevice *device);

public slots:
    void openConfigFileDialog();
    void fillButtons();
    void saveDeviceSettings(bool sync=false);
    void loadDeviceSettings();
    void changeNameDisplay(bool displayNames);
    void changeCurrentSet(int index);
    void loadConfigFile(QString fileLocation);
    void refreshButtons();

private slots:
    void saveConfigFile();
    void resetJoystick();
    void saveAsConfig();
    void removeConfig();
    void changeJoyConfig(int index);
    void showAxisDialog();
    void showButtonDialog();
    void showStickAssignmentDialog();
    void showQuickSetDialog();
    void showKeyDelayDialog();
    void showSetNamesDialog();
    void toggleNames();

    void changeSetOne();
    void changeSetTwo();
    void changeSetThree();
    void changeSetFour();
    void changeSetFive();
    void changeSetSix();
    void changeSetSeven();
    void changeSetEight();
    void displayProfileEditNotification();
    void removeProfileEditNotification();
    void checkForUnsavedProfile(int newindex=-1);

    void checkStickDisplay();
    void checkDPadButtonDisplay();
    void checkAxisButtonDisplay();
    void checkButtonDisplay();

    void checkStickEmptyDisplay();
    void checkDPadButtonEmptyDisplay();
    void checkAxisButtonEmptyDisplay();
    void checkButtonEmptyDisplay();
    void editCurrentProfileItemText(QString text);
    void refreshCopySetActions();
    void performSetCopy();
    void disableCopyCurrentSet();
    void refreshSetButtons();
    void openGameControllerMappingWindow();
    void propogateMappingUpdate(QString mapping, InputDevice *device);

private:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *configHorizontalLayout;
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

    InputDevice *joystick;
    bool displayingNames;
    AntiMicroSettings *settings;
    int comboBoxIndex;
    bool hideEmptyButtons;
    QString oldProfileName;

    JoyTabWidgetHelper tabHelper;

};

#endif // JOYTABWIDGET_H
