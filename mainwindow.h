#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHash>
#include <QSystemTrayIcon>
#include <QAction>
#include <QFileDialog>
#include <QHideEvent>
#include <QShowEvent>

#include "joystick.h"
#include "aboutdialog.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QHash<int, Joystick*> *joysticks, QWidget *parent = 0);
    ~MainWindow();
    
protected:
    QHash<int, Joystick*> *joysticks;
    QSystemTrayIcon *trayIcon;
    QAction *hideAction;
    QAction *restoreAction;
    QAction *closeAction;
    QAction *updateJoy;
    QMenu *trayIconMenu;
    AboutDialog *aboutDialog;
    bool signalDisconnect;

    virtual void hideEvent(QHideEvent * event);
    virtual void showEvent(QShowEvent *event);

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
    void nothinButLuigi();

private slots:
    void quitProgram();
    void refreshTrayIconMenu();
    void trayIconClickAction(QSystemTrayIcon::ActivationReason reason);
    void mainMenuChange();
    void saveAppConfig();
    void loadAppConfig();
    void disableFlashActions();
    void enableFlashActions();
    void joystickRefreshPropogate(Joystick *joystick);
    void trayMenuChangeJoyConfig(QAction *action);
    void joystickTrayShow();
    void populateTrayIcon();
    void openAboutDialog();
};

#endif // MAINWINDOW_H
