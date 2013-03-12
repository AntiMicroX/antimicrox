#include "sdleventreader.h"

SDLEventReader::SDLEventReader(QHash<int, Joystick*> *joysticks, QObject *parent) :
    QObject(parent)
{
    this->joysticks = joysticks;
    initSDL();
}

SDLEventReader::~SDLEventReader()
{
    closeSDL();
}

void SDLEventReader::initSDL()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    SDL_JoystickEventState(SDL_ENABLE);
    sdlIsOpen = true;
}

void SDLEventReader::closeSDL()
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

    sdlIsOpen = false;
}

void SDLEventReader::performWork()
{
    SDL_Event event;
    int status = SDL_WaitEvent(&event);
    if (status)
    {
        currentEvent = event;
        emit eventRaised();
    }
}

void SDLEventReader::stop()
{
    SDL_Event event;
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
}

SDL_Event& SDLEventReader::getCurrentEvent()
{
    return currentEvent;
}

void SDLEventReader::refresh()
{
    if (sdlIsOpen)
    {
        stop();
        closeSDL();
    }

    initSDL();
}
