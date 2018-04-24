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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <SDL2/SDL_joystick.h>

#include <QMainWindow>
#include <QMap>
#include <QIcon>
#include <QSystemTrayIcon>
#include <QFileDialog>

class InputDevice;
class CommandLineUtility;
class AntiMicroSettings;
class QWidget;
class QTranslator;
class AutoProfileInfo;
class AutoProfileWatcher;
class QLocalServer;
class AboutDialog;
class QAction;
class QMenu;
class QShowEvent;
class QEvent;
class QCloseEvent;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{

    Q_OBJECT
    
public:
    MainWindow(QMap<SDL_JoystickID, InputDevice*> *joysticks,
               CommandLineUtility *cmdutility,
               AntiMicroSettings *settings,
               bool graphical=true, QWidget *parent = nullptr);
    ~MainWindow();

    bool getGraphicalStatus();

    void setTranslator(QTranslator *translator);
    QTranslator* getTranslator() const;

    void setAppTranslator(QTranslator *translator);
    QTranslator* getAppTranslator() const;

    QMap<SDL_JoystickID, InputDevice*> *getJoysticks() const;

    QSystemTrayIcon *getTrayIcon() const;
    QAction *getHideAction() const;
    QAction *getRestoreAction() const;
    QAction *getCloseAction() const;
    QAction *getUpdateJoy() const;
    QMenu *getTrayIconMenu() const;
    QMap<int, QList<QAction*> >* getProfileActions();
    AboutDialog *getAboutDialog() const;
    QLocalServer *getLocalServer() const;
    CommandLineUtility *getCmdutility() const;
    AntiMicroSettings *getSettings() const;
    AutoProfileWatcher *getAppWatcher() const;

protected:
    virtual void showEvent(QShowEvent *event);
    virtual void changeEvent(QEvent *event);
    virtual void closeEvent(QCloseEvent *event);
    void retranslateUi();
    void loadConfigFile(QString fileLocation, int joystickIndex=0);
    void loadConfigFile(QString fileLocation, QString controllerID);
    void unloadCurrentConfig(int joystickIndex=0);
    void unloadCurrentConfig(QString controllerID);
    void changeStartSetNumber(int startSetNumber, QString controllerID); // (unsigned, ..)
    void changeStartSetNumber(int startSetNumber, int joystickIndex=0); // (unsigned, unsigned)

signals:
    void joystickRefreshRequested();
    void readConfig(int index);
    void mappingUpdated(QString mapping, InputDevice *device);


public slots:
    void fillButtons();
    void makeJoystickTabs();
    void alterConfigFromSettings();
    void fillButtons(InputDevice *joystick);
    void fillButtons(QMap<SDL_JoystickID, InputDevice*> *joysticks);
    void startJoystickRefresh();
    void hideWindow();
    void saveAppConfig();
    void loadAppConfig(bool forceRefresh=false);
    void removeJoyTabs();
    void quitProgram();
    void changeWindowStatus();
    void refreshTabHelperThreads();
    void controllerMapOpening();
    void testMappingUpdateNow(int index, InputDevice *device);
    void removeJoyTab(SDL_JoystickID deviceID);
    void addJoyTab(InputDevice *device);
    void selectControllerJoyTab(QString GUID);
    void selectControllerJoyTab(int index); // (unsigned)


private slots:
    void refreshTrayIconMenu();
    void trayIconClickAction(QSystemTrayIcon::ActivationReason reason);
    void mainMenuChange();
    void disableFlashActions();
    void enableFlashActions();

    void joystickTrayShow();
    void singleTrayProfileMenuShow();
    void profileTrayActionTriggered(bool checked);

    void populateTrayIcon();
    void openAboutDialog();
    void handleInstanceDisconnect();
    void openJoystickStatusWindow();
    void openKeyCheckerDialog();
    void openGitHubPage();
    void openIssuesPage();
    void openWikiPage();
    void propogateNameDisplayStatus(bool displayNames);
    void changeLanguage(QString language);
    void openMainSettingsDialog();
    void showStickAssignmentDialog();
    void checkHideEmptyOption();

#ifdef Q_OS_WIN
    void checkKeyRepeatOptions();
    void restartAsElevated();
#endif

    void openGameControllerMappingWindow(bool openAsMain=false);
    void propogateMappingUpdate(QString mapping, InputDevice *device);
    void autoprofileLoad(AutoProfileInfo *info);
    void checkAutoProfileWatcherTimer();
    void updateMenuOptions();

private:
    Ui::MainWindow *ui;

    QMap<SDL_JoystickID, InputDevice*> *joysticks;

    QSystemTrayIcon *trayIcon;
    QAction *hideAction;
    QAction *restoreAction;
    QAction *closeAction;
    QAction *updateJoy;
    QMenu *trayIconMenu;
    QMap<int, QList<QAction*> > profileActions;
    AboutDialog *aboutDialog;
    bool signalDisconnect;
    bool showTrayIcon;
    bool graphical;
    QLocalServer *localServer;
    CommandLineUtility *cmdutility;
    AntiMicroSettings *settings;
    QTranslator *translator;
    QTranslator *appTranslator;
    AutoProfileWatcher *appWatcher;

};

#endif // MAINWINDOW_H
