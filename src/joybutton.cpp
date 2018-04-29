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

#include "joybutton.h"

#include "messagehandler.h"
#include "setjoystick.h"
#include "inputdevice.h"
#include "vdpad.h"
#include "event.h"
#include "logger.h"
#include "SDL2/SDL_events.h"

#ifdef Q_OS_WIN
  #include "eventhandlerfactory.h"
#endif

#include <cmath>

#include <QDebug>
#include <QThread>
#include <QStringList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QSharedPointer>

const QString JoyButton::xmlName = "button";

// Set default values for many properties.
const int JoyButton::ENABLEDTURBODEFAULT = 100;
const double JoyButton::DEFAULTMOUSESPEEDMOD = 1.0;
double JoyButton::mouseSpeedModifier = JoyButton::DEFAULTMOUSESPEEDMOD;
const int JoyButton::DEFAULTKEYREPEATDELAY = 600; // 600 ms
const int JoyButton::DEFAULTKEYREPEATRATE = 40; // 40 ms. 25 times per second
const JoyButton::JoyMouseCurve JoyButton::DEFAULTMOUSECURVE = JoyButton::EnhancedPrecisionCurve;
const bool JoyButton::DEFAULTTOGGLE = false;
const int JoyButton::DEFAULTTURBOINTERVAL = 0;
const bool JoyButton::DEFAULTUSETURBO = false;
const int JoyButton::DEFAULTMOUSESPEEDX = 50;
const int JoyButton::DEFAULTMOUSESPEEDY = 50;
const int JoyButton::DEFAULTSETSELECTION = -1;
const JoyButton::SetChangeCondition JoyButton::DEFAULTSETCONDITION = JoyButton::SetChangeDisabled;
const JoyButton::JoyMouseMovementMode JoyButton::DEFAULTMOUSEMODE = JoyButton::MouseCursor;
const int JoyButton::DEFAULTSPRINGWIDTH = 0;
const int JoyButton::DEFAULTSPRINGHEIGHT = 0;
const double JoyButton::DEFAULTSENSITIVITY = 1.0;
const int JoyButton::DEFAULTWHEELX = 20;
const int JoyButton::DEFAULTWHEELY = 20;
const bool JoyButton::DEFAULTCYCLERESETACTIVE = false;
const int JoyButton::DEFAULTCYCLERESET = 0;
const bool JoyButton::DEFAULTRELATIVESPRING = false;
const JoyButton::TurboMode JoyButton::DEFAULTTURBOMODE = JoyButton::NormalTurbo;
const double JoyButton::DEFAULTEASINGDURATION = 0.5;
const double JoyButton::MINIMUMEASINGDURATION = 0.2;
const double JoyButton::MAXIMUMEASINGDURATION = 5.0;
const int JoyButton::MINCYCLERESETTIME = 10;
const int JoyButton::MAXCYCLERESETTIME = 60000;

const int JoyButton::DEFAULTMOUSEHISTORYSIZE = 10;
const double JoyButton::DEFAULTWEIGHTMODIFIER = 0.2;
const int JoyButton::MAXIMUMMOUSEHISTORYSIZE = 100;
const double JoyButton::MAXIMUMWEIGHTMODIFIER = 1.0;
const int JoyButton::MAXIMUMMOUSEREFRESHRATE = 16;
int JoyButton::IDLEMOUSEREFRESHRATE = (5 * 20);
const int JoyButton::DEFAULTIDLEMOUSEREFRESHRATE = 100;
const double JoyButton::DEFAULTEXTRACCELVALUE = 2.0;
const double JoyButton::DEFAULTMINACCELTHRESHOLD = 10.0;
const double JoyButton::DEFAULTMAXACCELTHRESHOLD = 100.0;
const double JoyButton::DEFAULTSTARTACCELMULTIPLIER = 0.0;
const double JoyButton::DEFAULTACCELEASINGDURATION = 0.1;
const JoyButton::JoyExtraAccelerationCurve
JoyButton::DEFAULTEXTRAACCELCURVE = JoyButton::LinearAccelCurve;

const int JoyButton::DEFAULTSPRINGRELEASERADIUS = 0;

// Keep references to active keys and mouse buttons.
QHash<int, int> JoyButton::activeKeys;
QHash<int, int> JoyButton::activeMouseButtons;
JoyButtonSlot* JoyButton::lastActiveKey = nullptr;

// Keep track of active Mouse Speed Mod slots.
QList<JoyButtonSlot*> JoyButton::mouseSpeedModList;

// Lists used for cursor mode calculations.
QList<JoyButton::mouseCursorInfo> JoyButton::cursorXSpeeds;
QList<JoyButton::mouseCursorInfo> JoyButton::cursorYSpeeds;

// Lists used for spring mode calculations.
QList<PadderCommon::springModeInfo> JoyButton::springXSpeeds;
QList<PadderCommon::springModeInfo> JoyButton::springYSpeeds;

// Temporary test object to test old mouse time behavior.
QTime JoyButton::testOldMouseTime;

// Helper object to have a single mouse event for all JoyButton
// instances.
JoyButtonMouseHelper JoyButton::mouseHelper;

QTimer JoyButton::staticMouseEventTimer;
QList<JoyButton*> JoyButton::pendingMouseButtons;

// History buffers used for mouse smoothing routine.
QList<double> JoyButton::mouseHistoryX;
QList<double> JoyButton::mouseHistoryY;

// Carry over remainder of a cursor move for the next mouse event.
double JoyButton::cursorRemainderX = 0.0;
double JoyButton::cursorRemainderY = 0.0;

double JoyButton::weightModifier = 0;
int JoyButton::mouseHistorySize = 1;

int JoyButton::mouseRefreshRate = 5;
int JoyButton::springModeScreen = -1;
int JoyButton::gamepadRefreshRate = 10;

#ifdef Q_OS_WIN
JoyKeyRepeatHelper JoyButton::repeatHelper;
#endif

static const double PI = acos(-1.0);

JoyButton::JoyButton(int index, int originset, SetJoystick *parentSet,
                     QObject *parent) :
    QObject(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    #ifndef QT_DEBUG_NO_OUTPUT
    qDebug() << "Thread in Joybutton";
    #endif

    vdpad = nullptr;
    slotiter = nullptr;

    turboTimer.setParent(this);
    pauseTimer.setParent(this);
    holdTimer.setParent(this);
    pauseWaitTimer.setParent(this);
    createDeskTimer.setParent(this);
    releaseDeskTimer.setParent(this);
    mouseWheelVerticalEventTimer.setParent(this);
    mouseWheelHorizontalEventTimer.setParent(this);
    setChangeTimer.setParent(this);
    keyPressTimer.setParent(this);
    delayTimer.setParent(this);
    slotSetChangeTimer.setParent(this);
    activeZoneTimer.setParent(this);

    setChangeTimer.setSingleShot(true);
    slotSetChangeTimer.setSingleShot(true);
    this->parentSet = parentSet;

    connect(&pauseWaitTimer, SIGNAL(timeout()), this, SLOT(pauseWaitEvent()));
    connect(&keyPressTimer, SIGNAL(timeout()), this, SLOT(keyPressEvent()));
    connect(&holdTimer, SIGNAL(timeout()), this, SLOT(holdEvent()));
    connect(&delayTimer, SIGNAL(timeout()), this, SLOT(delayEvent()));
    connect(&createDeskTimer, SIGNAL(timeout()), this, SLOT(waitForDeskEvent()));
    connect(&releaseDeskTimer, SIGNAL(timeout()), this, SLOT(waitForReleaseDeskEvent()));
    connect(&turboTimer, SIGNAL(timeout()), this, SLOT(turboEvent()));
    connect(&mouseWheelVerticalEventTimer, SIGNAL(timeout()), this, SLOT(wheelEventVertical()));
    connect(&mouseWheelHorizontalEventTimer, SIGNAL(timeout()), this, SLOT(wheelEventHorizontal()));
    connect(&setChangeTimer, SIGNAL(timeout()), this, SLOT(checkForSetChange()));
    connect(&slotSetChangeTimer, SIGNAL(timeout()), this, SLOT(slotSetChange()));
    connect(&activeZoneTimer, SIGNAL(timeout()), this, SLOT(buildActiveZoneSummaryString()));

    activeZoneTimer.setInterval(0);
    activeZoneTimer.setSingleShot(true);

    // Will only matter on the first call
    establishMouseTimerConnections();

    // Make sure to call before calling reset
    this->resetProperties();

    this->index = index;
    this->originset = originset;

    quitEvent = true;
}

JoyButton::~JoyButton()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    reset();
}

void JoyButton::queuePendingEvent(bool pressed, bool ignoresets)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    pendingEvent = false;
    pendingPress = false;
    pendingIgnoreSets = false;

    if (this->vdpad != nullptr)
    {
        vdpadPassEvent(pressed, ignoresets);
    }
    else
    {
        pendingEvent = true;
        pendingPress = pressed;
        pendingIgnoreSets = ignoresets;
    }
}

void JoyButton::activatePendingEvent()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (pendingEvent)
    {
        joyEvent(pendingPress, pendingIgnoreSets);

        pendingEvent = false;
        pendingPress = false;
        pendingIgnoreSets = false;
    }
}

bool JoyButton::hasPendingEvent()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return pendingEvent;
}

void JoyButton::clearPendingEvent()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    pendingEvent = false;
    pendingPress = false;
    pendingIgnoreSets = false;
}

void JoyButton::vdpadPassEvent(bool pressed, bool ignoresets)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((this->vdpad != nullptr) && (pressed != isButtonPressed))
    {
        isButtonPressed = pressed;
        if (isButtonPressed)
        {
            emit clicked(index);
        }
        else
        {
            emit released(index);
        }

        if (!ignoresets)
        {
            this->vdpad->queueJoyEvent(ignoresets);
        }
        else
        {
            this->vdpad->joyEvent(pressed, ignoresets);
        }
    }
}

void JoyButton::joyEvent(bool pressed, bool ignoresets)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((this->vdpad != nullptr) && !pendingEvent)
    {
        vdpadPassEvent(pressed, ignoresets);
    }
    else if (ignoreEvents)
    {
        if (pressed != isButtonPressed)
        {
            isButtonPressed = pressed;
            if (isButtonPressed)
            {
                emit clicked(index);
            }
            else
            {
                emit released(index);
            }
        }
    }
    else
    {
        if (pressed != isDown)
        {
            if (pressed)
            {
                emit clicked(index);
                if (updateInitAccelValues)
                {
                    oldAccelMulti = updateOldAccelMulti = 0.0;
                    accelTravel = 0.0;
                }
            }
            else
            {
                emit released(index);
            }

            bool activePress = pressed;
            setChangeTimer.stop();

            if (toggle && pressed)
            {
                isDown = true;
                toggleActiveState = !toggleActiveState;

                if (!isButtonPressed)
                {
                    this->ignoresets = ignoresets;
                    isButtonPressed = !isButtonPressed;

                    ignoreSetQueue.enqueue(ignoresets);
                    isButtonPressedQueue.enqueue(isButtonPressed);
                }
                else
                {
                    activePress = false;
                }
            }
            else if (toggle && !pressed && isDown)
            {
                isDown = false;

                if (!toggleActiveState)
                {
                    this->ignoresets = ignoresets;
                    isButtonPressed = !isButtonPressed;

                    ignoreSetQueue.enqueue(ignoresets);
                    isButtonPressedQueue.enqueue(isButtonPressed);
                }
            }
            else
            {
                this->ignoresets = ignoresets;
                isButtonPressed = isDown = pressed;

                ignoreSetQueue.enqueue(ignoresets);
                isButtonPressedQueue.enqueue(isButtonPressed);
            }

            if (useTurbo)
            {
                if (isButtonPressed && activePress && !turboTimer.isActive())
                {
                    if (cycleResetActive &&
                        (cycleResetHold.elapsed() >= cycleResetInterval) && (slotiter != nullptr))
                    {
                        slotiter->toFront();
                        currentCycle = nullptr;
                        previousCycle = nullptr;
                    }

                    buttonHold.restart();
                    buttonHeldRelease.restart();
                    keyPressHold.restart();
                    cycleResetHold.restart();
                    turboTimer.start();

                    // Newly activated button. Just entered safe zone.
                    if (updateInitAccelValues)
                    {
                        initializeDistanceValues();
                    }
                    currentAccelerationDistance = getAccelerationDistance();

                    Logger::LogDebug(trUtf8("Processing turbo for #%1 - %2")
                                    .arg(parentSet->getInputDevice()->getRealJoyNumber())
                                    .arg(getPartialName()));

                    turboEvent();
                }
                else if (!isButtonPressed && !activePress && turboTimer.isActive())
                {
                    turboTimer.stop();
                    Logger::LogDebug(trUtf8("Finishing turbo for button #%1 - %2")
                                    .arg(parentSet->getInputDevice()->getRealJoyNumber())
                                    .arg(getPartialName()));

                    if (isKeyPressed)
                    {
                        turboEvent();
                    }
                    else
                    {
                        lastDistance = getMouseDistanceFromDeadZone();
                    }

                    activeZoneTimer.start();
                }
            }
            // Toogle is enabled and a controller button change has occurred.
            // Switch to a different distance zone if appropriate
            else if (toggle && !activePress && isButtonPressed)
            {
                bool releasedCalled = distanceEvent();
                if (releasedCalled)
                {
                    quitEvent = true;
                    buttonHold.restart();
                    buttonHeldRelease.restart();
                    keyPressHold.restart();
                    releaseDeskTimer.stop();
                    if (!keyPressTimer.isActive())
                    {
                        waitForDeskEvent();
                    }
                }
            }
            else if (isButtonPressed && activePress)
            {
                if (cycleResetActive &&
                    (cycleResetHold.elapsed() >= cycleResetInterval) && (slotiter != nullptr))
                {
                    slotiter->toFront();
                    currentCycle = nullptr;
                    previousCycle = nullptr;
                }

                buttonHold.restart();
                buttonHeldRelease.restart();
                cycleResetHold.restart();
                keyPressHold.restart();
                releaseDeskTimer.stop();

                // Newly activated button. Just entered safe zone.
                if (updateInitAccelValues)
                {
                    initializeDistanceValues();
                }

                currentAccelerationDistance = getAccelerationDistance();

                Logger::LogDebug(trUtf8("Processing press for button #%1 - %2")
                                .arg(parentSet->getInputDevice()->getRealJoyNumber())
                                .arg(getPartialName()));

                if (!keyPressTimer.isActive())
                {
                    checkForPressedSetChange();
                    if (!setChangeTimer.isActive())
                    {
                        waitForDeskEvent();
                    }
                }


            }
            else if (!isButtonPressed && !activePress)
            {
                Logger::LogDebug(trUtf8("Processing release for button #%1 - %2")
                                .arg(parentSet->getInputDevice()->getRealJoyNumber())
                                .arg(getPartialName()));

                waitForReleaseDeskEvent();
            }

            if (updateInitAccelValues)
            {
                updateLastMouseDistance = false;
                updateStartingMouseDistance = false;
                updateOldAccelMulti = 0.0;
            }

        }
        else if (!useTurbo && isButtonPressed)
        {
            resetAccelerationDistances();
            currentAccelerationDistance = getAccelerationDistance();

            if (!setChangeTimer.isActive())
            {
                bool releasedCalled = distanceEvent();
                if (releasedCalled)
                {
                    Logger::LogDebug(trUtf8("Distance change for button #%1 - %2")
                                    .arg(parentSet->getInputDevice()->getRealJoyNumber())
                                    .arg(getPartialName()));

                    quitEvent = true;
                    buttonHold.restart();
                    buttonHeldRelease.restart();
                    keyPressHold.restart();
                    releaseDeskTimer.stop();
                    if (!keyPressTimer.isActive())
                    {
                        waitForDeskEvent();
                    }
                }
            }
        }
    }

    updateInitAccelValues = true;
}

/**
 * @brief Get 0 indexed number of button
 * @return 0 indexed button index number
 */
int JoyButton::getJoyNumber()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return index;
}

/**
 * @brief Get a 1 indexed number of button
 * @return 1 indexed button index number
 */
int JoyButton::getRealJoyNumber() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return index + 1;
}

void JoyButton::setJoyNumber(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->index = index;
}

void JoyButton::setToggle(bool toggle)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (toggle != this->toggle)
    {
        this->toggle = toggle;
        emit toggleChanged(toggle);
        emit propertyUpdated();
    }
}

void JoyButton::setTurboInterval(int interval)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((interval >= 10) && (interval != this->turboInterval))
    {
        this->turboInterval = interval;
        emit turboIntervalChanged(interval);
        emit propertyUpdated();
    }
    else if ((interval < 10) && (interval != this->turboInterval))
    {
        interval = 0;
        this->setUseTurbo(false);
        this->turboInterval = interval;
        emit turboIntervalChanged(interval);
        emit propertyUpdated();
    }
}

void JoyButton::reset()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    disconnectPropertyUpdatedConnections();

    turboTimer.stop();
    pauseWaitTimer.stop();
    createDeskTimer.stop();
    releaseDeskTimer.stop();
    holdTimer.stop();
    mouseWheelVerticalEventTimer.stop();
    mouseWheelHorizontalEventTimer.stop();
    setChangeTimer.stop();
    keyPressTimer.stop();
    delayTimer.stop();

#ifdef Q_OS_WIN
    repeatHelper.getRepeatTimer()->stop();
#endif

    slotSetChangeTimer.stop();

    if (slotiter != nullptr)
    {
        delete slotiter;
        slotiter = nullptr;
    }

    releaseActiveSlots();
    clearAssignedSlots();

    isButtonPressedQueue.clear();
    ignoreSetQueue.clear();
    mouseEventQueue.clear();
    mouseWheelVerticalEventQueue.clear();
    mouseWheelHorizontalEventQueue.clear();

    resetProperties(); // quitEvent changed here
}

void JoyButton::reset(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButton::reset();
    this->index = index;
}

bool JoyButton::getToggleState()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return toggle;
}

int JoyButton::getTurboInterval()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return turboInterval;
}

void JoyButton::turboEvent()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (!isKeyPressed)
    {
        if (!isButtonPressedQueue.isEmpty())
        {
            ignoreSetQueue.clear();
            isButtonPressedQueue.clear();

            ignoreSetQueue.enqueue(false);
            isButtonPressedQueue.enqueue(isButtonPressed);
        }

        createDeskEvent();
        isKeyPressed = true;
        if (turboTimer.isActive())
        {
            int tempInterval = turboInterval / 2;
            if (turboTimer.interval() != tempInterval)
            {
                turboTimer.start(tempInterval);
            }
        }
    }
    else
    {
        if (!isButtonPressedQueue.isEmpty())
        {
            ignoreSetQueue.enqueue(false);
            isButtonPressedQueue.enqueue(!isButtonPressed);
        }

        releaseDeskEvent();

        isKeyPressed = false;
        if (turboTimer.isActive())
        {
            int tempInterval = turboInterval / 2;
            if (turboTimer.interval() != tempInterval)
            {
                turboTimer.start(tempInterval);
            }

        }

    }
}

bool JoyButton::distanceEvent()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool released = false;

    if (slotiter != nullptr)
    {
        QReadLocker tempLocker(&assignmentsLock);

        bool distanceFound = containsDistanceSlots();

        if (distanceFound)
        {
            double currentDistance = getDistanceFromDeadZone();
            double tempDistance = 0.0;
            JoyButtonSlot *previousDistanceSlot = nullptr;
            QListIterator<JoyButtonSlot*> iter(*getAssignedSlots());

            if (previousCycle != nullptr)
            {
                #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << "find previous Cycle in next steps in assignments and skip to it";
                #endif

                iter.findNext(previousCycle);
            }

            while (iter.hasNext())
            {
                JoyButtonSlot *slot = iter.next();
                int tempcode = slot->getSlotCode();
                if (slot->getSlotMode() == JoyButtonSlot::JoyDistance)
                {
                    tempDistance += (tempcode / 100.0);

                    if (currentDistance < tempDistance)
                    {
                        iter.toBack();
                    }
                    else
                    {
                        previousDistanceSlot = slot;
                    }
                }
                else if (slot->getSlotMode() == JoyButtonSlot::JoyCycle)
                {
                    tempDistance = 0.0;
                    iter.toBack();
                }
            }

            // No applicable distance slot
            if (!previousDistanceSlot)
            {
                if (this->currentDistance != nullptr)
                {
                    // Distance slot is currently active.
                    // Release slots, return iterator to
                    // the front, and nullify currentDistance
                    pauseWaitTimer.stop();
                    holdTimer.stop();

                    // Release stuff
                    releaseActiveSlots();
                    currentPause = currentHold = nullptr;

                    slotiter->toFront();
                    if (previousCycle != nullptr)
                    {
                        slotiter->findNext(previousCycle);
                    }

                    this->currentDistance = nullptr;
                    released = true;
                }
            }
            // An applicable distance slot was found
            else if (previousDistanceSlot)
            {
                if (this->currentDistance != previousDistanceSlot)
                {
                    // Active distance slot is not the applicable slot.
                    // Deactive slots in previous distance range and
                    // activate new slots. Set currentDistance to
                    // new slot.
                    pauseWaitTimer.stop();
                    holdTimer.stop();

                    // Release stuff
                    releaseActiveSlots();
                    currentPause = currentHold = nullptr;

                    #ifndef QT_DEBUG_NO_OUTPUT
                    qDebug() << "Deactive slots in previous range and activate new slots";
                    #endif

                    slotiter->toFront();
                    if (previousCycle != nullptr)
                    {
                        #ifndef QT_DEBUG_NO_OUTPUT
                        qDebug() << "Find previous Cycle in slotiter starting from beginning";
                        #endif

                        slotiter->findNext(previousCycle);
                    }

                    slotiter->findNext(previousDistanceSlot);

                    this->currentDistance = previousDistanceSlot;
                    released = true;
                }
            }
        }
    }

    return released;
}

