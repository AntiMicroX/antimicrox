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

#include "sdleventreader.h"

#include "messagehandler.h"
#include "inputdevice.h"
#include "antimicrosettings.h"
#include "common.h"
//#include "logger.h"

#include <QDebug>
#include <QStringListIterator>
#include <QVariant>
#include <QSettings>
#include <QMapIterator>




SDLEventReader::SDLEventReader(QMap<SDL_JoystickID, InputDevice *> *joysticks,
                               AntiMicroSettings *settings, QObject *parent) :
    QObject(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->joysticks = joysticks;
    this->settings = settings;
    settings->getLock()->lock();
    this->pollRate = settings->value("GamepadPollRate",
                                     AntiMicroSettings::defaultSDLGamepadPollRate).toUInt();
    settings->getLock()->unlock();

    pollRateTimer.setParent(this);
    pollRateTimer.setTimerType(Qt::PreciseTimer);


    initSDL();

    connect(&pollRateTimer, &QTimer::timeout, this, &SDLEventReader::performWork);
}

SDLEventReader::~SDLEventReader()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (sdlIsOpen)
    {
        closeSDL();
    }
}

void SDLEventReader::initSDL()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    // SDL_INIT_GAMECONTROLLER should automatically initialize SDL_INIT_JOYSTICK
    // but it doesn't seem to be the case with v2.0.4
    SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK);

    SDL_JoystickEventState(SDL_ENABLE);
    sdlIsOpen = true;

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

    pollRateTimer.stop();
    pollRateTimer.setInterval(pollRate);

    emit sdlStarted();
}

void SDLEventReader::closeSDL()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (sdlIsOpen)
    {
        stop();

        QTimer::singleShot(0, this, SLOT(secondaryRefresh()));
    }
}

void SDLEventReader::secondaryRefresh()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (sdlIsOpen)
    {
        closeSDL();
    }

    initSDL();
}

void SDLEventReader::clearEvents()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return sdlIsOpen;
}

int SDLEventReader::CheckForEvents()
{
    int result = 0;

    SDL_PumpEvents();
    switch (SDL_PeepEvents(nullptr, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT))
    {
        case -1:
        {
	  Logger::LogError(QString("SDL Error: %1").
			   arg(QString(SDL_GetError())),
			   true, true);
            result = 0;

            break;
        }
        case 0:
        {
            if (!pollRateTimer.isActive())
            {
                pollRateTimer.start();
            }

            break;
        }
        default:
        {
            result = 1;
            break;
        }
    }

    return result;
}

void SDLEventReader::updatePollRate(int tempPollRate)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((tempPollRate >= 1) && (tempPollRate <= 16))
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    joysticks = nullptr;
}

void SDLEventReader::quit()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (sdlIsOpen)
    {
        closeSDL();
        joysticks = nullptr;
    }
}

void SDLEventReader::closeDevices()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    PadderCommon::lockInputDevices();
    PadderCommon::unlockInputDevices();
}


QMap<SDL_JoystickID, InputDevice*> *SDLEventReader::getJoysticks() const {

    return joysticks;
}

AntiMicroSettings *SDLEventReader::getSettings() const {

    return settings;
}

QTimer const& SDLEventReader::getPollRateTimer() {

    return pollRateTimer;
}
