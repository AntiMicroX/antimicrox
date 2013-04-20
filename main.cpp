#include <QtGui/QApplication>
#include <QDebug>
#include <QThread>
#include <QDir>

#include <X11/Xlib.h>

#include "mainwindow.h"
#include "joystick.h"
#include "inputdaemon.h"
#include "xmlconfigreader.h"
#include "xmlconfigwriter.h"
#include "common.h"

int main(int argc, char *argv[])
{
    qRegisterMetaType<JoyButtonSlot*>();

    XInitThreads ();

    QHash<int, Joystick*> *joysticks = new QHash<int, Joystick*> ();

    QApplication a(argc, argv);
    //Q_INIT_RESOURCE(resources);
    //a.setQuitOnLastWindowClosed(false);
    MainWindow w(joysticks);

    QDir configDir (PadderCommon::configPath);
    if (!configDir.exists())
    {
        configDir.mkdir(PadderCommon::configPath);
    }

    InputDaemon *joypad_worker = new InputDaemon (joysticks);

    QObject::connect(joypad_worker, SIGNAL(joysticksRefreshed(QHash<int,Joystick*>*)), &w, SLOT(fillButtons(QHash<int,Joystick*>*)));
    QObject::connect(&w, SIGNAL(joystickRefreshRequested()), joypad_worker, SLOT(refresh()));
    QObject::connect(joypad_worker, SIGNAL(joystickRefreshed(Joystick*)), &w, SLOT(fillButtons(Joystick*)));
    QObject::connect(&w, SIGNAL(joystickRefreshRequested(Joystick*)), joypad_worker, SLOT(refreshJoystick(Joystick*)));
    QObject::connect(&a, SIGNAL(aboutToQuit()), &w, SLOT(saveAppConfig()));

    w.show();

    int app_result = a.exec();

    joypad_worker->quit();
    //joypad_worker->stop();

    delete joypad_worker;
    joypad_worker = 0;

    delete joysticks;
    joysticks = 0;

    return app_result;
}