void JoyButton::createDeskEvent()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    quitEvent = false;

    if (slotiter == nullptr)
    {
        assignmentsLock.lockForRead();
        slotiter = new QListIterator<JoyButtonSlot*>(*getAssignedSlots());
        assignmentsLock.unlock();

        distanceEvent();
    }
    else if (!slotiter->hasPrevious())
    {
        distanceEvent();
    }
    else if (currentCycle != nullptr)
    {
        currentCycle = nullptr;
        distanceEvent();

    }

    assignmentsLock.lockForRead();
    activateSlots();
    assignmentsLock.unlock();

    if (currentCycle != nullptr)
    {
        quitEvent = true;
    }
    else if ((currentPause == nullptr) && (currentHold == nullptr) && !keyPressTimer.isActive())
    {
        quitEvent = true;
    }
}

void JoyButton::activateSlots()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (slotiter != nullptr)
    {
        QWriteLocker tempLocker(&activeZoneLock);

        bool exit = false;
        bool delaySequence = false;
        #ifdef Q_OS_WIN
            bool changeRepeatState = false;
        #endif

        int i = 0;
        while (slotiter->hasNext() && !exit)
        {
            JoyButtonSlot *slot = slotiter->next();
            int tempcode = slot->getSlotCode();
            JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();

            if (mode == JoyButtonSlot::JoyKeyboard)
            {
                i++;

                #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << i << ": It's a JoyKeyboard with code: " << tempcode << " and name: " << slot->getSlotString();
                #endif

                sendevent(slot, true);
                getActiveSlotsLocal().append(slot);
                int oldvalue = activeKeys.value(tempcode, 0) + 1;
                activeKeys.insert(tempcode, oldvalue);
                if (!slot->isModifierKey())
                {
                    #ifndef QT_DEBUG_NO_OUTPUT
                    qDebug() << "There has been assigned a lastActiveKey";
                    #endif

                    lastActiveKey = slot;
                    #ifdef Q_OS_WIN
                        changeRepeatState = true;
                    #endif
                }
                else
                {
                    #ifndef QT_DEBUG_NO_OUTPUT
                    qDebug() << "It's not modifier key. lastActiveKey is null pointer";
                    #endif

                    lastActiveKey = nullptr;

                    #ifdef Q_OS_WIN
                        changeRepeatState = true;
                     #endif
                }
            }
            else if (mode == JoyButtonSlot::JoyMouseButton)
            {
                i++;

                #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << i << ": It's a JoyMouseButton with code: " << tempcode << " and name: " << slot->getSlotString();
                #endif

                if ((tempcode == static_cast<int>(JoyButtonSlot::MouseWheelUp)) ||
                    (tempcode == static_cast<int>(JoyButtonSlot::MouseWheelDown)))
                {
                    slot->getMouseInterval()->restart();
                    wheelVerticalTime.restart();
                    currentWheelVerticalEvent = slot;
                    getActiveSlotsLocal().append(slot);
                    wheelEventVertical();
                    currentWheelVerticalEvent = nullptr;
                }
                else if ((tempcode == static_cast<int>(JoyButtonSlot::MouseWheelLeft)) ||
                         (tempcode == static_cast<int>(JoyButtonSlot::MouseWheelRight)))
                {
                    slot->getMouseInterval()->restart();
                    wheelHorizontalTime.restart();
                    currentWheelHorizontalEvent = slot;
                    getActiveSlotsLocal().append(slot);
                    wheelEventHorizontal();
                    currentWheelHorizontalEvent = nullptr;
                }
                else
                {
                    sendevent(slot, true);
                    getActiveSlotsLocal().append(slot);
                    int oldvalue = activeMouseButtons.value(tempcode, 0) + 1;
                    activeMouseButtons.insert(tempcode, oldvalue);
                }
            }
            else if (mode == JoyButtonSlot::JoyMouseMovement)
            {
                i++;

                #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << i << ": It's a JoyMouseMovement with code: " << tempcode << " and name: " << slot->getSlotString();
                #endif

                slot->getMouseInterval()->restart();

                getActiveSlotsLocal().append(slot);

                if (pendingMouseButtons.size() == 0)
                {
                    mouseHelper.setFirstSpringStatus(true);
                }

                pendingMouseButtons.append(this);
                mouseEventQueue.enqueue(slot);

                // Temporarily lower timer interval. Helps improve mouse control
                // precision on the lower end of an axis.
                if (!staticMouseEventTimer.isActive() || (staticMouseEventTimer.interval() != 0))
                {
                    if (!staticMouseEventTimer.isActive() || (staticMouseEventTimer.interval() == IDLEMOUSEREFRESHRATE))
                    {
                        int tempRate = qBound(0, mouseRefreshRate - gamepadRefreshRate, MAXIMUMMOUSEREFRESHRATE);
                        staticMouseEventTimer.start(tempRate);
                        testOldMouseTime.restart();
                        accelExtraDurationTime.restart();
                    }
                }
            }
            else if (mode == JoyButtonSlot::JoyPause)
            {
                i++;

                #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << i << ": It's a JoyPause with code: " << tempcode << " and name: " << slot->getSlotString();
                #endif

                if (!getActiveSlots().isEmpty())
                {
                    #ifndef QT_DEBUG_NO_OUTPUT
                    qDebug() << "active slots QHash is not empty";
                    #endif

                    if (slotiter->hasPrevious())
                    {
                        i--;
                        slotiter->previous();
                    }
                    delaySequence = true;
                    exit = true;
                }
                // Segment can be ignored on a 0 interval pause
                else if (tempcode > 0)
                {
                    #ifndef QT_DEBUG_NO_OUTPUT
                    qDebug() << "active slots QHash is empty";
                    #endif

                    currentPause = slot;
                    pauseHold.restart();
                    inpauseHold.restart();
                    pauseWaitTimer.start(0);
                    exit = true;
                }
            }
            else if (mode == JoyButtonSlot::JoyHold)
            {
                i++;

                #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << i << ": It's a JoyHold with code: " << tempcode << " and name: " << slot->getSlotString();
                #endif

                currentHold = slot;
                holdTimer.start(0);
                exit = true;
            }
            else if (mode == JoyButtonSlot::JoyDelay)
            {
                i++;

                #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << i << ": It's a JoyDelay with code: " << tempcode << " and name: " << slot->getSlotString();
                #endif

                currentDelay = slot;
                buttonDelay.restart();
                delayTimer.start(0);
                exit = true;
            }
            else if (mode == JoyButtonSlot::JoyCycle)
            {
                i++;

                #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << i << ": It's a JoyCycle with code: " << tempcode << " and name: " << slot->getSlotString();
                #endif

                currentCycle = slot;
                exit = true;
            }
            else if (mode == JoyButtonSlot::JoyDistance)
            {
                i++;

                #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << i << ": It's a JoyDistance with code: " << tempcode << " and name: " << slot->getSlotString();
                #endif

                exit = true;
            }
            else if (mode == JoyButtonSlot::JoyRelease)
            {
                i++;

                #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << i << ": It's a JoyRelease with code: " << tempcode << " and name: " << slot->getSlotString();
                #endif

                if (currentRelease == nullptr)
                {
                    findReleaseEventEnd();
                }
                else if ((currentRelease != nullptr) && getActiveSlots().isEmpty())
                {
                    #ifndef QT_DEBUG_NO_OUTPUT
                    qDebug() << "current is release but activeSlots is empty";
                    #endif

                    exit = true;
                }
                else if ((currentRelease != nullptr) && !getActiveSlots().isEmpty())
                {
                    #ifndef QT_DEBUG_NO_OUTPUT
                    qDebug() << "current is release and activeSlots is not empty";
                    #endif

                    if (slotiter->hasPrevious())
                    {
                        #ifndef QT_DEBUG_NO_OUTPUT
                        qDebug() << "Back to previous slotiter from release";
                        #endif

                        i--;
                        slotiter->previous();
                    }
                    delaySequence = true;
                    exit = true;
                }
            }
            else if (mode == JoyButtonSlot::JoyMouseSpeedMod)
            {
                i++;

                #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << i << ": It's a JoyMouseSpeedMod with code: " << tempcode << " and name: " << slot->getSlotString();
                #endif

                mouseSpeedModifier = tempcode * 0.01;
                mouseSpeedModList.append(slot);
                getActiveSlotsLocal().append(slot);
            }
            else if (mode == JoyButtonSlot::JoyKeyPress)
            {
                i++;

                #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << i << ": It's a JoyKeyPress with code: " << tempcode << " and name: " << slot->getSlotString();
                #endif

                if (getActiveSlots().isEmpty())
                {
                    #ifndef QT_DEBUG_NO_OUTPUT
                    qDebug() << "activeSlots is empty. It's a true delaySequence and assigned currentKeyPress";
                    #endif

                    delaySequence = true;
                    currentKeyPress = slot;
                }
                else
                {
                    #ifndef QT_DEBUG_NO_OUTPUT
                    qDebug() << "activeSlots is not empty. It's a true delaySequence and exit";
                    #endif

                    if (slotiter->hasPrevious())
                    {
                        #ifndef QT_DEBUG_NO_OUTPUT
                        qDebug() << "Back to previous slotiter from JoyKeyPress";
                        #endif

                        i--;
                        slotiter->previous();
                    }
                    delaySequence = true;
                    exit = true;
                }
            }
            else if (mode == JoyButtonSlot::JoyLoadProfile)
            {
                i++;

                #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << i << ": It's a JoyLoadProfile with code: " << tempcode << " and name: " << slot->getSlotString();
                #endif

                releaseActiveSlots();
                slotiter->toBack();
                exit = true;

                QString location = slot->getTextData();
                if (!location.isEmpty())
                {
                    parentSet->getInputDevice()->sendLoadProfileRequest(location);
                }
            }
            else if (mode == JoyButtonSlot::JoySetChange)
            {
                i++;

                #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << i << ": It's a JoySetChange with code: " << tempcode << " and name: " << slot->getSlotString();
                #endif

                getActiveSlotsLocal().append(slot);
            }
            else if (mode == JoyButtonSlot::JoyTextEntry)
            {
                i++;

                #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << i << ": It's a JoyTextEntry with code: " << tempcode << " and name: " << slot->getSlotString();
                #endif

                sendevent(slot, true);
            }
            else if (mode == JoyButtonSlot::JoyExecute)
            {
                i++;

                #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << i << ": It's a JoyExecute with code: " << tempcode << " and name: " << slot->getSlotString();
                #endif

                sendevent(slot, true);
            }
        }

#ifdef Q_OS_WIN
        BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();
#endif

        if (delaySequence && !getActiveSlots().isEmpty())
        {
            keyPressHold.restart();
            keyPressEvent();
        }

#ifdef Q_OS_WIN
        else if (handler && (handler->getIdentifier() == "sendinput") &&
                 changeRepeatState && !useTurbo)
        {
            InputDevice *device = getParentSet()->getInputDevice();
            if (device->isKeyRepeatEnabled())
            {
                if (lastActiveKey && activeSlots.contains(lastActiveKey))
                {
                    repeatHelper.setLastActiveKey(lastActiveKey);
                    repeatHelper.setKeyRepeatRate(device->getKeyRepeatRate());
                    repeatHelper.getRepeatTimer()->start(device->getKeyRepeatDelay());
                }
                else if (repeatHelper.getRepeatTimer()->isActive())
                {
                    repeatHelper.setLastActiveKey(0);
                    repeatHelper.getRepeatTimer()->stop();
                }
            }
        }
#endif

        activeZoneTimer.start();
    }
}

void JoyButton::slotSetChange()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (currentSetChangeSlot != nullptr)
    {
        // Get set change slot and then remove reference.
        int setChangeIndex = currentSetChangeSlot->getSlotCode();
        currentSetChangeSlot = nullptr;

        // Ensure that a change to the current set is not attempted.
        if (setChangeIndex != originset)
        {
            emit released(index);
            emit setChangeActivated(setChangeIndex);
        }
    }
}

/**
 * @brief Calculate mouse movement coordinates for mouse movement slots
 *     currently active.
 */
