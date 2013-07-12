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
    explicit MainWindow(QHash<int, Joystick*> *joysticks, CommandLineUtility *cmdutility, bool graphical=true, QWidget *parent = 0);
    ~MainWindow();
    
protected:
    virtual void hideEvent(QHideEvent * event);
    virtual void showEvent(QShowEvent *event);
    void loadConfigFile(QString fileLocation, int joystickIndex=0);

    QHash<int, Joystick*> *joysticks;
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

private:
    Ui::MainWindow *ui;

signals:
    void joystickRefreshRequested();
    void joystickRefreshRequested(Joystick *joystick);
    void readConfig(int index);

public slots:
    void fillButtons(Joystick *joystick);
    void fillButtons(QHash<int, Joystick*>* joysticks);
    void startJoystickRefresh();
    void hideWindow();
    void saveAppConfig();
    void loadAppConfig(bool forceRefresh=false);
    void removeJoyTabs();

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
};

#endif // MAINWINDOW_H
