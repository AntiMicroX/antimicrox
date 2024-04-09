/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
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

#include "antimicrosettings.h"
#include "common.h"
#include "globalvariables.h"
#include "inputdevice.h"
//#include "logger.h"

#include <SDL2/SDL.h>

#include <QDebug>
#include <QMapIterator>
#include <QSettings>
#include <QVariant>

SDLEventReader::SDLEventReader(QMap<SDL_JoystickID, InputDevice *> *joysticks, AntiMicroSettings *settings, QObject *parent)
    : QObject(parent)
{
    this->joysticks = joysticks;
    this->settings = settings;
    settings->getLock()->lock();
    this->pollRate =
        settings->value("GamepadPollRate", GlobalVariables::AntimicroSettings::defaultSDLGamepadPollRate).toUInt();
    settings->getLock()->unlock();

    pollRateTimer.setParent(this);
    pollRateTimer.setTimerType(Qt::PreciseTimer);

    initSDL();

    connect(&pollRateTimer, &QTimer::timeout, this, &SDLEventReader::performWork);
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
    // SDL_INIT_GAMECONTROLLER should automatically initialize SDL_INIT_JOYSTICK
    // but it doesn't seem to be the case with v2.0.4
    // Passing SDL_INIT_SENSOR here triggers bug libsdl-org/SDL#4276 on windows
    // with v2.0.20. However, sensors works without in Linux and Windows so
    // skip it.
    //#if SDL_VERSION_ATLEAST(2, 0, 14)
    //    SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK | SDL_INIT_SENSOR);
    //#else
    SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK);
    //#endif
    SDL_JoystickEventState(SDL_ENABLE);

    sdlIsOpen = true;
    loadSdlMappingsFromDatabase();
    settings->getLock()->lock();
    settings->beginGroup("Mappings");
    QStringList mappings = settings->allKeys();

    for (auto &&tempstring : mappings)
    {
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
    if (sdlIsOpen && (eventStatus() > 0))
    {
        pollRateTimer.stop();
        emit eventRaised();
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
        closeSDL();

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

bool SDLEventReader::isSDLOpen() { return sdlIsOpen; }

int SDLEventReader::eventStatus()
{
    int result = 0;

    SDL_PumpEvents();

    switch (SDL_PeepEvents(nullptr, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT))
    {
    case -1: {
        qCritical() << QString("SDL Error: %1").arg(QString(SDL_GetError()));
        result = 0;

        break;
    }
    case 0: {
        if (!pollRateTimer.isActive())
            pollRateTimer.start();

        break;
    }
    default: {
        result = 1;
        break;
    }
    }

    return result;
}

void SDLEventReader::updatePollRate(int tempPollRate)
{
    if ((tempPollRate >= 1) && (tempPollRate <= 16))
    {
        bool pollTimerWasActive = pollRateTimer.isActive();
        pollRateTimer.stop();

        this->pollRate = tempPollRate;
        pollRateTimer.setInterval(pollRate);

        if (pollTimerWasActive)
            pollRateTimer.start();
    }
}

void SDLEventReader::resetJoystickMap() { joysticks = nullptr; }

void SDLEventReader::quit()
{
    if (sdlIsOpen)
    {
        closeSDL();
        joysticks = nullptr;
    }
}

void SDLEventReader::closeDevices()
{
    if (sdlIsOpen && (joysticks != nullptr))
    {
        QMapIterator<SDL_JoystickID, InputDevice *> iter(*joysticks);

        while (iter.hasNext())
        {
            iter.next();
            InputDevice *current = iter.value();
            current->closeSDLDevice();
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

/**
 * @brief Loading additional gamepad mappings from database
 *
 */
void SDLEventReader::loadSdlMappingsFromDatabase()
{
    QString database_file;
    database_file = QApplication::applicationDirPath().append("/../share/antimicrox/gamecontrollerdb.txt");
    if (QFile::exists(database_file))
    {
        int result = SDL_GameControllerAddMappingsFromFile(database_file.toStdString().c_str());
        if (result == -1)
            qWarning() << "Loading game controller mappings from database: " << database_file << " failed";
        else
            DEBUG() << "Loaded " << result << " game controller mappings from database";
    }
#ifndef QT_DEBUG
    else
    {
        qWarning() << "File with game controller mappings " << database_file << " does not exist";
    }
#endif
}

QMap<SDL_JoystickID, InputDevice *> *SDLEventReader::getJoysticks() const { return joysticks; }

AntiMicroSettings *SDLEventReader::getSettings() const { return settings; }

QTimer const &SDLEventReader::getPollRateTimer() { return pollRateTimer; }