void JoyButton::mouseEvent()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButtonSlot *buttonslot = 0;
    bool singleShot = false;
    if (currentMouseEvent != nullptr)
    {
        buttonslot = currentMouseEvent;
        singleShot = true;
    }

    if ((buttonslot != nullptr) || !mouseEventQueue.isEmpty())
    {
        updateLastMouseDistance = true;
        updateStartingMouseDistance = true;
        updateOldAccelMulti = 0.0;

        QQueue<JoyButtonSlot*> tempQueue;

        if (buttonslot == nullptr)
        {
            buttonslot = mouseEventQueue.dequeue();
        }

        int timeElapsed = testOldMouseTime.elapsed();

        // Presumed initial mouse movement. Use full duration rather than
        // partial.
        if (staticMouseEventTimer.interval() < mouseRefreshRate)
        {
            timeElapsed = getMouseRefreshRate() + (timeElapsed - staticMouseEventTimer.interval());
        }

        while (buttonslot != nullptr)
        {
            QElapsedTimer* mouseInterval = buttonslot->getMouseInterval();

            int mousedirection = buttonslot->getSlotCode();
            JoyButton::JoyMouseMovementMode mousemode = getMouseMode();

            bool isActive = getActiveSlots().contains(buttonslot);
            if (isActive)
            {
                int mousespeed = 0;

                if (mousemode == JoyButton::MouseCursor)
                {
                    if (mousedirection == static_cast<int>(JoyButtonSlot::MouseRight))
                    {
                        mousespeed = mouseSpeedX;
                    }
                    else if (mousedirection == static_cast<int>(JoyButtonSlot::MouseLeft))
                    {
                        mousespeed = mouseSpeedX;
                    }
                    else if (mousedirection == static_cast<int>(JoyButtonSlot::MouseDown))
                    {
                        mousespeed = mouseSpeedY;
                    }
                    else if (mousedirection == static_cast<int>(JoyButtonSlot::MouseUp))
                    {
                        mousespeed = mouseSpeedY;
                    }

                    double difference = getMouseDistanceFromDeadZone();

                    double mouse1 = 0;
                    double mouse2 = 0;
                    double sumDist = buttonslot->getMouseDistance();
                    JoyMouseCurve currentCurve = getMouseCurve();

                    switch (currentCurve)
                    {
                        case LinearCurve:
                        {
                            break;
                        }
                        case QuadraticCurve:
                        {
                            difference = difference * difference;
                            break;
                        }
                        case CubicCurve:
                        {
                            difference = difference * difference * difference;
                            break;
                        }
                        case QuadraticExtremeCurve:
                        {
                            double temp = difference;
                            difference = difference * difference;
                            difference = (temp >= 0.95) ? (difference * 1.5) : difference;
                            break;
                        }
                        case PowerCurve:
                        {
                            double tempsensitive = qMin(qMax(sensitivity, 1.0e-3), 1.0e+3);
                            double temp = qMin(qMax(pow(difference, 1.0 / tempsensitive), 0.0), 1.0);
                            difference = temp;
                            break;
                        }
                        case EnhancedPrecisionCurve:
                        {
                            // Perform different forms of acceleration depending on
                            // the range of the element from its assigned dead zone.
                            // Useful for more precise controls with an axis.
                            double temp = difference;
                            if (temp <= 0.4)
                            {
                                // Low slope value for really slow acceleration
                                difference = (difference * 0.37);
                            }
                            else if (temp <= 0.75)
                            {
                                // Perform Linear accleration with an appropriate
                                // offset.
                                difference = (difference - 0.252);
                            }
                            else if (temp > 0.75)
                            {
                                // Perform mouse acceleration. Make up the difference
                                // due to the previous two segments. Maxes out at 1.0.
                                difference = (difference * 2.008) - 1.008;
                            }

                            break;
                        }
                        case EasingQuadraticCurve:
                        case EasingCubicCurve:
                        {
                            // Perform different forms of acceleration depending on
                            // the range of the element from its assigned dead zone.
                            // Useful for more precise controls with an axis.
                            double temp = difference;
                            if (temp <= 0.4)
                            {
                                // Low slope value for really slow acceleration
                                difference = (difference * 0.38);

                                // Out of high end. Reset easing status.
                                if (buttonslot->isEasingActive())
                                {
                                    buttonslot->setEasingStatus(false);
                                    buttonslot->getEasingTime()->restart();
                                }
                            }
                            else if (temp <= 0.75)
                            {
                                // Perform Linear accleration with an appropriate
                                // offset.
                                difference = (difference - 0.248);

                                // Out of high end. Reset easing status.
                                if (buttonslot->isEasingActive())
                                {
                                    buttonslot->setEasingStatus(false);
                                    buttonslot->getEasingTime()->restart();
                                }
                            }
                            else if (temp > 0.75)
                            {
                                // Gradually increase the mouse speed until the specified elapsed duration
                                // time has passed.
                                int easingElapsed = buttonslot->getEasingTime()->elapsed();
                                double easingDuration = this->easingDuration; // Time in seconds
                                if (!buttonslot->isEasingActive())
                                {
                                    buttonslot->setEasingStatus(true);
                                    buttonslot->getEasingTime()->restart();
                                    easingElapsed = 0;
                                }

                                // Determine the multiplier to use for the current maximum mouse speed
                                // based on how much time has passed.
                                double elapsedDiff = 1.0;
                                if ((easingDuration > 0.0) && ((easingElapsed * .001) < easingDuration))
                                {
                                    elapsedDiff = ((easingElapsed * .001) / easingDuration);
                                    if (currentCurve == EasingQuadraticCurve)
                                    {
                                        elapsedDiff = (1.5 - 1.0) * elapsedDiff * elapsedDiff + 1.0;
                                    }
                                    else
                                    {
                                        elapsedDiff = (1.5 - 1.0) * (elapsedDiff * elapsedDiff
                                                     * elapsedDiff) + 1.0;
                                    }
                                }
                                else
                                {
                                    elapsedDiff = 1.5;
                                }

                                // Allow gradient control on the high end of an axis.
                                difference = (elapsedDiff * difference);
                                difference = (difference * 1.33067 - 0.496005);
                            }
                            break;
                        }
                    }

                    double distance = 0;
                    difference = (mouseSpeedModifier == 1.0) ? difference : (difference * mouseSpeedModifier);

                    double mintravel = minMouseDistanceAccelThreshold * 0.01;
                    double minstop = qMax(0.05, mintravel);
                    //double currentTravel = getAccelerationDistance() - lastAccelerationDistance;

                    // Last check ensures that acceleration is only applied for the same direction.
                    if (extraAccelerationEnabled && isPartRealAxis() &&
                        (fabs(getAccelerationDistance() - lastAccelerationDistance) >= mintravel) &&
                        (((getAccelerationDistance() - lastAccelerationDistance) >= 0) == (getAccelerationDistance() >= 0)))
                    {
                        double magfactor = extraAccelerationMultiplier;
                        double minfactor = qMax((DEFAULTSTARTACCELMULTIPLIER * 0.001) + 1.0, magfactor * (startAccelMultiplier * 0.01));
                        double maxtravel = maxMouseDistanceAccelThreshold * 0.01;
                        double slope = (magfactor - minfactor)/(maxtravel - mintravel);
                        double intercept = minfactor - (slope * mintravel);

                        double intermediateTravel = qMin(maxtravel, fabs(getAccelerationDistance() - lastAccelerationDistance));
                        if ((currentAccelMulti > 1.0) && (oldAccelMulti == 0.0))
                        {
                            intermediateTravel = qMin(maxtravel, intermediateTravel + mintravel);
                        }

                        double currentAccelMultiTemp = (slope * intermediateTravel + intercept);
                        if (extraAccelCurve == EaseOutSineCurve)
                        {
                            double getMultiDiff2 = ((currentAccelMultiTemp - minfactor) / (extraAccelerationMultiplier - minfactor));
                            currentAccelMultiTemp = (extraAccelerationMultiplier - minfactor) * sin(getMultiDiff2 * (PI/2.0)) + minfactor;
                        }
                        else if (extraAccelCurve == EaseOutQuadAccelCurve)
                        {
                            double getMultiDiff2 = ((currentAccelMultiTemp - minfactor) / (extraAccelerationMultiplier - minfactor));
                            currentAccelMultiTemp = -(extraAccelerationMultiplier - minfactor) * (getMultiDiff2 * (getMultiDiff2 - 2)) + minfactor;
                        }
                        else if (extraAccelCurve == EaseOutCubicAccelCurve)
                        {
                            double getMultiDiff = ((currentAccelMultiTemp - minfactor) / (extraAccelerationMultiplier - minfactor)) - 1;
                            currentAccelMultiTemp = (extraAccelerationMultiplier - minfactor) * ((getMultiDiff) * (getMultiDiff) * (getMultiDiff) + 1) + minfactor;
                        }

                        difference = difference * currentAccelMultiTemp;
                        currentAccelMulti = currentAccelMultiTemp;
                        updateOldAccelMulti = currentAccelMulti;
                        accelTravel = intermediateTravel;
                        accelExtraDurationTime.restart();
                    }
                    else if (extraAccelerationEnabled && isPartRealAxis() && (accelDuration > 0.0) &&
                             (currentAccelMulti > 0.0) &&
                             (fabs(getAccelerationDistance() - startingAccelerationDistance) < minstop))
                    {
                        #ifndef QT_DEBUG_NO_OUTPUT
                        qDebug() << "Keep Trying: " << fabs(getAccelerationDistance() - lastAccelerationDistance);
                        qDebug() << "MIN TRAVEL: " << mintravel;
                        #endif

                        updateStartingMouseDistance = true;
                        double magfactor = extraAccelerationMultiplier;
                        double minfactor = qMax((DEFAULTSTARTACCELMULTIPLIER * 0.001) + 1.0, magfactor * (startAccelMultiplier * 0.01));
                        double maxtravel = maxMouseDistanceAccelThreshold * 0.01;
                        double slope = (magfactor - minfactor)/(maxtravel - mintravel);
                        double intercept = minfactor - (slope * mintravel);

                        int elapsedElapsed = accelExtraDurationTime.elapsed();

                        double intermediateTravel = accelTravel;
                        if (((getAccelerationDistance() - startingAccelerationDistance) >= 0) != (getAccelerationDistance() >= 0))
                        {
                            // Travelling towards dead zone. Decrease acceleration and duration.
                            intermediateTravel = qMax(intermediateTravel - fabs(getAccelerationDistance() - startingAccelerationDistance), mintravel);
                        }

                        // Linear case
                        double currentAccelMultiTemp = (slope * intermediateTravel + intercept);
                        double elapsedDuration = accelDuration *
                                ((currentAccelMultiTemp - minfactor) / (extraAccelerationMultiplier - minfactor));

                        if (extraAccelCurve == EaseOutSineCurve)
                        {
                            double multiDiff = ((currentAccelMultiTemp - minfactor) / (extraAccelerationMultiplier - minfactor));
                            double temp = sin(multiDiff * (PI/2.0));
                            elapsedDuration = accelDuration * temp + 0;
                            currentAccelMultiTemp = (extraAccelerationMultiplier - minfactor) * sin(multiDiff * (PI/2.0)) + minfactor;
                        }
                        else if (extraAccelCurve == EaseOutQuadAccelCurve)
                        {
                            double getMultiDiff2 = ((currentAccelMultiTemp - minfactor) / (extraAccelerationMultiplier - minfactor));
                            double temp = (getMultiDiff2 * (getMultiDiff2 - 2));
                            elapsedDuration = -accelDuration * temp + 0;
                            currentAccelMultiTemp = -(extraAccelerationMultiplier - minfactor) * temp + minfactor;
                        }
                        else if (extraAccelCurve == EaseOutCubicAccelCurve)
                        {
                            double getMultiDiff = ((currentAccelMultiTemp - minfactor) / (extraAccelerationMultiplier - minfactor)) - 1;
                            double temp = ((getMultiDiff) * (getMultiDiff) * (getMultiDiff) + 1);
                            elapsedDuration = accelDuration * temp + 0;
                            currentAccelMultiTemp = (extraAccelerationMultiplier - minfactor) * temp + minfactor;
                        }

                        double tempAccel = currentAccelMultiTemp;
                        double elapsedDiff = 1.0;
                        if ((elapsedDuration > 0.0) && ((elapsedElapsed * 0.001) < elapsedDuration))
                        {
                            elapsedDiff = ((elapsedElapsed * 0.001) / elapsedDuration);
                            elapsedDiff = (1.0 - tempAccel) * (elapsedDiff * elapsedDiff * elapsedDiff) + tempAccel;

                            difference = elapsedDiff * difference;

                            // As acceleration is applied, do not update last
                            // distance values when not necessary.
                            updateStartingMouseDistance = false;
                            updateOldAccelMulti = currentAccelMulti;
                        }
                        else
                        {
                            elapsedDiff = 1.0;
                            currentAccelMulti = 0.0;
                            updateOldAccelMulti = 0.0;
                            accelTravel = 0.0;
                        }
                    }
                    else if (extraAccelerationEnabled && isPartRealAxis())
                    {
                        currentAccelMulti = 0.0;
                        updateStartingMouseDistance = true;
                        oldAccelMulti = updateOldAccelMulti = 0.0;
                        accelTravel = 0.0;
                    }

                    sumDist += difference * (mousespeed * JoyButtonSlot::JOYSPEED * timeElapsed) * 0.001;
                    distance = sumDist;

                    if (mousedirection == static_cast<int>(JoyButtonSlot::MouseRight))
                    {
                        mouse1 = distance;
                    }
                    else if (mousedirection == static_cast<int>(JoyButtonSlot::MouseLeft))
                    {
                        mouse1 = -distance;
                    }
                    else if (mousedirection == static_cast<int>(JoyButtonSlot::MouseDown))
                    {
                        mouse2 = distance;
                    }
                    else if (mousedirection == static_cast<int>(JoyButtonSlot::MouseUp))
                    {
                        mouse2 = -distance;
                    }

                    mouseCursorInfo infoX;
                    infoX.code = mouse1;
                    infoX.slot = buttonslot;
                    cursorXSpeeds.append(infoX);

                    mouseCursorInfo infoY;
                    infoY.code = mouse2;
                    infoY.slot = buttonslot;
                    cursorYSpeeds.append(infoY);
                    sumDist = 0;

                    buttonslot->setDistance(sumDist);
                }
                else if (mousemode == JoyButton::MouseSpring)
                {
                    double mouse1 = -2.0;
                    double mouse2 = -2.0;
                    double difference = getMouseDistanceFromDeadZone();

                    if (mousedirection == static_cast<int>(JoyButtonSlot::MouseRight))
                    {
                        mouse1 = difference;
                        if (mouseHelper.getFirstSpringStatus())
                        {
                            mouse2 = 0.0;
                            mouseHelper.setFirstSpringStatus(false);
                        }
                    }
                    else if (mousedirection == static_cast<int>(JoyButtonSlot::MouseLeft))
                    {
                        mouse1 = -difference;
                        if (mouseHelper.getFirstSpringStatus())
                        {
                            mouse2 = 0.0;
                            mouseHelper.setFirstSpringStatus(false);
                        }
                    }
                    else if (mousedirection == static_cast<int>(JoyButtonSlot::MouseDown))
                    {
                        if (mouseHelper.getFirstSpringStatus())
                        {
                            mouse1 = 0.0;
                            mouseHelper.setFirstSpringStatus(false);
                        }

                        mouse2 = difference;
                    }
                    else if (mousedirection == static_cast<int>(JoyButtonSlot::MouseUp))
                    {
                        if (mouseHelper.getFirstSpringStatus())
                        {
                            mouse1 = 0.0;
                            mouseHelper.setFirstSpringStatus(false);
                        }

                        mouse2 = -difference;
                    }

                    PadderCommon::springModeInfo infoX;
                    infoX.displacementX = mouse1;
                    infoX.springDeadX = 0.0;
                    infoX.width = springWidth;
                    infoX.height = springHeight;
                    infoX.relative = relativeSpring;
                    infoX.screen = springModeScreen;
                    springXSpeeds.append(infoX);

                    PadderCommon::springModeInfo infoY;
                    infoY.displacementY = mouse2;
                    infoY.springDeadY = 0.0;
                    infoY.width = springWidth;
                    infoY.height = springHeight;
                    infoY.relative = relativeSpring;
                    infoY.screen = springModeScreen;
                    springYSpeeds.append(infoY);

                    mouseInterval->restart();
                }

                tempQueue.enqueue(buttonslot);
            }

            if (!mouseEventQueue.isEmpty() && !singleShot)
            {
                buttonslot = mouseEventQueue.dequeue();
            }
            else
            {
                buttonslot = 0;
            }
        }

        if (!tempQueue.isEmpty())
        {
            while (!tempQueue.isEmpty())
            {
                JoyButtonSlot *tempslot = tempQueue.dequeue();
                mouseEventQueue.enqueue(tempslot);
            }
        }
    }
}

void JoyButton::wheelEventVertical()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButtonSlot *buttonslot = 0;
    if (currentWheelVerticalEvent != nullptr)
    {
        buttonslot = currentWheelVerticalEvent;
    }

    if (buttonslot && (wheelSpeedY != 0))
    {
        bool isActive = getActiveSlots().contains(buttonslot);
        if (isActive)
        {
            sendevent(buttonslot, true);
            sendevent(buttonslot, false);
            mouseWheelVerticalEventQueue.enqueue(buttonslot);
            mouseWheelVerticalEventTimer.start(1000 / wheelSpeedY);
        }
        else
        {
            mouseWheelVerticalEventTimer.stop();
        }
    }
    else if (!mouseWheelVerticalEventQueue.isEmpty() && (wheelSpeedY != 0))
    {
        QQueue<JoyButtonSlot*> tempQueue;
        while (!mouseWheelVerticalEventQueue.isEmpty())
        {
            buttonslot = mouseWheelVerticalEventQueue.dequeue();
            bool isActive = getActiveSlots().contains(buttonslot);
            if (isActive)
            {
                sendevent(buttonslot, true);
                sendevent(buttonslot, false);
                tempQueue.enqueue(buttonslot);
            }
        }

        if (!tempQueue.isEmpty())
        {
            mouseWheelVerticalEventQueue = tempQueue;
            mouseWheelVerticalEventTimer.start(1000 / wheelSpeedY);
        }
        else
        {
            mouseWheelVerticalEventTimer.stop();
        }
    }
    else
    {
        mouseWheelVerticalEventTimer.stop();
    }
}

void JoyButton::wheelEventHorizontal()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButtonSlot *buttonslot = 0;
    if (currentWheelHorizontalEvent != nullptr)
    {
        buttonslot = currentWheelHorizontalEvent;
    }

    if (buttonslot && (wheelSpeedX != 0))
    {
        bool isActive = getActiveSlots().contains(buttonslot);
        if (isActive)
        {
            sendevent(buttonslot, true);
            sendevent(buttonslot, false);
            mouseWheelHorizontalEventQueue.enqueue(buttonslot);
            mouseWheelHorizontalEventTimer.start(1000 / wheelSpeedX);
        }
        else
        {
            mouseWheelHorizontalEventTimer.stop();
        }
    }
    else if (!mouseWheelHorizontalEventQueue.isEmpty() && (wheelSpeedX != 0))
    {
        QQueue<JoyButtonSlot*> tempQueue;
        while (!mouseWheelHorizontalEventQueue.isEmpty())
        {
            buttonslot = mouseWheelHorizontalEventQueue.dequeue();
            bool isActive = getActiveSlots().contains(buttonslot);
            if (isActive)
            {
                sendevent(buttonslot, true);
                sendevent(buttonslot, false);
                tempQueue.enqueue(buttonslot);
            }
        }

        if (!tempQueue.isEmpty())
        {
            mouseWheelHorizontalEventQueue = tempQueue;
            mouseWheelHorizontalEventTimer.start(1000 / wheelSpeedX);
        }
        else
        {
            mouseWheelHorizontalEventTimer.stop();
        }
    }
    else
    {
        mouseWheelHorizontalEventTimer.stop();
    }
}

void JoyButton::setUseTurbo(bool useTurbo)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool initialState = this->useTurbo;

    if (useTurbo != this->useTurbo)
    {
        if (useTurbo && this->containsSequence())
        {
            this->useTurbo = false;
        }
        else
        {
            this->useTurbo = useTurbo;
        }

        if (initialState != this->useTurbo)
        {
            emit turboChanged(this->useTurbo);
            emit propertyUpdated();

            if (this->useTurbo && (this->turboInterval == 0))
            {
                this->setTurboInterval(ENABLEDTURBODEFAULT);
            }
        }
    }
}

bool JoyButton::isUsingTurbo()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return useTurbo;
}

QString JoyButton::getXmlName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return this->xmlName;
}

void JoyButton::readConfig(QXmlStreamReader *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (xml->isStartElement() && (xml->name() == getXmlName()))
    {
        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != getXmlName())))
        {
            bool found = readButtonConfig(xml);
            if (!found)
            {
                xml->skipCurrentElement();
            }
            else
            {
                buildActiveZoneSummaryString();
            }

            xml->readNextStartElement();
        }
    }
}

void JoyButton::writeConfig(QXmlStreamWriter *xml)
{

    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (!isDefault())
    {
        xml->writeStartElement(getXmlName());
        xml->writeAttribute("index", QString::number(getRealJoyNumber()));

        if (toggle != DEFAULTTOGGLE)
        {
            xml->writeTextElement("toggle", toggle ? "true" : "false");
        }

        if (turboInterval != DEFAULTTURBOINTERVAL)
        {
            xml->writeTextElement("turbointerval", QString::number(turboInterval));
        }

        if (currentTurboMode != DEFAULTTURBOMODE)
        {
            if (currentTurboMode == GradientTurbo)
            {
                xml->writeTextElement("turbomode", "gradient");
            }
            else if (currentTurboMode == PulseTurbo)
            {
                xml->writeTextElement("turbomode", "pulse");
            }
        }

        if (useTurbo != DEFAULTUSETURBO)
        {
            xml->writeTextElement("useturbo", useTurbo ? "true" : "false");
        }

        if (mouseSpeedX != DEFAULTMOUSESPEEDX)
        {
            xml->writeTextElement("mousespeedx", QString::number(mouseSpeedX));
        }

        if (mouseSpeedY != DEFAULTMOUSESPEEDY)
        {
            xml->writeTextElement("mousespeedy", QString::number(mouseSpeedY));
        }


        if (mouseMode != DEFAULTMOUSEMODE)
        {
            if (mouseMode == MouseCursor)
            {
                xml->writeTextElement("mousemode", "cursor");
            }
            else if (mouseMode == MouseSpring)
            {
                xml->writeTextElement("mousemode", "spring");
                xml->writeTextElement("mousespringwidth", QString::number(springWidth));
                xml->writeTextElement("mousespringheight", QString::number(springHeight));
            }
        }

        if (mouseCurve != DEFAULTMOUSECURVE)
        {
            if (mouseCurve == LinearCurve)
            {
                xml->writeTextElement("mouseacceleration", "linear");
            }
            else if (mouseCurve == QuadraticCurve)
            {
                xml->writeTextElement("mouseacceleration", "quadratic");
            }
            else if (mouseCurve == CubicCurve)
            {
                xml->writeTextElement("mouseacceleration", "cubic");
            }
            else if (mouseCurve == QuadraticExtremeCurve)
            {
                xml->writeTextElement("mouseacceleration", "quadratic-extreme");
            }
            else if (mouseCurve == PowerCurve)
            {
                xml->writeTextElement("mouseacceleration", "power");
                xml->writeTextElement("mousesensitivity", QString::number(sensitivity));
            }
            else if (mouseCurve == EnhancedPrecisionCurve)
            {
                xml->writeTextElement("mouseacceleration", "precision");
            }
            else if (mouseCurve == EasingQuadraticCurve)
            {
                xml->writeTextElement("mouseacceleration", "easing-quadratic");
            }
            else if (mouseCurve == EasingCubicCurve)
            {
                xml->writeTextElement("mouseacceleration", "easing-cubic");
            }
        }

        if (wheelSpeedX != DEFAULTWHEELX)
        {
            xml->writeTextElement("wheelspeedx", QString::number(wheelSpeedX));
        }

        if (wheelSpeedY != DEFAULTWHEELY)
        {
            xml->writeTextElement("wheelspeedy", QString::number(wheelSpeedY));
        }

        if (!isModifierButton())
        {
            if (setSelectionCondition != SetChangeDisabled)
            {
                xml->writeTextElement("setselect", QString::number(setSelection+1));

                QString temptext = QString();
                if (setSelectionCondition == SetChangeOneWay)
                {
                    temptext = "one-way";
                }
                else if (setSelectionCondition == SetChangeTwoWay)
                {
                    temptext = "two-way";
                }
                else if (setSelectionCondition == SetChangeWhileHeld)
                {
                    temptext = "while-held";
                }
                xml->writeTextElement("setselectcondition", temptext);
            }
        }

        if (!actionName.isEmpty())
        {
            xml->writeTextElement("actionname", actionName);
        }

        if (cycleResetActive)
        {
            xml->writeTextElement("cycleresetactive", "true");
        }

        if (cycleResetInterval >= MINCYCLERESETTIME)
        {
            xml->writeTextElement("cycleresetinterval", QString::number(cycleResetInterval));
        }

        if (relativeSpring == true)
        {
            xml->writeTextElement("relativespring", "true");
        }

        if (easingDuration != DEFAULTEASINGDURATION)
        {
            xml->writeTextElement("easingduration", QString::number(easingDuration));
        }

        if (extraAccelerationEnabled)
        {
            xml->writeTextElement("extraacceleration", "true");
        }

        if (extraAccelerationMultiplier != DEFAULTEXTRACCELVALUE)
        {
            xml->writeTextElement("accelerationmultiplier", QString::number(extraAccelerationMultiplier));
        }

        if (startAccelMultiplier != DEFAULTSTARTACCELMULTIPLIER)
        {
            xml->writeTextElement("startaccelmultiplier", QString::number(startAccelMultiplier));
        }

        if (minMouseDistanceAccelThreshold != DEFAULTMINACCELTHRESHOLD)
        {
            xml->writeTextElement("minaccelthreshold", QString::number(minMouseDistanceAccelThreshold));
        }

        if (maxMouseDistanceAccelThreshold != DEFAULTMAXACCELTHRESHOLD)
        {
            xml->writeTextElement("maxaccelthreshold", QString::number(maxMouseDistanceAccelThreshold));
        }

        if (accelDuration != DEFAULTACCELEASINGDURATION)
        {
            xml->writeTextElement("accelextraduration", QString::number(accelDuration));
        }

        if (springDeadCircleMultiplier != DEFAULTSPRINGRELEASERADIUS)
        {
            xml->writeTextElement("springreleaseradius", QString::number(springDeadCircleMultiplier));
        }

        if (extraAccelCurve != DEFAULTEXTRAACCELCURVE)
        {
            QString temp = QString();
            if (extraAccelCurve == LinearAccelCurve)
            {
                temp = "linear";
            }
            else if (extraAccelCurve == EaseOutSineCurve)
            {
                temp = "easeoutsine";
            }
            else if (extraAccelCurve == EaseOutQuadAccelCurve)
            {
                temp = "easeoutquad";
            }
            else if (extraAccelCurve == EaseOutCubicAccelCurve)
            {
                temp = "easeoutcubic";
            }

            if (!temp.isEmpty())
            {
                xml->writeTextElement("extraaccelerationcurve", temp);
            }
        }

        // Write information about assigned slots.
        if (!getAssignedSlots()->isEmpty())
        {
            xml->writeStartElement("slots");
            QListIterator<JoyButtonSlot*> iter(*getAssignedSlots());
            while (iter.hasNext())
            {
                JoyButtonSlot *buttonslot = iter.next();
                buttonslot->writeConfig(xml);
            }
            xml->writeEndElement();
        }


        xml->writeEndElement();
    }
}

