//#include <QDebug>
#include <QTime>
#include <QTimer>
#include <QEventLoop>
#include <QMapIterator>

#include "inputdaemon.h"

const int InputDaemon::GAMECONTROLLERTRIGGERRELEASE = 16384;

InputDaemon::InputDaemon(QMap<SDL_JoystickID, InputDevice*> *joysticks, AntiMicroSettings *settings, bool graphical, QObject *parent) :
    QObject(parent)
{
    this->joysticks = joysticks;
    this->stopped = false;
    this->graphical = graphical;
    this->settings = settings;

    eventWorker = new SDLEventReader(joysticks, settings);
    thread = new QThread();
    eventWorker->moveToThread(thread);

    if (graphical)
    {
        connect(thread, SIGNAL(started()), eventWorker, SLOT(performWork()));
        connect(eventWorker, SIGNAL(eventRaised()), this, SLOT(run()));
        thread->start();
    }
    refreshJoysticks();
}

InputDaemon::~InputDaemon()
{
    if (eventWorker)
    {
        quit();
    }

    if (thread)
    {
        thread->quit();
        thread->wait();
        delete thread;
        thread = 0;
    }
}

void InputDaemon::startWorker()
{
    if (!thread->isRunning())
    {
        connect(thread, SIGNAL(started()), eventWorker, SLOT(performWork()));
        connect(eventWorker, SIGNAL(eventRaised()), this, SLOT(run()));
        thread->start();
    }
}

void InputDaemon::run ()
{
    //SDL_Event event;

    if (!stopped)
    {
        QQueue<SDL_Event> sdlEventQueue;

        firstInputPass(&sdlEventQueue);
        clearUnplugEvents(&sdlEventQueue);
        secondInputPass(&sdlEventQueue);

        clearBitArrayStatusInstances();
    }

    //qDebug() << QTime::currentTime() << ": " << "END";
    if (stopped)
    {
        if (joysticks->count() > 0)
        {
            emit complete(joysticks->value(0));
        }
        emit complete();
        stopped = false;
    }
    else
    {
        QTimer::singleShot(0, eventWorker, SLOT(performWork()));
    }
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
#endif

#ifdef USE_SDL_2
    settings->beginGroup("Mappings");
#endif

    for (int i=0; i < SDL_NumJoysticks(); i++)
    {
#ifdef USE_SDL_2

        SDL_Joystick *joystick = SDL_JoystickOpen(i);

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
            SDL_Joystick *sdlStick = SDL_GameControllerGetJoystick(controller);
            SDL_JoystickID joystickID = SDL_JoystickInstanceID(sdlStick);
            joysticks->insert(joystickID, damncontroller);
            trackcontrollers.insert(joystickID, damncontroller);
        }
        else
        {
            Joystick *curJoystick = new Joystick(joystick, i, settings, this);
            SDL_JoystickID joystickID = SDL_JoystickInstanceID(joystick);
            joysticks->insert(joystickID, curJoystick);
            trackjoysticks.insert(joystickID, curJoystick);
        }
#else
        SDL_Joystick *joystick = SDL_JoystickOpen(i);
        Joystick *curJoystick = new Joystick(joystick, i, settings, this);
        joysticks->insert(i, curJoystick);
#endif
    }

#ifdef USE_SDL_2
    settings->endGroup();
#endif

    emit joysticksRefreshed(joysticks);
}

void InputDaemon::stop()
{
    stopped = true;
}

void InputDaemon::refresh()
{
    stop();

    eventWorker->refresh();

    QEventLoop q;
    connect(eventWorker, SIGNAL(sdlStarted()), &q, SLOT(quit()));
    q.exec();
    disconnect(eventWorker, SIGNAL(sdlStarted()), &q, SLOT(quit()));

    // Put in an extra delay before refreshing the joysticks
    QTimer temp;
    connect(&temp, SIGNAL(timeout()), &q, SLOT(quit()));
    temp.start(100);
    q.exec();

    refreshJoysticks();
    QTimer::singleShot(0, eventWorker, SLOT(performWork()));
}

