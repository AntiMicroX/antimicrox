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
#include <QTime>
#include <QTimer>
#include <QEventLoop>
#include <QMapIterator>

#include "inputdaemon.h"
#include "logger.h"
#include "common.h"

//#define USE_NEW_ADD
#define USE_NEW_REFRESH

const int InputDaemon::GAMECONTROLLERTRIGGERRELEASE = 16384;

InputDaemon::InputDaemon(QMap<SDL_JoystickID, InputDevice*> *joysticks,
                         AntiMicroSettings *settings,
                         bool graphical, QObject *parent) :
    QObject(parent),
    pollResetTimer(this)
{
    this->joysticks = joysticks;
    this->stopped = false;
    this->graphical = graphical;
    this->settings = settings;

    eventWorker = new SDLEventReader(joysticks, settings);
    refreshJoysticks();

    sdlWorkerThread = 0;
    if (graphical)
    {
        sdlWorkerThread = new QThread();
        eventWorker->moveToThread(sdlWorkerThread);
    }

    if (graphical)
    {
        connect(sdlWorkerThread, SIGNAL(started()), eventWorker, SLOT(performWork()));
        connect(eventWorker, SIGNAL(eventRaised()), this, SLOT(run()));

        connect(JoyButton::getMouseHelper(), SIGNAL(gamepadRefreshRateUpdated(uint)),
                eventWorker, SLOT(updatePollRate(uint)));

        connect(JoyButton::getMouseHelper(), SIGNAL(gamepadRefreshRateUpdated(uint)),
                this, SLOT(updatePollResetRate(uint)));
        connect(JoyButton::getMouseHelper(), SIGNAL(mouseRefreshRateUpdated(uint)),
                this, SLOT(updatePollResetRate(uint)));

        // Timer in case SDL does not produce an axis event during a joystick
        // poll.
        pollResetTimer.setSingleShot(true);
        pollResetTimer.setInterval(
                    qMax(JoyButton::getMouseRefreshRate(),
                         JoyButton::getGamepadRefreshRate()) + 1);

        connect(&pollResetTimer, SIGNAL(timeout()), this,
                SLOT(resetActiveButtonMouseDistances()));

        //sdlWorkerThread->start(QThread::HighPriority);
        //QMetaObject::invokeMethod(eventWorker, "performWork", Qt::QueuedConnection);
    }
}

InputDaemon::~InputDaemon()
{
    if (eventWorker)
    {
        quit();
    }

    if (sdlWorkerThread)
    {
        sdlWorkerThread->quit();
        sdlWorkerThread->wait();
        delete sdlWorkerThread;
        sdlWorkerThread = 0;
    }
}

void InputDaemon::startWorker()
{
    if (!sdlWorkerThread->isRunning())
    {
        sdlWorkerThread->start(QThread::HighPriority);
        //pollResetTimer.start();
    }
}

void InputDaemon::run ()
{
    PadderCommon::inputDaemonMutex.lock();

    // SDL has found events. The timeout is not necessary.
    pollResetTimer.stop();

    if (!stopped)
    {
        //Logger::LogInfo(QString("Gamepad Poll %1").arg(QTime::currentTime().toString("hh:mm:ss.zzz")));
        JoyButton::resetActiveButtonMouseDistances();

        QQueue<SDL_Event> sdlEventQueue;

        firstInputPass(&sdlEventQueue);

#ifdef USE_SDL_2
        modifyUnplugEvents(&sdlEventQueue);
#endif

        secondInputPass(&sdlEventQueue);

        clearBitArrayStatusInstances();
    }

    if (stopped)
    {
        if (joysticks->size() > 0)
        {
            emit complete(joysticks->value(0));
        }
        emit complete();
        stopped = false;
    }
    else
    {
        QTimer::singleShot(0, eventWorker, SLOT(performWork()));
        pollResetTimer.start();
    }

    PadderCommon::inputDaemonMutex.unlock();
}

