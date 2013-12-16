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
    MainWindow(QHash<SDL_JoystickID, Joystick*> *joysticks, CommandLineUtility *cmdutility, bool graphical=true, QWidget *parent = 0);
#else
    MainWindow(QHash<int, Joystick*> *joysticks, CommandLineUtility *cmdutility, bool graphical=true, QWidget *parent = 0);
#endif
    ~MainWindow();
    
protected:
    virtual void hideEvent(QHideEvent * event);
    virtual void showEvent(QShowEvent *event);
    void loadConfigFile(QString fileLocation, int joystickIndex=0);

#ifdef USE_SDL_2
    QHash<SDL_JoystickID, Joystick*> *joysticks;
#else
    QHash<int, Joystick*> *joysticks;
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

private:
    Ui::MainWindow *ui;

signals:
    void joystickRefreshRequested();
    void joystickRefreshRequested(Joystick *joystick);
    void readConfig(int index);

public slots:
    void fillButtons(Joystick *joystick);
#ifdef USE_SDL_2
    void fillButtons(QHash<SDL_JoystickID, Joystick*> *joysticks);
#else
    void fillButtons(QHash<int, Joystick*> *joysticks);
#endif
    void startJoystickRefresh();
    void hideWindow();
    void saveAppConfig();
    void loadAppConfig(bool forceRefresh=false);
    void removeJoyTabs();
    void startLocalServer();

private slots:
    void quitProgram();
    void refreshTrayIconMenu();
    void trayIconClickAction(QSystemTrayIcon::ActivationReason reason);
    void mainMenuChange();
    void disableFlashActions();
    void enableFlashActions();
    void joystickRefreshPropogate(Joystick *joystick);
    void trayMenuChangeJoyConfig(QAction *action);
    void joystickTrayShow();
    void populateTrayIcon();
    void openAboutDialog();
    void handleOutsideConnection();
    void handleSocketDisconnect();
    void openJoystickStatusWindow();
};

#endif // MAINWINDOW_H
