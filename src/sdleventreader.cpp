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

    emit sdlStarted();
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
    if (sdlIsOpen)
    {
        SDL_Event event;

        int status = SDL_WaitEvent(&event);
        if (status)
        {
            currentEvent = event;
            emit eventRaised();
            if (event.type == SDL_QUIT)
            {
                emit finished();
            }
        }
    }
}

void SDLEventReader::stop()
{
    if (sdlIsOpen)
    {
        SDL_Event event;
        event.type = SDL_QUIT;
        SDL_PushEvent(&event);
    }
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
        connect(this, SIGNAL(finished()), this, SLOT(secondaryRefresh()));
    }
}

void SDLEventReader::secondaryRefresh()
{
    disconnect(this, SIGNAL(finished()), 0, 0);

    if (sdlIsOpen)
    {
        closeSDL();
    }

    initSDL();
}

void SDLEventReader::clearEvents()
{
    if (sdlIsOpen)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event) > 0)
        {
        }
    }
}

bool SDLEventReader::isSDLOpen()
{
    return sdlIsOpen;
}