void InputDaemon::refreshJoysticks()
{
    QMapIterator<SDL_JoystickID, InputDevice*> iter(*joysticks);

    while (iter.hasNext())
    {
        InputDevice *joystick = iter.next().value();
        if (joystick)
        {
            delete joystick;
            joystick = 0;
        }
    }

    joysticks->clear();
#ifdef USE_SDL_2
    trackjoysticks.clear();
    trackcontrollers.clear();

    settings->getLock()->lock();
    settings->beginGroup("Mappings");
#endif

    for (int i=0; i < SDL_NumJoysticks(); i++)
    {
#ifdef USE_SDL_2
#ifdef USE_NEW_REFRESH
        int index = i;

        // Check if device is considered a Game Controller at the start.
        if (SDL_IsGameController(index))
        {
            SDL_GameController *controller = SDL_GameControllerOpen(index);
            if (controller)
            {
                SDL_Joystick *sdlStick = SDL_GameControllerGetJoystick(controller);
                SDL_JoystickID tempJoystickID = SDL_JoystickInstanceID(sdlStick);

                // Check if device has already been grabbed.
                if (!joysticks->contains(tempJoystickID))
                {
                    //settings->getLock()->lock();
                    //settings->beginGroup("Mappings");

                    QString temp;
                    SDL_JoystickGUID tempGUID = SDL_JoystickGetGUID(sdlStick);
                    char guidString[65] = {'0'};
                    SDL_JoystickGetGUIDString(tempGUID, guidString, sizeof(guidString));
                    temp = QString(guidString);

                    bool disableGameController = settings->value(QString("%1Disable").arg(temp), false).toBool();

                    //settings->endGroup();
                    //settings->getLock()->unlock();

                    // Check if user has designated device Joystick mode.
                    if (!disableGameController)
                    {
                        GameController *damncontroller = new GameController(controller, index, settings, this);
                        connect(damncontroller, SIGNAL(requestWait()), eventWorker, SLOT(haltServices()));
                        joysticks->insert(tempJoystickID, damncontroller);
                        trackcontrollers.insert(tempJoystickID, damncontroller);

                        emit deviceAdded(damncontroller);
                    }
                    else
                    {
                        // Check if joystick is considered connected.
                        /*SDL_Joystick *joystick = SDL_JoystickOpen(index);
                        if (joystick)
                        {
                            SDL_JoystickID tempJoystickID = SDL_JoystickInstanceID(joystick);

                            Joystick *curJoystick = new Joystick(joystick, index, settings, this);
                            joysticks->insert(tempJoystickID, curJoystick);
                            trackjoysticks.insert(tempJoystickID, curJoystick);

                            emit deviceAdded(curJoystick);
                        }
                        */
                        Joystick *joystick = openJoystickDevice(index);
                        if (joystick)
                        {
                            emit deviceAdded(joystick);
                        }

                    }
                }
                else
                {
                    // Make sure to decrement reference count
                    SDL_GameControllerClose(controller);
                }
            }
        }
        else
        {
            // Check if joystick is considered connected.
            /*SDL_Joystick *joystick = SDL_JoystickOpen(index);
            if (joystick)
            {
                SDL_JoystickID tempJoystickID = SDL_JoystickInstanceID(joystick);

                Joystick *curJoystick = new Joystick(joystick, index, settings, this);
                joysticks->insert(tempJoystickID, curJoystick);
                trackjoysticks.insert(tempJoystickID, curJoystick);

                emit deviceAdded(curJoystick);
            }
            */
            Joystick *joystick = openJoystickDevice(index);
            if (joystick)
            {
                emit deviceAdded(joystick);
            }
        }


#else
        SDL_Joystick *joystick = SDL_JoystickOpen(i);
        if (joystick)
        {
            QString temp;
            SDL_JoystickGUID tempGUID = SDL_JoystickGetGUID(joystick);
            char guidString[65] = {'0'};
            SDL_JoystickGetGUIDString(tempGUID, guidString, sizeof(guidString));
            temp = QString(guidString);

            bool disableGameController = settings->value(QString("%1Disable").arg(temp), false).toBool();

            if (SDL_IsGameController(i) && !disableGameController)
            {
                SDL_GameController *controller = SDL_GameControllerOpen(i);
                GameController *damncontroller = new GameController(controller, i, settings, this);
                connect(damncontroller, SIGNAL(requestWait()), eventWorker, SLOT(haltServices()));
                SDL_Joystick *sdlStick = SDL_GameControllerGetJoystick(controller);
                SDL_JoystickID joystickID = SDL_JoystickInstanceID(sdlStick);
                joysticks->insert(joystickID, damncontroller);
                trackcontrollers.insert(joystickID, damncontroller);
            }
            else
            {
                Joystick *curJoystick = new Joystick(joystick, i, settings, this);
                connect(curJoystick, SIGNAL(requestWait()), eventWorker, SLOT(haltServices()));
                SDL_JoystickID joystickID = SDL_JoystickInstanceID(joystick);
                joysticks->insert(joystickID, curJoystick);
                trackjoysticks.insert(joystickID, curJoystick);
            }
        }
#endif

#else
        SDL_Joystick *joystick = SDL_JoystickOpen(i);
        if (joystick)
        {
            Joystick *curJoystick = new Joystick(joystick, i, settings, this);
            connect(curJoystick, SIGNAL(requestWait()), eventWorker, SLOT(haltServices()));
            joysticks->insert(i, curJoystick);
        }

#endif
    }

#ifdef USE_SDL_2
    settings->endGroup();
    settings->getLock()->unlock();
#endif

    emit joysticksRefreshed(joysticks);
}