bool JoyButton::readButtonConfig(QXmlStreamReader *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool found = false;

    if ((xml->name() == "toggle") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        if (temptext == "true")
        {
            this->setToggle(true);
        }
    }
    else if ((xml->name() == "turbointerval") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        this->setTurboInterval(tempchoice);
    }
    else if ((xml->name() == "turbomode") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        if (temptext == "normal")
        {
            this->setTurboMode(NormalTurbo);
        }
        else if (temptext == "gradient")
        {
            this->setTurboMode(GradientTurbo);
        }
        else if (temptext == "pulse")
        {
            this->setTurboMode(PulseTurbo);
        }
    }
    else if ((xml->name() == "useturbo") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        if (temptext == "true")
        {
            this->setUseTurbo(true);
        }
    }
    else if ((xml->name() == "mousespeedx") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        this->setMouseSpeedX(tempchoice);
    }
    else if ((xml->name() == "mousespeedy") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        this->setMouseSpeedY(tempchoice);
    }
    else if ((xml->name() == "cycleresetactive") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        if (temptext == "true")
        {
            this->setCycleResetStatus(true);
        }
    }
    else if ((xml->name() == "cycleresetinterval") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        if (tempchoice >= MINCYCLERESETTIME)
        {
            this->setCycleResetTime(tempchoice);
        }
    }
    else if ((xml->name() == "slots") && xml->isStartElement())
    {
        found = true;
        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != "slots")))
        {
            if ((xml->name() == "slot") && xml->isStartElement())
            {
                JoyButtonSlot *buttonslot = new JoyButtonSlot(this);
                buttonslot->readConfig(xml);
                bool validSlot = buttonslot->isValidSlot();
                if (validSlot)
                {
                    bool inserted = insertAssignedSlot(buttonslot, false);
                    if (!inserted)
                    {
                        delete buttonslot;
                        buttonslot = 0;
                    }
                }
                else
                {
                    delete buttonslot;
                    buttonslot = 0;
                }
            }
            else
            {
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }
    }
    else if ((xml->name() == "setselect") && xml->isStartElement())
    {
        if (!isModifierButton())
        {
            found = true;
            QString temptext = xml->readElementText();
            int tempchoice = temptext.toInt();
            if ((tempchoice >= 0) && (tempchoice <= InputDevice::NUMBER_JOYSETS))
            {
                this->setChangeSetSelection(tempchoice - 1, false);
            }
        }
    }
    else if ((xml->name() == "setselectcondition") && xml->isStartElement())
    {
        if (!isModifierButton())
        {
            found = true;
            QString temptext = xml->readElementText();
            SetChangeCondition tempcondition = SetChangeDisabled;
            if (temptext == "one-way")
            {
                tempcondition = SetChangeOneWay;
            }
            else if (temptext == "two-way")
            {
                tempcondition = SetChangeTwoWay;
            }
            else if (temptext == "while-held")
            {
                tempcondition = SetChangeWhileHeld;
            }

            if (tempcondition != SetChangeDisabled)
            {
                this->setChangeSetCondition(tempcondition, false, false);
            }
        }
    }
    else if ((xml->name() == "mousemode") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        if (temptext == "cursor")
        {
            setMouseMode(MouseCursor);
        }
        else if (temptext == "spring")
        {
            setMouseMode(MouseSpring);
        }
    }
    else if ((xml->name() == "mouseacceleration") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        if (temptext == "linear")
        {
            setMouseCurve(LinearCurve);
        }
        else if (temptext == "quadratic")
        {
            setMouseCurve(QuadraticCurve);
        }
        else if (temptext == "cubic")
        {
            setMouseCurve(CubicCurve);
        }
        else if (temptext == "quadratic-extreme")
        {
            setMouseCurve(QuadraticExtremeCurve);
        }
        else if (temptext == "power")
        {
            setMouseCurve(PowerCurve);
        }
        else if (temptext == "precision")
        {
            setMouseCurve(EnhancedPrecisionCurve);
        }
        else if (temptext == "easing-quadratic")
        {
            setMouseCurve(EasingQuadraticCurve);
        }
        else if (temptext == "easing-cubic")
        {
            setMouseCurve(EasingCubicCurve);
        }
    }
    else if ((xml->name() == "mousespringwidth") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        setSpringWidth(tempchoice);
    }
    else if ((xml->name() == "mousespringheight") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        setSpringHeight(tempchoice);
    }
    else if ((xml->name() == "mousesensitivity") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        double tempchoice = temptext.toDouble();
        setSensitivity(tempchoice);
    }
    else if ((xml->name() == "actionname") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        if (!temptext.isEmpty())
        {
            setActionName(temptext);
        }
    }
    else if ((xml->name() == "wheelspeedx") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        setWheelSpeedX(tempchoice);
    }
    else if ((xml->name() == "wheelspeedy") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        setWheelSpeedY(tempchoice);
    }
    else if ((xml->name() == "relativespring") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        if (temptext == "true")
        {
            this->setSpringRelativeStatus(true);
        }
    }
    else if ((xml->name() == "easingduration") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        double tempchoice = temptext.toDouble();
        setEasingDuration(tempchoice);
    }
    else if ((xml->name() == "extraacceleration") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        if (temptext == "true")
        {
            setExtraAccelerationStatus(true);
        }
    }
    else if ((xml->name() == "accelerationmultiplier") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        double tempchoice = temptext.toDouble();
        setExtraAccelerationMultiplier(tempchoice);
    }
    else if ((xml->name() == "startaccelmultiplier") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        double tempchoice = temptext.toDouble();
        setStartAccelMultiplier(tempchoice);
    }
    else if ((xml->name() == "minaccelthreshold") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        double tempchoice = temptext.toDouble();
        setMinAccelThreshold(tempchoice);
    }
    else if ((xml->name() == "maxaccelthreshold") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        double tempchoice = temptext.toDouble();
        setMaxAccelThreshold(tempchoice);
    }
    else if ((xml->name() == "accelextraduration") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        double tempchoice = temptext.toDouble();
        setAccelExtraDuration(tempchoice);
    }
    else if ((xml->name() == "extraaccelerationcurve") && xml->isStartElement())
    {
        found = true;

        QString temptext = xml->readElementText();
        JoyExtraAccelerationCurve tempcurve = DEFAULTEXTRAACCELCURVE;
        if (temptext == "linear")
        {
            tempcurve = LinearAccelCurve;
        }
        else if (temptext == "easeoutsine")
        {
            tempcurve = EaseOutSineCurve;
        }
        else if (temptext == "easeoutquad")
        {
            tempcurve = EaseOutQuadAccelCurve;
        }
        else if (temptext == "easeoutcubic")
        {
            tempcurve = EaseOutCubicAccelCurve;
        }

        setExtraAccelerationCurve(tempcurve);
    }
    else if ((xml->name() == "springreleaseradius") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        if (!relativeSpring)
        {
            setSpringDeadCircleMultiplier(tempchoice);
        }
    }

    return found;
}

QString JoyButton::getName(bool forceFullFormat, bool displayNames)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString newlabel = getPartialName(forceFullFormat, displayNames);
    newlabel.append(": ");
    if (!actionName.isEmpty() && displayNames)
    {
        newlabel.append(actionName);
    }
    else
    {
        newlabel.append(getCalculatedActiveZoneSummary());
    }
    return newlabel;
}

QString JoyButton::getPartialName(bool forceFullFormat, bool displayNames) const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();
    if (!buttonName.isEmpty() && displayNames)
    {
        if (forceFullFormat)
        {
            temp.append(trUtf8("Button")).append(" ");
        }
        temp.append(buttonName);
    }
    else if (!defaultButtonName.isEmpty())
    {
        if (forceFullFormat)
        {
            temp.append(trUtf8("Button")).append(" ");
        }
        temp.append(defaultButtonName);
    }
    else
    {
        temp.append(trUtf8("Button")).append(" ").append(QString::number(getRealJoyNumber()));
    }

    return temp;
}

/**
 * @brief Generate a string representing a summary of the slots currently
 *     assigned to a button
 * @return String of currently assigned slots
 */
QString JoyButton::getSlotsSummary()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString newlabel = QString();
    int slotCount = getAssignedSlots()->size();

    if (slotCount > 0)
    {
        QListIterator<JoyButtonSlot*> iter(*getAssignedSlots());
        QStringList stringlist = QStringList();

        int i = 0;
        while (iter.hasNext())
        {
            JoyButtonSlot *slot = iter.next();
            stringlist.append(slot->getSlotString());
            i++;

            if ((i > 4) && iter.hasNext())
            {
                stringlist.append(" ...");
                iter.toBack();
            }
        }

        newlabel = stringlist.join(", ");
    }
    else
    {
        newlabel = newlabel.append(trUtf8("[NO KEY]"));
    }

    return newlabel;
}

/**
 * @brief Generate a string that represents slots that will be activated or
 *     slots that are currently active if a button is pressed
 * @return String of currently applicable slots for a button
 */
QString JoyButton::getActiveZoneSummary()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QList<JoyButtonSlot*> tempList = getActiveZoneList();
    QString temp = buildActiveZoneSummary(tempList);
    return temp;
}

QString JoyButton::getCalculatedActiveZoneSummary()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    activeZoneStringLock.lockForRead();
    QString temp = this->activeZoneString;
    activeZoneStringLock.unlock();
    return temp;
}

/**
 * @brief Generate active zone string and notify other objects.
 */
void JoyButton::buildActiveZoneSummaryString()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    activeZoneStringLock.lockForWrite();
    this->activeZoneString = getActiveZoneSummary();
    activeZoneStringLock.unlock();

    emit activeZoneChanged();
}

/**
 * @brief Generate active zone string but do not notify any other object.
 */
void JoyButton::localBuildActiveZoneSummaryString()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    activeZoneStringLock.lockForWrite();
    this->activeZoneString = getActiveZoneSummary();
    activeZoneStringLock.unlock();
}

QString JoyButton::buildActiveZoneSummary(QList<JoyButtonSlot *> &tempList)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString newlabel = QString();
    QListIterator<JoyButtonSlot*> iter(tempList);
    QStringList stringlist = QStringList();
    int i = 0;
    bool slotsActive = !getActiveSlots().isEmpty();

    if (setSelectionCondition == SetChangeOneWay)
    {
        newlabel.append(trUtf8("[Set %1 1W]").arg(setSelection+1));
        if (iter.hasNext())
        {
            newlabel.append(" ");
        }
    }
    else if (setSelectionCondition == SetChangeTwoWay)
    {
        newlabel = newlabel.append(trUtf8("[Set %1 2W]").arg(setSelection+1));
        if (iter.hasNext())
        {
           newlabel.append(" ");
        }
    }

    if (setSelectionCondition == SetChangeWhileHeld)
    {
        newlabel.append(trUtf8("[Set %1 WH]").arg(setSelection+1));
    }
    else if (iter.hasNext())
    {
        bool behindHold = false;
        while (iter.hasNext())
        {
            JoyButtonSlot *slot = iter.next();
            JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();
            switch (mode)
            {
                case JoyButtonSlot::JoyKeyboard:
                case JoyButtonSlot::JoyMouseButton:
                case JoyButtonSlot::JoyMouseMovement:
                {
                    QString temp = slot->getSlotString();
                    if (behindHold)
                    {
                        temp.prepend("[H] ");
                        behindHold = false;
                    }

                    stringlist.append(temp);
                    i++;
                    break;
                }
                case JoyButtonSlot::JoyKeyPress:
                {
                    // Skip slot if a press time slot was inserted.
                    break;
                }
                case JoyButtonSlot::JoyHold:
                {
                    if (!slotsActive && (i == 0))
                    {
                        // If button is not active and first slot is a hold,
                        // keep processing slots but take note of the hold.
                        behindHold = true;
                    }
                    else
                    {
                        // Move iter to back so loop will end.
                        iter.toBack();
                    }

                    break;
                }
                case JoyButtonSlot::JoyLoadProfile:
                case JoyButtonSlot::JoySetChange:
                case JoyButtonSlot::JoyTextEntry:
                case JoyButtonSlot::JoyExecute:
                {
                    QString temp = slot->getSlotString();
                    if (behindHold)
                    {
                        temp.prepend("[H] ");
                        behindHold = false;
                    }

                    stringlist.append(temp);
                    i++;
                    break;
                }
                case JoyButtonSlot::JoyRelease:
                {
                    if (currentRelease == nullptr)
                    {
                        findReleaseEventIterEnd(&iter);
                    }

                    break;
                }

                case JoyButtonSlot::JoyDistance:
                {
                    iter.toBack();
                    break;
                }

                case JoyButtonSlot::JoyDelay:
                {
                    iter.toBack();
                    break;
                }
                case JoyButtonSlot::JoyCycle:
                {
                    iter.toBack();
                    break;
                }
                default:

                break;
            }

            if ((i > 4) && iter.hasNext())
            {
                stringlist.append(" ...");
                iter.toBack();
            }
        }

        newlabel.append(stringlist.join(", "));
    }
    else if (setSelectionCondition == SetChangeDisabled)
    {
        newlabel.append(trUtf8("[NO KEY]"));
    }

    return newlabel;
}

QList<JoyButtonSlot*> JoyButton::getActiveZoneList()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QListIterator<JoyButtonSlot*> activeSlotsIter(getActiveSlots());
    QListIterator<JoyButtonSlot*> assignmentsIter(*getAssignedSlots());

    QListIterator<JoyButtonSlot*> *iter = 0;
    QReadWriteLock *tempLock = 0;

    activeZoneLock.lockForRead();
    int numActiveSlots = getActiveSlots().size();
    activeZoneLock.unlock();

    if (numActiveSlots > 0)
    {
        tempLock = &activeZoneLock;
        iter = &activeSlotsIter;
    }
    else
    {
        tempLock = &assignmentsLock;
        iter = &assignmentsIter;
    }

    QReadLocker tempLocker(tempLock);
    Q_UNUSED(tempLocker);

    if (tempLock == &assignmentsLock)
    {
        if (previousCycle != nullptr)
        {
            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "if there exists previous Cycle, find it in activeSlots";
            #endif

            iter->findNext(previousCycle);
        }
    }

    QList<JoyButtonSlot*> tempSlotList;

    if ((setSelectionCondition != SetChangeWhileHeld) && iter->hasNext())
    {
        while (iter->hasNext())
        {
            JoyButtonSlot *slot = iter->next();
            JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();
            switch (mode)
            {
                case JoyButtonSlot::JoyKeyboard:
                case JoyButtonSlot::JoyMouseButton:
                case JoyButtonSlot::JoyMouseMovement:
                {
                    tempSlotList.append(slot);
                    break;
                }
                case JoyButtonSlot::JoyKeyPress:
                case JoyButtonSlot::JoyHold:
                case JoyButtonSlot::JoyLoadProfile:
                case JoyButtonSlot::JoySetChange:
                case JoyButtonSlot::JoyTextEntry:
                case JoyButtonSlot::JoyExecute:
                {
                    tempSlotList.append(slot);
                    break;
                }
                case JoyButtonSlot::JoyRelease:
                {
                    if (currentRelease == nullptr)
                    {
                        findReleaseEventIterEnd(iter);
                    }

                    break;
                }
                case JoyButtonSlot::JoyDistance:
                {
                    iter->toBack();
                    break;
                }
                case JoyButtonSlot::JoyCycle:
                {
                    iter->toBack();
                    break;
                }
            case JoyButtonSlot::JoyPause:
            {

                break;
            }
            case JoyButtonSlot::JoyMouseSpeedMod:
            {

                break;
            }
            case JoyButtonSlot::JoyDelay:
            {

                break;
            }

            }
        }
    }

    return tempSlotList;
}

/**
 * @brief Generate a string representing all the currently assigned slots for
 *     a button
 * @return String representing all assigned slots for a button
 */
QString JoyButton::getSlotsString()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString label = QString();

    if (getAssignedSlots()->size() > 0)
    {
        #ifndef QT_DEBUG_NO_OUTPUT
        qDebug() << "There is more assignments than 0 in getSlotsString(): " << getAssignedSlots()->count();
        #endif

        QListIterator<JoyButtonSlot*> iter(*getAssignedSlots());
        QStringList stringlist = QStringList();

        while (iter.hasNext())
        {
            JoyButtonSlot *slot = iter.next();
            stringlist.append(slot->getSlotString());
        }

        label = stringlist.join(", ");
    }
    else
    {
        #ifndef QT_DEBUG_NO_OUTPUT
        qDebug() << "There is no assignments for button in getSlotsString()";
        #endif

        label = label.append(trUtf8("[NO KEY]"));
    }

    return label;
}


void JoyButton::setCustomName(QString name)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    customName = name;
}

QString JoyButton::getCustomName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return customName;
}

/**
 * @brief Create new JoyButtonSlot with data provided and append the new slot
 *     an existing to the assignment list.
 * @param Native virtual code being used.
 * @param Mode of the slot.
 * @return Whether the new slot was successfully added to the assignment list.
 */
bool JoyButton::setAssignedSlot(int code, JoyButtonSlot::JoySlotInputAction mode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool slotInserted = false;
    JoyButtonSlot *slot = new JoyButtonSlot(code, mode, this);
    if (slot->getSlotMode() == JoyButtonSlot::JoyDistance)
    {
        if ((slot->getSlotCode() >= 1) && (slot->getSlotCode() <= 100))
        {
            double tempDistance = getTotalSlotDistance(slot);
            if (tempDistance <= 1.0)
            {
                assignmentsLock.lockForWrite();
                getAssignmentsLocal().append(slot);
                assignmentsLock.unlock();

                buildActiveZoneSummaryString();
                slotInserted = true;
            }
        }
    }
    else if (slot->getSlotCode() >= 0)
    {
        assignmentsLock.lockForWrite();
        getAssignmentsLocal().append(slot);
        assignmentsLock.unlock();

        buildActiveZoneSummaryString();
        slotInserted = true;
    }

    if (slotInserted)
    {
        checkTurboCondition(slot);
        emit slotsChanged();
    }
    else
    {
        if (slot)
        {
            delete slot;
            slot = 0;
        }
    }

    return slotInserted;
}

/**
 * @brief Create new JoyButtonSlot with data provided and append the new slot
 *     an existing to the assignment list.
 * @param Native virtual code being used.
 * @param Qt key alias used for abstracting native virtual code.
 * @param Mode of the slot.
 * @return Whether the new slot was successfully added to the assignment list.
 */
bool JoyButton::setAssignedSlot(int code, int alias,
                                JoyButtonSlot::JoySlotInputAction mode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool slotInserted = false;
    JoyButtonSlot *slot = new JoyButtonSlot(code, alias, mode, this);

    if (slot->getSlotMode() == JoyButtonSlot::JoyDistance)
    {
        if ((slot->getSlotCode() >= 1) && (slot->getSlotCode() <= 100))
        {
            double tempDistance = getTotalSlotDistance(slot);
            if (tempDistance <= 1.0)
            {
                assignmentsLock.lockForWrite();
                getAssignmentsLocal().append(slot);
                assignmentsLock.unlock();

                buildActiveZoneSummaryString();
                slotInserted = true;
            }
        }
    }
    else if (slot->getSlotCode() >= 0)
    {
        assignmentsLock.lockForWrite();
        getAssignmentsLocal().append(slot);
        assignmentsLock.unlock();

        buildActiveZoneSummaryString();
        slotInserted = true;
    }

    if (slotInserted)
    {
        checkTurboCondition(slot);
        emit slotsChanged();
    }
    else
    {
        if (slot != nullptr)
        {
            delete slot;
            slot = nullptr;
        }
    }

    return slotInserted;
}

/**
 * @brief Create new JoyButtonSlot with data provided and replace an existing
 *     slot in the assignment list if one exists.
 * @param Native virtual code being used.
 * @param Qt key alias used for abstracting native virtual code.
 * @param Index number in the list.
 * @param Mode of the slot.
 * @return Whether the new slot was successfully added to the assignment list.
 */
bool JoyButton::setAssignedSlot(int code, int alias, int index,
                                JoyButtonSlot::JoySlotInputAction mode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool permitSlot = true;

    JoyButtonSlot *slot = new JoyButtonSlot(code, alias, mode, this);
    if (slot->getSlotMode() == JoyButtonSlot::JoyDistance)
    {
        if ((slot->getSlotCode() >= 1) && (slot->getSlotCode() <= 100))
        {
            double tempDistance = getTotalSlotDistance(slot);
            if (tempDistance > 1.0)
            {
                permitSlot = false;
            }
        }
        else
        {
            permitSlot = false;
        }
    }
    else if (slot->getSlotCode() < 0)
    {
        permitSlot = false;
    }

    if (permitSlot)
    {
        assignmentsLock.lockForWrite();

        if ((index >= 0) && (index < getAssignedSlots()->count()))
        {
            // Insert slot and move existing slots.
            JoyButtonSlot *temp = getAssignedSlots()->at(index);
            if (temp)
            {
                delete temp;
                temp = 0;
            }

            getAssignmentsLocal().replace(index, slot);
        }
        else if (index >= getAssignmentsLocal().count())
        {
            // Append code into a new slot
            getAssignmentsLocal().append(slot);
        }

        checkTurboCondition(slot);
        assignmentsLock.unlock();

        buildActiveZoneSummaryString();

        emit slotsChanged();
    }
    else
    {
        if (slot)
        {
            delete slot;
            slot = 0;
        }
    }

    return permitSlot;
}

/**
 * @brief Create new JoyButtonSlot with data provided and insert it into
 *     assignments list if it is valid.
 * @param Native virtual code being used.
 * @param Qt key alias used for abstracting native virtual code.
 * @param Index number in the list.
 * @param Mode of the slot.
 * @return Whether the new slot was successfully added to the assignment list.
 */
