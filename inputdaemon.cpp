#include <QtDebug>
#include <QTimer>
#include <QEventLoop>
#include <SDL/SDL.h>

#include "inputdaemon.h"

InputDaemon::InputDaemon(QHash<int, Joystick*> *joysticks, QObject *parent) :
    QObject(parent)
{
    this->joysticks = joysticks;
    this->stopped = false;
    this->sdlIgnoreEvent = true;

    eventWorker = new SDLEventReader(joysticks);
    thread = new QThread();
    eventWorker->moveToThread(thread);

    QTimer::singleShot(0, this, SLOT(refreshJoysticks()));
    connect(thread, SIGNAL(started()), eventWorker, SLOT(performWork()));
    connect(eventWorker, SIGNAL(eventRaised()), this, SLOT(run()));
    thread->start();
}

InputDaemon::~InputDaemon()
{
    if (eventWorker)
    {
        eventWorker->stop();
        delete eventWorker;
        eventWorker = 0;
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
    if (joysticks->count() > 0 && !sdlIgnoreEvent)
    {
        SDL_Event event;
        event = eventWorker->getCurrentEvent();

        do
        {
            switch (event.type)
            {
                case SDL_JOYBUTTONDOWN:
                {
                    Joystick *joy = joysticks->value(event.button.which);
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyButton *button = set->getJoyButton(event.button.button);

                    if (button)
                    {
                        button->joyEvent(true);
                    }
                    break;
                }

                case SDL_JOYBUTTONUP:
                {
                    Joystick *joy = joysticks->value(event.button.which);
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyButton *button = set->getJoyButton(event.button.button);

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
    else if (joysticks->count() > 0 && sdlIgnoreEvent)
    {
        // SDL will queue events for each axis detected on
        // a controller when first opened. Ignore initial axis motion events.
        // Old SDL_EventState solution was not good enough
        SDL_Event event;
        while (SDL_PollEvent(&event) > 0)
        {
        }
        sdlIgnoreEvent = false;
    }

    if (stopped)
    {
        if (joysticks->count() > 0)
        {
            emit complete(joysticks->value(0));
        }
        emit complete();
        stopped = false;
        sdlIgnoreEvent = true;
    }
    else
    {
        QTimer::singleShot(0, eventWorker, SLOT(performWork()));
    }
}

void InputDaemon::refreshJoysticks()
{
    joysticks->clear();

    for (int i=0; i < SDL_NumJoysticks(); i++)
    {
        SDL_Joystick* joystick = SDL_JoystickOpen (i);
        Joystick *curJoystick = new Joystick (joystick, this);
        curJoystick->reset();

        joysticks->insert(i, curJoystick);
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

    refreshJoysticks();
    QTimer::singleShot(0, eventWorker, SLOT(performWork()));
}

void InputDaemon::refreshJoystick(Joystick *joystick)
{
    joystick->reset();

    emit joystickRefreshed(joystick);
}
