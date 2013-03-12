#ifndef INPUTDAEMONTHREAD_H
#define INPUTDAEMONTHREAD_H

#include <QHash>
#include <QThread>

#include "joystick.h"
#include "sdleventreader.h"

class InputDaemon : public QObject
{
    Q_OBJECT
public:
    InputDaemon (QHash<int, Joystick*> *joysticks, QObject *parent=0);
    ~InputDaemon();

protected:
    QHash<int, Joystick*> *joysticks;
    bool stopped;
    bool performRefresh;

    SDLEventReader *eventWorker;
    QThread *thread;

signals:
    void joystickRefreshed (Joystick *joystick);
    void joysticksRefreshed(QHash<int, Joystick*> *joysticks);
    void complete(Joystick* joystick);
    void complete();

public slots:
    void run ();
    void stop ();
    void refresh();
    void refreshJoystick(Joystick *joystick);

private slots:
    void refreshJoysticks ();
};

#endif // INPUTDAEMONTHREAD_H
