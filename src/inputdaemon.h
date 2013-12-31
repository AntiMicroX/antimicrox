#ifndef INPUTDAEMONTHREAD_H
#define INPUTDAEMONTHREAD_H

#include <QHash>
#include <QThread>

#ifdef USE_SDL_2
#include <SDL2/SDL_joystick.h>
#include <SDL2/SDL_events.h>
#include "gamecontroller/gamecontroller.h"

#else
#include <SDL/SDL_joystick.h>
#include <SDL/SDL_events.h>

#endif

#include "joystick.h"
#include "sdleventreader.h"

class InputDaemon : public QObject
{
    Q_OBJECT
public:
#ifdef USE_SDL_2
    explicit InputDaemon (QHash<SDL_JoystickID, InputDevice*> *joysticks, bool graphical=true, QObject *parent=0);
#else
    explicit InputDaemon (QHash<int, InputDevice*> *joysticks, bool graphical=true, QObject *parent=0);
#endif
    ~InputDaemon();

protected:
#ifdef USE_SDL_2
    QHash<SDL_JoystickID, InputDevice*> *joysticks;
    QHash<SDL_JoystickID, Joystick*> trackjoysticks;
    QHash<SDL_JoystickID, GameController*> trackcontrollers;

#else
    QHash<int, InputDevice*> *joysticks;
#endif
    bool stopped;
    bool graphical;

    SDLEventReader *eventWorker;
    QThread *thread;

signals:
    void joystickRefreshed (InputDevice *joystick);
#ifdef USE_SDL_2
    void joysticksRefreshed(QHash<SDL_JoystickID, InputDevice*> *joysticks);
#else
    void joysticksRefreshed(QHash<int, InputDevice*> *joysticks);
#endif
    void complete(InputDevice* joystick);
    void complete();

#ifdef USE_SDL_2
    void deviceUpdated(int index, InputDevice *device);
    void deviceRemoved(SDL_JoystickID deviceID);
    void deviceAdded(InputDevice *device);
#endif

public slots:
    void run();
    void quit();
    void refresh();
    void refreshJoystick(InputDevice *joystick);
    void refreshJoysticks();
#ifdef USE_SDL_2
    void refreshMapping(QString mapping, InputDevice *device);
    void removeDevice(InputDevice *device);
    void addInputDevice(int index);
    void refreshIndexes();

#endif

private slots:
    void stop();
};

#endif // INPUTDAEMONTHREAD_H