bool JoyButton::insertAssignedSlot(int code, int alias, int index,
                                   JoyButtonSlot::JoySlotInputAction mode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool permitSlot = true;

    JoyButtonSlot *slot = new JoyButtonSlot(code, alias, mode, this);
    if (slot->getSlotMode() == JoyButtonSlot::JoyDistance)
    {
        if ((slot->getSlotCode() >= 1) && (slot->getSlotCode() <= 100))
        {
            double tempDistance = getTotalSlotDistance(slot);
            if (tempDistance > 1.0)
            {
                permitSlot = false;
            }
        }
        else
        {
            permitSlot = false;
        }
    }
    else if (slot->getSlotCode() < 0)
    {
        permitSlot = false;
    }

    if (permitSlot)
    {
        assignmentsLock.lockForWrite();

        if ((index >= 0) && (index < getAssignedSlots()->count()))
        {
            // Insert new slot into list. Move old slots if needed.
            getAssignmentsLocal().insert(index, slot);
        }
        else if (index >= getAssignedSlots()->count())
        {
            // Append new slot into list.
            getAssignmentsLocal().append(slot);
        }

        #ifndef QT_DEBUG_NO_OUTPUT
        qDebug() << "assignments variable in joybutton has now: " << getAssignedSlots()->count() << " input slots";
        #endif

        checkTurboCondition(slot);
        assignmentsLock.unlock();

        buildActiveZoneSummaryString();

        emit slotsChanged();
    }
    else
    {
        if (slot)
        {
            delete slot;
            slot = 0;
        }
    }

    return permitSlot;
}

bool JoyButton::insertAssignedSlot(JoyButtonSlot *newSlot, bool updateActiveString)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool permitSlot = false;

    if (newSlot->getSlotMode() == JoyButtonSlot::JoyDistance)
    {
        if ((newSlot->getSlotCode() >= 1) && (newSlot->getSlotCode() <= 100))
        {
            double tempDistance = getTotalSlotDistance(newSlot);
            if (tempDistance <= 1.0)
            {
                permitSlot = true;
            }
        }
    }
    else if (newSlot->getSlotMode() == JoyButtonSlot::JoyLoadProfile)
    {
        permitSlot = true;
    }
    else if (newSlot->getSlotMode() == JoyButtonSlot::JoyTextEntry &&
             !newSlot->getTextData().isEmpty())
    {
        permitSlot = true;
    }
    else if (newSlot->getSlotMode() == JoyButtonSlot::JoyExecute &&
             !newSlot->getTextData().isEmpty())
    {
        permitSlot = true;
    }
    else if (newSlot->getSlotCode() >= 0)
    {
        permitSlot = true;
    }

    if (permitSlot)
    {
        assignmentsLock.lockForWrite();
        checkTurboCondition(newSlot);
        getAssignmentsLocal().append(newSlot);
        assignmentsLock.unlock();

        if (updateActiveString)
        {
            buildActiveZoneSummaryString();
        }


        emit slotsChanged();
    }

    return permitSlot;
}


bool JoyButton::setAssignedSlot(JoyButtonSlot *otherSlot, int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool permitSlot = false;
    JoyButtonSlot *newslot = new JoyButtonSlot(otherSlot, this);

    if (newslot->getSlotMode() == JoyButtonSlot::JoyDistance)
    {
        if ((newslot->getSlotCode() >= 1) && (newslot->getSlotCode() <= 100))
        {
            double tempDistance = getTotalSlotDistance(newslot);
            if (tempDistance <= 1.0)
            {
                permitSlot = true;
            }
        }
    }
    else if (newslot->getSlotMode() == JoyButtonSlot::JoyLoadProfile)
    {
        permitSlot = true;
    }
    else if ((newslot->getSlotMode() == JoyButtonSlot::JoyTextEntry) &&
             !newslot->getTextData().isEmpty())
    {
        permitSlot = true;
    }
    else if ((newslot->getSlotMode() == JoyButtonSlot::JoyExecute) &&
             !newslot->getTextData().isEmpty())
    {
        permitSlot = true;
    }
    else if (newslot->getSlotCode() >= 0)
    {
        permitSlot = true;
    }

    if (permitSlot)
    {
        assignmentsLock.lockForWrite();

        checkTurboCondition(newslot);

        if ((index >= 0) && (index < getAssignedSlots()->count()))
        {
            // Slot already exists. Override code and place into desired slot
            JoyButtonSlot *temp = getAssignedSlots()->at(index);
            if (temp)
            {
                delete temp;
                temp = 0;
            }

            getAssignmentsLocal().replace(index, newslot);
        }
        else if (index >= getAssignmentsLocal().count())
        {
            // Append code into a new slot
            getAssignmentsLocal().append(newslot);
        }

        assignmentsLock.unlock();

        buildActiveZoneSummaryString();

        emit slotsChanged();
    }
    else
    {
        delete newslot;
        newslot = 0;
    }

    return permitSlot;
}


QList<JoyButtonSlot*>* JoyButton::getAssignedSlots()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return &assignments;
}

QList<JoyButtonSlot*> const& JoyButton::getActiveSlots() {

    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return activeSlots;
}


void JoyButton::setMouseSpeedX(int speed)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((speed >= 1) && (speed <= 300))
    {
        mouseSpeedX = speed;
        emit propertyUpdated();
    }
}


int JoyButton::getMouseSpeedX()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return mouseSpeedX;
}

void JoyButton::setMouseSpeedY(int speed)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((speed >= 1) && (speed <= 300))
    {
        mouseSpeedY = speed;
        emit propertyUpdated();
    }
}


int JoyButton::getMouseSpeedY()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return mouseSpeedY;
}


void JoyButton::setChangeSetSelection(int index, bool updateActiveString)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((index >= -1) && (index <= 7))
    {
        setSelection = index;

        if (updateActiveString)
        {
            buildActiveZoneSummaryString();
        }

        emit propertyUpdated();
    }
}

int JoyButton::getSetSelection()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return setSelection;
}

void JoyButton::setChangeSetCondition(SetChangeCondition condition,
                                      bool passive, bool updateActiveString)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    SetChangeCondition oldCondition = setSelectionCondition;

    if ((condition != setSelectionCondition) && !passive)
    {
        if ((condition == SetChangeWhileHeld) || (condition == SetChangeTwoWay))
        {
            // Set new condition
            emit setAssignmentChanged(index, setSelection, condition);
        }
        else if ((setSelectionCondition == SetChangeWhileHeld) || (setSelectionCondition == SetChangeTwoWay))
        {
            // Remove old condition
            emit setAssignmentChanged(index, setSelection, SetChangeDisabled);
        }

        setSelectionCondition = condition;
    }
    else if (passive)
    {
        setSelectionCondition = condition;
    }

    if (setSelectionCondition == SetChangeDisabled)
    {
        setChangeSetSelection(-1);
    }

    if (setSelectionCondition != oldCondition)
    {
        if (updateActiveString)
        {
            buildActiveZoneSummaryString();
        }

        emit propertyUpdated();
    }
}

JoyButton::SetChangeCondition JoyButton::getChangeSetCondition()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return setSelectionCondition;
}

bool JoyButton::getButtonState()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return isButtonPressed;
}

int JoyButton::getOriginSet()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return originset;
}

void JoyButton::pauseWaitEvent()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (currentPause != nullptr)
    {
        if (!isButtonPressedQueue.isEmpty() && createDeskTimer.isActive())
        {
            if (slotiter != nullptr)
            {
                slotiter->toBack();

                bool lastIgnoreSetState = ignoreSetQueue.last();
                bool lastIsButtonPressed = isButtonPressedQueue.last();
                ignoreSetQueue.clear();
                isButtonPressedQueue.clear();

                ignoreSetQueue.enqueue(lastIgnoreSetState);
                isButtonPressedQueue.enqueue(lastIsButtonPressed);
                currentPause = nullptr;
                currentRelease = nullptr;
                releaseDeskTimer.stop();
                pauseWaitTimer.stop();

                slotiter->toFront();
                if (previousCycle != nullptr)
                {
                    slotiter->findNext(previousCycle);
                }

                quitEvent = true;
                keyPressHold.restart();
            }
        }
    }

    if (currentPause != nullptr)
    {
        // If release timer is active, temporarily
        // disable it
        if (releaseDeskTimer.isActive())
        {
            releaseDeskTimer.stop();
        }

        if (inpauseHold.elapsed() < currentPause->getSlotCode())
        {
            int proposedInterval = currentPause->getSlotCode() - inpauseHold.elapsed();
            proposedInterval = proposedInterval > 0 ? proposedInterval : 0;
            int newTimerInterval = qMin(10, proposedInterval);
            pauseWaitTimer.start(newTimerInterval);
        }
        else
        {
            pauseWaitTimer.stop();
            createDeskTimer.stop();
            currentPause = nullptr;
            createDeskEvent();

            // If release timer was disabled but if the button
            // is not pressed, restart the release timer.
            if (!releaseDeskTimer.isActive() && (isButtonPressedQueue.isEmpty() || !isButtonPressedQueue.last()))
            {
                waitForReleaseDeskEvent();
            }
        }
    }
    else
    {
        pauseWaitTimer.stop();
    }
}

void JoyButton::checkForSetChange()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (!ignoreSetQueue.isEmpty() && !isButtonPressedQueue.isEmpty())
    {
        bool tempFinalState = isButtonPressedQueue.last();
        bool tempFinalIgnoreSetsState = ignoreSetQueue.last();

        if (!tempFinalIgnoreSetsState)
        {
            if (!tempFinalState && (setSelectionCondition == SetChangeOneWay) && (setSelection > -1))
            {
                // If either timer is currently active,
                // stop the timer
                if (createDeskTimer.isActive())
                {
                    createDeskTimer.stop();
                }

                if (releaseDeskTimer.isActive())
                {
                    releaseDeskTimer.stop();
                }

                isButtonPressedQueue.clear();
                ignoreSetQueue.clear();

                emit released(index);
                emit setChangeActivated(setSelection);
            }
            else if (!tempFinalState && (setSelectionCondition == SetChangeTwoWay) && (setSelection > -1))
            {
                // If either timer is currently active,
                // stop the timer
                if (createDeskTimer.isActive())
                {
                    createDeskTimer.stop();
                }

                if (releaseDeskTimer.isActive())
                {
                    releaseDeskTimer.stop();
                }

                isButtonPressedQueue.clear();
                ignoreSetQueue.clear();

                emit released(index);
                emit setChangeActivated(setSelection);
            }
            else if ((setSelectionCondition == SetChangeWhileHeld) && (setSelection > -1))
            {
                if (tempFinalState)
                {
                    whileHeldStatus = true;
                }
                else if (!tempFinalState)
                {
                    whileHeldStatus = false;
                }

                // If either timer is currently active,
                // stop the timer
                if (createDeskTimer.isActive())
                {
                    createDeskTimer.stop();
                }

                if (releaseDeskTimer.isActive())
                {
                    releaseDeskTimer.stop();
                }

                isButtonPressedQueue.clear();
                ignoreSetQueue.clear();

                emit released(index);
                emit setChangeActivated(setSelection);
            }
        }

        // Clear queue except for a press if it is last in
        if (!isButtonPressedQueue.isEmpty())
        {
            isButtonPressedQueue.clear();
            if (tempFinalState)
            {
                isButtonPressedQueue.enqueue(tempFinalState);
            }
        }

        // Clear queue except for a press if it is last in
        if (!ignoreSetQueue.isEmpty())
        {
            bool tempFinalIgnoreSetsState = ignoreSetQueue.last();
            ignoreSetQueue.clear();
            if (tempFinalState)
            {
                ignoreSetQueue.enqueue(tempFinalIgnoreSetsState);
            }
        }
    }
}

void JoyButton::waitForDeskEvent()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (quitEvent && !isButtonPressedQueue.isEmpty() && isButtonPressedQueue.last())
    {
        if (createDeskTimer.isActive())
        {
            keyPressTimer.stop();
            createDeskTimer.stop();
            releaseDeskTimer.stop();
            createDeskEvent();
        }
        else
        {
            keyPressTimer.stop();
            releaseDeskTimer.stop();
            createDeskEvent();
        }
    }
    else if (!createDeskTimer.isActive())
    {
#ifdef Q_CC_MSVC
        createDeskTimer.start(5);
        releaseDeskTimer.stop();
#else
        createDeskTimer.start(0);
        releaseDeskTimer.stop();
#endif
    }
    else if (createDeskTimer.isActive())
    {
        // Decrease timer interval of active timer.
        createDeskTimer.start(0);
        releaseDeskTimer.stop();
    }
}

void JoyButton::waitForReleaseDeskEvent()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (quitEvent && !keyPressTimer.isActive())
    {
        if (releaseDeskTimer.isActive())
        {
            releaseDeskTimer.stop();
        }
        createDeskTimer.stop();
        keyPressTimer.stop();
        releaseDeskEvent();
    }
    else if (!releaseDeskTimer.isActive())
    {
#ifdef Q_CC_MSVC
        releaseDeskTimer.start(1);
        createDeskTimer.stop();
#else
        releaseDeskTimer.start(1);
        createDeskTimer.stop();
#endif
    }
    else if (releaseDeskTimer.isActive())
    {
        createDeskTimer.stop();
    }
}

bool JoyButton::containsSequence()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = false;

    assignmentsLock.lockForRead();
    QListIterator<JoyButtonSlot*> tempiter(*getAssignedSlots());
    while (tempiter.hasNext())
    {
        JoyButtonSlot *slot = tempiter.next();
        JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();
        if ((mode == JoyButtonSlot::JoyPause) ||
            (mode == JoyButtonSlot::JoyHold) ||
            (mode == JoyButtonSlot::JoyDistance)
           )
        {
            result = true;
            tempiter.toBack();
        }
    }
    assignmentsLock.unlock();

    return result;
}

void JoyButton::holdEvent()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (currentHold != nullptr)
    {
        bool currentlyPressed = false;
        if (!isButtonPressedQueue.isEmpty())
        {
            currentlyPressed = isButtonPressedQueue.last();
        }

        // Activate hold event
        if (currentlyPressed && (buttonHold.elapsed() > currentHold->getSlotCode()))
        {
            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << buttonHold.elapsed() << " > " << currentHold->getSlotCode();
            qDebug() << "Activate hold event";
            #endif

            releaseActiveSlots();
            currentHold = nullptr;
            holdTimer.stop();
            buttonHold.restart();
            createDeskEvent();
        }
        // Elapsed time has not occurred
        else if (currentlyPressed)
        {
            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "Elapsed time has not occurred, because buttonHold: " << buttonHold.elapsed() << " is not greater than currentHoldCode: " << currentHold->getSlotCode();
            #endif

            int holdTime = currentHold->getSlotCode();
            int proposedInterval = holdTime - buttonHold.elapsed();
            proposedInterval = (proposedInterval > 0) ? proposedInterval : 0;
            int newTimerInterval = qMin(10, proposedInterval);
            holdTimer.start(newTimerInterval);
        }
        // Pre-emptive release
        else
        {
            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "Hold button is not pressed";
            #endif

            currentHold = nullptr;
            holdTimer.stop();

            if (slotiter != nullptr)
            {
                #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << "slotiter exists";
                #endif

                findHoldEventEnd();
                createDeskEvent();
            }
        }
    }
    else
    {
        holdTimer.stop();
    }
}


void JoyButton::delayEvent()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (currentDelay != nullptr)
    {
        bool currentlyPressed = false;
        if (!isButtonPressedQueue.isEmpty())
        {
            currentlyPressed = isButtonPressedQueue.last();
        }

        // Delay time has elapsed. Continue processing slots.
        if ((currentDelay != nullptr) && (buttonDelay.elapsed() > currentDelay->getSlotCode()))
        {
            currentDelay = nullptr;
            delayTimer.stop();
            buttonDelay.restart();
            createDeskEvent();
        }
        // Elapsed time has not occurred
        else if (currentlyPressed)
        {
            int delayTime = currentDelay->getSlotCode();
            int proposedInterval = delayTime - buttonDelay.elapsed();
            proposedInterval = (proposedInterval > 0) ? proposedInterval : 0;
            int newTimerInterval = qMin(10, proposedInterval);
            delayTimer.start(newTimerInterval);
        }
        // Pre-emptive release
        else
        {
            currentDelay = nullptr;
            delayTimer.stop();
        }
    }
    else
    {
        delayTimer.stop();
    }
}

void JoyButton::releaseDeskEvent(bool skipsetchange)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    quitEvent = false;

    pauseWaitTimer.stop();
    holdTimer.stop();
    createDeskTimer.stop();
    keyPressTimer.stop();
    delayTimer.stop();
#ifdef Q_OS_WIN
    repeatHelper.getRepeatTimer()->stop();
#endif
    setChangeTimer.stop();

    releaseActiveSlots();
    if (!isButtonPressedQueue.isEmpty() && (currentRelease == nullptr))
    {
        releaseSlotEvent();
    }
    else if (currentRelease != nullptr)
    {
        currentRelease = nullptr;
    }

    if (!skipsetchange && (setSelectionCondition != SetChangeDisabled) &&
        !isButtonPressedQueue.isEmpty() && (currentRelease == nullptr))
    {
        bool tempButtonPressed = isButtonPressedQueue.last();
        bool tempFinalIgnoreSetsState = ignoreSetQueue.last();

        if (!tempButtonPressed && !tempFinalIgnoreSetsState)
        {
            if ((setSelectionCondition == SetChangeWhileHeld) && whileHeldStatus)
            {
                setChangeTimer.start(0);
            }
            else if (setSelectionCondition != SetChangeWhileHeld)
            {
                setChangeTimer.start();
            }
        }
        else
        {
            bool tempFinalState = false;
            if (!isButtonPressedQueue.isEmpty())
            {
                tempFinalState = isButtonPressedQueue.last();
                isButtonPressedQueue.clear();
                if (tempFinalState)
                {
                    isButtonPressedQueue.enqueue(tempFinalState);
                }
            }

            if (!ignoreSetQueue.isEmpty())
            {
                bool tempFinalIgnoreSetsState = ignoreSetQueue.last();
                ignoreSetQueue.clear();
                if (tempFinalState)
                {
                    ignoreSetQueue.enqueue(tempFinalIgnoreSetsState);
                }
            }
        }
    }
    else
    {
        bool tempFinalState = false;
        if (!isButtonPressedQueue.isEmpty())
        {
            tempFinalState = isButtonPressedQueue.last();
            isButtonPressedQueue.clear();
            if (tempFinalState || (currentRelease != nullptr))
            {
                isButtonPressedQueue.enqueue(tempFinalState);
            }
        }

        if (!ignoreSetQueue.isEmpty())
        {
            bool tempFinalIgnoreSetsState = ignoreSetQueue.last();
            ignoreSetQueue.clear();
            if (tempFinalState || (currentRelease != nullptr))
            {
                ignoreSetQueue.enqueue(tempFinalIgnoreSetsState);
            }
        }
    }

    if (currentRelease == nullptr)
    {
        lastAccelerationDistance = 0.0;
        currentAccelMulti = 0.0;
        currentAccelerationDistance = 0.0;
        startingAccelerationDistance = 0.0;
        oldAccelMulti = updateOldAccelMulti = 0.0;
        accelTravel = 0.0;
        accelExtraDurationTime.restart();

        lastMouseDistance = 0.0;
        currentMouseDistance = 0.0;
        updateStartingMouseDistance = true;

        if ((slotiter != nullptr) && !slotiter->hasNext())
        {
            // At the end of the list of assignments.

            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "There is end of slotiter. Set currentCycle and previousCycle as null pointers";
            #endif

            currentCycle = nullptr;
            previousCycle = nullptr;
            slotiter->toFront();
        }
        else if ((slotiter != nullptr) && slotiter->hasNext() && (currentCycle != nullptr))
        {
            // Cycle at the end of a segment.
            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "There exists next element in slotiter and exists currentCycle. Skip to currentCycle in slotiter starting from beginning";
            #endif

            slotiter->toFront();
            slotiter->findNext(currentCycle);
        }
        else if ((slotiter != nullptr) && slotiter->hasPrevious() && slotiter->hasNext() && (currentCycle == nullptr))
        {
            // Check if there is a cycle action slot after
            // current slot. Useful after dealing with pause
            // actions.

            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "There exists next element and previous element in slotiter but doesn't exists currentCycle. From current point in slotiter find JoyButtonSlot::JoyCycle as slotMode and assign to currentCycle";
            #endif

            JoyButtonSlot *tempslot = nullptr;
            bool exit = false;
            while (slotiter->hasNext() && !exit)
            {
                tempslot = slotiter->next();
                if (tempslot->getSlotMode() == JoyButtonSlot::JoyCycle)
                {
                    currentCycle = tempslot;
                    exit = true;
                }
            }

            // Didn't find any cycle. Move iterator
            // to the front.
            if (currentCycle == nullptr)
            {
                #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << "Didn't find any cycle. Back to start of slotiter";
                #endif

                slotiter->toFront();
                previousCycle = nullptr;
            }
        }

        if (currentCycle != nullptr)
        {
            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "currentCycle exists and previousCycle will be current but current will be null pointer";
            #endif

            previousCycle = currentCycle;
            currentCycle = nullptr;
        }
        else if ((slotiter != nullptr) && slotiter->hasNext() && containsReleaseSlots())
        {
            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "Slotiter has next element on the list. In assignments exists JoyButtonSlot::JoyRelease starting from current point. CurrentCycle and previousCycle are set null pointers now";
            #endif

            currentCycle = nullptr;
            previousCycle = nullptr;
            slotiter->toFront();
        }

        this->currentDistance = nullptr;
        this->currentKeyPress = nullptr;
        quitEvent = true;
    }


}

