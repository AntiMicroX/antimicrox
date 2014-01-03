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
#include <QLocalServer>

#ifdef USE_SDL_2
#include <SDL2/SDL_joystick.h>
#endif

#include "joystick.h"
#include "aboutdialog.h"
#include "commandlineutility.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
#ifdef USE_SDL_2
    MainWindow(QHash<SDL_JoystickID, InputDevice*> *joysticks, CommandLineUtility *cmdutility, bool graphical=true, QWidget *parent = 0);
#else
    MainWindow(QHash<int, InputDevice*> *joysticks, CommandLineUtility *cmdutility, bool graphical=true, QWidget *parent = 0);
#endif
    ~MainWindow();
    
protected:
    virtual void hideEvent(QHideEvent * event);
    virtual void showEvent(QShowEvent *event);
    void loadConfigFile(QString fileLocation, int joystickIndex=0);
    void loadConfigFile(QString fileLocation, QString controllerID);
    void unloadCurrentConfig(int joystickIndex=0);
    void unloadCurrentConfig(QString controllerID);

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
#ifdef USE_SDL_2
    void testMappingUpdateNow(int index, InputDevice *device);
    void removeJoyTab(SDL_JoystickID deviceID);
    void addJoyTab(InputDevice *device);
#endif

private slots:
    void quitProgram();
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
#ifdef USE_SDL_2
    void openGameControllerMappingWindow();
    void propogateMappingUpdate(QString mapping, InputDevice *device);
    void openMainSettingsDialog();
#endif
};

#endif // MAINWINDOW_H
