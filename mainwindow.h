#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHash>
#include <QSystemTrayIcon>
#include <QAction>
#include <QFileDialog>

#include "joystick.h"

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
    void populateTrayIcon();

    QHash<int, Joystick*> *joysticks;
    QSystemTrayIcon *trayIcon;
    QAction *hideAction;
    QAction *restoreAction;
    QAction *closeAction;
    QAction *updateJoy;

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
    void restoreWindow();

private slots:
    void quitProgram();
    void refreshTrayIconMenu();
    void mainMenuChange();
    void saveAppConfig();
    void loadAppConfig();
    void disableFlashActions();
    void enableFlashActions();
    void joystickRefreshPropogate(Joystick *joystick);
};

#endif // MAINWINDOW_H
