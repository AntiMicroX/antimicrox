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

#ifndef INPUTDAEMONTHREAD_H
#define INPUTDAEMONTHREAD_H

#include "gamecontroller/gamecontroller.h"
//#include "fakeclasses/xbox360wireless.h"
#include <SDL2/SDL_events.h>

class InputDevice;
class AntiMicroSettings;
class InputDeviceBitArrayStatus;
class Joystick;
class GameController;
class SDLEventReader;
class QThread;

/**
 * @brief Fetches hardware events from SDL and dispatch them to
 *  input objects like JoyAxis or JoyButton.
 *  Runs in a separate thread.
 */
class InputDaemon : public QObject
{
    Q_OBJECT

  public:
    explicit InputDaemon(QMap<SDL_JoystickID, InputDevice *> *joysticks, AntiMicroSettings *settings, bool graphical = true,
                         QObject *parent = 0);
    ~InputDaemon();

  protected:
    InputDeviceBitArrayStatus *createOrGrabBitStatusEntry(QHash<InputDevice *, InputDeviceBitArrayStatus *> *statusHash,
                                                          InputDevice *device, bool readCurrent = true);

    QString getJoyInfo(SDL_JoystickGUID sdlvalue);
    QString getJoyInfo(Uint16 sdlvalue);

    void firstInputPass(QQueue<SDL_Event> *sdlEventQueue);
    void secondInputPass(QQueue<SDL_Event> *sdlEventQueue);
    void modifyUnplugEvents(QQueue<SDL_Event> *sdlEventQueue);
    QBitArray createUnplugEventBitArray(InputDevice *device);
    Joystick *openJoystickDevice(int index);

    void clearBitArrayStatusInstances();
    void convertMappingsToUnique(QSettings *sett, QString guidString, QString uniqueIdString);

  signals:
    void joystickRefreshed(InputDevice *joystick);
    void joysticksRefreshed(QMap<SDL_JoystickID, InputDevice *> *joysticks);
    void complete(InputDevice *joystick);
    void complete();

    void deviceUpdated(int index, InputDevice *device);
    void deviceRemoved(SDL_JoystickID deviceID);
    void deviceAdded(InputDevice *device);

  public slots:
    void run();
    void quit();
    void refresh();
    void refreshJoystick(InputDevice *joystick);
    void refreshJoysticks();
    void startWorker();
    void refreshMapping(QString mapping, InputDevice *device);
    void removeDevice(InputDevice *device);
    void addInputDevice(int index, QMap<QString, int> &uniques, int &counterUniques, bool &duplicatedGamepad);
    void refreshIndexes();

  private slots:
    void stop();
    void resetActiveButtonMouseDistances();
    void updatePollResetRate(int tempPollRate);

  private:
    QHash<SDL_JoystickID, Joystick *> &getTrackjoysticksLocal();
    QHash<InputDevice *, InputDeviceBitArrayStatus *> &getReleaseEventsGeneratedLocal();
    QHash<InputDevice *, InputDeviceBitArrayStatus *> &getPendingEventValuesLocal();

    QMap<SDL_JoystickID, InputDevice *> *m_joysticks;
    QHash<SDL_JoystickID, Joystick *> trackjoysticks;
    QHash<SDL_JoystickID, GameController *> trackcontrollers;

    QHash<InputDevice *, InputDeviceBitArrayStatus *> releaseEventsGenerated;
    QHash<InputDevice *, InputDeviceBitArrayStatus *> pendingEventValues;

    bool stopped;
    bool m_graphical;

    SDLEventReader *eventWorker;
    QThread *sdlWorkerThread;
    AntiMicroSettings *m_settings;
    QTimer pollResetTimer;
    // SDL_Joystick* xbox360;
};

#endif // INPUTDAEMONTHREAD_H
