#include <QtDebug>
#include <QTimer>
#include <QEventLoop>
#include <QHashIterator>

#include "inputdaemon.h"

#ifdef USE_SDL_2
InputDaemon::InputDaemon(QHash<SDL_JoystickID, InputDevice*> *joysticks, bool graphical, QObject *parent) :
#else
InputDaemon::InputDaemon(QHash<int, InputDevice*> *joysticks, bool graphical, QObject *parent) :
#endif
    QObject(parent)
{
    this->joysticks = joysticks;
    this->stopped = false;
    this->graphical = graphical;

    eventWorker = new SDLEventReader(joysticks);
    thread = new QThread();
    eventWorker->moveToThread(thread);

    if (graphical)
    {
        connect(thread, SIGNAL(started()), eventWorker, SLOT(performWork()));
        connect(eventWorker, SIGNAL(eventRaised()), this, SLOT(run()));
        thread->start();
    }
    refreshJoysticks();
}

InputDaemon::~InputDaemon()
{
    if (eventWorker)
    {
        quit();
    }

    if (thread)
    {
        thread->quit();
        thread->wait();
        delete thread;
        thread = 0;
    }
}

void InputDaemon::run ()
{
    SDL_Event event;
#ifdef USE_SDL_2
    event.type = SDL_FIRSTEVENT;
#else
    event.type = SDL_NOEVENT;
#endif

    if (joysticks->count() > 0 && !stopped)
    {
        event = eventWorker->getCurrentEvent();

        do
        {
            switch (event.type)
            {
                case SDL_JOYBUTTONDOWN:
                {
#ifdef USE_SDL_2
                    InputDevice *joy = trackjoysticks.value(event.jbutton.which);
#else
                    InputDevice *joy = joysticks->value(event.jbutton.which);
#endif
                    if (joy)
                    {
                        SetJoystick* set = joy->getActiveSetJoystick();
                        JoyButton *button = set->getJoyButton(event.jbutton.button);

                        if (button)
                        {
                            button->joyEvent(true);
                        }
                    }

                    break;
                }

                case SDL_JOYBUTTONUP:
                {
#ifdef USE_SDL_2
                    InputDevice *joy = trackjoysticks.value(event.jbutton.which);
#else
                    InputDevice *joy = joysticks->value(event.jbutton.which);
#endif
                    if (joy)
                    {
                        SetJoystick* set = joy->getActiveSetJoystick();
                        JoyButton *button = set->getJoyButton(event.jbutton.button);

                        if (button)
                        {
                            button->joyEvent(false);
                        }
                    }

                    break;
                }

                case SDL_JOYAXISMOTION:
                {
#ifdef USE_SDL_2
                    InputDevice *joy = trackjoysticks.value(event.jaxis.which);
#else
                    InputDevice *joy = joysticks->value(event.jaxis.which);
#endif
                    if (joy)
                    {
                        SetJoystick* set = joy->getActiveSetJoystick();
                        JoyAxis *axis = set->getJoyAxis(event.jaxis.axis);
                        if (axis)
                        {
                            axis->joyEvent(event.jaxis.value);
                        }
                    }

                    break;
                }

                case SDL_JOYHATMOTION:
                {
#ifdef USE_SDL_2
                    InputDevice *joy = trackjoysticks.value(event.jhat.which);
#else
                    InputDevice *joy = joysticks->value(event.jhat.which);
#endif
                    if (joy)
                    {
                        SetJoystick* set = joy->getActiveSetJoystick();
                        JoyDPad *dpad = set->getJoyDPad(event.jhat.hat);
                        if (dpad)
                        {
                            dpad->joyEvent(event.jhat.value);
                        }
                    }

                    break;
                }

#ifdef USE_SDL_2
                case SDL_CONTROLLERAXISMOTION:
                {
                    InputDevice *joy = trackcontrollers.value(event.caxis.which);
                    if (joy)
                    {
                        SetJoystick* set = joy->getActiveSetJoystick();
                        JoyAxis *axis = set->getJoyAxis(event.caxis.axis);
                        if (axis)
                        {
                            axis->joyEvent(event.caxis.value);
                        }
                    }
                    break;
                }

                case SDL_CONTROLLERBUTTONDOWN:
                case SDL_CONTROLLERBUTTONUP:
                {
                    //InputDevice *joy = joysticks->value(event.cbutton.which);
                    InputDevice *joy = trackcontrollers.value(event.cbutton.which);
                    if (joy)
                    {
                        SetJoystick* set = joy->getActiveSetJoystick();
                        JoyButton *button = set->getJoyButton(event.cbutton.button);

                        if (button)
                        {
                            button->joyEvent(event.type == SDL_CONTROLLERBUTTONDOWN ? true : false);
                        }
                    }

                    break;
                }
#endif

                case SDL_QUIT:
                {
                    stopped = true;
                    break;
                }

                default:
                    break;
            }
        }
        while (SDL_PollEvent(&event) > 0);
    }

    if (stopped)
    {
        if (joysticks->count() > 0)
        {
            emit complete(joysticks->value(0));
        }
        emit complete();
        stopped = false;

        // Check for a grabbed instance of an SDL_QUIT event. If the last event was
        // not an SDL_QUIT event, push an event onto the queue so SdlEventReader
        // will finish properly.
#ifdef USE_SDL_2
        if (event.type != SDL_FIRSTEVENT && event.type != SDL_QUIT)
#else
        if (event.type != SDL_NOEVENT && event.type != SDL_QUIT)
#endif
        {
            event.type = SDL_QUIT;
            SDL_PushEvent(&event);
            QTimer::singleShot(0, eventWorker, SLOT(performWork()));
        }
    }
    else
    {
        QTimer::singleShot(0, eventWorker, SLOT(performWork()));
    }
}

