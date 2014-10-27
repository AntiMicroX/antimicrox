//#include <QDebug>
#include <QTime>
#include <QTimer>
#include <QEventLoop>
#include <QMapIterator>

#include "inputdaemon.h"

InputDaemon::InputDaemon(QMap<SDL_JoystickID, InputDevice*> *joysticks, AntiMicroSettings *settings, bool graphical, QObject *parent) :
    QObject(parent)
{
    this->joysticks = joysticks;
    this->stopped = false;
    this->graphical = graphical;
    this->settings = settings;

    eventWorker = new SDLEventReader(joysticks);
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
    SDL_Event event;
#ifdef USE_SDL_2
    event.type = SDL_FIRSTEVENT;

#else
    event.type = SDL_NOEVENT;
#endif

    if (!stopped)
    {
        event = eventWorker->getCurrentEvent();

        do
        {
            switch (event.type)
            {
                //qDebug() << QTime::currentTime() << " :";
                case SDL_JOYBUTTONDOWN:
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
                            button->joyEvent(true);
                        }
                    }

                    break;
                }

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
                            button->joyEvent(false);
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
        }
        while (SDL_PollEvent(&event) > 0);
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

        // Check for a grabbed instance of an SDL_QUIT event. If the last event was
        // not an SDL_QUIT event, push an event onto the queue so SdlEventReader
        // will finish properly.
#ifdef USE_SDL_2
        if (event.type != SDL_FIRSTEVENT && event.type != SDL_QUIT)
#else
        if (event.type != SDL_NOEVENT && event.type != SDL_QUIT)
#endif
        {
            event.type = SDL_QUIT;
            SDL_PushEvent(&event);
            QTimer::singleShot(0, eventWorker, SLOT(performWork()));
        }
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
            GameController *damncontroller = new GameController(controller, i, this);
            SDL_Joystick *sdlStick = SDL_GameControllerGetJoystick(controller);
            SDL_JoystickID joystickID = SDL_JoystickInstanceID(sdlStick);
            joysticks->insert(joystickID, damncontroller);
            trackcontrollers.insert(joystickID, damncontroller);
        }
        else
        {
            Joystick *curJoystick = new Joystick(joystick, i, this);
            SDL_JoystickID joystickID = SDL_JoystickInstanceID(joystick);
            joysticks->insert(joystickID, curJoystick);
            trackjoysticks.insert(joystickID, curJoystick);
        }
#else
        SDL_Joystick *joystick = SDL_JoystickOpen(i);
        Joystick *curJoystick = new Joystick(joystick, i, this);
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

    // Wait for SDL to finish. Let worker destructor close SDL.
    // Let InputDaemon destructor close thread instance.
    if (graphical)
    {
        QEventLoop q;
        QTimer temptime;
        connect(eventWorker, SIGNAL(finished()), &q, SLOT(quit()));
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
                    GameController *damncontroller = new GameController(controller, i, this);
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
                        GameController *damncontroller = new GameController(controller, index, this);
                        joysticks->insert(tempJoystickID, damncontroller);
                        trackcontrollers.insert(tempJoystickID, damncontroller);

                        settings->endGroup();

                        emit deviceAdded(damncontroller);
                    }
                }
            }
            else
            {
                Joystick *curJoystick = new Joystick(joystick, index, this);
                joysticks->insert(tempJoystickID, curJoystick);
                trackjoysticks.insert(tempJoystickID, curJoystick);

                settings->endGroup();

                emit deviceAdded(curJoystick);
            }
        }
    }
}

#endif
