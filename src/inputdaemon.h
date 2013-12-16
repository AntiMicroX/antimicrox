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
#ifdef USE_SDL_2
    explicit InputDaemon (QHash<SDL_JoystickID, Joystick*> *joysticks, bool graphical=true, QObject *parent=0);
#else
    explicit InputDaemon (QHash<int, Joystick*> *joysticks, bool graphical=true, QObject *parent=0);
#endif
    ~InputDaemon();

protected:
#ifdef USE_SDL_2
    QHash<SDL_JoystickID, Joystick*> *joysticks;
#else
    QHash<int, Joystick*> *joysticks;
#endif
    bool stopped;
    bool graphical;

    SDLEventReader *eventWorker;
    QThread *thread;

signals:
    void joystickRefreshed (Joystick *joystick);
#ifdef USE_SDL_2
    void joysticksRefreshed(QHash<SDL_JoystickID, Joystick*> *joysticks);
#else
    void joysticksRefreshed(QHash<int, Joystick*> *joysticks);
#endif
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
