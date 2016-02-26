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

#ifndef INPUTDAEMONTHREAD_H
#define INPUTDAEMONTHREAD_H

#include <QHash>
#include <QMap>
#include <QThread>
#include <QQueue>

#ifdef USE_SDL_2
#include <SDL2/SDL_joystick.h>
#include <SDL2/SDL_events.h>
#include "gamecontroller/gamecontroller.h"

#else
#include <SDL/SDL_joystick.h>
#include <SDL/SDL_events.h>

#endif

#include "joystick.h"
#include "sdleventreader.h"
#include "antimicrosettings.h"
#include "inputdevicebitarraystatus.h"


class InputDaemon : public QObject
{
    Q_OBJECT
public:
    explicit InputDaemon (QMap<SDL_JoystickID, InputDevice*> *joysticks,
                          AntiMicroSettings *settings, bool graphical=true,
                          QObject *parent=0);
    ~InputDaemon();

protected:
    InputDeviceBitArrayStatus* createOrGrabBitStatusEntry(
            QHash<InputDevice*, InputDeviceBitArrayStatus*> *statusHash,
            InputDevice *device, bool readCurrent=true);

    void firstInputPass(QQueue<SDL_Event> *sdlEventQueue);
    void secondInputPass(QQueue<SDL_Event> *sdlEventQueue);
#ifdef USE_SDL_2
    void modifyUnplugEvents(QQueue<SDL_Event> *sdlEventQueue);
    QBitArray createUnplugEventBitArray(InputDevice *device);
    Joystick* openJoystickDevice(int index);
#endif

    void clearBitArrayStatusInstances();

    QMap<SDL_JoystickID, InputDevice*> *joysticks;

#ifdef USE_SDL_2
    QHash<SDL_JoystickID, Joystick*> trackjoysticks;
    QHash<SDL_JoystickID, GameController*> trackcontrollers;

#endif

    QHash<InputDevice*, InputDeviceBitArrayStatus*> releaseEventsGenerated;
    QHash<InputDevice*, InputDeviceBitArrayStatus*> pendingEventValues;

    bool stopped;
    bool graphical;

    SDLEventReader *eventWorker;
    QThread *sdlWorkerThread;
    AntiMicroSettings *settings;
    QTimer pollResetTimer;

    static const int GAMECONTROLLERTRIGGERRELEASE;

signals:
    void joystickRefreshed (InputDevice *joystick);
    void joysticksRefreshed(QMap<SDL_JoystickID, InputDevice*> *joysticks);
    void complete(InputDevice *joystick);
    void complete();

#ifdef USE_SDL_2
    void deviceUpdated(int index, InputDevice *device);
    void deviceRemoved(SDL_JoystickID deviceID);
    void deviceAdded(InputDevice *device);
#endif

public slots:
    void run();
    void quit();
    void refresh();
    void refreshJoystick(InputDevice *joystick);
    void refreshJoysticks();
    void deleteJoysticks();
    void startWorker();

#ifdef USE_SDL_2
    void refreshMapping(QString mapping, InputDevice *device);
    void removeDevice(InputDevice *device);
    void addInputDevice(int index);
    void refreshIndexes();

#endif

private slots:
    void stop();
    void resetActiveButtonMouseDistances();
    void updatePollResetRate(unsigned int tempPollRate);
};

#endif // INPUTDAEMONTHREAD_H