/**
 * @brief Get the distance that an element is away from its assigned
 *     dead zone
 * @return Normalized distance away from dead zone
 */
double JoyButton::getDistanceFromDeadZone()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    double distance = 0.0;
    if (isButtonPressed)
    {
        distance = 1.0;
    }

    return distance;
}

double JoyButton::getAccelerationDistance()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return this->getDistanceFromDeadZone();
}

/**
 * @brief Get the distance factor that should be used for mouse movement
 * @return Distance factor that should be used for mouse movement
 */
double JoyButton::getMouseDistanceFromDeadZone()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return this->getDistanceFromDeadZone();
}

double JoyButton::getTotalSlotDistance(JoyButtonSlot *slot)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    double tempDistance = 0.0;

    QListIterator<JoyButtonSlot*> iter(*getAssignedSlots());
    while (iter.hasNext())
    {
        JoyButtonSlot *currentSlot = iter.next();
        int tempcode = currentSlot->getSlotCode();
        JoyButtonSlot::JoySlotInputAction mode = currentSlot->getSlotMode();
        if (mode == JoyButtonSlot::JoyDistance)
        {
            tempDistance += tempcode / 100.0;
            if (slot == currentSlot)
            {
                // Current slot found. Go to end of iterator
                // so loop will exit
                iter.toBack();
            }
        }
        // Reset tempDistance
        else if (mode == JoyButtonSlot::JoyCycle)
        {
            tempDistance = 0.0;
        }
    }

    return tempDistance;
}

bool JoyButton::containsDistanceSlots()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = false;
    QListIterator<JoyButtonSlot*> iter(*getAssignedSlots());
    while (iter.hasNext())
    {
        JoyButtonSlot *slot = iter.next();
        if (slot->getSlotMode() == JoyButtonSlot::JoyDistance)
        {
            result = true;
            iter.toBack();
        }
    }

    return result;
}

void JoyButton::clearAssignedSlots(bool signalEmit)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QListIterator<JoyButtonSlot*> iter(*getAssignedSlots());
    while (iter.hasNext())
    {
        JoyButtonSlot *slot = iter.next();
        if (slot != nullptr)
        {
            delete slot;
            slot = nullptr;
        }
    }

    getAssignmentsLocal().clear();

    if (signalEmit)
    {
        emit slotsChanged();
    }
}

void JoyButton::removeAssignedSlot(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QWriteLocker tempAssignLocker(&assignmentsLock);

    if ((index >= 0) && (index < getAssignedSlots()->size()))
    {
        JoyButtonSlot *slot = getAssignedSlots()->takeAt(index);
        if (slot)
        {
            delete slot;
            slot = 0;
        }

        tempAssignLocker.unlock();

        buildActiveZoneSummaryString();
        emit slotsChanged();
    }
}

void JoyButton::clearSlotsEventReset(bool clearSignalEmit)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QWriteLocker tempAssignLocker(&assignmentsLock);

    turboTimer.stop();
    pauseWaitTimer.stop();
    createDeskTimer.stop();
    releaseDeskTimer.stop();
    holdTimer.stop();
    mouseWheelVerticalEventTimer.stop();
    mouseWheelHorizontalEventTimer.stop();
    setChangeTimer.stop();
    keyPressTimer.stop();
    delayTimer.stop();
    activeZoneTimer.stop();

#ifdef Q_OS_WIN
    repeatHelper.getRepeatTimer()->stop();
#endif

    if (slotiter != nullptr)
    {
        delete slotiter;
        slotiter = nullptr;
    }

    releaseActiveSlots();
    clearAssignedSlots(clearSignalEmit);

    isButtonPressedQueue.clear();
    ignoreSetQueue.clear();
    mouseEventQueue.clear();
    mouseWheelVerticalEventQueue.clear();
    mouseWheelHorizontalEventQueue.clear();

    #ifndef QT_DEBUG_NO_OUTPUT
    qDebug() << "all current slots and previous slots ale cleared";
    #endif

    currentCycle = nullptr;
    previousCycle = nullptr;
    currentPause = nullptr;
    currentHold = nullptr;
    currentDistance = nullptr;
    currentRawValue = 0;
    currentMouseEvent = nullptr;
    currentRelease = nullptr;
    currentWheelVerticalEvent = nullptr;
    currentWheelHorizontalEvent = nullptr;
    currentKeyPress = nullptr;
    currentDelay = nullptr;

    isKeyPressed = isButtonPressed = false;

    activeZoneTimer.start();
    quitEvent = true;


}

void JoyButton::eventReset()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QWriteLocker tempAssignLocker(&assignmentsLock);

    turboTimer.stop();
    pauseWaitTimer.stop();
    createDeskTimer.stop();
    releaseDeskTimer.stop();
    holdTimer.stop();
    mouseWheelVerticalEventTimer.stop();
    mouseWheelHorizontalEventTimer.stop();
    setChangeTimer.stop();
    keyPressTimer.stop();
    delayTimer.stop();
    activeZoneTimer.stop();

#ifdef Q_OS_WIN
    repeatHelper.getRepeatTimer()->stop();
#endif

    if (slotiter != nullptr)
    {
        delete slotiter;
        slotiter = nullptr;
    }

    isButtonPressedQueue.clear();
    ignoreSetQueue.clear();
    mouseEventQueue.clear();
    mouseWheelVerticalEventQueue.clear();
    mouseWheelHorizontalEventQueue.clear();

    #ifndef QT_DEBUG_NO_OUTPUT
    qDebug() << "all current slots and previous slots ale cleared";
    #endif

    currentCycle = nullptr;
    previousCycle = nullptr;
    currentPause = nullptr;
    currentHold = nullptr;
    currentDistance = nullptr;
    currentRawValue = 0;
    currentMouseEvent = nullptr;
    currentRelease = nullptr;
    currentWheelVerticalEvent = nullptr;
    currentWheelHorizontalEvent = nullptr;
    currentKeyPress = nullptr;
    currentDelay = nullptr;

    isKeyPressed = isButtonPressed = false;

    releaseActiveSlots();

    quitEvent = true;
}

void JoyButton::releaseActiveSlots()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (!getActiveSlots().isEmpty())
    {
        QWriteLocker tempLocker(&activeZoneLock);

        #ifdef Q_OS_WIN
            bool changeRepeatState = false;
        #endif
        QListIterator<JoyButtonSlot*> iter(getActiveSlots());

        iter.toBack();
        while (iter.hasPrevious())
        {
            JoyButtonSlot *slot = iter.previous();
            int tempcode = slot->getSlotCode();
            JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();

            if (mode == JoyButtonSlot::JoyKeyboard)
            {
                int referencecount = activeKeys.value(tempcode, 1) - 1;
                if (referencecount <= 0)
                {
                    sendevent(slot, false);
                    activeKeys.remove(tempcode);
                    #ifdef Q_OS_WIN
                        changeRepeatState = true;
                    #endif
                }
                else
                {
                    activeKeys.insert(tempcode, referencecount);
                }

                if ((lastActiveKey == slot) && (referencecount <= 0))
                {
                    lastActiveKey = nullptr;
                }
            }
            else if (mode == JoyButtonSlot::JoyMouseButton)
            {
                if ((tempcode != static_cast<int>(JoyButtonSlot::MouseWheelUp)) &&
                    (tempcode != static_cast<int>(JoyButtonSlot::MouseWheelDown)) &&
                    (tempcode != static_cast<int>(JoyButtonSlot::MouseWheelLeft)) &&
                    (tempcode != static_cast<int>(JoyButtonSlot::MouseWheelRight)))
                {
                    int referencecount = (activeMouseButtons.value(tempcode, 1) - 1);
                    if (referencecount <= 0)
                    {
                        sendevent(slot, false);
                        activeMouseButtons.remove(tempcode);
                    }
                    else
                    {
                        activeMouseButtons.insert(tempcode, referencecount);
                    }
                }
                else if ((tempcode == static_cast<int>(JoyButtonSlot::MouseWheelUp)) ||
                         (tempcode == static_cast<int>(JoyButtonSlot::MouseWheelDown)))
                {
                    mouseWheelVerticalEventQueue.removeAll(slot);
                }
                else if ((tempcode == static_cast<int>(JoyButtonSlot::MouseWheelLeft)) ||
                         (tempcode == static_cast<int>(JoyButtonSlot::MouseWheelRight)))
                {
                    mouseWheelHorizontalEventQueue.removeAll(slot);
                }

                slot->setDistance(0.0);
                slot->getMouseInterval()->restart();
            }
            else if (mode == JoyButtonSlot::JoyMouseMovement)
            {
                JoyMouseMovementMode mousemode = getMouseMode();
                if (mousemode == MouseCursor)
                {
                    QListIterator<mouseCursorInfo> iterX(cursorXSpeeds);
                    int i = cursorXSpeeds.length();

                    QList<int> indexesToRemove;
                    while (iterX.hasNext())
                    {
                        mouseCursorInfo info = iterX.next();
                        if (info.slot == slot)
                        {
                            indexesToRemove.append(i);
                        }

                        i++;
                    }

                    QListIterator<int> removeXIter(indexesToRemove);
                    while (removeXIter.hasPrevious())
                    {
                        int index = removeXIter.previous();
                        cursorXSpeeds.removeAt(index);
                    }
                    indexesToRemove.clear();

                    i = cursorYSpeeds.length();

                    QListIterator<mouseCursorInfo> iterY(cursorYSpeeds);
                    while (iterY.hasNext())
                    {
                        mouseCursorInfo info = iterY.next();
                        if (info.slot == slot)
                        {
                            indexesToRemove.append(i);
                        }
                        i++;
                    }

                    QListIterator<int> removeYIter(indexesToRemove);
                    while (removeYIter.hasPrevious())
                    {
                        int index = removeYIter.previous();
                        cursorYSpeeds.removeAt(index);
                    }
                    indexesToRemove.clear();

                    slot->getEasingTime()->restart();
                    slot->setEasingStatus(false);
                }
                else if (mousemode == JoyButton::MouseSpring)
                {
                    double mouse1 = (tempcode == static_cast<int>(JoyButtonSlot::MouseLeft) ||
                                     tempcode == static_cast<int>(JoyButtonSlot::MouseRight)) ? 0.0 : -2.0;
                    double mouse2 = (tempcode == static_cast<int>(JoyButtonSlot::MouseUp) ||
                                     tempcode == static_cast<int>(JoyButtonSlot::MouseDown)) ? 0.0 : -2.0;

                    double springDeadCircleX = 0.0;
                    if (getSpringDeadCircleMultiplier() > 0)
                    {
                        if (tempcode == static_cast<int>(JoyButtonSlot::MouseLeft))
                        {
                            double temp = getCurrentSpringDeadCircle();
                            if (temp > getLastMouseDistanceFromDeadZone())
                            {
                                springDeadCircleX = -getLastMouseDistanceFromDeadZone();
                            }
                            else
                            {
                                springDeadCircleX = -temp;
                            }
                        }
                        else if (tempcode == static_cast<int>(JoyButtonSlot::MouseRight))
                        {
                            double temp = getCurrentSpringDeadCircle();
                            if (temp > getLastMouseDistanceFromDeadZone())
                            {
                                springDeadCircleX = getLastMouseDistanceFromDeadZone();
                            }
                            else
                            {
                                springDeadCircleX = temp;
                            }
                        }
                    }

                    double springDeadCircleY = 0.0;
                    if (getSpringDeadCircleMultiplier() > 0)
                    {
                        if (tempcode == static_cast<int>(JoyButtonSlot::MouseUp))
                        {
                            double temp = getCurrentSpringDeadCircle();
                            if (temp > getLastMouseDistanceFromDeadZone())
                            {
                                springDeadCircleY = -getLastMouseDistanceFromDeadZone();
                            }
                            else
                            {
                                springDeadCircleY = -temp;
                            }
                        }
                        else if (tempcode == static_cast<int>(JoyButtonSlot::MouseDown))
                        {
                            double temp = getCurrentSpringDeadCircle();
                            if (temp > getLastMouseDistanceFromDeadZone())
                            {
                                springDeadCircleY = getLastMouseDistanceFromDeadZone();
                            }
                            else
                            {
                                springDeadCircleY = temp;
                            }
                        }
                    }

                    PadderCommon::springModeInfo infoX;
                    infoX.displacementX = mouse1;
                    infoX.displacementY = -2.0;
                    infoX.springDeadX = springDeadCircleX;
                    infoX.springDeadY = springDeadCircleY;
                    infoX.width = springWidth;
                    infoX.height = springHeight;
                    infoX.relative = relativeSpring;
                    infoX.screen = springModeScreen;
                    springXSpeeds.append(infoX);

                    PadderCommon::springModeInfo infoY;
                    infoY.displacementX = -2.0;
                    infoY.displacementY = mouse2;
                    infoY.springDeadX = springDeadCircleX;
                    infoY.springDeadY = springDeadCircleY;
                    infoY.width = springWidth;
                    infoY.height = springHeight;
                    infoY.relative = relativeSpring;
                    infoY.screen = springModeScreen;
                    springYSpeeds.append(infoY);
                }

                mouseEventQueue.removeAll(slot);
                slot->setDistance(0.0);
                slot->getMouseInterval()->restart();
            }
            else if (mode == JoyButtonSlot::JoyMouseSpeedMod)
            {
                int queueLength = mouseSpeedModList.length();
                if (!mouseSpeedModList.isEmpty())
                {
                    mouseSpeedModList.removeAll(slot);
                    queueLength -= 1;
                }

                if (queueLength <= 0)
                {
                    mouseSpeedModifier = DEFAULTMOUSESPEEDMOD;
                }
            }
            else if (mode == JoyButtonSlot::JoySetChange)
            {
                currentSetChangeSlot = slot;
                slotSetChangeTimer.start();
            }
        }

        getActiveSlotsLocal().clear();

        currentMouseEvent = nullptr;
        if (!mouseEventQueue.isEmpty())
        {
            mouseEventQueue.clear();
        }

        pendingMouseButtons.removeAll(this);
        currentWheelVerticalEvent = nullptr;
        currentWheelHorizontalEvent = nullptr;
        mouseWheelVerticalEventTimer.stop();
        mouseWheelHorizontalEventTimer.stop();

        if (!mouseWheelVerticalEventQueue.isEmpty())
        {
            mouseWheelVerticalEventQueue.clear();
            lastWheelVerticalDistance = getMouseDistanceFromDeadZone();
            wheelVerticalTime.restart();
        }

        if (!mouseWheelHorizontalEventQueue.isEmpty())
        {
            mouseWheelHorizontalEventQueue.clear();
            lastWheelHorizontalDistance = getMouseDistanceFromDeadZone();
            wheelHorizontalTime.restart();
        }

        // Check if mouse remainder should be zero.
        // Only need to check one list from cursor speeds and spring speeds
        // since the correspond Y lists will be the same size.
        if ((pendingMouseButtons.length() == 0) && (cursorXSpeeds.length() == 0) &&
            (springXSpeeds.length() == 0))
        {
            cursorRemainderX = 0;
            cursorRemainderY = 0;
        }

        activeZoneTimer.start();

#ifdef Q_OS_WIN
        BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();

        if (handler && (handler->getIdentifier() == "sendinput") &&
            changeRepeatState && lastActiveKey && !useTurbo)
        {
            InputDevice *device = getParentSet()->getInputDevice();
            if (device->isKeyRepeatEnabled())
            {
                if (lastActiveKey)
                {
                    repeatHelper.setLastActiveKey(lastActiveKey);
                    repeatHelper.setKeyRepeatRate(device->getKeyRepeatRate());
                    repeatHelper.getRepeatTimer()->start(device->getKeyRepeatDelay());
                }
                 else if (repeatHelper.getRepeatTimer()->isActive())
                 {
                     repeatHelper.setLastActiveKey(0);
                     repeatHelper.getRepeatTimer()->stop();
                 }
            }
        }

#endif
    }
}

bool JoyButton::containsReleaseSlots()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = false;
    QListIterator<JoyButtonSlot*> iter(*getAssignedSlots());
    while (iter.hasNext())
    {
        JoyButtonSlot *slot = iter.next();
        if (slot->getSlotMode() == JoyButtonSlot::JoyRelease)
        {
            result = true;
            iter.toBack();
        }
    }

    return result;
}

void JoyButton::releaseSlotEvent()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButtonSlot *temp = nullptr;

    int timeElapsed = buttonHeldRelease.elapsed();

    if (containsReleaseSlots())
    {
        QListIterator<JoyButtonSlot*> iter(*getAssignedSlots());
        if (previousCycle != nullptr)
        {
            iter.findNext(previousCycle);
        }

        int tempElapsed = 0;

        while (iter.hasNext())
        {
            JoyButtonSlot *currentSlot = iter.next();
            int tempcode = currentSlot->getSlotCode();
            JoyButtonSlot::JoySlotInputAction mode = currentSlot->getSlotMode();
            if (mode == JoyButtonSlot::JoyRelease)
            {


                tempElapsed += tempcode;
                if (tempElapsed <= timeElapsed)
                {
                    temp = currentSlot;
                }
                else if (tempElapsed > timeElapsed)
                {
                    iter.toBack();
                }
            }
            else if (mode == JoyButtonSlot::JoyCycle)
            {
                tempElapsed = 0;
                iter.toBack();
            }
        }

        if ((temp != nullptr) && (slotiter != nullptr))
        {
            slotiter->toFront();
            slotiter->findNext(temp);
            currentRelease = temp;

            activateSlots();
            if (!keyPressTimer.isActive() && !pauseWaitTimer.isActive())
            {
                releaseActiveSlots();
                currentRelease = nullptr;
            }

            // Stop hold timer here to be sure that
            // a hold timer that could be activated
            // during a release event is stopped.
            holdTimer.stop();
            if (currentHold != nullptr)
            {
                currentHold = nullptr;
            }
        }
    }
}



void JoyButton::findReleaseEventEnd()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool found = false;
    while (!found && slotiter->hasNext())
    {
        #ifndef QT_DEBUG_NO_OUTPUT
        qDebug() << "slotiter has next element";
        #endif

        JoyButtonSlot *currentSlot = slotiter->next();
        JoyButtonSlot::JoySlotInputAction mode = currentSlot->getSlotMode();

        if (mode == JoyButtonSlot::JoyRelease)
        {
            found = true;
        }
        else if (mode == JoyButtonSlot::JoyCycle)
        {
            found = true;
        }
        else if (mode == JoyButtonSlot::JoyHold)
        {
            found = true;
        }
    }

    if (found && slotiter->hasPrevious())
    {
        #ifndef QT_DEBUG_NO_OUTPUT
        qDebug() << "There has been found end release. Back to previous slotiter element";
        #endif

        slotiter->previous();
    }
}

void JoyButton::findReleaseEventIterEnd(QListIterator<JoyButtonSlot*> *tempiter)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (tempiter)
    {
        bool found = false;

        while (!found && tempiter->hasNext())
        {
            JoyButtonSlot *currentSlot = tempiter->next();
            JoyButtonSlot::JoySlotInputAction mode = currentSlot->getSlotMode();

            if (mode == JoyButtonSlot::JoyRelease)
            {
                found = true;
            }
            else if (mode == JoyButtonSlot::JoyCycle)
            {
                found = true;
            }
            else if (mode == JoyButtonSlot::JoyHold)
            {
                found = true;
            }
        }

        if (found && tempiter->hasPrevious())
        {
            tempiter->previous();
        }
    }
}

void JoyButton::findHoldEventEnd()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool found = false;

    while (!found && slotiter->hasNext())
    {
        #ifndef QT_DEBUG_NO_OUTPUT
        qDebug() << "There is next element in slotiter";
        #endif

        JoyButtonSlot *currentSlot = slotiter->next();
        JoyButtonSlot::JoySlotInputAction mode = currentSlot->getSlotMode();

        if (mode == JoyButtonSlot::JoyRelease)
        {
            found = true;
        }
        else if (mode == JoyButtonSlot::JoyCycle)
        {
            found = true;
        }
        else if (mode == JoyButtonSlot::JoyHold)
        {
            found = true;
        }
    }

    if (found && slotiter->hasPrevious())
    {
        slotiter->previous();

    } else {

        #ifndef QT_DEBUG_NO_OUTPUT
        qDebug() << "There wasn't end for hold";
        #endif
    }
}

