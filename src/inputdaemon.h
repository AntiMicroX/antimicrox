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
    InputDaemon (QHash<int, Joystick*> *joysticks, bool graphical=true, QObject *parent=0);
    ~InputDaemon();

protected:
    QHash<int, Joystick*> *joysticks;
    bool stopped;
    bool graphical;

    SDLEventReader *eventWorker;
    QThread *thread;

signals:
    void joystickRefreshed (Joystick *joystick);
    void joysticksRefreshed(QHash<int, Joystick*> *joysticks);
    void complete(Joystick* joystick);
    void complete();

public slots:
    void run();
    void quit();
    void refresh();
    void refreshJoystick(Joystick *joystick);
    void refreshJoysticks();

private slots:
    void stop();
};

#endif // INPUTDAEMONTHREAD_H
