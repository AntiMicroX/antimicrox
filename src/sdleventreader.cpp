#include <QStringListIterator>
#include <QDebug>
#include <QVariant>

#include "sdleventreader.h"

SDLEventReader::SDLEventReader(QHash<int, InputDevice *> *joysticks, QObject *parent) :
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
#ifdef USE_SDL_2
    SDL_Init(SDL_INIT_GAMECONTROLLER);
#else
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
#endif
    SDL_JoystickEventState(SDL_ENABLE);
    sdlIsOpen = true;

#ifdef USE_SDL_2
    QSettings settings(PadderCommon::controllerMappingFilePath, QSettings::IniFormat);
    QStringList mappings = settings.allKeys();
    QStringListIterator iter(mappings);
    while (iter.hasNext())
    {
        QString tempstring = iter.next();
        QString mappingSetting = settings.value(tempstring, QString()).toString();
        if (!mappingSetting.isEmpty())
        {
            QByteArray temparray = mappingSetting.toUtf8();
            char *mapping = temparray.data();
            SDL_GameControllerAddMapping(mapping); // Let SDL take care of validation
        }
    }

    //SDL_GameControllerAddMapping("03000000100800000100000010010000,Twin USB Joystick,a:b2,b:b1,x:b3,y:b0,back:b8,start:b9,leftshoulder:b6,rightshoulder:b7,leftstick:b10,rightstick:b11,leftx:a0,lefty:a1,rightx:a3,righty:a2,lefttrigger:b4,righttrigger:b5,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2");
#endif

    emit sdlStarted();
}

void SDLEventReader::closeSDL()
{
    SDL_Event event;
#ifdef USE_SDL_2
    QHashIterator<SDL_JoystickID, InputDevice*> iter(*joysticks);
    while (iter.hasNext())
    {
        iter.next();

        //SDL_JoystickID currentSdlId = iter.key();
        InputDevice *current = iter.value();
        //SDL_Joystick *sdlHandle = current->getSDLHandle();

        /*if (sdlHandle && SDL_JoystickGetAttached(sdlHandle))
        {
            SDL_JoystickClose(sdlHandle);
        }*/
        current->closeSDLDevice();
    }
#else
    QHashIterator<int, InputDevice*> iter(*joysticks);
    while (iter.hasNext())
    {
        iter.next();
        InputDevice *current = iter.value();
        current->closeSDLDevice();
    }
    /*for (int i=0; i < SDL_NumJoysticks(); i++)
    {

        if (SDL_JoystickOpened(i) && joysticks->value(i))
        {
            SDL_Joystick *handle = (joysticks->value(i))->getSDLHandle();
            SDL_JoystickClose(handle);
        }
    }*/
#endif

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