void JoyButton::setVDPad(VDPad *vdpad)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    joyEvent(false, true);
    this->vdpad = vdpad;
    emit propertyUpdated();
}

bool JoyButton::isPartVDPad()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return (this->vdpad != nullptr);
}

VDPad* JoyButton::getVDPad()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return this->vdpad;
}

void JoyButton::removeVDPad()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->vdpad = nullptr;
    emit propertyUpdated();
}

/**
 * @brief Check if button properties are at their default values
 * @return Status of possible property edits
 */
bool JoyButton::isDefault()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool value = true;
    value = value && (toggle == DEFAULTTOGGLE);
    value = value && (turboInterval == DEFAULTTURBOINTERVAL);
    value = value && (currentTurboMode == NormalTurbo);
    value = value && (useTurbo == DEFAULTUSETURBO);
    value = value && (mouseSpeedX == DEFAULTMOUSESPEEDX);
    value = value && (mouseSpeedY == DEFAULTMOUSESPEEDY);
    value = value && (setSelection == DEFAULTSETSELECTION);
    value = value && (setSelectionCondition == DEFAULTSETCONDITION);
    value = value && (getAssignedSlots()->isEmpty());
    value = value && (mouseMode == DEFAULTMOUSEMODE);
    value = value && (mouseCurve == DEFAULTMOUSECURVE);
    value = value && (springWidth == DEFAULTSPRINGWIDTH);
    value = value && (springHeight == DEFAULTSPRINGHEIGHT);
    value = value && (sensitivity == DEFAULTSENSITIVITY);
    value = value && (actionName.isEmpty());
    value = value && (wheelSpeedX == DEFAULTWHEELX);
    value = value && (wheelSpeedY == DEFAULTWHEELY);
    value = value && (cycleResetActive == DEFAULTCYCLERESETACTIVE);
    value = value && (cycleResetInterval == DEFAULTCYCLERESET);
    value = value && (relativeSpring == DEFAULTRELATIVESPRING);
    value = value && (easingDuration == DEFAULTEASINGDURATION);

    value = value && (extraAccelerationEnabled == false);
    value = value && (extraAccelerationMultiplier == DEFAULTEXTRACCELVALUE);
    value = value && (minMouseDistanceAccelThreshold == DEFAULTMINACCELTHRESHOLD);
    value = value && (maxMouseDistanceAccelThreshold == DEFAULTMAXACCELTHRESHOLD);
    value = value && (startAccelMultiplier == DEFAULTSTARTACCELMULTIPLIER);
    value = value && (accelDuration == DEFAULTACCELEASINGDURATION);
    value = value && (springDeadCircleMultiplier == DEFAULTSPRINGRELEASERADIUS);
    value = value && (extraAccelCurve == DEFAULTEXTRAACCELCURVE);
    return value;
}

void JoyButton::setIgnoreEventState(bool ignore)
{
   // qInstallMessageHandler(MessageHandler::myMessageOutput);

    ignoreEvents = ignore;
}

bool JoyButton::getIgnoreEventState()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return ignoreEvents;
}

void JoyButton::setMouseMode(JoyMouseMovementMode mousemode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->mouseMode = mousemode;
    emit propertyUpdated();
}

JoyButton::JoyMouseMovementMode JoyButton::getMouseMode()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return mouseMode;
}

void JoyButton::setMouseCurve(JoyMouseCurve selectedCurve)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    mouseCurve = selectedCurve;
    emit propertyUpdated();
}

JoyButton::JoyMouseCurve JoyButton::getMouseCurve()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return mouseCurve;
}

void JoyButton::setSpringWidth(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (value >= 0)
    {
        springWidth = value;
        emit propertyUpdated();
    }
}

int JoyButton::getSpringWidth()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return springWidth;
}

void JoyButton::setSpringHeight(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (springHeight >= 0)
    {
        springHeight = value;
        emit propertyUpdated();
    }
}

int JoyButton::getSpringHeight()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return springHeight;
}

void JoyButton::setSensitivity(double value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((value >= 0.001) && (value <= 1000))
    {
        sensitivity = value;
        emit propertyUpdated();
    }
}

double JoyButton::getSensitivity()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return sensitivity;
}

bool JoyButton::getWhileHeldStatus()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return whileHeldStatus;
}

void JoyButton::setWhileHeldStatus(bool status)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    whileHeldStatus = status;
}

void JoyButton::setActionName(QString tempName)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((tempName.length() <= 50) && (tempName != actionName))
    {
        actionName = tempName;
        emit actionNameChanged();
        emit propertyUpdated();
    }
}

QString JoyButton::getActionName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    #ifndef QT_DEBUG_NO_OUTPUT
    qDebug() << "Action name is: " << actionName;
    #endif

    return actionName;
}

void JoyButton::setButtonName(QString tempName)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((tempName.length() <= 20) && (tempName != buttonName))
    {
        buttonName = tempName;
        emit buttonNameChanged();
        emit propertyUpdated();
    }
}

QString JoyButton::getButtonName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return buttonName;
}

void JoyButton::setWheelSpeedX(int speed)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((speed >= 1) && (speed <= 100))
    {
        wheelSpeedX = speed;
        emit propertyUpdated();
    }
}

void JoyButton::setWheelSpeedY(int speed)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((speed >= 1) && (speed <= 100))
    {
        wheelSpeedY = speed;
        emit propertyUpdated();
    }
}

int JoyButton::getWheelSpeedX()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return wheelSpeedX;
}

int JoyButton::getWheelSpeedY()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return wheelSpeedY;
}

void JoyButton::setDefaultButtonName(QString tempname)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    defaultButtonName = tempname;
}

QString JoyButton::getDefaultButtonName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return defaultButtonName;
}

/**
 * @brief Take cursor mouse information provided by all buttons and
 *     send a cursor mode mouse event to the display server.
 */
void JoyButton::moveMouseCursor(int &movedX, int &movedY, int &movedElapsed)
{
   // qInstallMessageHandler(MessageHandler::myMessageOutput);

    movedX = 0;
    movedY = 0;
    int elapsedTime = testOldMouseTime.elapsed();

    movedElapsed = elapsedTime;
    if (staticMouseEventTimer.interval() < mouseRefreshRate)
    {
        elapsedTime = mouseRefreshRate + (elapsedTime - staticMouseEventTimer.interval());
        movedElapsed = elapsedTime;
    }

    if (mouseHistoryX.size() >= mouseHistorySize)
    {
        mouseHistoryX.removeLast();
    }

    if (mouseHistoryY.size() >= mouseHistorySize)
    {
        mouseHistoryY.removeLast();
    }

    /*
     * Combine all mouse events to find the distance to move the mouse
     * along the X and Y axis. If necessary, perform mouse smoothing.
     * The mouse smoothing technique used is an interpretation of the method
     * outlined at http://flipcode.net/archives/Smooth_Mouse_Filtering.shtml.
     */
    if ((cursorXSpeeds.length() == cursorYSpeeds.length()) &&
        (cursorXSpeeds.length() > 0))
    {
        int queueLength = cursorXSpeeds.length();
        double finalx = 0.0;
        double finaly = 0.0;

        for (int i=0; i < queueLength; i++)
        {
            mouseCursorInfo infoX = cursorXSpeeds.takeFirst();
            mouseCursorInfo infoY = cursorYSpeeds.takeFirst();
            if (infoX.code != 0)
            {
                finalx = (infoX.code < 0) ? qMin(infoX.code, finalx) :
                                            qMax(infoX.code, finalx);
            }

            if (infoY.code != 0)
            {
                finaly = (infoY.code < 0) ? qMin(infoY.code, finaly) :
                                            qMax(infoY.code, finaly);
            }

            infoX.slot->getMouseInterval()->restart();
            infoY.slot->getMouseInterval()->restart();
        }

        // Only apply remainder if both current displacement and remainder
        // follow the same direction.
        if ((cursorRemainderX >= 0) == (finalx >= 0))
        {
            finalx += cursorRemainderX;
        }

        // Cap maximum relative mouse movement.
        if (abs(finalx) > 127)
        {
            finalx = (finalx < 0) ? -127 : 127;
        }

        mouseHistoryX.prepend(finalx);

        // Only apply remainder if both current displacement and remainder
        // follow the same direction.
        if ((cursorRemainderY >= 0) == (finaly >= 0))
        {
            finaly += cursorRemainderY;
        }

        // Cap maximum relative mouse movement.
        if (abs(finaly) > 127)
        {
            finaly = (finaly < 0) ? -127 : 127;
        }

        mouseHistoryY.prepend(finaly);

        cursorRemainderX = 0;
        cursorRemainderY = 0;

        double adjustedX = 0;
        double adjustedY = 0;

        QListIterator<double> iterX(mouseHistoryX);
        double currentWeight = 1.0;
        double weightModifier = JoyButton::weightModifier;
        double finalWeight = 0.0;

        while (iterX.hasNext())
        {
            double temp = iterX.next();
            adjustedX += temp * currentWeight;
            finalWeight += currentWeight;
            currentWeight *= weightModifier;
        }

        if (fabs(adjustedX) > 0)
        {
            adjustedX = adjustedX / finalWeight;

            if (adjustedX > 0)
            {
                double oldX = adjustedX;
                adjustedX = floor(adjustedX);
                cursorRemainderX = oldX - adjustedX;
            }
            else
            {
                double oldX = adjustedX;
                adjustedX = static_cast<int>(ceil(adjustedX));
                cursorRemainderX = oldX - adjustedX;
            }

        }

        QListIterator<double> iterY(mouseHistoryY);
        currentWeight = 1.0;
        finalWeight = 0.0;

        while (iterY.hasNext())
        {
            double temp = iterY.next();
            adjustedY += temp * currentWeight;
            finalWeight += currentWeight;
            currentWeight *= weightModifier;
        }

        if (fabs(adjustedY) > 0)
        {
            adjustedY = adjustedY / finalWeight;
            if (adjustedY > 0)
            {
                double oldY = adjustedY;
                adjustedY = floor(adjustedY);
                cursorRemainderY = oldY - adjustedY;
            }
            else
            {
                double oldY = adjustedY;
                adjustedY = ceil(adjustedY);
                cursorRemainderY = oldY - adjustedY;
            }
        }

        // This check is more of a precaution than anything. No need to cause
        // a sync to happen when not needed.
        if ((adjustedX != 0) || (adjustedY != 0))
        {
            sendevent(adjustedX, adjustedY);
        }

        movedX = static_cast<int>(adjustedX);
        movedY = static_cast<int>(adjustedY);
    }
    else
    {
        mouseHistoryX.prepend(0);
        mouseHistoryY.prepend(0);
    }

    // Check if mouse event timer should use idle time.
    if (pendingMouseButtons.length() == 0)
    {
        if (staticMouseEventTimer.interval() != IDLEMOUSEREFRESHRATE)
        {
            staticMouseEventTimer.start(IDLEMOUSEREFRESHRATE);

            // Clear current mouse history
            mouseHistoryX.clear();
            mouseHistoryY.clear();

            // Fill history with zeroes.
            for (int i=0; i < mouseHistorySize; i++)
            {
                mouseHistoryX.append(0);
                mouseHistoryY.append(0);
            }
        }

        cursorRemainderX = 0;
        cursorRemainderY = 0;
    }
    else
    {
        if (staticMouseEventTimer.interval() != mouseRefreshRate)
        {
            // Restore intended QTimer interval.
            staticMouseEventTimer.start(mouseRefreshRate);
        }
    }


    cursorXSpeeds.clear();
    cursorYSpeeds.clear();
}

/**
 * @brief Take spring mouse information provided by all buttons and
 *     send a spring mode mouse event to the display server.
 */
void JoyButton::moveSpringMouse(int &movedX, int &movedY, bool &hasMoved)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    PadderCommon::springModeInfo fullSpring = {
        -2.0, -2.0, 0, 0, false, springModeScreen, 0.0, 0.0
    };
    PadderCommon::springModeInfo relativeSpring = {
        -2.0, -2.0, 0, 0, false, springModeScreen, 0.0, 0.0
    };

    int realMouseX = movedX = 0;
    int realMouseY = movedY = 0;
    hasMoved = false;

    if ((springXSpeeds.length() == springYSpeeds.length()) &&
        (springXSpeeds.length() > 0))
    {
        int queueLength = springXSpeeds.length();
        bool complete = false;
        for (int i=queueLength-1; i >= 0 && !complete; i--)
        {
            double tempx = -2.0;
            double tempy = -2.0;

            double tempSpringDeadX = 0.0;
            double tempSpringDeadY = 0.0;

            PadderCommon::springModeInfo infoX;
            PadderCommon::springModeInfo infoY;

            infoX = springXSpeeds.takeLast();
            infoY = springYSpeeds.takeLast();

            tempx = infoX.displacementX;
            tempy = infoY.displacementY;

            tempSpringDeadX = infoX.springDeadX;
            tempSpringDeadY = infoY.springDeadY;

            if (infoX.relative)
            {
                if (relativeSpring.displacementX == -2.0)
                {
                    relativeSpring.displacementX = tempx;
                }
                relativeSpring.relative = true;

                // Use largest found width for spring
                // mode dimensions.
                relativeSpring.width = qMax(infoX.width, relativeSpring.width);
            }
            else
            {
                if (fullSpring.displacementX == -2.0)
                {
                    fullSpring.displacementX = tempx;
                }

                if (fullSpring.springDeadX == 0.0)
                {
                    fullSpring.springDeadX = tempSpringDeadX;
                }

                // Use largest found width for spring
                // mode dimensions.
                fullSpring.width = qMax(infoX.width, fullSpring.width);
            }

            if (infoY.relative)
            {
                if (relativeSpring.displacementY == -2.0)
                {
                    relativeSpring.displacementY = tempy;
                }

                relativeSpring.relative = true;

                // Use largest found height for spring
                // mode dimensions.
                relativeSpring.height = qMax(infoX.height, relativeSpring.height);
            }
            else
            {
                if (fullSpring.displacementY == -2.0)
                {
                    fullSpring.displacementY = tempy;
                }

                if (fullSpring.springDeadY == 0.0)
                {
                    fullSpring.springDeadY = tempSpringDeadY;
                }

                // Use largest found height for spring
                // mode dimensions.
                fullSpring.height = qMax(infoX.height, fullSpring.height);
            }

            if (((relativeSpring.displacementX != -2.0) && (relativeSpring.displacementY != -2.0)) &&
                ((fullSpring.displacementX != -2.0) && (fullSpring.displacementY != -2.0)))
            {
                complete = true;
            }
            else if (((relativeSpring.springDeadX != 0.0) && (relativeSpring.springDeadY != 0.0)) &&
                     ((fullSpring.springDeadX != 0.0) && (fullSpring.springDeadY != 0.0)))
            {
                complete = true;
            }
        }

        fullSpring.screen = springModeScreen;
        relativeSpring.screen = springModeScreen;

        if (relativeSpring.relative)
        {
            sendSpringEvent(&fullSpring, &relativeSpring, &realMouseX, &realMouseY);
        }
        else
        {
            if (!hasFutureSpringEvents())
            {
                if (fullSpring.springDeadX != 0.0)
                {
                    fullSpring.displacementX = fullSpring.springDeadX;
                }

                if (fullSpring.springDeadY != 0.0)
                {
                    fullSpring.displacementY = fullSpring.springDeadY;
                }

                sendSpringEvent(&fullSpring, 0, &realMouseX, &realMouseY);
            }
            else
            {
                sendSpringEvent(&fullSpring, 0, &realMouseX, &realMouseY);
            }

        }

        movedX = realMouseX;
        movedY = realMouseY;
        hasMoved = true;
    }

    // Check if mouse event timer should use idle time.
    if (pendingMouseButtons.length() == 0)
    {
        staticMouseEventTimer.start(IDLEMOUSEREFRESHRATE);
    }
    else
    {
        if (staticMouseEventTimer.interval() != mouseRefreshRate)
        {
            // Restore intended QTimer interval.
            staticMouseEventTimer.start(mouseRefreshRate);
        }
    }

    springXSpeeds.clear();
    springYSpeeds.clear();
}

void JoyButton::keyPressEvent()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    //qDebug() << "RADIO EDIT: " << keyDelayHold.elapsed();
    if (keyPressTimer.isActive() && (keyPressHold.elapsed() >= getPreferredKeyPressTime()))
    {
        currentKeyPress = nullptr;

        keyPressTimer.stop();
        keyPressHold.restart();
        releaseActiveSlots();

        createDeskTimer.stop();

        if (currentRelease != nullptr)
        {
            releaseDeskTimer.stop();

            createDeskEvent();
            waitForReleaseDeskEvent();
        }
        else
        {
            createDeskEvent();
        }
    }
    else
    {
        createDeskTimer.stop();

        int preferredDelay = getPreferredKeyPressTime();
        int proposedInterval = preferredDelay - keyPressHold.elapsed();
        proposedInterval = proposedInterval > 0 ? proposedInterval : 0;
        int newTimerInterval = qMin(10, proposedInterval);
        keyPressTimer.start(newTimerInterval);
        // If release timer is active, push next run until
        // after keyDelayTimer will timeout again. Helps
        // reduce CPU usage of an excessively repeating timer.
        if (releaseDeskTimer.isActive())
        {
            releaseDeskTimer.start(proposedInterval);
        }
    }
}

/**
 * @brief TODO: CHECK IF METHOD WOULD BE USEFUL. CURRENTLY NOT USED.
 * @return Result
 */
bool JoyButton::checkForDelaySequence()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = false;

    QListIterator<JoyButtonSlot*> tempiter(*getAssignedSlots());

    // Move iterator to start of cycle.
    if (previousCycle != nullptr)
    {
        tempiter.findNext(previousCycle);
    }

    while (tempiter.hasNext())
    {
        JoyButtonSlot *slot = tempiter.next();
        JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();
        if ((mode == JoyButtonSlot::JoyPause) || (mode == JoyButtonSlot::JoyRelease))
        {
            result = true;
            tempiter.toBack();
        }
        else if (mode == JoyButtonSlot::JoyCycle)
        {
            result = false;
            tempiter.toBack();
        }
    }

    return result;
}

SetJoystick* JoyButton::getParentSet()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return parentSet;
}

void JoyButton::checkForPressedSetChange()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (!isButtonPressedQueue.isEmpty())
    {
        bool tempButtonPressed = isButtonPressedQueue.last();
        bool tempFinalIgnoreSetsState = ignoreSetQueue.last();

        if (!whileHeldStatus)
        {
            if (tempButtonPressed && !tempFinalIgnoreSetsState &&
                (setSelectionCondition == SetChangeWhileHeld) && (currentRelease == nullptr))
            {
                setChangeTimer.start(0);
                quitEvent = true;
            }
        }
    }
}

/**
 * @brief Obtain the appropriate key press time for the current event.
 *     Order of preference: active key press time slot value ->
 *     profile value -> program default value.
 * @return Appropriate key press time for current event.
 */
int JoyButton::getPreferredKeyPressTime()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int tempPressTime = InputDevice::DEFAULTKEYPRESSTIME;
    if ((currentKeyPress != nullptr) && (currentKeyPress->getSlotCode() > 0))
    {
        tempPressTime = currentKeyPress->getSlotCode();
    }
    else if (parentSet->getInputDevice()->getDeviceKeyPressTime() > 0)
    {
        tempPressTime = parentSet->getInputDevice()->getDeviceKeyPressTime();
    }

    return tempPressTime;
}

void JoyButton::setCycleResetTime(int interval)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (interval >= MINCYCLERESETTIME)
    {
        int ceiling = MAXCYCLERESETTIME;
        int temp = qBound(MINCYCLERESETTIME, interval, ceiling);
        cycleResetInterval = temp;
        emit propertyUpdated();
    }
    else
    {
        interval = 0;
        cycleResetActive = false;
        emit propertyUpdated();
    }
}

int JoyButton::getCycleResetTime()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return cycleResetInterval;
}

void JoyButton::setCycleResetStatus(bool enabled)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    cycleResetActive = enabled;
    emit propertyUpdated();
}

bool JoyButton::isCycleResetActive()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return cycleResetActive;
}

void JoyButton::establishPropertyUpdatedConnections()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    connect(this, SIGNAL(slotsChanged()), parentSet->getInputDevice(), SLOT(profileEdited()));
    connect(this, SIGNAL(propertyUpdated()), parentSet->getInputDevice(), SLOT(profileEdited()));
}

void JoyButton::disconnectPropertyUpdatedConnections()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    disconnect(this, SIGNAL(slotsChanged()), 0, 0);
    disconnect(this, SIGNAL(propertyUpdated()), parentSet->getInputDevice(), SLOT(profileEdited()));
}