void InputDaemon::refreshJoystick(InputDevice *joystick)
{
    joystick->reset();

    emit joystickRefreshed(joystick);
}

void InputDaemon::quit()
{
    stopped = true;
    disconnect(eventWorker, SIGNAL(eventRaised()), this, 0);

    // Wait for SDL to finish. Let worker destructor close SDL.
    // Let InputDaemon destructor close thread instance.
    if (graphical)
    {
        QEventLoop q;
        QTimer temptime;

        connect(eventWorker, SIGNAL(eventRaised()), &q, SLOT(quit()));
        connect(&temptime, SIGNAL(timeout()), &q, SLOT(quit()));

        eventWorker->stop();
        temptime.start(500);
        if (eventWorker->isSDLOpen())
        {
            q.exec();
        }
        temptime.stop();
    }
    else
    {
        eventWorker->stop();
    }

    delete eventWorker;
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
                    SDL_Joystick *sdlStick = SDL_GameControllerGetJoystick(controller);
                    joystickID = SDL_JoystickInstanceID(sdlStick);
                    joysticks->insert(joystickID, damncontroller);
                    trackcontrollers.insert(joystickID, damncontroller);
                    emit deviceUpdated(i, damncontroller);
                }
            }
        }
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
        device->deleteLater();
    }
}

void InputDaemon::refreshIndexes()
{
    for (int i = 0; i < SDL_NumJoysticks(); i++)
    {
        SDL_Joystick *joystick = SDL_JoystickOpen(i);
        SDL_JoystickID joystickID = SDL_JoystickInstanceID(joystick);
        InputDevice *tempdevice = joysticks->value(joystickID);
        if (tempdevice)
        {
            tempdevice->setIndex(i);
        }
    }
}

void InputDaemon::addInputDevice(int index)
{
    SDL_Joystick *joystick = SDL_JoystickOpen(index);
    if (joystick)
    {
        SDL_JoystickID tempJoystickID = SDL_JoystickInstanceID(joystick);

        if (!joysticks->contains(tempJoystickID))
        {
            settings->beginGroup("Mappings");

            QString temp;
            SDL_JoystickGUID tempGUID = SDL_JoystickGetGUID(joystick);
            char guidString[65] = {'0'};
            SDL_JoystickGetGUIDString(tempGUID, guidString, sizeof(guidString));
            temp = QString(guidString);

            bool disableGameController = settings->value(QString("%1Disable").arg(temp), false).toBool();

            if (SDL_IsGameController(index) && !disableGameController)
            {
                SDL_GameController *controller = SDL_GameControllerOpen(index);
                if (controller)
                {
                    SDL_Joystick *sdlStick = SDL_GameControllerGetJoystick(controller);
                    SDL_JoystickID tempJoystickID = SDL_JoystickInstanceID(sdlStick);
                    if (!joysticks->contains(tempJoystickID))
                    {
                        GameController *damncontroller = new GameController(controller, index, settings, this);
                        joysticks->insert(tempJoystickID, damncontroller);
                        trackcontrollers.insert(tempJoystickID, damncontroller);

                        settings->endGroup();

                        emit deviceAdded(damncontroller);
                    }
                }
                else
                {
                    settings->endGroup();
                }
            }
            else
            {
                Joystick *curJoystick = new Joystick(joystick, index, settings, this);
                joysticks->insert(tempJoystickID, curJoystick);
                trackjoysticks.insert(tempJoystickID, curJoystick);

                settings->endGroup();

                emit deviceAdded(curJoystick);
            }
        }
    }
}

#endif

