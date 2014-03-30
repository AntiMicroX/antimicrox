#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHash>
#include <QIcon>
#include <QSystemTrayIcon>
#include <QAction>
#include <QFileDialog>
#include <QHideEvent>
#include <QShowEvent>
#include <QCloseEvent>
#include <QLocalServer>
#include <QSettings>
#include <QTranslator>

#include "inputdevice.h"
#include "aboutdialog.h"
#include "commandlineutility.h"
#include "autoprofilewatcher.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
#ifdef USE_SDL_2
    //MainWindow(QHash<SDL_JoystickID, InputDevice*> *joysticks, CommandLineUtility *cmdutility, QSettings *settings, bool graphical=true, QWidget *parent = 0);
    MainWindow(QHash<SDL_JoystickID, InputDevice*> *joysticks, QTranslator *translator, CommandLineUtility *cmdutility, QSettings *settings, bool graphical=true, QWidget *parent = 0);
#else
    MainWindow(QHash<int, InputDevice*> *joysticks, QTranslator *translator, CommandLineUtility *cmdutility, QSettings *settings, bool graphical=true, QWidget *parent = 0);
    //MainWindow(QHash<int, InputDevice*> *joysticks, CommandLineUtility *cmdutility, QSettings *settings, bool graphical=true, QWidget *parent = 0);
#endif
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

#ifdef USE_SDL_2
    QHash<SDL_JoystickID, InputDevice*> *joysticks;
#else
    QHash<int, InputDevice*> *joysticks;
#endif

    QSystemTrayIcon *trayIcon;
    QAction *hideAction;
    QAction *restoreAction;
    QAction *closeAction;
    QAction *updateJoy;
    QMenu *trayIconMenu;
    AboutDialog *aboutDialog;
    bool signalDisconnect;
    bool showTrayIcon;
    bool graphical;
    QLocalServer *localServer;
    CommandLineUtility *cmdutility;
    QSettings *settings;
    QTranslator *translator;
    AutoProfileWatcher *appWatcher;

private:
    Ui::MainWindow *ui;

signals:
    void joystickRefreshRequested();
    void joystickRefreshRequested(InputDevice *joystick);
    void readConfig(int index);
    void mappingUpdated(QString mapping, InputDevice *device);

public slots:
    void fillButtons(InputDevice *joystick);
#ifdef USE_SDL_2
    void fillButtons(QHash<SDL_JoystickID, InputDevice*> *joysticks);
#else
    void fillButtons(QHash<int, InputDevice*> *joysticks);
#endif
    void startJoystickRefresh();
    void hideWindow();
    void saveAppConfig();
    void loadAppConfig(bool forceRefresh=false);
    void removeJoyTabs();
    void startLocalServer();
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
    void trayMenuChangeJoyConfig(QAction *action);
    void joystickTrayShow();
    void populateTrayIcon();
    void openAboutDialog();
    void handleOutsideConnection();
    void handleSocketDisconnect();
    void openJoystickStatusWindow();
    void openKeyCheckerDialog();
    void openProjectHomePage();
    void openGitHubPage();
    void propogateNameDisplayStatus(bool displayNames);
    void changeLanguage(QString language);
    void openMainSettingsDialog();

#ifdef USE_SDL_2
    void openGameControllerMappingWindow();
    void propogateMappingUpdate(QString mapping, InputDevice *device);
    void autoprofileLoad(QString guid, QString profileLocation);
    void checkAutoProfileWatcherTimer();
#endif
};

#endif // MAINWINDOW_H