void InputDaemon::deleteJoysticks()
{
    QMapIterator<SDL_JoystickID, InputDevice*> iter(*joysticks);

    while (iter.hasNext())
    {
        InputDevice *joystick = iter.next().value();
        if (joystick)
        {
            delete joystick;
            joystick = 0;
        }
    }

    joysticks->clear();
#ifdef USE_SDL_2
    trackjoysticks.clear();
    trackcontrollers.clear();
#endif
}

void InputDaemon::stop()
{
    stopped = true;
    pollResetTimer.stop();
}

void InputDaemon::refresh()
{
    stop();

    Logger::LogInfo("Refreshing joystick list");

    QEventLoop q;
    connect(eventWorker, SIGNAL(sdlStarted()), &q, SLOT(quit()));
    QMetaObject::invokeMethod(eventWorker, "refresh", Qt::BlockingQueuedConnection);

    if (eventWorker->isSDLOpen())
    {
        q.exec();
    }

    disconnect(eventWorker, SIGNAL(sdlStarted()), &q, SLOT(quit()));

    pollResetTimer.stop();

    // Put in an extra delay before refreshing the joysticks
    QTimer temp;
    connect(&temp, SIGNAL(timeout()), &q, SLOT(quit()));
    temp.start(100);
    q.exec();

    refreshJoysticks();
    QTimer::singleShot(100, eventWorker, SLOT(performWork()));

    stopped = false;
}

void InputDaemon::refreshJoystick(InputDevice *joystick)
{
    joystick->reset();

    emit joystickRefreshed(joystick);
}

void InputDaemon::quit()
{
    stopped = true;
    pollResetTimer.stop();

    disconnect(eventWorker, SIGNAL(eventRaised()), this, 0);

    // Wait for SDL to finish. Let worker destructor close SDL.
    // Let InputDaemon destructor close thread instance.
    if (graphical)
    {
        QMetaObject::invokeMethod(eventWorker, "stop");
        QMetaObject::invokeMethod(eventWorker, "quit");
        QMetaObject::invokeMethod(eventWorker, "deleteLater", Qt::BlockingQueuedConnection);
        //QMetaObject::invokeMethod(eventWorker, "deleteLater");
    }
    else
    {
        eventWorker->stop();
        eventWorker->quit();
        delete eventWorker;
    }

    eventWorker = 0;
}

#ifdef USE_SDL_2
void InputDaemon::refreshMapping(QString mapping, InputDevice *device)
{
    bool found = false;

    for (int i=0; i < SDL_NumJoysticks() && !found; i++)
    {
        SDL_Joystick *joystick = SDL_JoystickOpen(i);
        SDL_JoystickID joystickID = SDL_JoystickInstanceID(joystick);

        if (device->getSDLJoystickID() == joystickID)
        {
            found = true;

            if (SDL_IsGameController(i))
            {
                // Mapping string updated. Perform basic refresh
                QByteArray tempbarray = mapping.toUtf8();
                SDL_GameControllerAddMapping(tempbarray.data());
            }
            else
            {
                // Previously registered as a plain joystick. Add
                // mapping and check for validity. If SDL accepts it,
                // close current device and re-open as
                // a game controller.
                SDL_GameControllerAddMapping(mapping.toUtf8().constData());

                if (SDL_IsGameController(i))
                {
                    device->closeSDLDevice();
                    trackjoysticks.remove(joystickID);
                    joysticks->remove(joystickID);

                    SDL_GameController *controller = SDL_GameControllerOpen(i);
                    GameController *damncontroller = new GameController(controller, i, settings, this);
                    connect(damncontroller, SIGNAL(requestWait()), eventWorker, SLOT(haltServices()));
                    SDL_Joystick *sdlStick = SDL_GameControllerGetJoystick(controller);
                    joystickID = SDL_JoystickInstanceID(sdlStick);
                    joysticks->insert(joystickID, damncontroller);
                    trackcontrollers.insert(joystickID, damncontroller);
                    emit deviceUpdated(i, damncontroller);
                }
            }
        }

        // Make sure to decrement reference count
        SDL_JoystickClose(joystick);
    }
}

void InputDaemon::removeDevice(InputDevice *device)
{
    if (device)
    {
        SDL_JoystickID deviceID = device->getSDLJoystickID();

        joysticks->remove(deviceID);
        trackjoysticks.remove(deviceID);
        trackcontrollers.remove(deviceID);

        refreshIndexes();

        emit deviceRemoved(deviceID);
    }
}

void InputDaemon::refreshIndexes()
{
    for (int i = 0; i < SDL_NumJoysticks(); i++)
    {
        SDL_Joystick *joystick = SDL_JoystickOpen(i);
        SDL_JoystickID joystickID = SDL_JoystickInstanceID(joystick);
        // Make sure to decrement reference count
        SDL_JoystickClose(joystick);

        InputDevice *tempdevice = joysticks->value(joystickID);
        if (tempdevice)
        {
            tempdevice->setIndex(i);
        }
    }
}