InputDeviceBitArrayStatus* InputDaemon::createOrGrabBitStatusEntry(QHash<InputDevice *, InputDeviceBitArrayStatus *> *statusHash,
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
                        InputDeviceBitArrayStatus *temp = createOrGrabBitStatusEntry(&releaseEventsGenerated, joy, false);
                        temp->changeButtonStatus(event.jbutton.button, event.type == SDL_JOYBUTTONUP);

                        InputDeviceBitArrayStatus *pending = createOrGrabBitStatusEntry(&pendingEventValues, joy);
                        pending->changeButtonStatus(event.jbutton.button,
                                                  event.type == SDL_JOYBUTTONDOWN ? true : false);
                        sdlEventQueue->append(event);
                    }
                }

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
                        InputDeviceBitArrayStatus *temp = createOrGrabBitStatusEntry(&releaseEventsGenerated, joy, false);
                        temp->changeHatStatus(event.jhat.hat, event.jhat.value == 0);

                        InputDeviceBitArrayStatus *pending = createOrGrabBitStatusEntry(&pendingEventValues, joy);
                        pending->changeHatStatus(event.jhat.hat, event.jhat.value != 0 ? true : false);
                        sdlEventQueue->append(event);
                    }
                }

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
                        InputDeviceBitArrayStatus *temp = createOrGrabBitStatusEntry(&releaseEventsGenerated, joy, false);
                        temp->changeButtonStatus(event.cbutton.button, event.type == SDL_CONTROLLERBUTTONUP);

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

void InputDaemon::clearUnplugEvents(QQueue<SDL_Event> *sdlEventQueue)
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

        if (bitArraySize > 0 && tempBitArray.count(true) == bitArraySize)
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
                                if (event.jbutton.which != device->getSDLJoystickID())
                                {
                                    tempQueue.enqueue(event);
                                }
                                break;
                            }
                            case SDL_JOYAXISMOTION:
                            {
                                if (event.jaxis.which != device->getSDLJoystickID())
                                {
                                    tempQueue.enqueue(event);
                                }
                                break;
                            }
                            case SDL_JOYHATMOTION:
                            {
                                if (event.jhat.which != device->getSDLJoystickID())
                                {
                                    tempQueue.enqueue(event);
                                }
                                break;
                            }
#ifdef USE_SDL_2
                            case SDL_CONTROLLERAXISMOTION:
                            {
                                if (event.caxis.which != device->getSDLJoystickID())
                                {
                                    tempQueue.enqueue(event);
                                }
                                break;
                            }
                            case SDL_CONTROLLERBUTTONDOWN:
                            case SDL_CONTROLLERBUTTONUP:
                            {
                                if (event.cbutton.which != device->getSDLJoystickID())
                                {
                                    tempQueue.enqueue(event);
                                }
                                break;
                            }
#endif
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
                        button->joyEvent(event.type == SDL_JOYBUTTONDOWN ? true : false);

                        if (!activeDevices.contains(event.jbutton.which))
                        {
                            activeDevices.insert(event.jbutton.which, joy);
                        }
                    }
                }

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
                        axis->joyEvent(event.jaxis.value);

                        if (!activeDevices.contains(event.jaxis.which))
                        {
                            activeDevices.insert(event.jaxis.which, joy);
                        }
                    }
                }

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
                        dpad->joyEvent(event.jhat.value);

                        if (!activeDevices.contains(event.jhat.which))
                        {
                            activeDevices.insert(event.jhat.which, joy);
                        }
                    }
                }

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
                        axis->joyEvent(event.caxis.value);

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
                        button->joyEvent(event.type == SDL_CONTROLLERBUTTONDOWN ? true : false);

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

        // Active possible queued stick and vdpad events.
        QHashIterator<SDL_JoystickID, InputDevice*> activeDevIter(activeDevices);
        while (activeDevIter.hasNext())
        {
            InputDevice *tempDevice = activeDevIter.next().value();
            tempDevice->activatePossibleControlStickEvents();
            tempDevice->activatePossibleVDPadEvents();
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
