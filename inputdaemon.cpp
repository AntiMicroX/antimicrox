#include <QtDebug>
#include <QCoreApplication>
#include <QTimer>
#include <SDL/SDL.h>

#include "inputdaemon.h"

InputDaemon::InputDaemon(QHash<int, Joystick*> *joysticks, QObject *parent) :
    QObject(parent)
{
    this->joysticks = joysticks;
    this->stopped = true;
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
    stopped = false;

    initSDL();

    refreshJoysticks();

    if (joysticks->count() > 0)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event) >= 0 && !stopped)
        {
            switch (event.type)
            {
                case SDL_JOYBUTTONDOWN: {
                    Joystick *joy = joysticks->value(event.button.which);
                    JoyButton *button = joy->getJoyButton(event.button.button);
                    button->joyEvent(true);
                    break;
                }

                case SDL_JOYBUTTONUP:
                {
                    Joystick *joy = joysticks->value(event.button.which);
                    JoyButton *button = joy->getJoyButton(event.button.button);
                    button->joyEvent(false);
                    break;
                }

                case SDL_JOYAXISMOTION: {
                    Joystick *joy = joysticks->value(event.jaxis.which);
                    JoyAxis *axis = joy->getJoyAxis(event.jaxis.axis);
                    axis->joyEvent(event.jaxis.value);
                    break;
                }

                case SDL_JOYHATMOTION: {
                    Joystick *joy = joysticks->value(event.jhat.which);
                    JoyDPad *dpad = joy->getJoyDPad(event.jhat.hat);
                    dpad->joyEvent(event.jhat.value);

                    break;
                }

                default:
                    break;
            }

            QCoreApplication::processEvents();
        }
    }

    closeSDL();

    stopped = false;

    if (joysticks->count() > 0)
    {
        emit complete(joysticks->value(0));
    }

    emit complete();
}

void InputDaemon::refreshJoysticks()
{
    joysticks->clear();

    for (int i=0; i < SDL_NumJoysticks(); i++)
    {
        SDL_Joystick* joystick = SDL_JoystickOpen (i);
        Joystick *curJoystick = new Joystick (joystick, this);
        curJoystick->refreshAxes();
        curJoystick->refreshHats();
        curJoystick->refreshButtons();

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
}

void InputDaemon::refresh()
{
    stop();
    // Event loop should be reached after old instance of run has finished
    QTimer::singleShot(0, this, SLOT(run()));
}

bool InputDaemon::isRunning()
{
    return stopped;
}

void InputDaemon::refreshJoystick(Joystick *joystick)
{
    joystick->refreshAxes();
    joystick->refreshHats();
    joystick->refreshButtons();

    emit joystickRefreshed(joystick);
}
