#include <QtDebug>
#include <QCoreApplication>
#include <QTimer>
#include <SDL/SDL.h>

#include "inputdaemon.h"

InputDaemon::InputDaemon(QHash<int, Joystick*> *joysticks, QObject *parent) :
    QObject(parent)
{
    this->joysticks = joysticks;
    this->stopped = false;
    this->performRefresh = true;
    initSDL();
}

InputDaemon::~InputDaemon()
{
    closeSDL();
}

void InputDaemon::initSDL()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    SDL_JoystickEventState(SDL_ENABLE);
}

void InputDaemon::closeSDL()
{
    SDL_Event event;
    for (int i=0; i < SDL_NumJoysticks(); i++)
    {
        if (SDL_JoystickOpened(i) && joysticks->value(i))
        {
            SDL_Joystick *handle = (joysticks->value(i))->getSDLHandle();
            SDL_JoystickClose(handle);
        }
    }

    // Clear any pending events
    while (SDL_PollEvent(&event) > 0)
    {
    }
    SDL_Quit();
}

void InputDaemon::run ()
{
    if (performRefresh)
    {
        refreshJoysticks();
        performRefresh = false;
    }

    if (joysticks->count() > 0)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event) > 0)
        {
            switch (event.type)
            {
                case SDL_JOYBUTTONDOWN: {
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

                case SDL_JOYAXISMOTION: {
                    Joystick *joy = joysticks->value(event.jaxis.which);
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyAxis *axis = set->getJoyAxis(event.jaxis.axis);
                    if (axis)
                    {
                        axis->joyEvent(event.jaxis.value);
                    }
                    break;
                }

                case SDL_JOYHATMOTION: {
                    Joystick *joy = joysticks->value(event.jhat.which);
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyDPad *dpad = set->getJoyDPad(event.jhat.hat);
                    if (dpad)
                    {
                        dpad->joyEvent(event.jhat.value);
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }

    if (stopped)
    {
        if (joysticks->count() > 0)
        {
            emit complete(joysticks->value(0));
        }
        emit complete();
    }
    else
    {
        QTimer::singleShot(10, this, SLOT(run()));
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

    if (joysticks->count() > 0)
    {
        emit joysticksRefreshed(joysticks);
    }
}

void InputDaemon::stop()
{
    stopped = true;
    closeSDL();
}

void InputDaemon::refresh()
{
    stop();
    initSDL();
    refreshJoysticks();
    stopped = false;
}

bool InputDaemon::isRunning()
{
    return !stopped;
}

void InputDaemon::refreshJoystick(Joystick *joystick)
{
    joystick->reset();

    emit joystickRefreshed(joystick);
}
