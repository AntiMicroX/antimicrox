/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//#include <QDebug>
#include <QStringListIterator>
#include <QVariant>
#include <QSettings>
#include <QMapIterator>

//#include "logger.h"
#include "sdleventreader.h"

SDLEventReader::SDLEventReader(QMap<SDL_JoystickID, InputDevice *> *joysticks,
                               AntiMicroSettings *settings, QObject *parent) :
    QObject(parent)
{
    this->joysticks = joysticks;
    this->settings = settings;
    settings->getLock()->lock();
    this->pollRate = settings->value("GamepadPollRate",
                                     AntiMicroSettings::defaultSDLGamepadPollRate).toUInt();
    settings->getLock()->unlock();

    pollRateTimer.setParent(this);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    pollRateTimer.setTimerType(Qt::PreciseTimer);
#endif

    initSDL();

    connect(&pollRateTimer, SIGNAL(timeout()), this, SLOT(performWork()));
}

SDLEventReader::~SDLEventReader()
{
    if (sdlIsOpen)
    {
        closeSDL();
    }
}

void SDLEventReader::initSDL()
{
#ifdef USE_SDL_2
    // SDL_INIT_GAMECONTROLLER should automatically initialize SDL_INIT_JOYSTICK
    // but it doesn't seem to be the case with v2.0.4
    SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK);
#else
    // Video support is required to use event system in SDL 1.2.
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
#endif

    SDL_JoystickEventState(SDL_ENABLE);
    sdlIsOpen = true;

#ifdef USE_SDL_2
    //QSettings settings(PadderCommon::configFilePath, QSettings::IniFormat);
    settings->getLock()->lock();
    settings->beginGroup("Mappings");
    QStringList mappings = settings->allKeys();
    QStringListIterator iter(mappings);
    while (iter.hasNext())
    {
        QString tempstring = iter.next();
        QString mappingSetting = settings->value(tempstring, QString()).toString();
        if (!mappingSetting.isEmpty())
        {
            QByteArray temparray = mappingSetting.toUtf8();
            char *mapping = temparray.data();
            SDL_GameControllerAddMapping(mapping); // Let SDL take care of validation
        }
    }

    settings->endGroup();
    settings->getLock()->unlock();

    //SDL_GameControllerAddMapping("03000000100800000100000010010000,Twin USB Joystick,a:b2,b:b1,x:b3,y:b0,back:b8,start:b9,leftshoulder:b6,rightshoulder:b7,leftstick:b10,rightstick:b11,leftx:a0,lefty:a1,rightx:a3,righty:a2,lefttrigger:b4,righttrigger:b5,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2");
#endif

    pollRateTimer.stop();
    pollRateTimer.setInterval(pollRate);
    //pollRateTimer.start();
    //pollRateTimer.setSingleShot(true);

    emit sdlStarted();
}

void SDLEventReader::closeSDL()
{
    pollRateTimer.stop();

    SDL_Event event;

    closeDevices();

    // Clear any pending events
    while (SDL_PollEvent(&event) > 0)
    {
    }
    SDL_Quit();

    sdlIsOpen = false;

    emit sdlClosed();
}

void SDLEventReader::performWork()
{
    if (sdlIsOpen)
    {
        //int status = SDL_WaitEvent(NULL);
        int status = CheckForEvents();

        if (status)
        {
            pollRateTimer.stop();
            emit eventRaised();
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

    pollRateTimer.stop();
}

void SDLEventReader::refresh()
{
    if (sdlIsOpen)
    {
        stop();

        QTimer::singleShot(0, this, SLOT(secondaryRefresh()));
    }
}

void SDLEventReader::secondaryRefresh()
{
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

int SDLEventReader::CheckForEvents()
{
    int result = 0;
    bool exit = false;

    /*Logger::LogInfo(
                                QString("Gamepad Poll %1").arg(
                                    QTime::currentTime().toString("hh:mm:ss.zzz")),
                                true, true);
    */

    SDL_PumpEvents();
    #ifdef USE_SDL_2
    switch (SDL_PeepEvents(NULL, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT))
    #else
    switch (SDL_PeepEvents(NULL, 1, SDL_GETEVENT, 0xFFFF))
    #endif
    {
        case -1:
        {
	  Logger::LogError(QString("SDL Error: %1").
			   arg(QString(SDL_GetError())),
			   true, true);
            result = 0;
            exit = true;
            break;
        }
        case 0:
        {
            if (!pollRateTimer.isActive())
            {
                pollRateTimer.start();
            }
            //exit = true;
            //SDL_Delay(10);
            break;
        }
        default:
        {
            /*Logger::LogInfo(
                        QString("Gamepad Poll %1").arg(
                            QTime::currentTime().toString("hh:mm:ss.zzz")),
                        true, true);
            */

            result = 1;
            exit = true;
            break;
        }
    }

    return result;
}

void SDLEventReader::updatePollRate(unsigned int tempPollRate)
{
    if (tempPollRate >= 1 && tempPollRate <= 16)
    {
        bool wasActive = pollRateTimer.isActive();
        pollRateTimer.stop();

        this->pollRate = tempPollRate;
        pollRateTimer.setInterval(pollRate);

        if (wasActive)
        {
            pollRateTimer.start();
        }
    }
}

void SDLEventReader::resetJoystickMap()
{
    joysticks = 0;
}

void SDLEventReader::quit()
{
    if (sdlIsOpen)
    {
        closeSDL();
        joysticks = 0;
    }
}

void SDLEventReader::closeDevices()
{
    if (sdlIsOpen)
    {
        if (joysticks)
        {
            QMapIterator<SDL_JoystickID, InputDevice*> iter(*joysticks);
            while (iter.hasNext())
            {
                iter.next();
                InputDevice *current = iter.value();
                current->closeSDLDevice();
            }
        }
    }
}

/**
 * @brief Method to block activity on the SDLEventReader object and its thread
 *   event loop.
 */
void SDLEventReader::haltServices()
{
    PadderCommon::lockInputDevices();
    PadderCommon::unlockInputDevices();
}
