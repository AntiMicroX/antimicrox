#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QIcon>
#include <QSystemTrayIcon>
#include <QAction>
#include <QFileDialog>
#include <QHideEvent>
#include <QShowEvent>
#include <QCloseEvent>
#include <QLocalServer>
#include <QTranslator>

#include "inputdevice.h"
#include "aboutdialog.h"
#include "commandlineutility.h"
#include "antimicrosettings.h"
#include "autoprofilewatcher.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QMap<SDL_JoystickID, InputDevice*> *joysticks, CommandLineUtility *cmdutility, AntiMicroSettings *settings,
               bool graphical=true, QWidget *parent = 0);
    ~MainWindow();
    
protected:
    virtual void showEvent(QShowEvent *event);
    virtual void changeEvent(QEvent *event);
    virtual void closeEvent(QCloseEvent *event);
    void loadConfigFile(QString fileLocation, int joystickIndex=0);
    void loadConfigFile(QString fileLocation, QString controllerID);
    void unloadCurrentConfig(int joystickIndex=0);
    void unloadCurrentConfig(QString controllerID);
    void changeStartSetNumber(unsigned int startSetNumber, QString controllerID);
    void changeStartSetNumber(unsigned int startSetNumber, unsigned int joystickIndex=0);

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
    AutoProfileWatcher *appWatcher;

private:
    Ui::MainWindow *ui;

signals:
    void joystickRefreshRequested();
    void joystickRefreshRequested(InputDevice *joystick);
    void readConfig(int index);

#ifdef USE_SDL_2
    void mappingUpdated(QString mapping, InputDevice *device);
#endif

public slots:
    void fillButtons(InputDevice *joystick);
    void fillButtons(QMap<SDL_JoystickID, InputDevice*> *joysticks);
    void startJoystickRefresh();
    void hideWindow();
    void saveAppConfig();
    void loadAppConfig(bool forceRefresh=false);
    void removeJoyTabs();
    void quitProgram();

#ifdef USE_SDL_2
    void testMappingUpdateNow(int index, InputDevice *device);
    void removeJoyTab(SDL_JoystickID deviceID);
    void addJoyTab(InputDevice *device);
#endif

private slots:
    void refreshTrayIconMenu();
    void trayIconClickAction(QSystemTrayIcon::ActivationReason reason);
    void mainMenuChange();
    void disableFlashActions();
    void enableFlashActions();
    void joystickRefreshPropogate(InputDevice *joystick);

    void joystickTrayShow();
    void singleTrayProfileMenuShow();
    void profileTrayActionTriggered(bool checked);

    void populateTrayIcon();
    void openAboutDialog();
    void handleInstanceDisconnect();
    void openJoystickStatusWindow();
    void openKeyCheckerDialog();
    void openProjectHomePage();
    void openGitHubPage();
    void propogateNameDisplayStatus(bool displayNames);
    void changeLanguage(QString language);
    void openMainSettingsDialog();
    void showStickAssignmentDialog();
    void checkHideEmptyOption();

#ifdef Q_OS_WIN
    void checkKeyRepeatOptions();
#endif

#ifdef USE_SDL_2
    void openGameControllerMappingWindow();
    void propogateMappingUpdate(QString mapping, InputDevice *device);
    void autoprofileLoad(AutoProfileInfo *info);
    void checkAutoProfileWatcherTimer();
    void updateMenuOptions();
#endif
};

#endif // MAINWINDOW_H
