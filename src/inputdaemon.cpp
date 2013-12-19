#include <QtDebug>
#include <QTimer>
#include <QEventLoop>
#include <QHashIterator>

#ifdef USE_SDL_2
#include <SDL2/SDL_joystick.h>
#include <SDL2/SDL_events.h>

#else
#include <SDL/SDL_joystick.h>
#include <SDL/SDL_events.h>

#endif

#include "inputdaemon.h"

#ifdef USE_SDL_2
InputDaemon::InputDaemon(QHash<SDL_JoystickID, Joystick*> *joysticks, bool graphical, QObject *parent) :
#else
InputDaemon::InputDaemon(QHash<int, Joystick*> *joysticks, bool graphical, QObject *parent) :
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
                    Joystick *joy = joysticks->value(event.jbutton.which);
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyButton *button = set->getJoyButton(event.jbutton.button);

                    if (button)
                    {
                        button->joyEvent(true);
                    }
                    break;
                }

                case SDL_JOYBUTTONUP:
                {
                    Joystick *joy = joysticks->value(event.jbutton.which);
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyButton *button = set->getJoyButton(event.jbutton.button);

                    if (button)
                    {
                        button->joyEvent(false);
                    }
                    break;
                }

                case SDL_JOYAXISMOTION:
                {
                    Joystick *joy = joysticks->value(event.jaxis.which);
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyAxis *axis = set->getJoyAxis(event.jaxis.axis);
                    if (axis)
                    {
                        axis->joyEvent(event.jaxis.value);
                    }
                    break;
                }

                case SDL_JOYHATMOTION:
                {
                    Joystick *joy = joysticks->value(event.jhat.which);
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyDPad *dpad = set->getJoyDPad(event.jhat.hat);
                    if (dpad)
                    {
                        dpad->joyEvent(event.jhat.value);
                    }
                    break;
                }

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
    QHashIterator<int, Joystick*> iter(*joysticks);
    while (iter.hasNext())
    {
        Joystick *joystick = iter.next().value();
        if (joystick)
        {
            delete joystick;
            joystick = 0;
        }
    }

    joysticks->clear();

    for (int i=0; i < SDL_NumJoysticks(); i++)
    {
        SDL_Joystick *joystick = SDL_JoystickOpen(i);
        Joystick *curJoystick = new Joystick(joystick, this);
#ifdef USE_SDL_2
        SDL_JoystickID joystickID = SDL_JoystickInstanceID(joystick);
        joysticks->insert(joystickID, curJoystick);
#else
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

void InputDaemon::refreshJoystick(Joystick *joystick)
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