void InputDaemon::addInputDevice(int index)
{
  #ifdef USE_NEW_ADD
    // Check if device is considered a Game Controller at the start.
    if (SDL_IsGameController(index))
    {
        SDL_GameController *controller = SDL_GameControllerOpen(index);
        if (controller)
        {
            SDL_Joystick *sdlStick = SDL_GameControllerGetJoystick(controller);
            SDL_JoystickID tempJoystickID = SDL_JoystickInstanceID(sdlStick);

            // Check if device has already been grabbed.
            if (!joysticks->contains(tempJoystickID))
            {
                settings->getLock()->lock();
                settings->beginGroup("Mappings");

                QString temp;
                SDL_JoystickGUID tempGUID = SDL_JoystickGetGUID(sdlStick);
                char guidString[65] = {'0'};
                SDL_JoystickGetGUIDString(tempGUID, guidString, sizeof(guidString));
                temp = QString(guidString);

                bool disableGameController = settings->value(QString("%1Disable").arg(temp), false).toBool();

                settings->endGroup();
                settings->getLock()->unlock();

                // Check if user has designated device Joystick mode.
                if (!disableGameController)
                {
                    GameController *damncontroller = new GameController(controller, index, settings, this);
                    connect(damncontroller, SIGNAL(requestWait()), eventWorker, SLOT(haltServices()));
                    joysticks->insert(tempJoystickID, damncontroller);
                    trackcontrollers.insert(tempJoystickID, damncontroller);

                    Logger::LogInfo(QString("New game controller found - #%1 [%2]")
                                    .arg(index+1)
                                    .arg(QTime::currentTime().toString("hh:mm:ss.zzz")));

                    emit deviceAdded(damncontroller);
                }
                else
                {
                    // Check if joystick is considered connected.
                    /*SDL_Joystick *joystick = SDL_JoystickOpen(index);
                    if (joystick)
                    {
                        SDL_JoystickID tempJoystickID = SDL_JoystickInstanceID(joystick);

                        Joystick *curJoystick = new Joystick(joystick, index, settings, this);
                        joysticks->insert(tempJoystickID, curJoystick);
                        trackjoysticks.insert(tempJoystickID, curJoystick);

                        emit deviceAdded(curJoystick);
                    }
                    */
                    Joystick *joystick = openJoystickDevice(index);
                    if (joystick)
                    {
                        Logger::LogInfo(QString("New joystick found - #%1 [%2]")
                                        .arg(index+1)
                                        .arg(QTime::currentTime().toString("hh:mm:ss.zzz")));

                        emit deviceAdded(joystick);
                    }

                }
            }
            else
            {
                // Make sure to decrement reference count
                SDL_GameControllerClose(controller);
            }
        }
    }
    else
    {
        // Check if joystick is considered connected.
        /*SDL_Joystick *joystick = SDL_JoystickOpen(index);
        if (joystick)
        {
            SDL_JoystickID tempJoystickID = SDL_JoystickInstanceID(joystick);

            Joystick *curJoystick = new Joystick(joystick, index, settings, this);
            joysticks->insert(tempJoystickID, curJoystick);
            trackjoysticks.insert(tempJoystickID, curJoystick);

            emit deviceAdded(curJoystick);
        }
        */
        Joystick *joystick = openJoystickDevice(index);
        if (joystick)
        {
            Logger::LogInfo(QString("New joystick found - #%1 [%2]")
                            .arg(index+1)
                            .arg(QTime::currentTime().toString("hh:mm:ss.zzz")));

            emit deviceAdded(joystick);
        }
    }
#else
    SDL_Joystick *joystick = SDL_JoystickOpen(index);
    if (joystick)
    {
        SDL_JoystickID tempJoystickID = SDL_JoystickInstanceID(joystick);

        if (!joysticks->contains(tempJoystickID))
        {
            settings->getLock()->lock();
            settings->beginGroup("Mappings");

            QString temp;
            SDL_JoystickGUID tempGUID = SDL_JoystickGetGUID(joystick);
            char guidString[65] = {'0'};
            SDL_JoystickGetGUIDString(tempGUID, guidString, sizeof(guidString));
            temp = QString(guidString);

            bool disableGameController = settings->value(QString("%1Disable").arg(temp), false).toBool();

            if (SDL_IsGameController(index) && !disableGameController)
            {
                // Make sure to decrement reference count
                SDL_JoystickClose(joystick);

                SDL_GameController *controller = SDL_GameControllerOpen(index);
                if (controller)
                {
                    SDL_Joystick *sdlStick = SDL_GameControllerGetJoystick(controller);
                    SDL_JoystickID tempJoystickID = SDL_JoystickInstanceID(sdlStick);
                    if (!joysticks->contains(tempJoystickID))
                    {
                        GameController *damncontroller = new GameController(controller, index, settings, this);
                        connect(damncontroller, SIGNAL(requestWait()), eventWorker, SLOT(haltServices()));
                        joysticks->insert(tempJoystickID, damncontroller);
                        trackcontrollers.insert(tempJoystickID, damncontroller);

                        settings->endGroup();
                        settings->getLock()->unlock();

                        emit deviceAdded(damncontroller);
                    }
                }
                else
                {
                    settings->endGroup();
                    settings->getLock()->unlock();
                }
            }
            else
            {
                Joystick *curJoystick = new Joystick(joystick, index, settings, this);
                joysticks->insert(tempJoystickID, curJoystick);
                trackjoysticks.insert(tempJoystickID, curJoystick);

                settings->endGroup();
                settings->getLock()->unlock();

                emit deviceAdded(curJoystick);
            }
        }
        else
        {
            // Make sure to decrement reference count
            SDL_JoystickClose(joystick);
        }
    }
#endif
}