/**
 * @brief Change initial settings used for mouse event timer being used by
 *     the application.
 */
void JoyButton::establishMouseTimerConnections()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (staticMouseEventTimer.timerType() != Qt::PreciseTimer)
    {
        staticMouseEventTimer.setTimerType(Qt::PreciseTimer);
    }

    // Only one connection will be made for each.
    connect(&staticMouseEventTimer, SIGNAL(timeout()), &mouseHelper,
            SLOT(mouseEvent()), Qt::UniqueConnection);

    if (staticMouseEventTimer.interval() != IDLEMOUSEREFRESHRATE)
    {
        staticMouseEventTimer.setInterval(IDLEMOUSEREFRESHRATE);
    }
}

void JoyButton::setSpringRelativeStatus(bool value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (value != relativeSpring)
    {
        if (value)
        {
            setSpringDeadCircleMultiplier(0);
        }

        relativeSpring = value;
        emit propertyUpdated();
    }
}

bool JoyButton::isRelativeSpring()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return relativeSpring;
}

/**
 * @brief Copy assignments and properties from one button to another.
 *     Used for set copying.
 * @param Button instance that should be modified.
 */
void JoyButton::copyAssignments(JoyButton *destButton)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    destButton->eventReset();
    destButton->assignmentsLock.lockForWrite();
    destButton->getAssignmentsLocal().clear();
    destButton->assignmentsLock.unlock();

    assignmentsLock.lockForWrite();
    QListIterator<JoyButtonSlot*> iter(*getAssignedSlots());
    while (iter.hasNext())
    {
        JoyButtonSlot *slot = iter.next();
        JoyButtonSlot *newslot = new JoyButtonSlot(slot, destButton);
        destButton->insertAssignedSlot(newslot, false);
    }
    assignmentsLock.unlock();

    destButton->toggle = toggle;
    destButton->turboInterval = turboInterval;
    destButton->useTurbo = useTurbo;
    destButton->mouseSpeedX = mouseSpeedX;
    destButton->mouseSpeedY = mouseSpeedY;
    destButton->wheelSpeedX = wheelSpeedX;
    destButton->wheelSpeedY = wheelSpeedY;
    destButton->mouseMode = mouseMode;
    destButton->mouseCurve = mouseCurve;
    destButton->springWidth = springWidth;
    destButton->springHeight = springHeight;
    destButton->sensitivity = sensitivity;
    destButton->buttonName = buttonName;
    destButton->actionName = actionName;
    destButton->cycleResetActive = cycleResetActive;
    destButton->cycleResetInterval = cycleResetInterval;
    destButton->relativeSpring = relativeSpring;
    destButton->currentTurboMode = currentTurboMode;
    destButton->easingDuration = easingDuration;
    destButton->extraAccelerationEnabled = extraAccelerationEnabled;
    destButton->extraAccelerationMultiplier = extraAccelerationMultiplier;
    destButton->minMouseDistanceAccelThreshold = minMouseDistanceAccelThreshold;
    destButton->maxMouseDistanceAccelThreshold = maxMouseDistanceAccelThreshold;
    destButton->startAccelMultiplier = startAccelMultiplier;
    destButton->springDeadCircleMultiplier = springDeadCircleMultiplier;
    destButton->extraAccelCurve = extraAccelCurve;

    destButton->buildActiveZoneSummaryString();
    if (!destButton->isDefault())
    {
        emit propertyUpdated();
    }
}

/**
 * @brief Set the turbo mode that the button should use
 * @param Mode that should be used
 */
void JoyButton::setTurboMode(TurboMode mode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    currentTurboMode = mode;
}

/**
 * @brief Get currently assigned turbo mode
 * @return Currently assigned turbo mode
 */
JoyButton::TurboMode JoyButton::getTurboMode()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return currentTurboMode;
}

/**
 * @brief Check if button should be considered a part of a real controller
 *     axis. Needed for some dialogs so the program won't have to resort to
 *     type checking.
 * @return Status of being part of a real controller axis
 */
bool JoyButton::isPartRealAxis()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return false;
}

/**
 * @brief Calculate maximum mouse speed when using a given mouse curve.
 * @param Mouse curve
 * @param Mouse speed value
 * @return Final mouse speed
 */
int JoyButton::calculateFinalMouseSpeed(JoyMouseCurve curve, int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int result = JoyAxis::JOYSPEED * value;
    switch (curve)
    {
        case QuadraticExtremeCurve:
        case EasingQuadraticCurve:
        case EasingCubicCurve:
        {
            result *= 1.5;
            break;
        }
    case LinearCurve:
    {
        break;
    }
    case QuadraticCurve:
    {
        break;
    }
    case CubicCurve:
    {
        break;
    }
    case PowerCurve:
    {
        break;
    }
    default:
    {
        break;
    }
    }

    return result;
}

void JoyButton::setEasingDuration(double value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((value >= MINIMUMEASINGDURATION) && (value <= MAXIMUMEASINGDURATION) &&
        (value != easingDuration))
    {
        easingDuration = value;
        emit propertyUpdated();
    }
}

double JoyButton::getEasingDuration()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return easingDuration;
}

JoyButtonMouseHelper* JoyButton::getMouseHelper()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return &mouseHelper;
}

/**
 * @brief Get the list of buttons that have a pending mouse movement event.
 * @return QList<JoyButton*>*
 */
QList<JoyButton*>* JoyButton::getPendingMouseButtons()
{
   // qInstallMessageHandler(MessageHandler::myMessageOutput);

    return &pendingMouseButtons;
}

bool JoyButton::hasCursorEvents()
{
  //  qInstallMessageHandler(MessageHandler::myMessageOutput);

    return (cursorXSpeeds.length() != 0) || (cursorYSpeeds.length() != 0);
}

bool JoyButton::hasSpringEvents()
{
   // qInstallMessageHandler(MessageHandler::myMessageOutput);

    return (springXSpeeds.length() != 0) || (springYSpeeds.length() != 0);
}

/**
 * @brief Get the weight modifier being used for mouse smoothing.
 * @return Weight modifier in the range of 0.0 - 1.0.
 */
double JoyButton::getWeightModifier()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return weightModifier;
}

/**
 * @brief Set the weight modifier to use for mouse smoothing.
 * @param Weight modifier in the range of 0.0 - 1.0.
 */
void JoyButton::setWeightModifier(double modifier)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((modifier >= 0.0) && (modifier <= MAXIMUMWEIGHTMODIFIER))
    {
        weightModifier = modifier;
    }
}

/**
 * @brief Get mouse history buffer size.
 * @return Mouse history buffer size
 */
int JoyButton::getMouseHistorySize()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return mouseHistorySize;
}

/**
 * @brief Set mouse history buffer size used for mouse smoothing.
 * @param Mouse history buffer size
 */
void JoyButton::setMouseHistorySize(int size)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((size >= 1) && (size <= MAXIMUMMOUSEHISTORYSIZE))
    {
        mouseHistoryX.clear();
        mouseHistoryY.clear();

        mouseHistorySize = size;
    }
}

/**
 * @brief Get active mouse movement refresh rate.
 * @return
 */
int JoyButton::getMouseRefreshRate()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return mouseRefreshRate;
}

/**
 * @brief Set the mouse refresh rate when a mouse slot is active.
 * @param Refresh rate in ms.
 */
void JoyButton::setMouseRefreshRate(int refresh)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((refresh >= 1) && (refresh <= 16))
    {
        mouseRefreshRate = refresh;
        int temp = IDLEMOUSEREFRESHRATE;

        if (staticMouseEventTimer.isActive())
        {
            testOldMouseTime.restart();

            int tempInterval = staticMouseEventTimer.interval();

            if (tempInterval != temp &&
                tempInterval != 0)
            {
                QMetaObject::invokeMethod(&staticMouseEventTimer, "start",
                                          Q_ARG(int, mouseRefreshRate));
            }
            else
            {
                // Restart QTimer to keep QTimer in line with QTime
                QMetaObject::invokeMethod(&staticMouseEventTimer, "start",
                                          Q_ARG(int, temp));
            }

            // Clear current mouse history
            mouseHistoryX.clear();
            mouseHistoryY.clear();
        }
        else
        {
            staticMouseEventTimer.setInterval(IDLEMOUSEREFRESHRATE);
        }

        mouseHelper.carryMouseRefreshRateUpdate(mouseRefreshRate);
    }
}

/**
 * @brief Get the gamepad poll rate used by the application.
 * @return Poll rate in ms.
 */
int JoyButton::getGamepadRefreshRate()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return gamepadRefreshRate;
}

/**
 * @brief Set the gamepad poll rate to be used in the application.
 * @param Poll rate in ms.
 */
void JoyButton::setGamepadRefreshRate(int refresh)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((refresh >= 1) && (refresh <= 16))
    {
        gamepadRefreshRate = refresh;
        mouseHelper.carryGamePollRateUpdate(gamepadRefreshRate);
    }
}

/**
 * @brief Check if turbo should be disabled for a slot
 * @param JoyButtonSlot to check
 */
void JoyButton::checkTurboCondition(JoyButtonSlot *slot)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();
    switch (mode)
    {
        case JoyButtonSlot::JoyPause:
        case JoyButtonSlot::JoyHold:
        case JoyButtonSlot::JoyDistance:
        case JoyButtonSlot::JoyRelease:
        case JoyButtonSlot::JoyLoadProfile:
        case JoyButtonSlot::JoySetChange:
        {
            setUseTurbo(false);
            break;
        }
    case JoyButtonSlot::JoyMouseButton:
    {
        break;
    }
    case JoyButtonSlot::JoyMouseMovement:
    {
        break;
    }
    case JoyButtonSlot::JoyCycle:
    {
        break;
    }
    case JoyButtonSlot::JoyMouseSpeedMod:
    {
        break;
    }
    case JoyButtonSlot::JoyKeyPress:
    {
        break;
    }
    case JoyButtonSlot::JoyDelay:
    {
        break;
    }
    case JoyButtonSlot::JoyTextEntry:
    {
        break;
    }
    case JoyButtonSlot::JoyExecute:
    {
        break;
    }
       default:
    {
        break;
    }
    }
}

void JoyButton::resetProperties()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    #ifndef QT_DEBUG_NO_OUTPUT
    qDebug() << "all current slots and previous slots ale cleared";
    #endif

    currentCycle = nullptr;
    previousCycle = nullptr;
    currentPause = nullptr;
    currentHold = nullptr;
    currentDistance = nullptr;
    currentRawValue = 0;
    currentMouseEvent = nullptr;
    currentRelease = nullptr;
    currentWheelVerticalEvent = nullptr;
    currentWheelHorizontalEvent = nullptr;
    currentKeyPress = nullptr;
    currentDelay = nullptr;
    currentSetChangeSlot = nullptr;

    isKeyPressed = isButtonPressed = false;
    quitEvent = true;

    toggle = false;
    turboInterval = 0;
    isDown = false;
    toggleActiveState = false;
    useTurbo = false;
    mouseSpeedX = 50;
    mouseSpeedY = 50;
    wheelSpeedX = 20;
    wheelSpeedY = 20;
    mouseMode = MouseCursor;
    mouseCurve = DEFAULTMOUSECURVE;
    springWidth = 0;
    springHeight = 0;
    sensitivity = 1.0;
    setSelection = -1;
    setSelectionCondition = SetChangeDisabled;
    ignoresets = false;
    ignoreEvents = false;
    whileHeldStatus = false;
    buttonName.clear();
    actionName.clear();
    cycleResetActive = false;
    cycleResetInterval = 0;
    relativeSpring = false;
    lastDistance = 0.0;
    currentAccelMulti = 0.0;
    lastAccelerationDistance = 0.0;
    lastMouseDistance = 0.0;
    currentMouseDistance = 0.0;
    updateLastMouseDistance = false;
    updateStartingMouseDistance = false;
    updateOldAccelMulti = 0.0;
    updateInitAccelValues = true;

    currentAccelerationDistance = 0.0;
    startingAccelerationDistance = 0.0;
    lastWheelVerticalDistance = 0.0;
    lastWheelHorizontalDistance = 0.0;
    tempTurboInterval = 0;
    oldAccelMulti = 0.0;
    accelTravel = 0.0;
    currentTurboMode = DEFAULTTURBOMODE;
    easingDuration = DEFAULTEASINGDURATION;
    springDeadCircleMultiplier = DEFAULTSPRINGRELEASERADIUS;

    pendingEvent = false;
    pendingPress = false;
    pendingIgnoreSets = false;

    extraAccelerationEnabled = false;
    extraAccelerationMultiplier = DEFAULTEXTRACCELVALUE;
    minMouseDistanceAccelThreshold = DEFAULTMINACCELTHRESHOLD;
    maxMouseDistanceAccelThreshold = DEFAULTMAXACCELTHRESHOLD;
    startAccelMultiplier = DEFAULTSTARTACCELMULTIPLIER;
    accelDuration = DEFAULTACCELEASINGDURATION;
    extraAccelCurve = LinearAccelCurve;

    activeZoneStringLock.lockForWrite();
    activeZoneString = trUtf8("[NO KEY]");
    activeZoneStringLock.unlock();
}

bool JoyButton::isModifierButton()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return false;
}

void JoyButton::resetActiveButtonMouseDistances()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    mouseHelper.resetButtonMouseDistances();
}

void JoyButton::resetAccelerationDistances()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (updateLastMouseDistance)
    {
        lastAccelerationDistance = currentAccelerationDistance;
        lastMouseDistance = currentMouseDistance;
        updateLastMouseDistance = false;
    }

    if (updateStartingMouseDistance)
    {
        startingAccelerationDistance = lastAccelerationDistance;
        updateStartingMouseDistance = false;
    }

    if (updateOldAccelMulti >= 0.0)
    {
        oldAccelMulti = updateOldAccelMulti;
        updateOldAccelMulti = 0.0;
    }

    currentAccelerationDistance = getAccelerationDistance();
    currentMouseDistance = getMouseDistanceFromDeadZone();
}

void JoyButton::initializeDistanceValues()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    lastAccelerationDistance = getLastAccelerationDistance();
    currentAccelerationDistance = getAccelerationDistance();
    startingAccelerationDistance = lastAccelerationDistance;

    lastMouseDistance = getLastMouseDistanceFromDeadZone();
    currentMouseDistance = getMouseDistanceFromDeadZone();
}

double JoyButton::getLastMouseDistanceFromDeadZone()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return lastMouseDistance;
}

double JoyButton::getLastAccelerationDistance()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return lastAccelerationDistance;
}

void JoyButton::copyLastMouseDistanceFromDeadZone(JoyButton *srcButton)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->lastMouseDistance = srcButton->lastMouseDistance;
}

void JoyButton::copyLastAccelerationDistance(JoyButton *srcButton)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->lastAccelerationDistance = srcButton->lastAccelerationDistance;
}

bool JoyButton::isExtraAccelerationEnabled()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return extraAccelerationEnabled;
}

double JoyButton::getExtraAccelerationMultiplier()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return extraAccelerationMultiplier;
}

void JoyButton::setExtraAccelerationStatus(bool status)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (isPartRealAxis())
    {
        extraAccelerationEnabled = status;
        emit propertyUpdated();
    }
    else
    {
        extraAccelerationEnabled = false;
    }
}

void JoyButton::setExtraAccelerationMultiplier(double value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((value >= 1.0) && (value <= 200.0))
    {
        extraAccelerationMultiplier = value;
        emit propertyUpdated();
    }
}

void JoyButton::setMinAccelThreshold(double value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((value >= 1.0) && (value <= 100.0) && (value <= maxMouseDistanceAccelThreshold))
    {
        minMouseDistanceAccelThreshold = value;
        emit propertyUpdated();
    }
}

double JoyButton::getMinAccelThreshold()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return minMouseDistanceAccelThreshold;
}

void JoyButton::setMaxAccelThreshold(double value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((value >= 1.0) && (value <= 100.0) && (value >= minMouseDistanceAccelThreshold))
    {
        maxMouseDistanceAccelThreshold = value;
        emit propertyUpdated();
    }
}

double JoyButton::getMaxAccelThreshold()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return maxMouseDistanceAccelThreshold;
}

void JoyButton::setStartAccelMultiplier(double value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((value >= 0.0) && (value <= 100.0))
    {
        startAccelMultiplier = value;
        emit propertyUpdated();
    }
}

double JoyButton::getStartAccelMultiplier()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return startAccelMultiplier;
}

int JoyButton::getSpringModeScreen()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return springModeScreen;
}

void JoyButton::setSpringModeScreen(int screen)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (screen >= -1)
    {
        springModeScreen = screen;
    }
}

void JoyButton::setAccelExtraDuration(double value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((value >= 0.0) && (value <= 5.0))
    {
        accelDuration = value;
        emit propertyUpdated();
    }
}

double JoyButton::getAccelExtraDuration()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return accelDuration;
}

bool JoyButton::hasFutureSpringEvents()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = false;
    QListIterator<JoyButton*> iter(pendingMouseButtons);
    while (iter.hasNext())
    {
        JoyButton *temp = iter.next();
        if (temp->getMouseMode() == MouseSpring)
        {
            result = true;
            iter.toBack();
        }
    }

    return result;
}

void JoyButton::setSpringDeadCircleMultiplier(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((value >= 0) && (value <= 100))
    {
        springDeadCircleMultiplier = value;
        emit propertyUpdated();
    }
}

int JoyButton::getSpringDeadCircleMultiplier()
{
    return springDeadCircleMultiplier;
}

double JoyButton::getCurrentSpringDeadCircle()
{
    return (springDeadCircleMultiplier * 0.01);
}

void JoyButton::restartLastMouseTime()
{
    testOldMouseTime.restart();
}

void JoyButton::setStaticMouseThread(QThread *thread)
{
    int oldInterval = staticMouseEventTimer.interval();
    if (oldInterval == 0)
    {
        oldInterval = IDLEMOUSEREFRESHRATE;
    }

    staticMouseEventTimer.moveToThread(thread);
    mouseHelper.moveToThread(thread);

    QMetaObject::invokeMethod(&staticMouseEventTimer, "start",
                              Q_ARG(int, oldInterval));

    testOldMouseTime.start();

#ifdef Q_OS_WIN
    repeatHelper.moveToThread(thread);
#endif
}

void JoyButton::indirectStaticMouseThread(QThread *thread)
{
    QMetaObject::invokeMethod(&staticMouseEventTimer, "stop");
#ifdef Q_OS_WIN
    QMetaObject::invokeMethod(repeatHelper.getRepeatTimer(), "stop");
#endif
    QMetaObject::invokeMethod(&mouseHelper, "changeThread",
                              Q_ARG(QThread*, thread));
}

bool JoyButton::shouldInvokeMouseEvents()
{
    bool result = false;

    if ((pendingMouseButtons.size() > 0) && staticMouseEventTimer.isActive())
    {
        int timerInterval = staticMouseEventTimer.interval();
        if (timerInterval == 0)
        {
            result = true;
        }
        else if (testOldMouseTime.elapsed() >= timerInterval)
        {
            result = true;
        }
    }

    return result;
}

void JoyButton::invokeMouseEvents()
{
    mouseHelper.mouseEvent();
}

bool JoyButton::hasActiveSlots()
{
    return !getActiveSlots().isEmpty();
}

void JoyButton::setExtraAccelerationCurve(JoyExtraAccelerationCurve curve)
{
    extraAccelCurve = curve;
    emit propertyUpdated();
}

JoyButton::JoyExtraAccelerationCurve JoyButton::getExtraAccelerationCurve()
{
    return extraAccelCurve;
}

void JoyButton::copyExtraAccelerationState(JoyButton *srcButton)
{
    this->currentAccelMulti = srcButton->currentAccelMulti;
    this->oldAccelMulti = srcButton->oldAccelMulti;
    this->accelTravel = srcButton->accelTravel;

    this->startingAccelerationDistance = srcButton->startingAccelerationDistance;
    this->lastAccelerationDistance = srcButton->lastAccelerationDistance;
    this->lastMouseDistance = srcButton->lastMouseDistance;

    this->accelExtraDurationTime.setHMS(srcButton->accelExtraDurationTime.hour(),
                                        srcButton->accelExtraDurationTime.minute(),
                                        srcButton->accelExtraDurationTime.second(),
                                        srcButton->accelExtraDurationTime.msec());

    this->updateOldAccelMulti = srcButton->updateOldAccelMulti;
    this->updateStartingMouseDistance = srcButton->updateStartingMouseDistance;
    this->updateLastMouseDistance = srcButton->lastMouseDistance;
}

void JoyButton::setUpdateInitAccel(bool state)
{
    this->updateInitAccelValues = state;
}

QList<JoyButtonSlot*>& JoyButton::getAssignmentsLocal() {

    return assignments;
}

QList<JoyButtonSlot*>& JoyButton::getActiveSlotsLocal() {

    return activeSlots;
}
