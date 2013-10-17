#include <QtDebug>
#include <QTimer>
#include <QEventLoop>
#include <QHashIterator>
#include <SDL/SDL_joystick.h>
#include <SDL/SDL_events.h>

#include "inputdaemon.h"

InputDaemon::InputDaemon(QHash<int, Joystick*> *joysticks, bool graphical, QObject *parent) :
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
    if (joysticks->count() > 0 && !stopped)
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

    if (stopped)
    {
        if (joysticks->count() > 0)
        {
            emit complete(joysticks->value(0));
        }
        emit complete();
        stopped = false;
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
        SDL_Joystick* joystick = SDL_JoystickOpen (i);
        Joystick *curJoystick = new Joystick (joystick, this);

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