Joystick *InputDaemon::openJoystickDevice(int index)
{
    // Check if joystick is considered connected.
    SDL_Joystick *joystick = SDL_JoystickOpen(index);
    Joystick *curJoystick = 0;
    if (joystick)
    {
        SDL_JoystickID tempJoystickID = SDL_JoystickInstanceID(joystick);

        curJoystick = new Joystick(joystick, index, settings, this);
        joysticks->insert(tempJoystickID, curJoystick);
        trackjoysticks.insert(tempJoystickID, curJoystick);

    }

    return curJoystick;
}

#endif

InputDeviceBitArrayStatus*
InputDaemon::createOrGrabBitStatusEntry(QHash<InputDevice *, InputDeviceBitArrayStatus *> *statusHash,
                                        InputDevice *device, bool readCurrent)
{
    InputDeviceBitArrayStatus *bitArrayStatus = 0;

    if (!statusHash->contains(device))
    {
        bitArrayStatus = new InputDeviceBitArrayStatus(device, readCurrent);
        statusHash->insert(device, bitArrayStatus);
    }
    else
    {
        bitArrayStatus = statusHash->value(device);
    }

    return bitArrayStatus;
}

void InputDaemon::firstInputPass(QQueue<SDL_Event> *sdlEventQueue)
{
    SDL_Event event;

    while (SDL_PollEvent(&event) > 0)
    {
        switch (event.type)
        {
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
            {
#ifdef USE_SDL_2
                InputDevice *joy = trackjoysticks.value(event.jbutton.which);
#else
                InputDevice *joy = joysticks->value(event.jbutton.which);
#endif
                if (joy)
                {
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyButton *button = set->getJoyButton(event.jbutton.button);

                    if (button)
                    {
                        //InputDeviceBitArrayStatus *temp = createOrGrabBitStatusEntry(&releaseEventsGenerated, joy, false);
                        //temp->changeButtonStatus(event.jbutton.button, event.type == SDL_JOYBUTTONUP);

                        InputDeviceBitArrayStatus *pending = createOrGrabBitStatusEntry(&pendingEventValues, joy);
                        pending->changeButtonStatus(event.jbutton.button,
                                                  event.type == SDL_JOYBUTTONDOWN ? true : false);
                        sdlEventQueue->append(event);
                    }
                }
#ifdef USE_SDL_2
                else
                {
                    sdlEventQueue->append(event);
                }
#endif

                break;
            }
            case SDL_JOYAXISMOTION:
            {
#ifdef USE_SDL_2
                InputDevice *joy = trackjoysticks.value(event.jaxis.which);
#else
                InputDevice *joy = joysticks->value(event.jaxis.which);
#endif
                if (joy)
                {
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyAxis *axis = set->getJoyAxis(event.jaxis.axis);

                    if (axis)
                    {
                        InputDeviceBitArrayStatus *temp = createOrGrabBitStatusEntry(&releaseEventsGenerated, joy, false);
                        temp->changeAxesStatus(event.jaxis.axis, event.jaxis.axis == 0);

                        InputDeviceBitArrayStatus *pending = createOrGrabBitStatusEntry(&pendingEventValues, joy);
                        pending->changeAxesStatus(event.jaxis.axis, !axis->inDeadZone(event.jaxis.value));
                        sdlEventQueue->append(event);
                    }
                }
#ifdef USE_SDL_2
                else
                {
                    sdlEventQueue->append(event);
                }
#endif

                break;
            }
            case SDL_JOYHATMOTION:
            {
#ifdef USE_SDL_2
                InputDevice *joy = trackjoysticks.value(event.jhat.which);
#else
                InputDevice *joy = joysticks->value(event.jhat.which);
#endif
                if (joy)
                {
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyDPad *dpad = set->getJoyDPad(event.jhat.hat);

                    if (dpad)
                    {
                        //InputDeviceBitArrayStatus *temp = createOrGrabBitStatusEntry(&releaseEventsGenerated, joy, false);
                        //temp->changeHatStatus(event.jhat.hat, event.jhat.value == 0);

                        InputDeviceBitArrayStatus *pending = createOrGrabBitStatusEntry(&pendingEventValues, joy);
                        pending->changeHatStatus(event.jhat.hat, event.jhat.value != 0 ? true : false);
                        sdlEventQueue->append(event);
                    }
                }
#ifdef USE_SDL_2
                else
                {
                    sdlEventQueue->append(event);
                }
#endif

                break;
            }

#ifdef USE_SDL_2
            case SDL_CONTROLLERAXISMOTION:
            {
                InputDevice *joy = trackcontrollers.value(event.caxis.which);
                if (joy)
                {
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyAxis *axis = set->getJoyAxis(event.caxis.axis);
                    if (axis)
                    {
                        InputDeviceBitArrayStatus *temp = createOrGrabBitStatusEntry(&releaseEventsGenerated, joy, false);
                        if (event.caxis.axis != SDL_CONTROLLER_AXIS_TRIGGERLEFT &&
                            event.caxis.axis != SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
                        {
                            temp->changeAxesStatus(event.caxis.axis, event.caxis.value == 0);
                        }
                        else
                        {
                            temp->changeAxesStatus(event.caxis.axis, event.caxis.value == GAMECONTROLLERTRIGGERRELEASE);
                        }

                        InputDeviceBitArrayStatus *pending = createOrGrabBitStatusEntry(&pendingEventValues, joy);
                        pending->changeAxesStatus(event.caxis.axis, !axis->inDeadZone(event.caxis.value));
                        sdlEventQueue->append(event);
                    }
                }
                break;
            }

            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
            {
                InputDevice *joy = trackcontrollers.value(event.cbutton.which);
                if (joy)
                {
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyButton *button = set->getJoyButton(event.cbutton.button);

                    if (button)
                    {
                        //InputDeviceBitArrayStatus *temp = createOrGrabBitStatusEntry(&releaseEventsGenerated, joy, false);
                        //temp->changeButtonStatus(event.cbutton.button, event.type == SDL_CONTROLLERBUTTONUP);

                        InputDeviceBitArrayStatus *pending = createOrGrabBitStatusEntry(&pendingEventValues, joy);
                        pending->changeButtonStatus(event.cbutton.button,
                                                  event.type == SDL_CONTROLLERBUTTONDOWN ? true : false);
                        sdlEventQueue->append(event);
                    }
                }

                break;
            }
            case SDL_JOYDEVICEREMOVED:
            case SDL_JOYDEVICEADDED:
            {
                sdlEventQueue->append(event);
                break;
            }
#endif
            case SDL_QUIT:
            {
                sdlEventQueue->append(event);
                break;
            }
        }
    }
}

#ifdef USE_SDL_2
void InputDaemon::modifyUnplugEvents(QQueue<SDL_Event> *sdlEventQueue)
{
    QHashIterator<InputDevice*, InputDeviceBitArrayStatus*> genIter(releaseEventsGenerated);
    while (genIter.hasNext())
    {
        genIter.next();
        InputDevice *device = genIter.key();
        InputDeviceBitArrayStatus *generatedTemp = genIter.value();
        QBitArray tempBitArray = generatedTemp->generateFinalBitArray();
        //qDebug() << "ARRAY: " << tempBitArray;

        unsigned int bitArraySize = tempBitArray.size();
        //qDebug() << "ARRAY SIZE: " << bitArraySize;

        if (bitArraySize > 0 && tempBitArray.count(true) == device->getNumberAxes())
        {
            if (pendingEventValues.contains(device))
            {
                InputDeviceBitArrayStatus *pendingTemp = pendingEventValues.value(device);
                QBitArray pendingBitArray = pendingTemp->generateFinalBitArray();
                QBitArray unplugBitArray = createUnplugEventBitArray(device);
                unsigned int pendingBitArraySize = pendingBitArray.size();

                if (bitArraySize == pendingBitArraySize &&
                    pendingBitArray == unplugBitArray)
                {
                    QQueue<SDL_Event> tempQueue;
                    while (!sdlEventQueue->isEmpty())
                    {
                        SDL_Event event = sdlEventQueue->dequeue();
                        switch (event.type)
                        {
                            case SDL_JOYBUTTONDOWN:
                            case SDL_JOYBUTTONUP:
                            {
                                tempQueue.enqueue(event);
                                break;
                            }
                            case SDL_JOYAXISMOTION:
                            {
                                if (event.jaxis.which != device->getSDLJoystickID())
                                {
                                    tempQueue.enqueue(event);
                                }
                                else
                                {
                                    InputDevice *joy = trackjoysticks.value(event.jaxis.which);

                                    if (joy)
                                    {
                                        JoyAxis *axis = joy->getActiveSetJoystick()->getJoyAxis(event.jaxis.axis);
                                        if (axis)
                                        {
                                            if (axis->getThrottle() != JoyAxis::NormalThrottle)
                                            {
                                                event.jaxis.value = axis->getProperReleaseValue();
                                            }
                                        }
                                    }

                                    tempQueue.enqueue(event);
                                }

                                break;
                            }
                            case SDL_JOYHATMOTION:
                            {
                                tempQueue.enqueue(event);
                                break;
                            }
                            case SDL_CONTROLLERAXISMOTION:
                            {
                                if (event.caxis.which != device->getSDLJoystickID())
                                {
                                    tempQueue.enqueue(event);
                                }
                                else
                                {
                                    InputDevice *joy = trackcontrollers.value(event.caxis.which);
                                    if (joy)
                                    {
                                        SetJoystick* set = joy->getActiveSetJoystick();
                                        JoyAxis *axis = set->getJoyAxis(event.caxis.axis);
                                        if (axis)
                                        {
                                            if (event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT ||
                                                event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
                                            {
                                                event.caxis.value = axis->getProperReleaseValue();
                                            }
                                        }
                                    }

                                    tempQueue.enqueue(event);
                                }

                                break;
                            }
                            case SDL_CONTROLLERBUTTONDOWN:
                            case SDL_CONTROLLERBUTTONUP:
                            {

                                tempQueue.enqueue(event);
                                break;
                            }
                            case SDL_JOYDEVICEREMOVED:
                            case SDL_JOYDEVICEADDED:
                            {
                                tempQueue.enqueue(event);
                                break;
                            }
                            default:
                            {
                                tempQueue.enqueue(event);
                            }
                        }
                    }

                    sdlEventQueue->swap(tempQueue);
                }
            }
        }
    }
}
#endif

#ifdef USE_SDL_2
QBitArray InputDaemon::createUnplugEventBitArray(InputDevice *device)
{
    InputDeviceBitArrayStatus tempStatus(device, false);

    for (int i=0; i < device->getNumberRawAxes(); i++)
    {
        JoyAxis *axis = device->getActiveSetJoystick()->getJoyAxis(i);
        if (axis && axis->getThrottle() != JoyAxis::NormalThrottle)
        {
            tempStatus.changeAxesStatus(i, true);
        }
    }

    QBitArray unplugBitArray = tempStatus.generateFinalBitArray();
    return unplugBitArray;
}
#endif

void InputDaemon::secondInputPass(QQueue<SDL_Event> *sdlEventQueue)
{
    QHash<SDL_JoystickID, InputDevice*> activeDevices;

    while (!sdlEventQueue->isEmpty())
    {
        SDL_Event event = sdlEventQueue->dequeue();

        switch (event.type)
        {
            //qDebug() << QTime::currentTime() << " :";
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
            {
#ifdef USE_SDL_2
                InputDevice *joy = trackjoysticks.value(event.jbutton.which);
#else
                InputDevice *joy = joysticks->value(event.jbutton.which);
#endif
                if (joy)
                {
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyButton *button = set->getJoyButton(event.jbutton.button);

                    if (button)
                    {
                        //button->joyEvent(event.type == SDL_JOYBUTTONDOWN ? true : false);
                        button->queuePendingEvent(event.type == SDL_JOYBUTTONDOWN ? true : false);

                        if (!activeDevices.contains(event.jbutton.which))
                        {
                            activeDevices.insert(event.jbutton.which, joy);
                        }
                    }
                }
#ifdef USE_SDL_2
                else if (trackcontrollers.contains(event.jbutton.which))
                {
                    GameController *gamepad = trackcontrollers.value(event.jbutton.which);
                    gamepad->rawButtonEvent(event.jbutton.button, event.type == SDL_JOYBUTTONDOWN ? true : false);
                }
#endif

                break;
            }

            case SDL_JOYAXISMOTION:
            {
#ifdef USE_SDL_2
                InputDevice *joy = trackjoysticks.value(event.jaxis.which);
#else
                InputDevice *joy = joysticks->value(event.jaxis.which);
#endif
                if (joy)
                {
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyAxis *axis = set->getJoyAxis(event.jaxis.axis);
                    if (axis)
                    {
                        //axis->joyEvent(event.jaxis.value);
                        axis->queuePendingEvent(event.jaxis.value);

                        if (!activeDevices.contains(event.jaxis.which))
                        {
                            activeDevices.insert(event.jaxis.which, joy);
                        }
                    }

                    joy->rawAxisEvent(event.jaxis.which, event.jaxis.value);
                }
#ifdef USE_SDL_2
                else if (trackcontrollers.contains(event.jaxis.which))
                {
                    GameController *gamepad = trackcontrollers.value(event.jaxis.which);
                    gamepad->rawAxisEvent(event.jaxis.axis, event.jaxis.value);
                }
#endif

                break;
            }

            case SDL_JOYHATMOTION:
            {
#ifdef USE_SDL_2
                InputDevice *joy = trackjoysticks.value(event.jhat.which);
#else
                InputDevice *joy = joysticks->value(event.jhat.which);
#endif
                if (joy)
                {
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyDPad *dpad = set->getJoyDPad(event.jhat.hat);
                    if (dpad)
                    {
                        //dpad->joyEvent(event.jhat.value);
                        dpad->joyEvent(event.jhat.value);

                        if (!activeDevices.contains(event.jhat.which))
                        {
                            activeDevices.insert(event.jhat.which, joy);
                        }
                    }
                }
#ifdef USE_SDL_2
                else if (trackcontrollers.contains(event.jhat.which))
                {
                    GameController *gamepad = trackcontrollers.value(event.jaxis.which);
                    gamepad->rawDPadEvent(event.jhat.hat, event.jhat.value);
                }
#endif

                break;
            }

#ifdef USE_SDL_2
            case SDL_CONTROLLERAXISMOTION:
            {
                InputDevice *joy = trackcontrollers.value(event.caxis.which);
                if (joy)
                {
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyAxis *axis = set->getJoyAxis(event.caxis.axis);
                    if (axis)
                    {
                        //qDebug() << QTime::currentTime() << ": " << "Axis " << event.caxis.axis+1
                        //         << ": " << event.caxis.value;
                        //axis->joyEvent(event.caxis.value);
                        axis->queuePendingEvent(event.caxis.value);

                        if (!activeDevices.contains(event.caxis.which))
                        {
                            activeDevices.insert(event.caxis.which, joy);
                        }
                    }
                }
                break;
            }

            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
            {
                InputDevice *joy = trackcontrollers.value(event.cbutton.which);
                if (joy)
                {
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyButton *button = set->getJoyButton(event.cbutton.button);

                    if (button)
                    {
                        //button->joyEvent(event.type == SDL_CONTROLLERBUTTONDOWN ? true : false);
                        button->queuePendingEvent(event.type == SDL_CONTROLLERBUTTONDOWN ? true : false);

                        if (!activeDevices.contains(event.cbutton.which))
                        {
                            activeDevices.insert(event.cbutton.which, joy);
                        }
                    }
                }

                break;
            }

            case SDL_JOYDEVICEREMOVED:
            {
                InputDevice *device = joysticks->value(event.jdevice.which);
                if (device)
                {
                    Logger::LogInfo(QString("Removing joystick #%1 [%2]")
                                    .arg(device->getRealJoyNumber())
                                    .arg(QTime::currentTime().toString("hh:mm:ss.zzz")));

                    //activeDevices.remove(event.jdevice.which);
                    removeDevice(device);
                }

                break;
            }

            case SDL_JOYDEVICEADDED:
            {
                addInputDevice(event.jdevice.which);
                break;
            }
#endif

            case SDL_QUIT:
            {
                stopped = true;
                break;
            }

            default:
                break;
        }

        // Active possible queued events.
        QHashIterator<SDL_JoystickID, InputDevice*> activeDevIter(activeDevices);
        while (activeDevIter.hasNext())
        {
            InputDevice *tempDevice = activeDevIter.next().value();
            tempDevice->activatePossibleControlStickEvents();
            tempDevice->activatePossibleAxisEvents();
            tempDevice->activatePossibleDPadEvents();
            tempDevice->activatePossibleVDPadEvents();
            tempDevice->activatePossibleButtonEvents();
        }

        if (JoyButton::shouldInvokeMouseEvents())
        {
            // Do not wait for next event loop run. Execute immediately.
            JoyButton::invokeMouseEvents();
        }
    }
}

void InputDaemon::clearBitArrayStatusInstances()
{
    QHashIterator<InputDevice*, InputDeviceBitArrayStatus*> genIter(releaseEventsGenerated);
    while (genIter.hasNext())
    {
        InputDeviceBitArrayStatus *temp = genIter.next().value();
        if (temp)
        {
            delete temp;
            temp = 0;
        }
    }

    releaseEventsGenerated.clear();

    QHashIterator<InputDevice*, InputDeviceBitArrayStatus*> pendIter(pendingEventValues);
    while (pendIter.hasNext())
    {
        InputDeviceBitArrayStatus *temp = pendIter.next().value();
        if (temp)
        {
            delete temp;
            temp = 0;
        }
    }

    pendingEventValues.clear();
}

void InputDaemon::resetActiveButtonMouseDistances()
{
    pollResetTimer.stop();

    JoyButton::resetActiveButtonMouseDistances();
}

void InputDaemon::updatePollResetRate(unsigned int tempPollRate)
{
    Q_UNUSED(tempPollRate);


    bool wasActive = pollResetTimer.isActive();
    pollResetTimer.stop();

    pollResetTimer.setInterval(
                qMax(JoyButton::getMouseRefreshRate(),
                     JoyButton::getGamepadRefreshRate()) + 1);

    if (wasActive)
    {
        pollResetTimer.start();
    }
}
