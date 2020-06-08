/* antimicroX Gamepad to KB+M event mapper
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

#include "inputdaemon.h"

#include "globalvariables.h"
#include "messagehandler.h"
#include "logger.h"
#include "common.h"
#include "joystick.h"
#include "joydpad.h"
#include "sdleventreader.h"
#include "antimicrosettings.h"
#include "inputdevicebitarraystatus.h"


#include <QDebug>
#include <QTime>
#include <QTimer>
#include <QEventLoop>
#include <QMapIterator>
#include <QThread>

#define USE_NEW_REFRESH

InputDaemon::InputDaemon(QMap<SDL_JoystickID, InputDevice*> *joysticks,
                         AntiMicroSettings *settings,
                         bool graphical, QObject *parent) :
    QObject(parent),
    pollResetTimer(this)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    m_joysticks = joysticks;
    //Xbox360Wireless* xbox360class = new Xbox360Wireless();
    //xbox360 = xbox360class->getResult();
    this->stopped = false;
    m_graphical = graphical;
    m_settings = settings;

    eventWorker = new SDLEventReader(joysticks, settings);
    refreshJoysticks();
    sdlWorkerThread = nullptr;

    if (m_graphical)
    {
        sdlWorkerThread = new QThread;
        eventWorker->moveToThread(sdlWorkerThread);

        connect(sdlWorkerThread, &QThread::started, eventWorker, &SDLEventReader::performWork);
        connect(eventWorker, &SDLEventReader::eventRaised, this, &InputDaemon::run);

        connect(JoyButton::getMouseHelper(), &JoyButtonMouseHelper::gamepadRefreshRateUpdated,
                eventWorker, &SDLEventReader::updatePollRate);

        connect(JoyButton::getMouseHelper(), &JoyButtonMouseHelper::gamepadRefreshRateUpdated,
                this, &InputDaemon::updatePollResetRate);
        connect(JoyButton::getMouseHelper(), &JoyButtonMouseHelper::mouseRefreshRateUpdated,
                this, &InputDaemon::updatePollResetRate);

        // Timer in case SDL does not produce an axis event during a joystick
        // poll.
        pollResetTimer.setSingleShot(true);
        pollResetTimer.setInterval(
                    qMax(GlobalVariables::JoyButton::mouseRefreshRate,
                         GlobalVariables::JoyButton::gamepadRefreshRate) + 1);

        connect(&pollResetTimer, &QTimer::timeout, this,
                &InputDaemon::resetActiveButtonMouseDistances);
    }
}

InputDaemon::~InputDaemon()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (eventWorker != nullptr) quit();

    if (sdlWorkerThread != nullptr)
    {
        sdlWorkerThread->quit();
        sdlWorkerThread->wait();
        delete sdlWorkerThread;
        sdlWorkerThread = nullptr;
    }
}

void InputDaemon::startWorker()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (!sdlWorkerThread->isRunning())
        sdlWorkerThread->start(QThread::HighPriority);
}

void InputDaemon::run ()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    PadderCommon::inputDaemonMutex.lock();

    // SDL has found events. The timeout is not necessary.
    pollResetTimer.stop();

    if (!stopped)
    {
        JoyButton::resetActiveButtonMouseDistances(JoyButton::getMouseHelper());

        QQueue<SDL_Event> sdlEventQueue;
        firstInputPass(&sdlEventQueue);
        modifyUnplugEvents(&sdlEventQueue);
        secondInputPass(&sdlEventQueue);
        clearBitArrayStatusInstances();
    }

    if (stopped)
    {
        if (m_joysticks->size() > 0) emit complete(m_joysticks->value(0));

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

QString InputDaemon::getJoyInfo(SDL_JoystickGUID sdlvalue)
{
    char buffer[65] = {'0'};

    SDL_JoystickGetGUIDString(sdlvalue, buffer, sizeof(buffer));

    return QString(buffer);
}

QString InputDaemon::getJoyInfo(Uint16 sdlvalue)
{
    char buffer[50] = {'0'};

    sprintf (buffer, "%u", sdlvalue);

    return QString(buffer);
}

void InputDaemon::refreshJoysticks()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QMapIterator<SDL_JoystickID, InputDevice*> iter(*m_joysticks);

    while (iter.hasNext())
    {
        InputDevice *joystick = iter.next().value();

        if (joystick != nullptr)
        {
            m_joysticks->remove(iter.key());
            delete joystick;
        }
    }

    m_joysticks->clear();
    getTrackjoysticksLocal().clear();
    trackcontrollers.clear();

    m_settings->getLock()->lock();
    m_settings->beginGroup("Mappings");

    QMap<QString,int> uniques = QMap<QString,int>();
    int counterUniques = 1;
    bool duplicatedGamepad = false;

    for (int i = 0; i < SDL_NumJoysticks(); i++)
    {
#ifdef USE_NEW_REFRESH
        int index = i;


        // Check if device is considered a Game Controller at the start.
        if (SDL_IsGameController(index))
        {
            SDL_GameController *controller = SDL_GameControllerOpen(index);

            if (controller != nullptr)
            {
                SDL_Joystick *sdlStick = SDL_GameControllerGetJoystick(controller);
                SDL_JoystickID tempJoystickID = SDL_JoystickInstanceID(sdlStick);

                // Check if device has already been grabbed.
                if (!m_joysticks->contains(tempJoystickID))
                {
                    QString guidText = getJoyInfo(SDL_JoystickGetGUID(sdlStick));

                    QString vendor = getJoyInfo(SDL_GameControllerGetVendor(controller));

                    QString productID = getJoyInfo(SDL_GameControllerGetProduct(controller));

                    if (uniques.contains(guidText))
                    {
                        productID = getJoyInfo(SDL_GameControllerGetProduct(controller) + ++uniques[guidText]);
                        duplicatedGamepad = true;

                        // previous value will be erased in map anyway
                        uniques.insert(guidText, uniques[guidText]);
                    }
                    else
                    {
                        uniques.insert(guidText, counterUniques);
                    }

                    convertMappingsToUnique(m_settings, guidText, guidText + vendor + productID);

                    bool disableGameController = m_settings->value(QString("%1Disable").arg(guidText + vendor + productID), false).toBool();

                    // Check if user has designated device Joystick mode.
                    if (!disableGameController)
                    {
                        int resultDuplicated = 0;
                        if (duplicatedGamepad) resultDuplicated = uniques.value(guidText);

                        GameController *damncontroller = new GameController(controller, index, m_settings, resultDuplicated, this);
                        duplicatedGamepad = false;
                        connect(damncontroller, &GameController::requestWait, eventWorker, &SDLEventReader::haltServices);
                        m_joysticks->insert(tempJoystickID, damncontroller);
                        trackcontrollers.insert(tempJoystickID, damncontroller);

                        emit deviceAdded(damncontroller);
                    }
                    else
                    {
                        Joystick *joystick = openJoystickDevice(index);

                        if (joystick != nullptr)
                            emit deviceAdded(joystick);
                    }
                }
                else
                {
                    SDL_GameControllerClose(controller); // Make sure to decrement reference count
                }
            }
        }
        else
        {
            Joystick *joystick = openJoystickDevice(index);

            if (joystick != nullptr)
                emit deviceAdded(joystick);
        }


#else
        SDL_Joystick *joystick = SDL_JoystickOpen(i);
        if (joystick != nullptr)
        {
            QString temp = QString();
            SDL_JoystickGUID tempGUID = SDL_JoystickGetGUID(joystick);
            char guidString[65] = {'0'};
            SDL_JoystickGetGUIDString(tempGUID, guidString, sizeof(guidString));
            temp = QString(guidString);

            bool disableGameController = m_settings->value(QString("%1Disable").arg(temp), false).toBool();

            if (SDL_IsGameController(i) && !disableGameController)
            {
                SDL_GameController *controller = SDL_GameControllerOpen(i);
                GameController *damncontroller = new GameController(controller, i, m_settings, this);
                connect(damncontroller, &GameController::requestWait, eventWorker, &SDLEventReader::haltServices);
                SDL_Joystick *sdlStick = SDL_GameControllerGetJoystick(controller);
                SDL_JoystickID joystickID = SDL_JoystickInstanceID(sdlStick);
                m_joysticks->insert(joystickID, damncontroller);
                trackcontrollers.insert(joystickID, damncontroller);
            }
            else
            {
                Joystick *curJoystick = new Joystick(joystick, i, m_settings, this);
                connect(curJoystick, &Joystick::requestWait, eventWorker, &SDLEventReader::haltServices);
                SDL_JoystickID joystickID = SDL_JoystickInstanceID(joystick);
                m_joysticks->insert(joystickID, curJoystick);
                trackjoysticks.insert(joystickID, curJoystick);
            }
        }
#endif
    }

    m_settings->endGroup();
    m_settings->getLock()->unlock();

    emit joysticksRefreshed(m_joysticks);
}

void InputDaemon::deleteJoysticks()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QMapIterator<SDL_JoystickID, InputDevice*> iter(*m_joysticks);

    while (iter.hasNext())
    {
        InputDevice *joystick = iter.next().value();

        for (auto el : joystick->getJoystick_sets().values())
        {

        }

        if (joystick != nullptr)
        {
            delete joystick;
            joystick = nullptr;
        }
    }

    m_joysticks->clear();
    getTrackjoysticksLocal().clear();
    trackcontrollers.clear();
}

void InputDaemon::stop()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    stopped = true;
    pollResetTimer.stop();
}

void InputDaemon::refresh()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);
    qDebug() << "REFRESH";

    stop();

    Logger::LogInfo("Refreshing joystick list");

    QEventLoop q;
    connect(eventWorker, &SDLEventReader::sdlStarted, &q, &QEventLoop::quit);
    QTimer::singleShot(0, eventWorker, SLOT(refresh()));
    //QMetaObject::invokeMethod(eventWorker, "refresh", Qt::BlockingQueuedConnection);

    if (eventWorker->isSDLOpen()) q.exec();

    disconnect(eventWorker, &SDLEventReader::sdlStarted, &q, &QEventLoop::quit);

    pollResetTimer.stop();

    // Put in an extra delay before refreshing the joysticks
    QTimer temp;
    connect(&temp, &QTimer::timeout, &q, &QEventLoop::quit);
    temp.start(100);
    q.exec();

    refreshJoysticks();
    QTimer::singleShot(100, eventWorker, SLOT(performWork()));

    stopped = false;

}

void InputDaemon::refreshJoystick(InputDevice *joystick)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    joystick->reset();

    emit joystickRefreshed(joystick);
}

void InputDaemon::quit()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    stopped = true;
    pollResetTimer.stop();

    disconnect(eventWorker, &SDLEventReader::eventRaised, this, nullptr);

    // Wait for SDL to finish. Let worker destructor close SDL.
    // Let InputDaemon destructor close thread instance.
    if (m_graphical)
    {
        QMetaObject::invokeMethod(eventWorker, "stop");
        QMetaObject::invokeMethod(eventWorker, "quit");
        QMetaObject::invokeMethod(eventWorker, "deleteLater", Qt::BlockingQueuedConnection);
    }
    else
    {
        eventWorker->stop();
        eventWorker->quit();
        delete eventWorker;
    }

    eventWorker = nullptr;
}


void InputDaemon::refreshMapping(QString mapping, InputDevice *device)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool found = false;

    QMap<QString,int> uniques = QMap<QString,int>();
    int counterUniques = 1;
    bool duplicatedGamepad = false;

    for (int i = 0; (i < SDL_NumJoysticks()) && !found; i++)
   // for (int i = 0; (i < 1) && !found; i++)
    {
        SDL_Joystick *joystick = SDL_JoystickOpen(i);
       // SDL_Joystick *joystick = xbox360;
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
                    getTrackjoysticksLocal().remove(joystickID);
                    m_joysticks->remove(joystickID);

                    SDL_GameController *controller = SDL_GameControllerOpen(i);

                    QString guidText = getJoyInfo(SDL_JoystickGetGUID(SDL_GameControllerGetJoystick(controller)));

                    if (uniques.contains(guidText))
                    {
                        ++uniques[guidText];
                        duplicatedGamepad = true;

                        // previous value will be erased in map anyway
                        uniques.insert(guidText, uniques[guidText]);
                    }
                    else
                    {
                        uniques.insert(guidText, counterUniques);
                    }

                    int resultDuplicated = 0;
                    if (duplicatedGamepad) resultDuplicated = counterUniques;

                    GameController *damncontroller = new GameController(controller, i, m_settings, resultDuplicated, this);
                    duplicatedGamepad = false;
                    connect(damncontroller, &GameController::requestWait, eventWorker, &SDLEventReader::haltServices);
                    SDL_Joystick *sdlStick = SDL_GameControllerGetJoystick(controller);
                    joystickID = SDL_JoystickInstanceID(sdlStick);
                    m_joysticks->insert(joystickID, damncontroller);
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (device != nullptr)
    {
        SDL_JoystickID deviceID = device->getSDLJoystickID();

        m_joysticks->remove(deviceID);
        getTrackjoysticksLocal().remove(deviceID);
        trackcontrollers.remove(deviceID);

        refreshIndexes();

        emit deviceRemoved(deviceID);
    }
}

void InputDaemon::refreshIndexes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    for (int i = 0; i < SDL_NumJoysticks(); i++)
    //for (int i = 0; i < 1; i++)
    {
        SDL_Joystick *joystick = SDL_JoystickOpen(i);
       // SDL_Joystick *joystick = xbox360;
        SDL_JoystickID joystickID = SDL_JoystickInstanceID(joystick);
        SDL_JoystickClose(joystick); // Make sure to decrement reference count
        InputDevice *tempdevice = m_joysticks->value(joystickID);

        if (tempdevice != nullptr)
            tempdevice->setIndex(i);

    }
}

void InputDaemon::addInputDevice(int index, QMap<QString,int>& uniques, int& counterUniques, bool& duplicatedGamepad)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

  #ifdef USE_NEW_ADD
    // Check if device is considered a Game Controller at the start.
    if (SDL_IsGameController(index))
    {
        SDL_GameController *controller = SDL_GameControllerOpen(index);
        if (controller != nullptr)
        {
            SDL_Joystick *sdlStick = SDL_GameControllerGetJoystick(controller);
            SDL_JoystickID tempJoystickID = SDL_JoystickInstanceID(sdlStick);

            // Check if device has already been grabbed.
            if (!m_joysticks->contains(tempJoystickID))
            {
                m_settings->getLock()->lock();
                m_settings->beginGroup("Mappings");

                QString temp = QString();
                SDL_JoystickGUID tempGUID = SDL_JoystickGetGUID(sdlStick);
                char guidString[65] = {'0'};
                SDL_JoystickGetGUIDString(tempGUID, guidString, sizeof(guidString));
                temp = QString(guidString);

                bool disableGameController = m_settings->value(QString("%1Disable").arg(temp), false).toBool();

                m_settings->endGroup();
                m_settings->getLock()->unlock();

                // Check if user has designated device Joystick mode.
                if (!disableGameController)
                {
                    GameController *damncontroller = new GameController(controller, index, m_settings, this);
                    connect(damncontroller, &GameController::requestWait, eventWorker, &SDLEventReader::haltServices);
                    m_joysticks->insert(tempJoystickID, damncontroller);
                    trackcontrollers.insert(tempJoystickID, damncontroller);

                    Logger::LogInfo(QString("New game controller found - #%1 [%2]")
                                    .arg(index+1)
                                    .arg(QTime::currentTime().toString("hh:mm:ss.zzz")));

                    emit deviceAdded(damncontroller);
                }
                else
                {
                    // Check if joystick is considered connected.

                    Joystick *joystick = openJoystickDevice(index);
                    if (joystick != nullptr)
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
        Joystick *joystick = openJoystickDevice(index);
        if (joystick != nullptr)
        {
            Logger::LogInfo(QString("New joystick found - #%1 [%2]")
                            .arg(index+1)
                            .arg(QTime::currentTime().toString("hh:mm:ss.zzz")));

            emit deviceAdded(joystick);
        }
    }
#else
    SDL_GameController *controller = SDL_GameControllerOpen(index);
    SDL_Joystick *joystick = SDL_JoystickOpen(index);

   // SDL_Joystick *joystick = xbox360;
   // SDL_GameController *controller = SDL_GameControllerFromInstanceID(xbox360->instance_id);

    if (joystick != nullptr)
    {
        SDL_JoystickID tempJoystickID_local = SDL_JoystickInstanceID(joystick);

        if (!m_joysticks->contains(tempJoystickID_local))
        {
            m_settings->getLock()->lock();
            m_settings->beginGroup("Mappings");

//            QString temp = QString();
//            SDL_JoystickGUID tempGUID = SDL_JoystickGetGUID(joystick);
//            char guidString[65] = {'0'};
//            SDL_JoystickGetGUIDString(tempGUID, guidString, sizeof(guidString));
//            temp = QString(guidString);

//            bool disableGameController = m_settings->value(QString("%1Disable").arg(temp), false).toBool();

            QString guidText = QString();

            SDL_JoystickGUID tempGUID = SDL_JoystickGetGUID(joystick);
            char guidString[65] = {'0'};
            SDL_JoystickGetGUIDString(tempGUID, guidString, sizeof(guidString));
            guidText = QString(guidString);

            QString vendor = QString();

            if (controller != nullptr)
            {
                    Uint16 tempVendor = SDL_GameControllerGetVendor(controller);
                    char buffer [50];
                    sprintf (buffer, "%u", tempVendor);

                    vendor = QString(buffer);
            }

            QString productID = QString();

            if (controller != nullptr)
            {
                    Uint16 tempProduct = SDL_GameControllerGetProduct(controller);
                    char buffer [50];
                    sprintf (buffer, "%u", tempProduct);

                    productID = QString(buffer);
            }

            if (uniques.contains(guidText))
            {
                productID = getJoyInfo(SDL_GameControllerGetProduct(controller) + ++uniques[guidText]);
                duplicatedGamepad = true;
                uniques.insert(guidText, uniques[guidText]);
            }
            else
            {
                uniques.insert(guidText, counterUniques);
            }

            convertMappingsToUnique(m_settings, guidText, guidText + vendor + productID);

            int resultDuplicated = 0;
            if (duplicatedGamepad) resultDuplicated = uniques[guidText];

            bool disableGameController = m_settings->value(QString("%1Disable").arg(guidText + vendor + productID), false).toBool();

            if (SDL_IsGameController(index) && !disableGameController)
            {
                // Make sure to decrement reference count
                SDL_JoystickClose(joystick);

                SDL_GameController *controller = SDL_GameControllerOpen(index);

                if (controller != nullptr)
                {
                    SDL_Joystick *sdlStick = SDL_GameControllerGetJoystick(controller);
                    SDL_JoystickID tempJoystickID_local_2 = SDL_JoystickInstanceID(sdlStick);

                    if (!m_joysticks->contains(tempJoystickID_local_2))
                    {
                        GameController *damncontroller = new GameController(controller, index, m_settings, resultDuplicated, this);
                        connect(damncontroller, &GameController::requestWait, eventWorker, &SDLEventReader::haltServices);
                        m_joysticks->insert(tempJoystickID_local_2, damncontroller);
                        trackcontrollers.insert(tempJoystickID_local_2, damncontroller);

                        m_settings->endGroup();
                        m_settings->getLock()->unlock();

                        emit deviceAdded(damncontroller);
                    }

                    duplicatedGamepad = false;
                }
                else
                {
                    m_settings->endGroup();
                    m_settings->getLock()->unlock();
                }
            }
            else
            {
                Joystick *curJoystick = new Joystick(joystick, index, m_settings, this);
                m_joysticks->insert(tempJoystickID_local, curJoystick);
                getTrackjoysticksLocal().insert(tempJoystickID_local, curJoystick);

                m_settings->endGroup();
                m_settings->getLock()->unlock();

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    // Check if joystick is considered connected.
    SDL_Joystick *joystick = SDL_JoystickOpen(index);
   // SDL_Joystick* joystick = xbox360;
    Joystick *curJoystick = nullptr;

    if (joystick != nullptr)
    {
        SDL_JoystickID tempJoystickID = SDL_JoystickInstanceID(joystick);

        curJoystick = new Joystick(joystick, index, m_settings, this);
        m_joysticks->insert(tempJoystickID, curJoystick);
        getTrackjoysticksLocal().insert(tempJoystickID, curJoystick);

    }

    return curJoystick;
}


InputDeviceBitArrayStatus*
InputDaemon::createOrGrabBitStatusEntry(QHash<InputDevice *, InputDeviceBitArrayStatus *> *statusHash,
                                        InputDevice *device, bool readCurrent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    InputDeviceBitArrayStatus *bitArrayStatus = nullptr;

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    SDL_Event event;

    while (SDL_PollEvent(&event) > 0)
    {
        switch (event.type)
        {
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
            {
                InputDevice *joy = getTrackjoysticksLocal().value(event.jbutton.which);

                if (joy != nullptr)
                {
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyButton *button = set->getJoyButton(event.jbutton.button);

                    if (button != nullptr)
                    {
                        InputDeviceBitArrayStatus *pending = createOrGrabBitStatusEntry(&pendingEventValues, joy);
                        pending->changeButtonStatus(event.jbutton.button,
                                                  event.type == SDL_JOYBUTTONDOWN ? true : false);
                        sdlEventQueue->append(event);
                    }
                }
                else
                {
                    sdlEventQueue->append(event);
                }

                break;
            }
            case SDL_JOYAXISMOTION:
            {
                InputDevice *joy = getTrackjoysticksLocal().value(event.jaxis.which);

                if (joy != nullptr)
                {
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyAxis *axis = set->getJoyAxis(event.jaxis.axis);

                    if (axis != nullptr)
                    {
                        InputDeviceBitArrayStatus *temp = createOrGrabBitStatusEntry(&releaseEventsGenerated, joy, false);
                        temp->changeAxesStatus(event.jaxis.axis, event.jaxis.axis == 0);

                        InputDeviceBitArrayStatus *pending = createOrGrabBitStatusEntry(&pendingEventValues, joy);
                        pending->changeAxesStatus(event.jaxis.axis, !axis->inDeadZone(event.jaxis.value));
                        sdlEventQueue->append(event);
                    }
                }
                else
                {
                    sdlEventQueue->append(event);
                }

                break;
            }
            case SDL_JOYHATMOTION:
            {
                InputDevice *joy = getTrackjoysticksLocal().value(event.jhat.which);

                if (joy != nullptr)
                {
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyDPad *dpad = set->getJoyDPad(event.jhat.hat);

                    if (dpad != nullptr)
                    {
                        InputDeviceBitArrayStatus *pending = createOrGrabBitStatusEntry(&pendingEventValues, joy);
                        pending->changeHatStatus(event.jhat.hat, (event.jhat.value != 0) ? true : false);
                        sdlEventQueue->append(event);
                    }
                }
                else
                {
                    sdlEventQueue->append(event);
                }

                break;
            }

            case SDL_CONTROLLERAXISMOTION:
            {
                InputDevice *joy = trackcontrollers.value(event.caxis.which);

                if (joy != nullptr)
                {
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyAxis *axis = set->getJoyAxis(event.caxis.axis);

                    if (axis != nullptr)
                    {
                        InputDeviceBitArrayStatus *temp = createOrGrabBitStatusEntry(&releaseEventsGenerated, joy, false);

                        if ((event.caxis.axis != SDL_CONTROLLER_AXIS_TRIGGERLEFT) &&
                            (event.caxis.axis != SDL_CONTROLLER_AXIS_TRIGGERRIGHT))
                        {
                            temp->changeAxesStatus(event.caxis.axis, event.caxis.value == 0);
                        }
                        else
                        {
                            temp->changeAxesStatus(event.caxis.axis, event.caxis.value == GlobalVariables::InputDaemon::GAMECONTROLLERTRIGGERRELEASE);
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

                if (joy != nullptr)
                {
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyButton *button = set->getJoyButton(event.cbutton.button);

                    if (button != nullptr)
                    {
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
            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED:
            {
                sdlEventQueue->append(event);
                break;
            }
            case SDL_QUIT:
            {
                sdlEventQueue->append(event);
                break;
            }
            default:
            {
                break;
            }
        }
    }
}


void InputDaemon::modifyUnplugEvents(QQueue<SDL_Event> *sdlEventQueue)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<InputDevice*, InputDeviceBitArrayStatus*> genIter(getReleaseEventsGeneratedLocal());

    while (genIter.hasNext())
    {
        genIter.next();
        InputDevice *device = genIter.key();
        InputDeviceBitArrayStatus *generatedTemp = genIter.value();
        QBitArray tempBitArray = generatedTemp->generateFinalBitArray();


            qDebug() << "ARRAY: " << tempBitArray;


        int bitArraySize = tempBitArray.size();

            qDebug() << "ARRAY SIZE: " << bitArraySize;


        if ((bitArraySize > 0) && (tempBitArray.count(true) == device->getNumberAxes()))
        {
            if (getPendingEventValuesLocal().contains(device))
            {
                InputDeviceBitArrayStatus *pendingTemp = getPendingEventValuesLocal().value(device);
                QBitArray pendingBitArray = pendingTemp->generateFinalBitArray();
                QBitArray unplugBitArray = createUnplugEventBitArray(device);
                int pendingBitArraySize = pendingBitArray.size();

                if ((bitArraySize == pendingBitArraySize) &&
                    (pendingBitArray == unplugBitArray))
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
                                    InputDevice *joy = getTrackjoysticksLocal().value(event.jaxis.which);

                                    if (joy != nullptr)
                                    {
                                        JoyAxis *axis = joy->getActiveSetJoystick()->getJoyAxis(event.jaxis.axis);
                                        if (axis != nullptr)
                                        {
                                            if (axis->getThrottle() != static_cast<int>(JoyAxis::NormalThrottle))
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

                                    if (joy != nullptr)
                                    {
                                        SetJoystick* set = joy->getActiveSetJoystick();
                                        JoyAxis *axis = set->getJoyAxis(event.caxis.axis);

                                        if (axis != nullptr)
                                        {
                                            if ((event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT) ||
                                                (event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT))
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
                            case SDL_CONTROLLERDEVICEREMOVED:
                            case SDL_CONTROLLERDEVICEADDED:
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


QBitArray InputDaemon::createUnplugEventBitArray(InputDevice *device)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    InputDeviceBitArrayStatus tempStatus(device, false);

    for (int i = 0; i < device->getNumberRawAxes(); i++)
    {
        JoyAxis *axis = device->getActiveSetJoystick()->getJoyAxis(i);

        if ((axis != nullptr) && (axis->getThrottle() != static_cast<int>(JoyAxis::NormalThrottle)))
            tempStatus.changeAxesStatus(i, true);
    }

    QBitArray unplugBitArray = tempStatus.generateFinalBitArray();
    return unplugBitArray;
}


void InputDaemon::secondInputPass(QQueue<SDL_Event> *sdlEventQueue)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QMap<QString,int> uniques = QMap<QString,int>();
    int counterUniques = 1;
    bool duplicatedGamepad = false;

    QHash<SDL_JoystickID, InputDevice*> activeDevices;

    while (!sdlEventQueue->isEmpty())
    {
        SDL_Event event = sdlEventQueue->dequeue();

        switch (event.type)
        {
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
            {
                InputDevice *joy = getTrackjoysticksLocal().value(event.jbutton.which);

                if (joy != nullptr)
                {
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyButton *button = set->getJoyButton(event.jbutton.button);

                    if (button != nullptr)
                    {
                        button->queuePendingEvent(event.type == SDL_JOYBUTTONDOWN ? true : false);

                        if (!activeDevices.contains(event.jbutton.which))
                            activeDevices.insert(event.jbutton.which, joy);
                    }
                }
                else if (trackcontrollers.contains(event.jbutton.which))
                {
                    GameController *gamepad = trackcontrollers.value(event.jbutton.which);
                    gamepad->rawButtonEvent(event.jbutton.button, event.type == SDL_JOYBUTTONDOWN ? true : false);
                }

                break;
            }

            case SDL_JOYAXISMOTION:
            {

                InputDevice *joy = getTrackjoysticksLocal().value(event.jaxis.which);

                if (joy != nullptr)
                {
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyAxis *axis = set->getJoyAxis(event.jaxis.axis);

                    if (axis != nullptr)
                    {
                        axis->queuePendingEvent(event.jaxis.value);

                        if (!activeDevices.contains(event.jaxis.which))
                            activeDevices.insert(event.jaxis.which, joy);
                    }

                    joy->rawAxisEvent(event.jaxis.which, event.jaxis.value);
                }
                else if (trackcontrollers.contains(event.jaxis.which))
                {
                    GameController *gamepad = trackcontrollers.value(event.jaxis.which);
                    gamepad->rawAxisEvent(event.jaxis.axis, event.jaxis.value);
                }

                break;
            }

            case SDL_JOYHATMOTION:
            {
                InputDevice *joy = getTrackjoysticksLocal().value(event.jhat.which);

                if (joy != nullptr)
                {
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyDPad *dpad = set->getJoyDPad(event.jhat.hat);

                    if (dpad != nullptr)
                    {
                        dpad->joyEvent(event.jhat.value);

                        if (!activeDevices.contains(event.jhat.which))
                            activeDevices.insert(event.jhat.which, joy);
                    }
                }
                else if (trackcontrollers.contains(event.jhat.which))
                {
                    GameController *gamepad = trackcontrollers.value(event.jaxis.which);
                    gamepad->rawDPadEvent(event.jhat.hat, event.jhat.value);
                }

                break;
            }

            case SDL_CONTROLLERAXISMOTION:
            {
                InputDevice *joy = trackcontrollers.value(event.caxis.which);

                if (joy != nullptr)
                {
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyAxis *axis = set->getJoyAxis(event.caxis.axis);

                    if (axis != nullptr)
                    {
                        axis->queuePendingEvent(event.caxis.value);

                        if (!activeDevices.contains(event.caxis.which))
                            activeDevices.insert(event.caxis.which, joy);
                    }
                }

                break;
            }

            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
            {
                InputDevice *joy = trackcontrollers.value(event.cbutton.which);

                if (joy != nullptr)
                {
                    SetJoystick* set = joy->getActiveSetJoystick();
                    JoyButton *button = set->getJoyButton(event.cbutton.button);

                    if (button != nullptr)
                    {
                        button->queuePendingEvent(event.type == SDL_CONTROLLERBUTTONDOWN ? true : false);

                        if (!activeDevices.contains(event.cbutton.which))
                            activeDevices.insert(event.cbutton.which, joy);
                    }
                }

                break;
            }

            case SDL_JOYDEVICEREMOVED:
            case SDL_CONTROLLERDEVICEREMOVED:
            {
                InputDevice *device = m_joysticks->value(event.jdevice.which);

                if (device != nullptr)
                {
                    Logger::LogInfo(QString("Removing joystick #%1 [%2]")
                                    .arg(device->getRealJoyNumber())
                                    .arg(QTime::currentTime().toString("hh:mm:ss.zzz")));

                    removeDevice(device);
                }

                break;
            }

            case SDL_JOYDEVICEADDED:
            case SDL_CONTROLLERDEVICEADDED:
            {
                addInputDevice(event.jdevice.which, uniques, counterUniques, duplicatedGamepad);
                break;
            }

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

        if (JoyButton::shouldInvokeMouseEvents(JoyButton::getPendingMouseButtons(), JoyButton::getStaticMouseEventTimer(), JoyButton::getTestOldMouseTime()))
            JoyButton::invokeMouseEvents(JoyButton::getMouseHelper()); // Do not wait for next event loop run. Execute immediately.
    }
}

void InputDaemon::clearBitArrayStatusInstances()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<InputDevice*, InputDeviceBitArrayStatus*> genIter(getReleaseEventsGeneratedLocal());

    while (genIter.hasNext())
    {
        InputDeviceBitArrayStatus *temp = genIter.next().value();

        if (temp != nullptr)
        {
            delete temp;
            temp = nullptr;
        }
    }

    getReleaseEventsGeneratedLocal().clear();

    QHashIterator<InputDevice*, InputDeviceBitArrayStatus*> pendIter(getPendingEventValuesLocal());

    while (pendIter.hasNext())
    {
        InputDeviceBitArrayStatus *temp = pendIter.next().value();

        if (temp != nullptr)
        {
            delete temp;
            temp = nullptr;
        }
    }

    getPendingEventValuesLocal().clear();
}

void InputDaemon::resetActiveButtonMouseDistances()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    pollResetTimer.stop();

    JoyButton::resetActiveButtonMouseDistances(JoyButton::getMouseHelper());
}

void InputDaemon::updatePollResetRate(int tempPollRate)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_UNUSED(tempPollRate);

    bool wasActive = pollResetTimer.isActive();
    pollResetTimer.stop();
    pollResetTimer.setInterval(
                qMax(GlobalVariables::JoyButton::mouseRefreshRate,
                     GlobalVariables::JoyButton::gamepadRefreshRate) + 1);

    if (wasActive) pollResetTimer.start();
}

void InputDaemon::convertMappingsToUnique(QSettings* sett, QString guidString, QString uniqueIdString)
{
    if (sett->contains(QString("%1Disable").arg(guidString)))
    {
        sett->setValue(QString("%1Disable").arg(uniqueIdString), sett->value(QString("%1Disable").arg(guidString)));
        sett->remove(QString("%1Disable").arg(guidString));
    }

    if (sett->contains(guidString))
    {
        QStringList gg = sett->value(guidString).toString().split(",");
        qDebug() << "Convert guidString to uniqueString 1): " << gg << endl;
        gg.removeFirst();
        qDebug() << "Convert guidString to uniqueString 2): " << gg << endl;
        gg.prepend(uniqueIdString);
        qDebug() << "Convert guidString to uniqueString 3): " << gg << endl;
        qDebug() << "Joined uniqueMapping: " << gg.join(",") << endl;
        sett->setValue(uniqueIdString, gg.join(","));
        sett->remove(guidString);
    }
}

QHash<SDL_JoystickID, Joystick*>& InputDaemon::getTrackjoysticksLocal() {

    return trackjoysticks;
}

QHash<InputDevice*, InputDeviceBitArrayStatus*>& InputDaemon::getReleaseEventsGeneratedLocal() {

    return releaseEventsGenerated;
}

QHash<InputDevice*, InputDeviceBitArrayStatus*>& InputDaemon::getPendingEventValuesLocal() {

    return pendingEventValues;
}