void InputDaemon::refreshJoysticks()
{
#ifdef USE_SDL_2
    QHashIterator<SDL_JoystickID, InputDevice*> iter(*joysticks);
#else
    QHashIterator<int, InputDevice*> iter(*joysticks);
#endif
    while (iter.hasNext())
    {
        InputDevice *joystick = iter.next().value();
        if (joystick)
        {
            delete joystick;
            joystick = 0;
        }
    }

    joysticks->clear();
#ifdef USE_SDL_2
    trackjoysticks.clear();
    trackcontrollers.clear();
#endif

    for (int i=0; i < SDL_NumJoysticks(); i++)
    {
#ifdef USE_SDL_2
        if (SDL_IsGameController(i))
        {
            SDL_GameController *controller = SDL_GameControllerOpen(i);
            GameController *damncontroller = new GameController(controller, this);
            SDL_Joystick *sdlStick = SDL_GameControllerGetJoystick(controller);
            SDL_JoystickID joystickID = SDL_JoystickInstanceID(sdlStick);
            joysticks->insert(joystickID, damncontroller);
            trackcontrollers.insert(joystickID, damncontroller);
        }
        else
        {
            SDL_Joystick *joystick = SDL_JoystickOpen(i);
            Joystick *curJoystick = new Joystick(joystick, this);
            SDL_JoystickID joystickID = SDL_JoystickInstanceID(joystick);
            joysticks->insert(joystickID, curJoystick);
            trackjoysticks.insert(joystickID, curJoystick);
        }
#else
        SDL_Joystick *joystick = SDL_JoystickOpen(i);
        Joystick *curJoystick = new Joystick(joystick, this);
        joysticks->insert(i, curJoystick);
#endif
    }

    emit joysticksRefreshed(joysticks);
}

void InputDaemon::stop()
{
    stopped = true;
}

void InputDaemon::refresh()
{
    stop();

    eventWorker->refresh();

    QEventLoop q;
    connect(eventWorker, SIGNAL(sdlStarted()), &q, SLOT(quit()));
    q.exec();
    disconnect(eventWorker, SIGNAL(sdlStarted()), &q, SLOT(quit()));

    // Put in an extra delay before refreshing the joysticks
    QTimer temp;
    connect(&temp, SIGNAL(timeout()), &q, SLOT(quit()));
    temp.start(100);
    q.exec();

    refreshJoysticks();
    QTimer::singleShot(0, eventWorker, SLOT(performWork()));
}

void InputDaemon::refreshJoystick(InputDevice *joystick)
{
    joystick->reset();

    emit joystickRefreshed(joystick);
}

void InputDaemon::quit()
{
    stopped = true;
    eventWorker->stop();

    // Wait for SDL to finish. Let worker destructor close SDL.
    // Let InputDaemon destructor close thread instance.
    if (graphical)
    {
        QEventLoop q;
        connect(eventWorker, SIGNAL(finished()), &q, SLOT(quit()));
        q.exec();
    }

    delete eventWorker;
    eventWorker = 0;
}

#ifdef USE_SDL_2
void InputDaemon::refreshMapping(QString mapping, InputDevice *device)
{
    bool found = false;

    for (int i=0; i < SDL_NumJoysticks() && !found; i++)
    {
        SDL_Joystick *joystick = SDL_JoystickOpen(i);
        SDL_JoystickID joystickID = SDL_JoystickInstanceID(joystick);
        if (device->getJoyNumber() == joystickID)
        {
            found = true;

            if (SDL_IsGameController(i))
            {
                // Mapping string updated. Perform basic refresh
                QByteArray tempbarray = mapping.toUtf8();
                SDL_GameControllerAddMapping(tempbarray.data());
            }
            else
            {
                // Previously registered as a plain joystick. Add
                // mapping and check for validity. If SDL accepts it,
                // close current device and re-open as
                // a game controller.
                SDL_GameControllerAddMapping(mapping.toUtf8().constData());

                if (SDL_IsGameController(i))
                {
                    device->closeSDLDevice();
                    trackjoysticks.remove(joystickID);
                    joysticks->remove(joystickID);

                    SDL_GameController *controller = SDL_GameControllerOpen(i);
                    GameController *damncontroller = new GameController(controller, this);
                    SDL_Joystick *sdlStick = SDL_GameControllerGetJoystick(controller);
                    joystickID = SDL_JoystickInstanceID(sdlStick);
                    joysticks->insert(joystickID, damncontroller);
                    trackcontrollers.insert(joystickID, damncontroller);
                    emit deviceUpdated(i, damncontroller);
                }
            }
        }
    }
}
#endif
