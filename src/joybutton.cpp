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

#include "joybutton.h"

#include "event.h"
#include "inputdevice.h"
#include "logger.h"
#include "messagehandler.h"
#include "setjoystick.h"
#include "vdpad.h"

#include "SDL2/SDL_events.h"
#include "eventhandlerfactory.h"

#include <QDebug>
//#include <QThread>
#include <QSharedPointer>
#include <QStringList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QtConcurrent>
#include <chrono>

const JoyButton::JoyMouseCurve JoyButton::DEFAULTMOUSECURVE = JoyButton::EnhancedPrecisionCurve;
const JoyButton::SetChangeCondition JoyButton::DEFAULTSETCONDITION = JoyButton::SetChangeDisabled;
const JoyButton::JoyMouseMovementMode JoyButton::DEFAULTMOUSEMODE = JoyButton::MouseCursor;
const JoyButton::TurboMode JoyButton::DEFAULTTURBOMODE = JoyButton::NormalTurbo;
const JoyButton::JoyExtraAccelerationCurve JoyButton::DEFAULTEXTRAACCELCURVE = JoyButton::LinearAccelCurve;

JoyButtonSlot *JoyButton::lastActiveKey = nullptr;

// Keep track of active Mouse Speed Mod slots.
QList<JoyButtonSlot *> JoyButton::mouseSpeedModList;

// Lists used for cursor mode calculations.
QList<JoyButton::mouseCursorInfo> JoyButton::cursorXSpeeds;
QList<JoyButton::mouseCursorInfo> JoyButton::cursorYSpeeds;

// Lists used for spring mode calculations.
QList<PadderCommon::springModeInfo> JoyButton::springXSpeeds;
QList<PadderCommon::springModeInfo> JoyButton::springYSpeeds;

// Temporary test object to test old mouse time behavior.
QTime JoyButton::testOldMouseTime;

// time when minislots next to each other in thread pool are waiting to execute function
// at the same time
int JoyButton::timeBetweenMiniSlots = 55;

int JoyButton::allSlotTimeBetweenSlots = 0;

// Helper object to have a single mouse event for all JoyButton
// instances.
JoyButtonMouseHelper JoyButton::mouseHelper;

QTimer JoyButton::staticMouseEventTimer;
QList<JoyButton *> JoyButton::pendingMouseButtons;

// IT CAN BE HERE
// LOOK FOR JoyCycle and put JoyMix next to the slots types
JoyButton::JoyButton(int index, int originset, SetJoystick *parentSet, QObject *parent)
    : QObject(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    qDebug() << "Thread in Joybutton";

    m_vdpad = nullptr;
    slotiter = nullptr;

    threadPool = QThreadPool::globalInstance();

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
    m_parentSet = parentSet;

    connect(&pauseWaitTimer, &QTimer::timeout, this, &JoyButton::pauseWaitEvent);
    connect(&keyPressTimer, &QTimer::timeout, this, &JoyButton::keyPressEvent);
    connect(&holdTimer, &QTimer::timeout, this, &JoyButton::holdEvent);
    connect(&delayTimer, &QTimer::timeout, this, &JoyButton::delayEvent);
    connect(&createDeskTimer, &QTimer::timeout, this, &JoyButton::waitForDeskEvent);
    connect(&releaseDeskTimer, &QTimer::timeout, this, &JoyButton::waitForReleaseDeskEvent);
    connect(&turboTimer, &QTimer::timeout, this, &JoyButton::turboEvent);
    connect(&mouseWheelVerticalEventTimer, &QTimer::timeout, this, &JoyButton::wheelEventVertical);
    connect(&mouseWheelHorizontalEventTimer, &QTimer::timeout, this, &JoyButton::wheelEventHorizontal);
    connect(&setChangeTimer, &QTimer::timeout, this, &JoyButton::checkForSetChange);
    connect(&slotSetChangeTimer, &QTimer::timeout, this, &JoyButton::slotSetChange);
    connect(&activeZoneTimer, &QTimer::timeout, this, &JoyButton::buildActiveZoneSummaryString);

    activeZoneTimer.setInterval(0);
    activeZoneTimer.setSingleShot(true);

    // Will only matter on the first call
    establishMouseTimerConnections();

    // Make sure to call before calling reset
    resetAllProperties();

    m_index = index;
    m_originset = originset;
    quitEvent = true;
}

JoyButton::~JoyButton()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    // threadPool->clear();

    reset();
    // resetPrivVars();
}

void JoyButton::queuePendingEvent(bool pressed, bool ignoresets)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    updatePendingParams(false, false, false);

    if (m_vdpad != nullptr)
        vdpadPassEvent(pressed, ignoresets);
    else
        updatePendingParams(true, pressed, ignoresets);
}

void JoyButton::activatePendingEvent()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (pendingEvent)
    {
        joyEvent(pendingPress, pendingIgnoreSets);
        updatePendingParams(false, false, false);
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

    updatePendingParams(false, false, false);
}

void JoyButton::vdpadPassEvent(bool pressed, bool ignoresets)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((m_vdpad != nullptr) && (pressed != isButtonPressed))
    {
        isButtonPressed = pressed;

        if (isButtonPressed)
            emit clicked(m_index);
        else
            emit released(m_index);

        if (!ignoresets)
            m_vdpad->queueJoyEvent(ignoresets);
        else
            m_vdpad->joyEvent(pressed, ignoresets);
    }
}

void JoyButton::joyEvent(bool pressed, bool ignoresets)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((m_vdpad != nullptr) && !pendingEvent)
    {
        vdpadPassEvent(pressed, ignoresets);
    } else if (ignoreEvents && (pressed != isButtonPressed))
    {
        isButtonPressed = pressed;
        if (isButtonPressed)
            emit clicked(m_index);
        else
            emit released(m_index);
    } else
    {
        if (pressed != isDown)
        {
            if (pressed)
            {
                emit clicked(m_index);
                if (updateInitAccelValues)
                    oldAccelMulti = updateOldAccelMulti = accelTravel = 0.0;
            } else
            {
                emit released(m_index);
            }

            bool activePress = pressed;
            setChangeTimer.stop();

            if (m_toggle && pressed)
            {
                isDown = true;
                toggleActiveState = !toggleActiveState;

                if (!isButtonPressed)
                {
                    m_ignoresets = ignoresets;
                    isButtonPressed = !isButtonPressed;

                    ignoreSetQueue.enqueue(ignoresets);
                    isButtonPressedQueue.enqueue(isButtonPressed);
                } else
                {
                    activePress = false;
                }
            } else if (m_toggle && !pressed && isDown)
            {
                isDown = false;

                if (!toggleActiveState)
                {
                    m_ignoresets = ignoresets;
                    isButtonPressed = !isButtonPressed;

                    ignoreSetQueue.enqueue(ignoresets);
                    isButtonPressedQueue.enqueue(isButtonPressed);
                }
            } else
            {
                m_ignoresets = ignoresets;
                isButtonPressed = isDown = pressed;

                ignoreSetQueue.enqueue(ignoresets);
                isButtonPressedQueue.enqueue(isButtonPressed);
            }

            if (m_useTurbo)
            {
                if (isButtonPressed && activePress && !turboTimer.isActive())
                {
                    startSequenceOfPressActive(true, tr("Processing turbo for #%1 - %2"));
                    turboEvent();
                } else if (!isButtonPressed && !activePress && turboTimer.isActive())
                {
                    turboTimer.stop();
                    Logger::LogDebug(tr("Finishing turbo for button #%1 - %2")
                                         .arg(m_parentSet->getInputDevice()->getRealJoyNumber())
                                         .arg(getPartialName()));

                    if (isKeyPressed)
                        turboEvent();
                    else
                        lastDistance = getMouseDistanceFromDeadZone();

                    activeZoneTimer.start();
                }
            }
            // Toogle is enabled and a controller button change has occurred.
            // Switch to a different distance zone if appropriate
            else if (m_toggle && !activePress && isButtonPressed)
            {
                updateParamsAfterDistEvent();
            } else if (isButtonPressed && activePress)
            {
                startSequenceOfPressActive(false, tr("Processing press for button #%1 - %2"));

                if (!keyPressTimer.isActive())
                {
                    checkForPressedSetChange();

                    if (!setChangeTimer.isActive())
                        waitForDeskEvent();
                }
            } else if (!isButtonPressed && !activePress)
            {
                Logger::LogDebug(tr("Processing release for button #%1 - %2")
                                     .arg(m_parentSet->getInputDevice()->getRealJoyNumber())
                                     .arg(getPartialName()));

                waitForReleaseDeskEvent();
            }

            if (updateInitAccelValues)
                updateMouseParams(false, false, 0.0);
        } else if (!m_useTurbo && isButtonPressed)
        {
            resetAccelerationDistances();
            currentAccelerationDistance = getAccelerationDistance();

            if (!setChangeTimer.isActive())
                updateParamsAfterDistEvent();
        }
    }

    updateInitAccelValues = true;
}

void JoyButton::updateParamsAfterDistEvent()
{
    if (distanceEvent())
    {
        Logger::LogDebug(tr("Distance change for button #%1 - %2")
                             .arg(m_parentSet->getInputDevice()->getRealJoyNumber())
                             .arg(getPartialName()));

        quitEvent = true;
        buttonHold.restart();
        buttonHeldRelease.restart();
        keyPressHold.restart();
        releaseDeskTimer.stop();

        if (!keyPressTimer.isActive())
            waitForDeskEvent();
    }
}

void JoyButton::startSequenceOfPressActive(bool isTurbo, QString debugText)
{
    if (cycleResetActive && (cycleResetHold.elapsed() >= cycleResetInterval) && (slotiter != nullptr))
    {
        slotiter->toFront();
        currentCycle = nullptr;
        previousCycle = nullptr;
    }

    buttonHold.restart();
    buttonHeldRelease.restart();
    keyPressHold.restart();
    cycleResetHold.restart();
    if (isTurbo)
        turboTimer.start();
    else
        releaseDeskTimer.stop();

    // Newly activated button. Just entered safe zone.
    if (updateInitAccelValues)
        initializeDistanceValues();

    currentAccelerationDistance = getAccelerationDistance();

    Logger::LogDebug(debugText.arg(m_parentSet->getInputDevice()->getRealJoyNumber()).arg(getPartialName()));
}

/**
 * @brief Get 0 indexed number of button
 * @return 0 indexed button index number
 */
int JoyButton::getJoyNumber()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_index;
}

/**
 * @brief Get a 1 indexed number of button
 * @return 1 indexed button index number
 */
int JoyButton::getRealJoyNumber() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_index + 1;
}

void JoyButton::setJoyNumber(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    m_index = index;
}

void JoyButton::setToggle(bool toggle)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (toggle != m_toggle)
    {
        m_toggle = toggle;
        emit toggleChanged(toggle);
        emit propertyUpdated();
    }
}

void JoyButton::setTurboInterval(int interval)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((interval < 10) && (interval != this->turboInterval))
    {
        this->setUseTurbo(false);
        interval = 0;
    }

    this->turboInterval = interval;
    emit turboIntervalChanged(interval);
    emit propertyUpdated();
}

void JoyButton::reset()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    resetPrivVars();
}

void JoyButton::resetPrivVars()
{
    disconnectPropertyUpdatedConnections();
    stopTimers(false, false, true);
    releaseActiveSlots();
    clearAssignedSlots();
    clearQueues();
    resetAllProperties(); // quitEvent changed here
}

void JoyButton::reset(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButton::reset();
    m_index = index;
}

bool JoyButton::getToggleState()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_toggle;
}

int JoyButton::getTurboInterval()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return turboInterval;
}

void JoyButton::turboEvent()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    changeTurboParams(isKeyPressed, isButtonPressed);
}

void JoyButton::changeTurboParams(bool _isKeyPressed, bool isButtonPressed)
{
    if (!isButtonPressedQueue.isEmpty())
    {
        if (!_isKeyPressed)
        {
            ignoreSetQueue.clear();
            isButtonPressedQueue.clear();
            isButtonPressedQueue.enqueue(isButtonPressed);
        } else
        {
            isButtonPressedQueue.enqueue(!isButtonPressed);
        }

        ignoreSetQueue.enqueue(false);
    }

    if (!_isKeyPressed)
        createDeskEvent();
    else
        releaseDeskEvent();
    isKeyPressed = !_isKeyPressed;

    if (turboTimer.isActive())
    {
        int tempInterval = turboInterval / 2;

        if (turboTimer.interval() != tempInterval)
            turboTimer.start(tempInterval);
    }
}

bool JoyButton::distanceEvent()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool released = false;

    if (slotiter != nullptr)
    {
        QReadLocker tempLocker(&assignmentsLock);

        if (containsDistanceSlots())
        {
            double currentDistance = getDistanceFromDeadZone();
            double tempDistance = 0.0;
            JoyButtonSlot *previousDistanceSlot = nullptr;
            QListIterator<JoyButtonSlot *> iter(*getAssignedSlots());

            if (previousCycle != nullptr)
            {
                qDebug() << "find previous Cycle in next steps in assignments and skip to it";

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
                        iter.toBack();
                    else
                        previousDistanceSlot = slot;
                } else if (slot->getSlotMode() == JoyButtonSlot::JoyCycle)
                {
                    tempDistance = 0.0;
                    iter.toBack();
                }
            }

            // No applicable distance slot
            if (!previousDistanceSlot)
            {
                if (m_currentDistance != nullptr)
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
                        slotiter->findNext(previousCycle);

                    m_currentDistance = nullptr;
                    released = true;
                }
            }
            // An applicable distance slot was found
            else
            {
                if (m_currentDistance != previousDistanceSlot)
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

                    qDebug() << "Deactive slots in previous range and activate new slots";

                    slotiter->toFront();

                    if (previousCycle != nullptr)
                    {
                        qDebug() << "Find previous Cycle in slotiter starting from beginning";

                        slotiter->findNext(previousCycle);
                    }

                    slotiter->findNext(previousDistanceSlot);

                    m_currentDistance = previousDistanceSlot;
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
        slotiter = new QListIterator<JoyButtonSlot *>(*getAssignedSlots());
        assignmentsLock.unlock();

        distanceEvent();
    } else if (!slotiter->hasPrevious())
    {
        distanceEvent();
    } else if (currentCycle != nullptr)
    {
        currentCycle = nullptr;
        distanceEvent();
    }

    assignmentsLock.lockForRead();
    activateSlots();
    assignmentsLock.unlock();

    if (currentCycle != nullptr)
        quitEvent = true;
    else if ((currentPause == nullptr) && (currentHold == nullptr) && !keyPressTimer.isActive())
        quitEvent = true;
}

void JoyButton::activateSlots()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool countForAllTime = false;

    if (allSlotTimeBetweenSlots == 0)
        countForAllTime = true;

    if (slotiter != nullptr)
    {
        QWriteLocker tempLocker(&activeZoneLock);

        bool exit = false;
        bool delaySequence = false;

        int i = 0;

        while (slotiter->hasNext() && !exit)
        {
            JoyButtonSlot *slot = slotiter->next();

            if (slot->getSlotMode() == JoyButtonSlot::JoyMix)
            {
                qDebug() << "JOYMIX IN ACTIVATESLOTS";

                if (slot->getMixSlots() != nullptr)
                {
                    QListIterator<JoyButtonSlot *> mini_it(*slot->getMixSlots());
                    QListIterator<JoyButtonSlot *> *it(&mini_it);

                    int countMinis = slot->getMixSlots()->count();
                    int timeX = countMinis;

                    std::chrono::time_point<std::chrono::high_resolution_clock> t1, t2;
                    t1 = std::chrono::high_resolution_clock::now();

                    while (it->hasNext())
                    {
                        JoyButtonSlot *slotmini = it->next();
                        qDebug() << "Run activated mini slot - name - deviceCode - mode: " << slotmini->getSlotString()
                                 << " - " << slotmini->getSlotCode() << " - " << slotmini->getSlotMode();

                        MiniSlotRun *minijob = new MiniSlotRun(slot, slotmini, this, timeBetweenMiniSlots * timeX);

                        minijob->setAutoDelete(false);

                        threadPool->start(minijob);

                        timeX--;

                        if (timeBetweenMiniSlots == 55)
                        {
                            t2 = std::chrono::high_resolution_clock::now();
                            timeBetweenMiniSlots = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
                        }
                    }

                    if (countForAllTime)
                    {
                        allSlotTimeBetweenSlots += countMinis * timeBetweenMiniSlots;
                    }

                    threadPool->waitForDone();

                    i++;
                    if (!slotiter->hasNext())
                        break;
                }
            } else
            {
                qDebug() << "Check now simple slots";
                addEachSlotToActives(slot, i, delaySequence, exit, slotiter);
            }
        }

        if (delaySequence && !getActiveSlots().isEmpty())
        {
            keyPressHold.restart();
            keyPressEvent();
        }

        activeZoneTimer.start();
    }
}

void JoyButton::activateMiniSlots(JoyButtonSlot *slot, JoyButtonSlot *mix)
{
    int tempcode = slot->getSlotCode();
    JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();

    switch (mode)
    {
    case JoyButtonSlot::JoyKeyboard: {
        sendKeybEvent(slot, true);

        getActiveSlotsLocal().append(slot);
        int oldvalue = GlobalVariables::JoyButton::JoyButton::activeKeys.value(tempcode, 0) + 1;
        GlobalVariables::JoyButton::JoyButton::activeKeys.insert(tempcode, oldvalue);

        if (!slot->isModifierKey())
        {
            qDebug() << "There has been assigned a lastActiveKey " << slot->getSlotString();

            lastActiveKey = mix;
        } else
        {
            qDebug() << "It's not modifier key. lastActiveKey is null pointer";

            lastActiveKey = nullptr;
        }

        break;
    }
    }
}

void JoyButton::addEachSlotToActives(JoyButtonSlot *slot, int &i, bool &delaySequence, bool &exit,
                                     QListIterator<JoyButtonSlot *> *slotiter)
{
    int tempcode = slot->getSlotCode();
    JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();

    switch (mode)
    {
    case JoyButtonSlot::JoyKeyboard: {
        i++;

        qDebug() << i << ": It's a JoyKeyboard with code: " << tempcode << " and name: " << slot->getSlotString();

        sendevent(slot, true);

        getActiveSlotsLocal().append(slot);
        int oldvalue = GlobalVariables::JoyButton::JoyButton::activeKeys.value(tempcode, 0) + 1;
        GlobalVariables::JoyButton::JoyButton::activeKeys.insert(tempcode, oldvalue);

        if (!slot->isModifierKey())
        {
            qDebug() << "There has been assigned a lastActiveKey " << slot->getSlotString();

            lastActiveKey = slot;
        } else
        {
            qDebug() << "It's not modifier key. lastActiveKey is null pointer";

            lastActiveKey = nullptr;
        }

        break;
    }
    case JoyButtonSlot::JoyMouseButton: {
        i++;

        qDebug() << i << ": It's a JoyMouseButton with code: " << tempcode << " and name: " << slot->getSlotString();

        if ((tempcode == static_cast<int>(JoyButtonSlot::MouseWheelUp)) ||
            (tempcode == static_cast<int>(JoyButtonSlot::MouseWheelDown)))
        {
            slot->getMouseInterval()->restart();
            wheelVerticalTime.restart();
            currentWheelVerticalEvent = slot;
            getActiveSlotsLocal().append(slot);
            wheelEventVertical();
            currentWheelVerticalEvent = nullptr;
        } else if ((tempcode == static_cast<int>(JoyButtonSlot::MouseWheelLeft)) ||
                   (tempcode == static_cast<int>(JoyButtonSlot::MouseWheelRight)))
        {
            slot->getMouseInterval()->restart();
            wheelHorizontalTime.restart();
            currentWheelHorizontalEvent = slot;
            getActiveSlotsLocal().append(slot);
            wheelEventHorizontal();
            currentWheelHorizontalEvent = nullptr;
        } else
        {
            sendevent(slot, true);
            getActiveSlotsLocal().append(slot);
            int oldvalue = GlobalVariables::JoyButton::JoyButton::activeMouseButtons.value(tempcode, 0) + 1;
            GlobalVariables::JoyButton::JoyButton::activeMouseButtons.insert(tempcode, oldvalue);
        }

        break;
    }
    case JoyButtonSlot::JoyMouseMovement: {
        i++;

        qDebug() << i << ": It's a JoyMouseMovement with code: " << tempcode << " and name: " << slot->getSlotString();

        slot->getMouseInterval()->restart();

        getActiveSlotsLocal().append(slot);

        if (pendingMouseButtons.size() == 0)
            mouseHelper.setFirstSpringStatus(true);

        pendingMouseButtons.append(this);
        mouseEventQueue.enqueue(slot);

        // Temporarily lower timer interval. Helps improve mouse control
        // precision on the lower end of an axis.
        if (!staticMouseEventTimer.isActive() || (staticMouseEventTimer.interval() != 0))
        {
            if (!staticMouseEventTimer.isActive() ||
                (staticMouseEventTimer.interval() == GlobalVariables::JoyButton::IDLEMOUSEREFRESHRATE))
            {
                int tempRate =
                    qBound(0, GlobalVariables::JoyButton::mouseRefreshRate - GlobalVariables::JoyButton::gamepadRefreshRate,
                           GlobalVariables::JoyButton::MAXIMUMMOUSEREFRESHRATE);
                staticMouseEventTimer.start(tempRate);
                testOldMouseTime.restart();
                accelExtraDurationTime.restart();
            }
        }

        break;
    }
    case JoyButtonSlot::JoyPause: {
        i++;

        qDebug() << i << ": It's a JoyPause with code: " << tempcode << " and name: " << slot->getSlotString();

        if (!getActiveSlots().isEmpty())
        {
            qDebug() << "active slots QHash is not empty";

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
            qDebug() << "active slots QHash is empty";

            currentPause = slot;
            pauseHold.restart();
            inpauseHold.restart();
            pauseWaitTimer.start(0);
            exit = true;
        }

        break;
    }
    case JoyButtonSlot::JoyHold: {
        i++;

        qDebug() << i << ": It's a JoyHold with code: " << tempcode << " and name: " << slot->getSlotString();

        currentHold = slot;
        holdTimer.start(0);
        exit = true;
        break;
    }
    case JoyButtonSlot::JoyDelay: {
        i++;

        qDebug() << i << ": It's a JoyDelay with code: " << tempcode << " and name: " << slot->getSlotString();

        currentDelay = slot;
        buttonDelay.restart();
        delayTimer.start(0);
        exit = true;
        break;
    }
    case JoyButtonSlot::JoyCycle: {
        i++;

        qDebug() << i << ": It's a JoyCycle with code: " << tempcode << " and name: " << slot->getSlotString();

        currentCycle = slot;
        exit = true;
        break;
    }
    case JoyButtonSlot::JoyDistance: {
        i++;

        qDebug() << i << ": It's a JoyDistance with code: " << tempcode << " and name: " << slot->getSlotString();

        exit = true;
        break;
    }
    case JoyButtonSlot::JoyRelease: {
        i++;

        qDebug() << i << ": It's a JoyRelease with code: " << tempcode << " and name: " << slot->getSlotString();

        if (currentRelease == nullptr)
        {
            findJoySlotsEnd(slotiter);
        } else if ((currentRelease != nullptr) && getActiveSlots().isEmpty())
        {
            qDebug() << "current is release but activeSlots is empty";

            exit = true;
        } else if ((currentRelease != nullptr) && !getActiveSlots().isEmpty())
        {
            qDebug() << "current is release and activeSlots is not empty";

            if (slotiter->hasPrevious())
            {
                qDebug() << "Back to previous slotiter from release";

                i--;
                slotiter->previous();
            }

            delaySequence = true;
            exit = true;
        }

        break;
    }
    case JoyButtonSlot::JoyMouseSpeedMod: {
        i++;

        qDebug() << i << ": It's a JoyMouseSpeedMod with code: " << tempcode << " and name: " << slot->getSlotString();

        GlobalVariables::JoyButton::mouseSpeedModifier = tempcode * 0.01;
        mouseSpeedModList.append(slot);
        getActiveSlotsLocal().append(slot);

        break;
    }
    case JoyButtonSlot::JoyKeyPress: {
        i++;

        qDebug() << i << ": It's a JoyKeyPress with code: " << tempcode << " and name: " << slot->getSlotString();

        if (getActiveSlots().isEmpty())
        {
            qDebug() << "activeSlots is empty. It's a true delaySequence and assigned currentKeyPress";

            delaySequence = true;
            currentKeyPress = slot;
        } else
        {
            qDebug() << "activeSlots is not empty. It's a true delaySequence and exit";

            if (slotiter->hasPrevious())
            {
                qDebug() << "Back to previous slotiter from JoyKeyPress";

                i--;
                slotiter->previous();
            }

            delaySequence = true;
            exit = true;
        }

        break;
    }
    case JoyButtonSlot::JoyLoadProfile: {
        i++;

        qDebug() << i << ": It's a JoyLoadProfile with code: " << tempcode << " and name: " << slot->getSlotString();

        releaseActiveSlots();
        slotiter->toBack();
        exit = true;

        QString location = slot->getTextData();

        if (!location.isEmpty())
            m_parentSet->getInputDevice()->sendLoadProfileRequest(location);

        break;
    }
    case JoyButtonSlot::JoySetChange: {
        i++;

        qDebug() << i << ": It's a JoySetChange with code: " << tempcode << " and name: " << slot->getSlotString();

        getActiveSlotsLocal().append(slot);

        break;
    }
    case JoyButtonSlot::JoyTextEntry:
    case JoyButtonSlot::JoyExecute: {
        i++;

        qDebug() << i << ": It's a JoyExecute or JoyTextEntry with code: " << tempcode
                 << " and name: " << slot->getSlotString();

        sendevent(slot, true);

        break;
    }
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
        if (setChangeIndex != m_originset)
        {
            emit released(m_index);
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

    JoyButtonSlot *buttonslot = nullptr;
    bool singleShot = false;

    if (currentMouseEvent != nullptr)
    {
        buttonslot = currentMouseEvent;
        singleShot = true;
    }

    if ((buttonslot != nullptr) || !mouseEventQueue.isEmpty())
    {
        updateMouseParams(true, true, 0.0);

        QQueue<JoyButtonSlot *> tempQueue;

        if (buttonslot == nullptr)
            buttonslot = mouseEventQueue.dequeue();

        int timeElapsed = testOldMouseTime.elapsed();

        // Presumed initial mouse movement. Use full duration rather than
        // partial.
        if (staticMouseEventTimer.interval() < GlobalVariables::JoyButton::mouseRefreshRate)
        {
            timeElapsed = GlobalVariables::JoyButton::mouseRefreshRate + (timeElapsed - staticMouseEventTimer.interval());
        }

        while (buttonslot != nullptr)
        {
            QElapsedTimer *mouseInterval = buttonslot->getMouseInterval();

            int mousedirection = buttonslot->getSlotCode();
            JoyButton::JoyMouseMovementMode mousemode = getMouseMode();

            bool isActive = getActiveSlots().contains(buttonslot);

            if (isActive)
            {
                int mousespeed = 0;

                if (mousemode == JoyButton::MouseCursor)
                {
                    switch (mousedirection)
                    {
                    case 1:
                    case 2:
                        mousespeed = mouseSpeedY;
                        break;

                    case 3:
                    case 4:
                        mousespeed = mouseSpeedX;
                        break;
                    }

                    double difference = getMouseDistanceFromDeadZone();
                    double mouse1 = 0;
                    double mouse2 = 0;
                    double sumDist = buttonslot->getMouseDistance();
                    JoyMouseCurve currentCurve = getMouseCurve();

                    switch (currentCurve)
                    {
                    case QuadraticCurve: {
                        difference = difference * difference;
                        break;
                    }
                    case CubicCurve: {
                        difference = difference * difference * difference;
                        break;
                    }
                    case QuadraticExtremeCurve: {
                        double temp = difference;
                        difference = difference * difference;
                        difference = (temp >= 0.95) ? (difference * 1.5) : difference;
                        break;
                    }
                    case PowerCurve: {
                        double tempsensitive = qMin(qMax(sensitivity, 1.0e-3), 1.0e+3);
                        double temp = qMin(qMax(pow(difference, 1.0 / tempsensitive), 0.0), 1.0);
                        difference = temp;
                        break;
                    }
                    case EnhancedPrecisionCurve: {
                        // Perform different forms of acceleration depending on
                        // the range of the element from its assigned dead zone.
                        // Useful for more precise controls with an axis.
                        double temp = difference;
                        if (temp <= 0.4)
                        {
                            // Low slope value for really slow acceleration
                            difference = (difference * 0.37);
                        } else if (temp <= 0.75)
                        {
                            // Perform Linear accleration with an appropriate
                            // offset.
                            difference = (difference - 0.252);
                        } else
                        {
                            // Perform mouse acceleration. Make up the difference
                            // due to the previous two segments. Maxes out at 1.0.
                            difference = (difference * 2.008) - 1.008;
                        }

                        break;
                    }
                    case EasingQuadraticCurve:
                    case EasingCubicCurve: {
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
                        } else if (temp <= 0.75)
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
                        } else
                        {
                            // Gradually increase the mouse speed until the specified elapsed duration
                            // time has passed.
                            int easingElapsed = buttonslot->getEasingTime()->elapsed();
                            double easingDuration = m_easingDuration; // Time in seconds
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
                                } else
                                {
                                    elapsedDiff = (1.5 - 1.0) * (elapsedDiff * elapsedDiff * elapsedDiff) + 1.0;
                                }
                            } else
                            {
                                elapsedDiff = 1.5;
                            }

                            // Allow gradient control on the high end of an axis.
                            difference = (elapsedDiff * difference);
                            difference = (difference * 1.33067 - 0.496005);
                        }
                        break;
                    }
                    default:
                        break;
                    }

                    double distance = 0;
                    difference = (GlobalVariables::JoyButton::mouseSpeedModifier == 1.0)
                                     ? difference
                                     : (difference * GlobalVariables::JoyButton::mouseSpeedModifier);

                    double mintravel = minMouseDistanceAccelThreshold * 0.01;
                    double minstop = qMax(0.05, mintravel);

                    // Last check ensures that acceleration is only applied for the same direction.
                    if (extraAccelerationEnabled && isPartRealAxis() &&
                        (fabs(getAccelerationDistance() - lastAccelerationDistance) >= mintravel) &&
                        (((getAccelerationDistance() - lastAccelerationDistance) >= 0) == (getAccelerationDistance() >= 0)))
                    {
                        double magfactor = extraAccelerationMultiplier;
                        double minfactor = qMax((GlobalVariables::JoyButton::DEFAULTSTARTACCELMULTIPLIER * 0.001) + 1.0,
                                                magfactor * (startAccelMultiplier * 0.01));
                        double maxtravel = maxMouseDistanceAccelThreshold * 0.01;
                        double slope = (magfactor - minfactor) / (maxtravel - mintravel);
                        double intercept = minfactor - (slope * mintravel);

                        double intermediateTravel =
                            qMin(maxtravel, fabs(getAccelerationDistance() - lastAccelerationDistance));
                        if ((currentAccelMulti > 1.0) && (oldAccelMulti == 0.0))
                        {
                            intermediateTravel = qMin(maxtravel, intermediateTravel + mintravel);
                        }

                        double currentAccelMultiTemp = (slope * intermediateTravel + intercept);
                        if (extraAccelCurve == EaseOutSineCurve)
                        {
                            double getMultiDiff2 =
                                ((currentAccelMultiTemp - minfactor) / (extraAccelerationMultiplier - minfactor));
                            currentAccelMultiTemp = (extraAccelerationMultiplier - minfactor) *
                                                        sin(getMultiDiff2 * (GlobalVariables::JoyControlStick::PI / 2.0)) +
                                                    minfactor;
                        } else if (extraAccelCurve == EaseOutQuadAccelCurve)
                        {
                            double getMultiDiff2 =
                                ((currentAccelMultiTemp - minfactor) / (extraAccelerationMultiplier - minfactor));
                            currentAccelMultiTemp =
                                -(extraAccelerationMultiplier - minfactor) * (getMultiDiff2 * (getMultiDiff2 - 2)) +
                                minfactor;
                        } else if (extraAccelCurve == EaseOutCubicAccelCurve)
                        {
                            double getMultiDiff =
                                ((currentAccelMultiTemp - minfactor) / (extraAccelerationMultiplier - minfactor)) - 1;
                            currentAccelMultiTemp = (extraAccelerationMultiplier - minfactor) *
                                                        ((getMultiDiff) * (getMultiDiff) * (getMultiDiff) + 1) +
                                                    minfactor;
                        }

                        difference = difference * currentAccelMultiTemp;
                        currentAccelMulti = currentAccelMultiTemp;
                        updateOldAccelMulti = currentAccelMulti;
                        accelTravel = intermediateTravel;
                        accelExtraDurationTime.restart();
                    } else if (extraAccelerationEnabled && isPartRealAxis() && (accelDuration > 0.0) &&
                               (currentAccelMulti > 0.0) &&
                               (fabs(getAccelerationDistance() - startingAccelerationDistance) < minstop))
                    {
                        qDebug() << "Keep Trying: " << fabs(getAccelerationDistance() - lastAccelerationDistance);
                        qDebug() << "MIN TRAVEL: " << mintravel;

                        updateStartingMouseDistance = true;
                        double magfactor = extraAccelerationMultiplier;
                        double minfactor = qMax((GlobalVariables::JoyButton::DEFAULTSTARTACCELMULTIPLIER * 0.001) + 1.0,
                                                magfactor * (startAccelMultiplier * 0.01));
                        double maxtravel = maxMouseDistanceAccelThreshold * 0.01;
                        double slope = (magfactor - minfactor) / (maxtravel - mintravel);
                        double intercept = minfactor - (slope * mintravel);

                        int elapsedElapsed = accelExtraDurationTime.elapsed();

                        double intermediateTravel = accelTravel;
                        if (((getAccelerationDistance() - startingAccelerationDistance) >= 0) !=
                            (getAccelerationDistance() >= 0))
                        {
                            // Travelling towards dead zone. Decrease acceleration and duration.
                            intermediateTravel =
                                qMax(intermediateTravel - fabs(getAccelerationDistance() - startingAccelerationDistance),
                                     mintravel);
                        }

                        // Linear case
                        double currentAccelMultiTemp = (slope * intermediateTravel + intercept);
                        double elapsedDuration = accelDuration * ((currentAccelMultiTemp - minfactor) /
                                                                  (extraAccelerationMultiplier - minfactor));

                        switch (extraAccelCurve)
                        {
                        case EaseOutSineCurve: {
                            double multiDiff =
                                ((currentAccelMultiTemp - minfactor) / (extraAccelerationMultiplier - minfactor));
                            double temp = sin(multiDiff * (GlobalVariables::JoyControlStick::PI / 2.0));
                            elapsedDuration = accelDuration * temp + 0;
                            currentAccelMultiTemp = (extraAccelerationMultiplier - minfactor) *
                                                        sin(multiDiff * (GlobalVariables::JoyControlStick::PI / 2.0)) +
                                                    minfactor;

                            break;
                        }
                        case EaseOutQuadAccelCurve: {
                            double getMultiDiff2 =
                                ((currentAccelMultiTemp - minfactor) / (extraAccelerationMultiplier - minfactor));
                            double temp = (getMultiDiff2 * (getMultiDiff2 - 2));
                            elapsedDuration = -accelDuration * temp + 0;
                            currentAccelMultiTemp = -(extraAccelerationMultiplier - minfactor) * temp + minfactor;

                            break;
                        }
                        case EaseOutCubicAccelCurve: {
                            double getMultiDiff =
                                ((currentAccelMultiTemp - minfactor) / (extraAccelerationMultiplier - minfactor)) - 1;
                            double temp = ((getMultiDiff) * (getMultiDiff) * (getMultiDiff) + 1);
                            elapsedDuration = accelDuration * temp + 0;
                            currentAccelMultiTemp = (extraAccelerationMultiplier - minfactor) * temp + minfactor;

                            break;
                        }
                        default: {
                            break;
                        }
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
                        } else
                        {
                            elapsedDiff = 1.0;
                            currentAccelMulti = 0.0;
                            updateOldAccelMulti = 0.0;
                            accelTravel = 0.0;
                        }
                    } else if (extraAccelerationEnabled && isPartRealAxis())
                    {
                        currentAccelMulti = 0.0;
                        updateStartingMouseDistance = true;
                        oldAccelMulti = updateOldAccelMulti = 0.0;
                        accelTravel = 0.0;
                    }

                    sumDist += difference * (mousespeed * GlobalVariables::JoyButtonSlot::JOYSPEED * timeElapsed) * 0.001;
                    distance = sumDist;

                    switch (mousedirection)
                    {
                    case 1:
                        mouse2 = -distance;
                        break;

                    case 2:
                        mouse2 = distance;
                        break;

                    case 3:
                        mouse1 = -distance;
                        break;

                    case 4:
                        mouse1 = distance;
                        break;
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
                } else if (mousemode == JoyButton::MouseSpring)
                {
                    double mouse1 = -2.0;
                    double mouse2 = -2.0;
                    double difference = getMouseDistanceFromDeadZone();

                    switch (mousedirection)
                    {
                    case 1:

                        setDistanceForSpring(mouseHelper, mouse1, mouse2, -difference);
                        break;

                    case 2:

                        setDistanceForSpring(mouseHelper, mouse1, mouse2, difference);
                        break;

                    case 3:

                        setDistanceForSpring(mouseHelper, mouse2, mouse1, -difference);
                        break;

                    case 4:

                        setDistanceForSpring(mouseHelper, mouse2, mouse1, difference);
                        break;
                    }

                    updateMouseProperties(mouse1, 0.0, springWidth, springHeight, relativeSpring,
                                          GlobalVariables::JoyButton::springModeScreen, springXSpeeds, 'X');
                    updateMouseProperties(mouse2, 0.0, springWidth, springHeight, relativeSpring,
                                          GlobalVariables::JoyButton::springModeScreen, springYSpeeds, 'Y');
                    mouseInterval->restart();
                }

                tempQueue.enqueue(buttonslot);
            }

            if (!mouseEventQueue.isEmpty() && !singleShot)
                buttonslot = mouseEventQueue.dequeue();
            else
                buttonslot = nullptr;
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

void JoyButton::setDistanceForSpring(JoyButtonMouseHelper &mouseHelper, double &mouseFirstAx, double &mouseSecondAx,
                                     double distanceFromDeadZone)
{
    if (mouseHelper.getFirstSpringStatus())
    {
        mouseFirstAx = 0.0;
        mouseHelper.setFirstSpringStatus(false);
    }

    mouseSecondAx = distanceFromDeadZone;
}

void JoyButton::updateMouseProperties(double newAxisValue, double newSpringDead, int newSpringWidth, int newSpringHeight,
                                      bool relatived, int modeScreen, QList<PadderCommon::springModeInfo> &springSpeeds,
                                      QChar axis, double newAxisValueY, double newSpringDeadY)
{
    PadderCommon::springModeInfo axisInfo;

    if (axis == 'X')
    {

        axisInfo.displacementX = newAxisValue;
        axisInfo.springDeadX = newSpringDead;

    } else if (axis == 'Y')
    {

        axisInfo.displacementY = newAxisValue;
        axisInfo.springDeadY = newSpringDead;

    } else
    {

        axisInfo.displacementX = newAxisValue;
        axisInfo.springDeadX = newSpringDead;
        axisInfo.displacementY = newAxisValueY;
        axisInfo.springDeadY = newSpringDeadY;
    }

    axisInfo.width = newSpringWidth;
    axisInfo.height = newSpringHeight;
    axisInfo.relative = relatived;
    axisInfo.screen = modeScreen;
    springSpeeds.append(axisInfo);
}

void JoyButton::wheelEventVertical()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButtonSlot *buttonslot = nullptr;

    if (currentWheelVerticalEvent != nullptr)
        buttonslot = currentWheelVerticalEvent;

    if (buttonslot && (wheelSpeedY != 0))
    {
        bool isActive = getActiveSlots().contains(buttonslot);

        if (isActive)
        {
            sendevent(buttonslot, true);
            sendevent(buttonslot, false);
            mouseWheelVerticalEventQueue.enqueue(buttonslot);
            mouseWheelVerticalEventTimer.start(1000 / wheelSpeedY);
        } else
        {
            mouseWheelVerticalEventTimer.stop();
        }
    } else if (!mouseWheelVerticalEventQueue.isEmpty() && (wheelSpeedY != 0))
    {
        QQueue<JoyButtonSlot *> tempQueue;

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
        } else
        {
            mouseWheelVerticalEventTimer.stop();
        }
    } else
    {
        mouseWheelVerticalEventTimer.stop();
    }
}

void JoyButton::wheelEventHorizontal()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButtonSlot *buttonslot = nullptr;

    if (currentWheelHorizontalEvent != nullptr)
        buttonslot = currentWheelHorizontalEvent;

    if (buttonslot && (wheelSpeedX != 0))
    {
        bool isActive = getActiveSlots().contains(buttonslot);

        if (isActive)
        {
            sendevent(buttonslot, true);
            sendevent(buttonslot, false);
            mouseWheelHorizontalEventQueue.enqueue(buttonslot);
            mouseWheelHorizontalEventTimer.start(1000 / wheelSpeedX);
        } else
        {
            mouseWheelHorizontalEventTimer.stop();
        }
    } else if (!mouseWheelHorizontalEventQueue.isEmpty() && (wheelSpeedX != 0))
    {
        QQueue<JoyButtonSlot *> tempQueue;

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
        } else
        {
            mouseWheelHorizontalEventTimer.stop();
        }
    } else
    {
        mouseWheelHorizontalEventTimer.stop();
    }
}

void JoyButton::setUseTurbo(bool useTurbo)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool initialState = m_useTurbo;

    if (useTurbo != m_useTurbo)
    {
        if (useTurbo && this->containsSequence())
            m_useTurbo = false;
        else
            m_useTurbo = useTurbo;

        if (initialState != m_useTurbo)
        {
            emit turboChanged(m_useTurbo);
            emit propertyUpdated();

            if (m_useTurbo && (this->turboInterval == 0))
                this->setTurboInterval(GlobalVariables::JoyButton::ENABLEDTURBODEFAULT);
        }
    }
}

bool JoyButton::isUsingTurbo()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_useTurbo;
}

QString JoyButton::getXmlName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return GlobalVariables::JoyButton::xmlName;
}

QString JoyButton::getName(bool forceFullFormat, bool displayNames)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString newlabel = getPartialName(forceFullFormat, displayNames);
    qDebug() << "partial name is: " << newlabel;
    newlabel.append(": ");

    qDebug() << "actionName is " << actionName;
    if (!actionName.isEmpty() && displayNames)
        newlabel.append(actionName);
    else
        newlabel.append(getCalculatedActiveZoneSummary());

    qDebug() << "name in getName(bool forceFullFormat, bool displayNames) is now: " << newlabel;
    return newlabel;
}

QString JoyButton::getPartialName(bool forceFullFormat, bool displayNames) const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();

    if (!buttonName.isEmpty() && displayNames)
    {
        if (forceFullFormat)
            temp.append(tr("Button")).append(" ");

        temp.append(buttonName);
    } else if (!defaultButtonName.isEmpty())
    {
        if (forceFullFormat)
            temp.append(tr("Button")).append(" ");

        temp.append(defaultButtonName);
    } else
    {
        temp.append(tr("Button")).append(" ").append(QString::number(getRealJoyNumber()));
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
        QListIterator<JoyButtonSlot *> iter(*getAssignedSlots());
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
    } else
    {
        newlabel = newlabel.append(tr("[NO KEY]"));
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

    QList<JoyButtonSlot *> tempList = getActiveZoneList();
    return buildActiveZoneSummary(tempList);
}

QString JoyButton::getCalculatedActiveZoneSummary()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();
    lockForWritedString(temp, activeZoneString);
    return temp;
}

/**
 * @brief Generate active zone string and notify other objects.
 */
void JoyButton::buildActiveZoneSummaryString()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    lockForWritedString(activeZoneString, getActiveZoneSummary());
    qDebug() << "activeZoneString after getActiveZoneSummary() is: " << activeZoneString;
    emit activeZoneChanged();
}

/**
 * @brief Generate active zone string but do not notify any other object.
 */
void JoyButton::localBuildActiveZoneSummaryString()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    lockForWritedString(activeZoneString, getActiveZoneSummary());
}

QString JoyButton::buildActiveZoneSummary(QList<JoyButtonSlot *> &tempList)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString newlabel = QString();
    QListIterator<JoyButtonSlot *> iter(tempList);
    QListIterator<JoyButtonSlot *> *iterMain(&iter);
    QStringList stringlist = QStringList();
    QStringList stringListMix = QStringList();
    int i = 0;
    int j = 0;
    bool slotsActive = !getActiveSlots().isEmpty();

    if (setSelectionCondition == SetChangeOneWay)
    {
        newlabel.append(tr("[Set %1 1W]").arg(setSelection + 1));
        if (iterMain->hasNext())
            newlabel.append(" ");
    } else if (setSelectionCondition == SetChangeTwoWay)
    {
        newlabel = newlabel.append(tr("[Set %1 2W]").arg(setSelection + 1));
        if (iterMain->hasNext())
            newlabel.append(" ");
    }

    if (setSelectionCondition == SetChangeWhileHeld)
    {
        newlabel.append(tr("[Set %1 WH]").arg(setSelection + 1));
    } else if (iterMain->hasNext())
    {
        bool behindHold = false;

        while (iterMain->hasNext())
        {
            JoyButtonSlot *slot = iterMain->next();
            JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();

            if (mode == JoyButtonSlot::JoySlotInputAction::JoyMix)
            {
                QListIterator<JoyButtonSlot *> iterMini(*slot->getMixSlots());
                QListIterator<JoyButtonSlot *> *iterM(&iterMini);

                while (iterM->hasNext())
                {
                    JoyButtonSlot *slotMini = iterM->next();
                    JoyButtonSlot::JoySlotInputAction modeMini = slotMini->getSlotMode();
                    qDebug() << "modeMini is " << modeMini;
                    qDebug() << "slotsActive are empty? " << slotsActive;
                    buildActiveZoneSummarySwitchSlots(modeMini, slotMini, behindHold, &stringListMix, j, iterM, slotsActive);

                    stringListMix.append("+");

                    qDebug() << "Create summary for JoyMix. Progress: " << stringListMix;
                }

                j = 0;
                i++;

                if (!stringListMix.isEmpty())
                {
                    if (stringListMix.last() == '+')
                        stringListMix.removeLast();

                    qDebug() << "Create summary for JoyMix. Progress: " << stringListMix;

                    QString res = "";

                    for (const QString &strListEl : stringListMix)
                        res += strListEl;

                    stringlist.append(res);
                    stringListMix.clear();
                } else
                {
                    stringlist.append(slot->getTextData());
                    stringListMix.clear();
                }

                behindHold = false;

            } else
            {
                buildActiveZoneSummarySwitchSlots(mode, slot, behindHold, &stringlist, i, iterMain, slotsActive);
            }

            if ((i > 4) && iterMain->hasNext())
            {
                stringlist.append(" ...");
                iterMain->toBack();
            }
        }

        newlabel.append(stringlist.join(", "));
    } else if (setSelectionCondition == SetChangeDisabled)
    {
        newlabel.append(tr("[NO KEY]"));
    }

    qDebug() << "NEW LABEL IS: " << newlabel;
    qDebug() << "i: " << i;
    qDebug() << "j: " << j;
    return newlabel;
}

void JoyButton::buildActiveZoneSummarySwitchSlots(JoyButtonSlot::JoySlotInputAction mode, JoyButtonSlot *slot,
                                                  bool &behindHold, QStringList *stringlist, int &i,
                                                  QListIterator<JoyButtonSlot *> *iter, bool slotsActive)
{
    switch (mode)
    {
    case JoyButtonSlot::JoyKeyboard:
    case JoyButtonSlot::JoyMouseButton:
    case JoyButtonSlot::JoyMouseMovement: {
        QString temp = slot->getSlotString();

        if (behindHold)
        {
            temp.prepend("[H] ");
            behindHold = false;
        }

        stringlist->append(temp);
        i++;
        break;
    }
    case JoyButtonSlot::JoyKeyPress: {
        // Skip slot if a press time slot was inserted.
        break;
    }
    case JoyButtonSlot::JoyHold: {
        if (!slotsActive && (i == 0))
        {
            // If button is not active and first slot is a hold,
            // keep processing slots but take note of the hold.
            behindHold = true;
        } else
        {
            // Move iter to back so loop will end.
            iter->toBack();
        }

        break;
    }
    case JoyButtonSlot::JoyLoadProfile:
    case JoyButtonSlot::JoySetChange:
    case JoyButtonSlot::JoyTextEntry:
    case JoyButtonSlot::JoyExecute: {
        QString temp = slot->getSlotString();

        if (behindHold)
        {
            temp.prepend("[H] ");
            behindHold = false;
        }

        stringlist->append(temp);
        i++;
        break;
    }
    case JoyButtonSlot::JoyRelease: {
        if (currentRelease == nullptr)
            findJoySlotsEnd(iter);

        break;
    }
    case JoyButtonSlot::JoyDistance: {
        iter->toBack();
        break;
    }
    case JoyButtonSlot::JoyDelay: {
        iter->toBack();
        break;
    }
    case JoyButtonSlot::JoyCycle: {
        iter->toBack();
        break;
    }

    default:

        if (mode > 15 || mode < 0)
        {
            QString temp = slot->getSlotString();

            if (behindHold)
            {
                temp.prepend("[H] ");
                behindHold = false;
            }

            stringlist->append(temp);
            i++;
            break;
        }

        break;
    }
}

QList<JoyButtonSlot *> JoyButton::getActiveZoneList()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QListIterator<JoyButtonSlot *> activeSlotsIter(getActiveSlots());
    QListIterator<JoyButtonSlot *> assignmentsIter(*getAssignedSlots());
    QListIterator<JoyButtonSlot *> *iter = nullptr;
    QReadWriteLock *tempLock = nullptr;

    qDebug() << "Active slots are: ";

    int x, y;
    x = 0;
    y = 0;
    for (auto actSlot : getActiveSlots())
    {
        x++;
        qDebug() << x << ") " << actSlot->getSlotString();
    }

    qDebug() << "Assigned slots are: ";
    for (auto assignedSlot : *getAssignedSlots())
    {
        y++;
        qDebug() << y << ") " << assignedSlot->getSlotString();
    }

    activeZoneLock.lockForRead();
    int numActiveSlots = getActiveSlots().size();
    activeZoneLock.unlock();

    if (numActiveSlots > 0)
    {
        tempLock = &activeZoneLock;
        iter = &activeSlotsIter;
    } else
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
            qDebug() << "if there exists previous Cycle, find it in activeSlots";

            iter->findNext(previousCycle);
        }
    }

    QList<JoyButtonSlot *> tempSlotList;

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
            case JoyButtonSlot::JoyKeyPress:
            case JoyButtonSlot::JoyHold:
            case JoyButtonSlot::JoyLoadProfile:
            case JoyButtonSlot::JoySetChange:
            case JoyButtonSlot::JoyTextEntry:
            case JoyButtonSlot::JoyExecute:
            case JoyButtonSlot::JoyMix: {
                tempSlotList.append(slot);
                break;
            }
            case JoyButtonSlot::JoyRelease: {
                if (currentRelease == nullptr)
                    findJoySlotsEnd(iter);

                break;
            }
            case JoyButtonSlot::JoyDistance:
            case JoyButtonSlot::JoyCycle: {
                iter->toBack();
                break;
            }
            default: {
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
        qDebug() << "There is more assignments than 0 in getSlotsString(): " << getAssignedSlots()->count();

        QListIterator<JoyButtonSlot *> iter(*getAssignedSlots());
        QStringList stringlist = QStringList();

        while (iter.hasNext())
        {
            JoyButtonSlot *slot = iter.next();
            qDebug() << "deviceCode = " << slot->getSlotCode();
            qDebug() << "slotMode = " << slot->getSlotMode();
            QString slotString = slot->getSlotString();

            if (slotString == tr("[NO KEY]"))
            {
                qDebug() << "EMPTY ASSIGNED SLOT";
            }

            stringlist.append(slotString); // tu
        }

        label = stringlist.join(", ");
    } else
    {
        qDebug() << "There is no assignments for button in getSlotsString()";

        label = label.append(tr("[NO KEY]"));
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

    if (slot->getSlotMode() == JoyButtonSlot::JoyDistance && (slot->getSlotCode() >= 1) && (slot->getSlotCode() <= 100) &&
        (getTotalSlotDistance(slot) <= 1.0))
    {
        assignmentsLock.lockForWrite();
        getAssignmentsLocal().append(slot);
        assignmentsLock.unlock();

        buildActiveZoneSummaryString();
        slotInserted = true;
    } else if (slot->getSlotCode() >= 0)
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
    } else if (slot != nullptr)
    {
        delete slot;
        slot = nullptr;
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
bool JoyButton::setAssignedSlot(int code, int alias, JoyButtonSlot::JoySlotInputAction mode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool slotInserted = false;
    JoyButtonSlot *slot = new JoyButtonSlot(code, alias, mode, this);

    if (slot->getSlotMode() == JoyButtonSlot::JoyDistance && (slot->getSlotCode() >= 1) && (slot->getSlotCode() <= 100) &&
        (getTotalSlotDistance(slot) <= 1.0))
    {
        assignmentsLock.lockForWrite();
        getAssignmentsLocal().append(slot);
        assignmentsLock.unlock();

        buildActiveZoneSummaryString();
        slotInserted = true;
    } else if (slot->getSlotCode() >= 0)
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
    } else if (slot != nullptr)
    {
        delete slot;
        slot = nullptr;
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
bool JoyButton::setAssignedSlot(int code, int alias, int index, JoyButtonSlot::JoySlotInputAction mode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool permitSlot = true;
    JoyButtonSlot *slot = new JoyButtonSlot(code, alias, mode, this);

    if ((slot->getSlotMode() == JoyButtonSlot::JoyDistance) && (slot->getSlotCode() >= 1) && (slot->getSlotCode() <= 100))
    {
        if (getTotalSlotDistance(slot) > 1.0)
            permitSlot = false;
    } else if (slot->getSlotCode() < 0)
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
            if (temp != nullptr)
            {
                delete temp;
                temp = nullptr;
            }

            getAssignmentsLocal().replace(index, slot);
        } else if (index >= getAssignmentsLocal().count())
        {
            // Append code into a new slot
            getAssignmentsLocal().append(slot);
        }

        checkTurboCondition(slot);
        assignmentsLock.unlock();
        buildActiveZoneSummaryString();
        emit slotsChanged();
    } else if (slot != nullptr)
    {
        delete slot;
        slot = nullptr;
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
bool JoyButton::insertAssignedSlot(int code, int alias, int index, JoyButtonSlot::JoySlotInputAction mode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool permitSlot = true;
    JoyButtonSlot *slot = new JoyButtonSlot(code, alias, mode, this);

    if (slot->getSlotMode() == JoyButtonSlot::JoyDistance && (slot->getSlotCode() >= 1) && (slot->getSlotCode() <= 100))
    {
        if (getTotalSlotDistance(slot) > 1.0)
            permitSlot = false;
    } else if (slot->getSlotCode() < 0)
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
        } else if (index >= getAssignedSlots()->count())
        {
            // Append new slot into list.
            getAssignmentsLocal().append(slot);
        }

        qDebug() << "assignments variable in joybutton has now: " << getAssignedSlots()->count() << " input slots";

        checkTurboCondition(slot);
        assignmentsLock.unlock();
        buildActiveZoneSummaryString();
        emit slotsChanged();
    } else if (slot != nullptr)
    {
        delete slot;
        slot = nullptr;
    }

    return permitSlot;
}

bool JoyButton::insertAssignedSlot(JoyButtonSlot *newSlot, bool updateActiveString)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool permitSlot = false;

    if ((newSlot->getSlotMode() == JoyButtonSlot::JoyDistance) && (newSlot->getSlotCode() >= 1) &&
        (newSlot->getSlotCode() <= 100))
    {
        if (getTotalSlotDistance(newSlot) <= 1.0)
            permitSlot = true;
    } else if ((newSlot->getSlotMode() == JoyButtonSlot::JoyTextEntry ||
                newSlot->getSlotMode() == JoyButtonSlot::JoyExecute) &&
               !newSlot->getTextData().isEmpty())
    {
        permitSlot = true;
    } else if ((newSlot->getSlotMode() == JoyButtonSlot::JoyLoadProfile) || newSlot->getSlotCode() >= 0)
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
            buildActiveZoneSummaryString();

        emit slotsChanged();
    }

    return permitSlot;
}

bool JoyButton::insertAssignedSlot(JoyButtonSlot *newSlot, int index, bool updateActiveString)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool permitSlot = true;
    JoyButtonSlot *slot = new JoyButtonSlot(newSlot, this);

    if (slot->getSlotMode() == JoyButtonSlot::JoyDistance && (slot->getSlotCode() >= 1) && (slot->getSlotCode() <= 100))
    {
        if (getTotalSlotDistance(slot) > 1.0)
            permitSlot = false;
    } else if (slot->getSlotCode() < 0)
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
        } else if (index >= getAssignedSlots()->count())
        {
            // Append new slot into list.
            getAssignmentsLocal().append(slot);
        }

        qDebug() << "assignments variable in joybutton has now: " << getAssignedSlots()->count() << " input slots";

        checkTurboCondition(slot);
        assignmentsLock.unlock();
        buildActiveZoneSummaryString();
        emit slotsChanged();
    } else if (slot != nullptr)
    {
        /*if (slot->getSlotMode() == 15)
        {
            qDeleteAll(*slot->getMixSlots());
        }*/

        delete slot;
        slot = nullptr;
    }

    return permitSlot;
}

bool JoyButton::setAssignedSlot(JoyButtonSlot *otherSlot, int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool permitSlot = false;
    JoyButtonSlot *newslot = new JoyButtonSlot(otherSlot, this);

    if (newslot->getSlotMode() == JoyButtonSlot::JoyDistance && (newslot->getSlotCode() >= 1) &&
        (newslot->getSlotCode() <= 100))
    {
        if (getTotalSlotDistance(newslot) <= 1.0)
            permitSlot = true;
    } else if (newslot->getSlotMode() == JoyButtonSlot::JoyLoadProfile)
    {
        permitSlot = true;
    } else if (newslot->getSlotMode() == JoyButtonSlot::JoyMix && newslot->getMixSlots()->count() > 1)
    {
        permitSlot = true;
    } else if ((newslot->getSlotMode() == JoyButtonSlot::JoyExecute ||
                newslot->getSlotMode() == JoyButtonSlot::JoyTextEntry) &&
               !newslot->getTextData().isEmpty())
    {
        permitSlot = true;
    } else if (newslot->getSlotCode() >= 0)
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

            if (temp->getSlotMode() == JoyButtonSlot::JoySlotInputAction::JoyMix)
            {
                for (auto minislot : *temp->getMixSlots())
                {
                    delete minislot;
                    minislot = nullptr;
                }

                delete temp->getMixSlots();
                temp->assignMixSlotsToNull();
            }

            if (temp != nullptr)
            {
                delete temp;
                temp = nullptr;
            }

            getAssignmentsLocal().replace(index, newslot);
        } else if (index >= getAssignmentsLocal().count())
        {
            // Append code into a new slot
            getAssignmentsLocal().append(newslot);
        }

        assignmentsLock.unlock();
        buildActiveZoneSummaryString();
        emit slotsChanged();
    } else if (newslot != nullptr)
    {
        delete newslot;
        newslot = nullptr;
    }

    return permitSlot;
}

QList<JoyButtonSlot *> *JoyButton::getAssignedSlots()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return &assignments;
}

QList<JoyButtonSlot *> const &JoyButton::getActiveSlots()
{

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
            buildActiveZoneSummaryString();

        emit propertyUpdated();
    }
}

int JoyButton::getSetSelection()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return setSelection;
}

void JoyButton::setChangeSetCondition(SetChangeCondition condition, bool passive, bool updateActiveString)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    SetChangeCondition oldCondition = setSelectionCondition;

    if ((condition != setSelectionCondition) && !passive)
    {
        if ((condition == SetChangeWhileHeld) || (condition == SetChangeTwoWay))
        {
            // Set new condition
            emit setAssignmentChanged(m_index, setSelection, condition);
        } else if ((setSelectionCondition == SetChangeWhileHeld) || (setSelectionCondition == SetChangeTwoWay))
        {
            // Remove old condition
            emit setAssignmentChanged(m_index, setSelection, SetChangeDisabled);
        }

        setSelectionCondition = condition;
    } else if (passive)
    {
        setSelectionCondition = condition;
    }

    if (setSelectionCondition == SetChangeDisabled)
        setChangeSetSelection(-1);

    if (setSelectionCondition != oldCondition)
    {
        if (updateActiveString)
            buildActiveZoneSummaryString();

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

    return m_originset;
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
                    slotiter->findNext(previousCycle);

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
            releaseDeskTimer.stop();

        if (inpauseHold.elapsed() < currentPause->getSlotCode())
        {
            startTimerOverrun(currentPause->getSlotCode(), &inpauseHold, &pauseWaitTimer, 0);
        } else
        {
            pauseWaitTimer.stop();
            createDeskTimer.stop();
            currentPause = nullptr;
            createDeskEvent();

            // If release timer was disabled but if the button
            // is not pressed, restart the release timer.
            if (!releaseDeskTimer.isActive() && (isButtonPressedQueue.isEmpty() || !isButtonPressedQueue.last()))
                waitForReleaseDeskEvent();
        }
    } else
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
                restartAllForSetChange();
            } else if (!tempFinalState && (setSelectionCondition == SetChangeTwoWay) && (setSelection > -1))
            {
                restartAllForSetChange();
            } else if ((setSelectionCondition == SetChangeWhileHeld) && (setSelection > -1))
            {
                if (tempFinalState)
                    whileHeldStatus = true;
                else
                    whileHeldStatus = false;

                restartAllForSetChange();
            }
        }

        // Clear queue except for a press if it is last in
        if (!isButtonPressedQueue.isEmpty())
        {
            isButtonPressedQueue.clear();

            if (tempFinalState)
                isButtonPressedQueue.enqueue(tempFinalState);
        }

        // Clear queue except for a press if it is last in
        if (!ignoreSetQueue.isEmpty())
        {
            bool tempFinalIgnoreSetsState_local = ignoreSetQueue.last();
            ignoreSetQueue.clear();

            if (tempFinalState)
                ignoreSetQueue.enqueue(tempFinalIgnoreSetsState_local);
        }
    }
}

void JoyButton::restartAllForSetChange()
{
    if (createDeskTimer.isActive())
        createDeskTimer.stop();

    if (releaseDeskTimer.isActive())
        releaseDeskTimer.stop();

    isButtonPressedQueue.clear();
    ignoreSetQueue.clear();

    emit released(m_index);
    emit setChangeActivated(setSelection);
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
        } else
        {
            keyPressTimer.stop();
            releaseDeskTimer.stop();
            createDeskEvent();
        }
    } else if (!createDeskTimer.isActive())
    {
#ifdef Q_CC_MSVC
        createDeskTimer.start(5);
        releaseDeskTimer.stop();
#else
        createDeskTimer.start(0);
        releaseDeskTimer.stop();
#endif
    } else if (createDeskTimer.isActive())
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
            releaseDeskTimer.stop();

        createDeskTimer.stop();
        keyPressTimer.stop();
        releaseDeskEvent();
    } else if (!releaseDeskTimer.isActive())
    {
#ifdef Q_CC_MSVC
        releaseDeskTimer.start(1);
        createDeskTimer.stop();
#else
        releaseDeskTimer.start(1);
        createDeskTimer.stop();
#endif
    } else if (releaseDeskTimer.isActive())
    {
        createDeskTimer.stop();
    }
}

bool JoyButton::containsSequence()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = false;

    assignmentsLock.lockForRead();
    QListIterator<JoyButtonSlot *> tempiter(*getAssignedSlots());

    while (tempiter.hasNext())
    {
        JoyButtonSlot *slot = tempiter.next();
        JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();

        if ((mode == JoyButtonSlot::JoyPause) || (mode == JoyButtonSlot::JoyHold) || (mode == JoyButtonSlot::JoyDistance))
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
            currentlyPressed = isButtonPressedQueue.last();

        // Activate hold event
        if (currentlyPressed && (buttonHold.elapsed() > currentHold->getSlotCode()))
        {
            qDebug() << buttonHold.elapsed() << " > " << currentHold->getSlotCode();
            qDebug() << "Activate hold event";

            releaseActiveSlots();
            currentHold = nullptr;
            holdTimer.stop();
            buttonHold.restart();
            createDeskEvent();
        }
        // Elapsed time has not occurred
        else if (currentlyPressed)
        {
            qDebug() << "Elapsed time has not occurred, because buttonHold: " << buttonHold.elapsed()
                     << " is not greater than currentHoldCode: " << currentHold->getSlotCode();

            startTimerOverrun(currentHold->getSlotCode(), &buttonHold, &holdTimer);
        }
        // Pre-emptive release
        else
        {
            qDebug() << "Hold button is not pressed";

            currentHold = nullptr;
            holdTimer.stop();

            if (slotiter != nullptr)
            {
                qDebug() << "slotiter exists";

                findJoySlotsEnd(slotiter);
                createDeskEvent();
            }
        }
    } else
    {
        holdTimer.stop();
    }
}

void JoyButton::startTimerOverrun(int slotCode, QElapsedTimer *currSlotTime, QTimer *currSlotTimer, bool releasedDeskTimer)
{
    int proposedInterval = slotCode - currSlotTime->elapsed();
    proposedInterval = (proposedInterval > 0) ? proposedInterval : 0;
    int newTimerInterval = qMin(10, proposedInterval);
    currSlotTimer->start(newTimerInterval);

    if (releasedDeskTimer)
    {

        // If release timer is active, push next run until
        // after keyDelayTimer will timeout again. Helps
        // reduce CPU usage of an excessively repeating timer.
        if (releaseDeskTimer.isActive())
            releaseDeskTimer.start(proposedInterval);
    }
}

void JoyButton::delayEvent()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (currentDelay != nullptr)
    {
        bool currentlyPressed = false;

        if (!isButtonPressedQueue.isEmpty())
            currentlyPressed = isButtonPressedQueue.last();

        if ((currentDelay != nullptr) && (buttonDelay.elapsed() > currentDelay->getSlotCode()))
        {
            // Delay time has elapsed. Continue processing slots.
            currentDelay = nullptr;
            delayTimer.stop();
            buttonDelay.restart();
            createDeskEvent();
        } else if (currentlyPressed)
        {
            // Elapsed time has not occurred
            startTimerOverrun(currentDelay->getSlotCode(), &buttonDelay, &delayTimer);
        } else
        {
            // Pre-emptive release
            currentDelay = nullptr;
            delayTimer.stop();
        }
    } else
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
    setChangeTimer.stop();
    releaseActiveSlots();

    if (!isButtonPressedQueue.isEmpty() && (currentRelease == nullptr))
        releaseSlotEvent();
    else
        currentRelease = nullptr;

    if (!skipsetchange && (setSelectionCondition != SetChangeDisabled) && !isButtonPressedQueue.isEmpty() &&
        (currentRelease == nullptr))
    {
        bool tempButtonPressed = isButtonPressedQueue.last();
        bool tempFinalIgnoreSetsState = ignoreSetQueue.last();

        if (!tempButtonPressed && !tempFinalIgnoreSetsState)
        {
            if ((setSelectionCondition == SetChangeWhileHeld) && whileHeldStatus)
                setChangeTimer.start(0);
            else if (setSelectionCondition != SetChangeWhileHeld)
                setChangeTimer.start();
        } else
        {
            changeStatesQueue(false);
        }
    } else
    {
        changeStatesQueue(true);
    }

    if (currentRelease == nullptr)
    {
        restartAccelParams(true, true, false);
        lastMouseDistance = 0.0;
        currentMouseDistance = 0.0;
        updateStartingMouseDistance = true;

        if ((slotiter != nullptr) && !slotiter->hasNext())
        {
            // At the end of the list of assignments.

            qDebug() << "There is end of slotiter. Set currentCycle and previousCycle as null pointers";

            currentCycle = nullptr;
            previousCycle = nullptr;
            slotiter->toFront();
        } else if ((slotiter != nullptr) && slotiter->hasNext() && (currentCycle != nullptr))
        {
            // Cycle at the end of a segment.
            qDebug() << "There exists next element in slotiter and exists currentCycle. Skip to currentCycle in slotiter "
                        "starting from beginning";

            slotiter->toFront();
            slotiter->findNext(currentCycle);
        } else if ((slotiter != nullptr) && slotiter->hasPrevious() && slotiter->hasNext() && (currentCycle == nullptr))
        {
            // Check if there is a cycle action slot after
            // current slot. Useful after dealing with pause
            // actions.

            qDebug() << "There exists next element and previous element in slotiter but doesn't exists currentCycle. From "
                        "current point in slotiter find JoyButtonSlot::JoyCycle as slotMode and assign to currentCycle";

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
                qDebug() << "Didn't find any cycle. Back to start of slotiter";

                slotiter->toFront();
                previousCycle = nullptr;
            }
        }

        if (currentCycle != nullptr)
        {
            qDebug() << "currentCycle exists and previousCycle will be current but current will be null pointer";

            previousCycle = currentCycle;
            currentCycle = nullptr;
        } else if ((slotiter != nullptr) && slotiter->hasNext() && containsReleaseSlots())
        {
            qDebug() << "Slotiter has next element on the list. In assignments exists JoyButtonSlot::JoyRelease starting "
                        "from current point. CurrentCycle and previousCycle are set null pointers now";

            currentCycle = nullptr;
            previousCycle = nullptr;
            slotiter->toFront();
        }

        m_currentDistance = nullptr;
        this->currentKeyPress = nullptr;
        quitEvent = true;
    }
}

void JoyButton::changeStatesQueue(bool currentReleased)
{
    bool tempFinalState = false;

    if (!isButtonPressedQueue.isEmpty())
    {
        tempFinalState = isButtonPressedQueue.last();
        isButtonPressedQueue.clear();

        if (currentReleased)
        {

            if (tempFinalState || (currentRelease != nullptr))
                isButtonPressedQueue.enqueue(tempFinalState);
        } else
        {
            if (tempFinalState)
                isButtonPressedQueue.enqueue(tempFinalState);
        }
    }

    if (!ignoreSetQueue.isEmpty())
    {
        bool tempFinalIgnoreSetsState = ignoreSetQueue.last();
        ignoreSetQueue.clear();

        if (currentReleased)
        {

            if (tempFinalState || (currentRelease != nullptr))
                ignoreSetQueue.enqueue(tempFinalIgnoreSetsState);
        } else
        {
            if (tempFinalState)
                ignoreSetQueue.enqueue(tempFinalIgnoreSetsState);
        }
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

    return isButtonPressed ? 1.0 : 0.0;
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

    QListIterator<JoyButtonSlot *> iter(*getAssignedSlots());

    while (iter.hasNext())
    {
        JoyButtonSlot *currentSlot = iter.next();
        int tempcode = currentSlot->getSlotCode();
        JoyButtonSlot::JoySlotInputAction mode = currentSlot->getSlotMode();

        if (mode == JoyButtonSlot::JoyDistance)
        {
            tempDistance += tempcode / 100.0;

            if (slot == currentSlot)
                iter.toBack();

        } else if (mode == JoyButtonSlot::JoyCycle) // Reset tempDistance
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
    QListIterator<JoyButtonSlot *> iter(*getAssignedSlots());

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

    QListIterator<JoyButtonSlot *> iter(*getAssignedSlots());

    while (iter.hasNext())
    {
        auto *slot = iter.next();

        qDebug() << "AssignedSLot mode: " << slot->getSlotMode();
        qDebug() << "cleared assigned slot's mode: " << slot->getSlotMode();
        qDebug() << "list of mix slots is a null pointer? " << ((slot->getMixSlots() == nullptr) ? "yes" : "no");

        if (slot != nullptr)
        {
            if (slot->getMixSlots() != nullptr && slot->getMixSlots() != NULL)
            {
                slot->cleanMixSlots();
            }

            delete slot;
            slot = nullptr;
        }
    }

    getAssignmentsLocal().clear();
    if (signalEmit)
        emit slotsChanged();
}

void JoyButton::removeAssignedSlot(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QWriteLocker tempAssignLocker(&assignmentsLock);

    int j = 0;
    qDebug() << "Assigned list slots after joining";
    for (auto el : *getAssignedSlots())
    {
        qDebug() << j << ")";
        qDebug() << "code: " << el->getSlotCode();
        qDebug() << "mode: " << el->getSlotMode();
        qDebug() << "string: " << el->getSlotString();
        j++;
    }

    if ((index >= 0) && (index < getAssignedSlots()->size()))
    {
        JoyButtonSlot *slot = getAssignedSlots()->takeAt(index);

        if (slot->getSlotMode() == JoyButtonSlot::JoyMix)
        {
            for (auto minislot : *slot->getMixSlots())
            {
                delete minislot;
                minislot = nullptr;
            }

            slot->getMixSlots()->clear();
            delete slot->getMixSlots();
            slot->assignMixSlotsToNull();

            getAssignedSlots()->removeAt(index);
        } else
        {
            delete slot;
            slot = nullptr;
        }

        tempAssignLocker.unlock();
        buildActiveZoneSummaryString();
        emit slotsChanged();
    }
}

void JoyButton::clearSlotsEventReset(bool clearSignalEmit)
{
    qWarning() << "RECEIVED";
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QWriteLocker tempAssignLocker(&assignmentsLock);

    resetSlotsProp();
    stopTimers(true, true, false);
    releaseActiveSlots();
    clearAssignedSlots(clearSignalEmit);
    clearQueues();

    qWarning() << "all current slots and previous slots ale cleared";
}

void JoyButton::eventReset()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QWriteLocker tempAssignLocker(&assignmentsLock);

    resetSlotsProp();
    stopTimers(false, true, false);
    clearQueues();

    qDebug() << "all current slots and previous slots ale cleared";

    releaseActiveSlots();
}

void JoyButton::releaseActiveSlots()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (!getActiveSlots().isEmpty())
    {
        QWriteLocker tempLocker(&activeZoneLock);

        bool changeRepeatState = false;

        QListIterator<JoyButtonSlot *> iter(getActiveSlots());
        iter.toBack();

        while (iter.hasPrevious())
        {
            JoyButtonSlot *slot = iter.previous();
            int tempcode = slot->getSlotCode();
            int references = 0;
            JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();

            if (mode == JoyButtonSlot::JoySlotInputAction::JoyMix)
            {
                QListIterator<JoyButtonSlot *> iterMini(*slot->getMixSlots());
                iterMini.toBack();

                // go through all slots in JoyMix slot
                while (iterMini.hasPrevious())
                {
                    JoyButtonSlot *slotMini = iterMini.previous();
                    int tempcodeMini = slotMini->getSlotCode();
                    int referencesMini = 0;
                    JoyButtonSlot::JoySlotInputAction mode = slotMini->getSlotMode();

                    releaseEachSlot(changeRepeatState, referencesMini, tempcodeMini, mode, slotMini);
                }

                if (!slot->getMixSlots()->isEmpty())
                {
                    qDeleteAll(*slot->getMixSlots());
                    slot->getMixSlots()->clear();
                    delete slot->getMixSlots();
                    slot->assignMixSlotsToNull();
                }
            } else
            {
                releaseEachSlot(changeRepeatState, references, tempcode, mode, slot);
            }
        }

        getActiveSlotsLocal().clear();
        currentMouseEvent = nullptr;

        if (!mouseEventQueue.isEmpty())
            mouseEventQueue.clear();

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
        if ((pendingMouseButtons.length() == 0) && (cursorXSpeeds.length() == 0) && (springXSpeeds.length() == 0))
        {
            GlobalVariables::JoyButton::cursorRemainderX = 0;
            GlobalVariables::JoyButton::cursorRemainderY = 0;
        }

        activeZoneTimer.start();
    }
}

void JoyButton::releaseEachSlot(bool &changeRepeatState, int &references, int tempcode,
                                JoyButtonSlot::JoySlotInputAction mode, JoyButtonSlot *slot)
{
    if (mode == JoyButtonSlot::JoyKeyboard)
    {
        countActiveSlots(tempcode, references, slot, GlobalVariables::JoyButton::activeKeys, changeRepeatState, true);

        if ((lastActiveKey == slot) && (references <= 0))
            lastActiveKey = nullptr;
    } else if (mode == JoyButtonSlot::JoyMouseButton)
    {
        if ((tempcode != static_cast<int>(JoyButtonSlot::MouseWheelUp)) &&
            (tempcode != static_cast<int>(JoyButtonSlot::MouseWheelDown)) &&
            (tempcode != static_cast<int>(JoyButtonSlot::MouseWheelLeft)) &&
            (tempcode != static_cast<int>(JoyButtonSlot::MouseWheelRight)))
        {
            countActiveSlots(tempcode, references, slot, GlobalVariables::JoyButton::activeMouseButtons, changeRepeatState);
        } else if ((tempcode == static_cast<int>(JoyButtonSlot::MouseWheelUp)) ||
                   (tempcode == static_cast<int>(JoyButtonSlot::MouseWheelDown)))
        {
            mouseWheelVerticalEventQueue.removeAll(slot);
        } else if ((tempcode == static_cast<int>(JoyButtonSlot::MouseWheelLeft)) ||
                   (tempcode == static_cast<int>(JoyButtonSlot::MouseWheelRight)))
        {
            mouseWheelHorizontalEventQueue.removeAll(slot);
        }

        slot->setDistance(0.0);
        slot->getMouseInterval()->restart();
    } else if (mode == JoyButtonSlot::JoyMouseMovement)
    {
        JoyMouseMovementMode mousemode = getMouseMode();

        if (mousemode == MouseCursor)
        {
            QList<int> indexesToRemove;

            releaseMoveSlots(cursorXSpeeds, slot, indexesToRemove);
            releaseMoveSlots(cursorYSpeeds, slot, indexesToRemove);
            slot->getEasingTime()->restart();
            slot->setEasingStatus(false);
        } else if (mousemode == JoyButton::MouseSpring)
        {
            double mouse1 = (tempcode == static_cast<int>(JoyButtonSlot::MouseLeft) ||
                             tempcode == static_cast<int>(JoyButtonSlot::MouseRight))
                                ? 0.0
                                : -2.0;
            double mouse2 = (tempcode == static_cast<int>(JoyButtonSlot::MouseUp) ||
                             tempcode == static_cast<int>(JoyButtonSlot::MouseDown))
                                ? 0.0
                                : -2.0;

            double springDeadCircleX = 0.0;
            double springDeadCircleY = 0.0;

            checkSpringDeadCircle(tempcode, springDeadCircleX, JoyButtonSlot::MouseLeft, JoyButtonSlot::MouseRight);
            checkSpringDeadCircle(tempcode, springDeadCircleY, JoyButtonSlot::MouseUp, JoyButtonSlot::MouseDown);
            updateMouseProperties(mouse1, springDeadCircleX, springWidth, springHeight, relativeSpring,
                                  GlobalVariables::JoyButton::springModeScreen, springXSpeeds, 'n', -2.0, springDeadCircleY);
            updateMouseProperties(-2.0, springDeadCircleX, springWidth, springHeight, relativeSpring,
                                  GlobalVariables::JoyButton::springModeScreen, springYSpeeds, 'n', mouse2,
                                  springDeadCircleY);
        }

        mouseEventQueue.removeAll(slot);
        slot->setDistance(0.0);
        slot->getMouseInterval()->restart();
    } else if (mode == JoyButtonSlot::JoyMouseSpeedMod)
    {
        int queueLength = mouseSpeedModList.length();

        if (!mouseSpeedModList.isEmpty())
        {
            mouseSpeedModList.removeAll(slot);
            queueLength -= 1;
        }

        if (queueLength <= 0)
            GlobalVariables::JoyButton::mouseSpeedModifier = GlobalVariables::JoyButton::DEFAULTMOUSESPEEDMOD;
    } else if (mode == JoyButtonSlot::JoySetChange)
    {
        currentSetChangeSlot = slot;
        slotSetChangeTimer.start();
    }
}

void JoyButton::countActiveSlots(int tempcode, int &references, JoyButtonSlot *slot, QHash<int, int> &activeSlotsHash,
                                 bool &changeRepeatState, bool activeSlotHashWindows)
{
    changeRepeatState = false;
    references = activeSlotsHash.value(tempcode, 1) - 1;

    if (references <= 0)
    {
        sendevent(slot, false);
        activeSlotsHash.remove(tempcode);
    } else
    {
        activeSlotsHash.insert(tempcode, references);
    }
}

void JoyButton::setSpringDeadCircle(double &springDeadCircle, int mouseDirection)
{
    if (getCurrentSpringDeadCircle() > getLastMouseDistanceFromDeadZone())
        springDeadCircle = (mouseDirection == JoyButtonSlot::MouseLeft || mouseDirection == JoyButtonSlot::MouseUp)
                               ? -getLastMouseDistanceFromDeadZone()
                               : getLastMouseDistanceFromDeadZone();
    else
        springDeadCircle = (mouseDirection == JoyButtonSlot::MouseLeft || mouseDirection == JoyButtonSlot::MouseUp)
                               ? -getCurrentSpringDeadCircle()
                               : getCurrentSpringDeadCircle();
}

void JoyButton::checkSpringDeadCircle(int tempcode, double &springDeadCircle, int mouseSlot1, int mouseSlot2)
{
    if (getSpringDeadCircleMultiplier() > 0)
    {
        if (tempcode == mouseSlot1)
        {
            setSpringDeadCircle(springDeadCircle, mouseSlot1);
        } else if (tempcode == mouseSlot2)
        {
            setSpringDeadCircle(springDeadCircle, mouseSlot2);
        }
    }
}

void JoyButton::releaseMoveSlots(QList<JoyButton::mouseCursorInfo> &cursorSpeeds, JoyButtonSlot *slot,
                                 QList<int> &indexesToRemove)
{
    QListIterator<mouseCursorInfo> iter(cursorSpeeds);
    int i = cursorSpeeds.length();

    while (iter.hasNext())
    {
        mouseCursorInfo info = iter.next();
        if (info.slot == slot)
            indexesToRemove.append(i);
        i++;
    }

    QListIterator<int> removeIter(indexesToRemove);
    while (removeIter.hasPrevious())
    {
        int index = removeIter.previous();
        cursorSpeeds.removeAt(index);
    }

    indexesToRemove.clear();
}

bool JoyButton::containsReleaseSlots()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = false;
    QListIterator<JoyButtonSlot *> iter(*getAssignedSlots());

    while (iter.hasNext())
    {
        if (iter.next()->getSlotMode() == JoyButtonSlot::JoyRelease)
        {
            result = true;
            iter.toBack();
        }
    }

    return result;
}

bool JoyButton::containsJoyMixSlot()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = false;
    QListIterator<JoyButtonSlot *> iter(*getAssignedSlots());

    while (iter.hasNext())
    {
        if (iter.next()->getSlotMode() == JoyButtonSlot::JoyMix)
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
        QListIterator<JoyButtonSlot *> iter(*getAssignedSlots());

        if (previousCycle != nullptr)
            iter.findNext(previousCycle);

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
                    temp = currentSlot;
                else
                    iter.toBack();
            } else if (mode == JoyButtonSlot::JoyCycle)
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

            currentHold = nullptr;
        }
    }
}

void JoyButton::findJoySlotsEnd(QListIterator<JoyButtonSlot *> *slotiter)
{
    if (slotiter != nullptr)
    {

        bool found = false;
        while (!found && slotiter->hasNext())
        {
            qDebug() << "slotiter has next element";

            JoyButtonSlot::JoySlotInputAction mode = slotiter->next()->getSlotMode();

            switch (mode)
            {
            case JoyButtonSlot::JoyRelease:
            case JoyButtonSlot::JoyCycle:
            case JoyButtonSlot::JoyHold:

                found = true;
                break;

            default:
                break;
            }
        }

        if (found && slotiter->hasPrevious())
            slotiter->previous();
    }
}

void JoyButton::setVDPad(VDPad *vdpad)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    joyEvent(false, true);
    m_vdpad = vdpad;
    emit propertyUpdated();
}

bool JoyButton::isPartVDPad()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return (m_vdpad != nullptr);
}

VDPad *JoyButton::getVDPad()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_vdpad;
}

void JoyButton::removeVDPad()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    m_vdpad = nullptr;
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

    value = value && (m_toggle == GlobalVariables::JoyButton::DEFAULTTOGGLE);
    value = value && (turboInterval == GlobalVariables::JoyButton::DEFAULTTURBOINTERVAL);
    value = value && (currentTurboMode == NormalTurbo);
    value = value && (m_useTurbo == GlobalVariables::JoyButton::DEFAULTUSETURBO);
    value = value && (mouseSpeedX == GlobalVariables::JoyButton::DEFAULTMOUSESPEEDX);
    value = value && (mouseSpeedY == GlobalVariables::JoyButton::DEFAULTMOUSESPEEDY);
    value = value && (setSelection == GlobalVariables::JoyButton::DEFAULTSETSELECTION);
    value = value && (setSelectionCondition == DEFAULTSETCONDITION);
    value = value && (getAssignedSlots()->isEmpty());
    value = value && (mouseMode == DEFAULTMOUSEMODE);
    value = value && (mouseCurve == DEFAULTMOUSECURVE);
    value = value && (springWidth == GlobalVariables::JoyButton::DEFAULTSPRINGWIDTH);
    value = value && (springHeight == GlobalVariables::JoyButton::DEFAULTSPRINGHEIGHT);
    value = value && qFuzzyCompare(sensitivity, GlobalVariables::JoyButton::DEFAULTSENSITIVITY);
    value = value && (actionName.isEmpty());
    value = value && (wheelSpeedX == GlobalVariables::JoyButton::DEFAULTWHEELX);
    value = value && (wheelSpeedY == GlobalVariables::JoyButton::DEFAULTWHEELY);
    value = value && (cycleResetActive == GlobalVariables::JoyButton::DEFAULTCYCLERESETACTIVE);
    value = value && (cycleResetInterval == GlobalVariables::JoyButton::DEFAULTCYCLERESET);
    value = value && (relativeSpring == GlobalVariables::JoyButton::DEFAULTRELATIVESPRING);
    value = value && qFuzzyCompare(m_easingDuration, GlobalVariables::JoyButton::DEFAULTEASINGDURATION);
    value = value && !extraAccelerationEnabled;
    value = value && qFuzzyCompare(extraAccelerationMultiplier, GlobalVariables::JoyButton::DEFAULTEXTRACCELVALUE);
    value = value && qFuzzyCompare(minMouseDistanceAccelThreshold, GlobalVariables::JoyButton::DEFAULTMINACCELTHRESHOLD);
    value = value && qFuzzyCompare(maxMouseDistanceAccelThreshold, GlobalVariables::JoyButton::DEFAULTMAXACCELTHRESHOLD);
    value = value && qFuzzyCompare(startAccelMultiplier, GlobalVariables::JoyButton::DEFAULTSTARTACCELMULTIPLIER);
    value = value && qFuzzyCompare(accelDuration, GlobalVariables::JoyButton::DEFAULTACCELEASINGDURATION);
    value = value && (springDeadCircleMultiplier == GlobalVariables::JoyButton::DEFAULTSPRINGRELEASERADIUS);
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

    qDebug() << "Action name is: " << actionName;

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

void JoyButton::setWheelSpeed(int speed, QChar ax)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((speed >= 1) && (speed <= 100))
    {
        if (ax == 'X')
            wheelSpeedX = speed;
        else if (ax == 'Y')
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
void JoyButton::moveMouseCursor(int &movedX, int &movedY, int &movedElapsed, QList<double> *mouseHistoryX,
                                QList<double> *mouseHistoryY, QTime *testOldMouseTime, QTimer *staticMouseEventTimer,
                                int mouseRefreshRate, int mouseHistorySize, QList<JoyButton::mouseCursorInfo> *cursorXSpeeds,
                                QList<JoyButton::mouseCursorInfo> *cursorYSpeeds, double &cursorRemainderX,
                                double &cursorRemainderY, double weightModifier, int idleMouseRefrRate,
                                QList<JoyButton *> *pendingMouseButtons)
{
    // qInstallMessageHandler(MessageHandler::myMessageOutput);

    movedX = 0;
    movedY = 0;
    int elapsedTime = testOldMouseTime->elapsed();
    movedElapsed = elapsedTime;

    if (staticMouseEventTimer->interval() < mouseRefreshRate)
        movedElapsed = mouseRefreshRate + (elapsedTime - staticMouseEventTimer->interval());

    if (mouseHistoryX->size() >= mouseHistorySize)
        mouseHistoryX->removeLast();

    if (mouseHistoryY->size() >= mouseHistorySize)
        mouseHistoryY->removeLast();

    /*
     * Combine all mouse events to find the distance to move the mouse
     * along the X and Y axis. If necessary, perform mouse smoothing.
     * The mouse smoothing technique used is an interpretation of the method
     * outlined at http://flipcode.net/archives/Smooth_Mouse_Filtering.shtml.
     */
    if ((cursorXSpeeds->length() == cursorYSpeeds->length()) && (cursorXSpeeds->length() > 0))
    {
        int queueLength = cursorXSpeeds->length();
        double finalx = 0.0;
        double finaly = 0.0;

        for (int i = 0; i < queueLength; i++)
        {
            mouseCursorInfo infoX = cursorXSpeeds->takeFirst();
            mouseCursorInfo infoY = cursorYSpeeds->takeFirst();

            distanceForMovingAx(finalx, infoX);
            distanceForMovingAx(finaly, infoY);

            infoX.slot->getMouseInterval()->restart();
            infoY.slot->getMouseInterval()->restart();
        }

        // Only apply remainder if both current displacement and remainder
        // follow the same direction.
        if ((cursorRemainderX >= 0) == (finalx >= 0))
            finalx += cursorRemainderX;

        // Cap maximum relative mouse movement.
        if (abs(finalx) > 127)
            finalx = (finalx < 0) ? -127 : 127;

        mouseHistoryX->prepend(finalx);

        // Only apply remainder if both current displacement and remainder
        // follow the same direction.
        if ((cursorRemainderY >= 0) == (finaly >= 0))
            finaly += cursorRemainderY;

        // Cap maximum relative mouse movement.
        if (abs(finaly) > 127)
            finaly = (finaly < 0) ? -127 : 127;

        mouseHistoryY->prepend(finaly);

        cursorRemainderX = 0;
        cursorRemainderY = 0;
        double adjustedX = 0;
        double adjustedY = 0;

        adjustAxForCursor(mouseHistoryX, adjustedX, cursorRemainderX, weightModifier);
        adjustAxForCursor(mouseHistoryY, adjustedY, cursorRemainderY, weightModifier);

        // This check is more of a precaution than anything. No need to cause
        // a sync to happen when not needed.
        if (!qFuzzyIsNull(adjustedX) || !qFuzzyIsNull(adjustedY))
            sendevent(adjustedX, adjustedY);

        movedX = adjustedX;
        movedY = adjustedY;
    } else
    {
        mouseHistoryX->prepend(0);
        mouseHistoryY->prepend(0);
    }

    // Check if mouse event timer should use idle time.
    if (pendingMouseButtons->length() == 0)
    {
        if (staticMouseEventTimer->interval() != idleMouseRefrRate)
        {
            staticMouseEventTimer->start(idleMouseRefrRate);

            // Clear current mouse history
            mouseHistoryX->clear();
            mouseHistoryY->clear();

            // Fill history with zeroes.
            for (int i = 0; i < mouseHistorySize; i++)
            {
                mouseHistoryX->append(0);
                mouseHistoryY->append(0);
            }
        }

        cursorRemainderX = 0;
        cursorRemainderY = 0;
    } else
    {
        if (staticMouseEventTimer->interval() != mouseRefreshRate)
            staticMouseEventTimer->start(mouseRefreshRate); // Restore intended QTimer interval.
    }

    cursorXSpeeds->clear();
    cursorYSpeeds->clear();
}

void JoyButton::distanceForMovingAx(double &finalAx, mouseCursorInfo infoAx)
{
    if (!qFuzzyIsNull(infoAx.code))
    {
        finalAx = (infoAx.code < 0) ? qMin(infoAx.code, finalAx) : qMax(infoAx.code, finalAx);
    }
}

void JoyButton::adjustAxForCursor(QList<double> *mouseHistoryList, double &adjustedAx, double &cursorRemainder,
                                  double weightModifier)
{
    double currentWeight = 1.0;
    double finalWeight = 0.0;

    QListIterator<double> mouseHist(*mouseHistoryList);
    while (mouseHist.hasNext())
    {
        adjustedAx += mouseHist.next() * currentWeight;
        finalWeight += currentWeight;
        currentWeight *= weightModifier;
    }

    if (fabs(adjustedAx) > 0)
    {
        adjustedAx = adjustedAx / finalWeight;
        double oldAx = adjustedAx;

        if (adjustedAx > 0)
            adjustedAx = floor(adjustedAx);
        else
            adjustedAx = ceil(adjustedAx);

        cursorRemainder = oldAx - adjustedAx;
    }
}

/**
 * @brief Take spring mouse information provided by all buttons and
 *     send a spring mode mouse event to the display server.
 */
void JoyButton::moveSpringMouse(int &movedX, int &movedY, bool &hasMoved, int springModeScreen,
                                QList<PadderCommon::springModeInfo> *springXSpeeds,
                                QList<PadderCommon::springModeInfo> *springYSpeeds, QList<JoyButton *> *pendingMouseButtons,
                                int mouseRefreshRate, int idleMouseRefrRate, QTimer *staticMouseEventTimer)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    PadderCommon::springModeInfo fullSpring = {-2.0, -2.0, 0, 0, false, springModeScreen, 0.0, 0.0};

    PadderCommon::springModeInfo relativeSpring = {-2.0, -2.0, 0, 0, false, springModeScreen, 0.0, 0.0};

    int realMouseX = movedX = 0;
    int realMouseY = movedY = 0;
    hasMoved = false;

    if ((springXSpeeds->length() == springYSpeeds->length()) && (springXSpeeds->length() > 0))
    {
        bool complete = false;

        for (int i = (springXSpeeds->length() - 1); (i >= 0) && !complete; i--)
        {

            double tempx = -2.0;
            double tempy = -2.0;
            double tempSpringDeadX = 0.0;
            double tempSpringDeadY = 0.0;

            PadderCommon::springModeInfo infoX;
            PadderCommon::springModeInfo infoY;

            infoX = springXSpeeds->takeLast();
            infoY = springYSpeeds->takeLast();

            tempx = infoX.displacementX;
            tempy = infoY.displacementY;
            tempSpringDeadX = infoX.springDeadX;
            tempSpringDeadY = infoY.springDeadY;

            if (infoX.relative)
            {
                if (qFuzzyCompare(relativeSpring.displacementX, -2.0))
                    relativeSpring.displacementX = tempx;

                relativeSpring.relative = true;

                // Use largest found width for spring
                // mode dimensions.
                relativeSpring.width = qMax(infoX.width, relativeSpring.width);
            } else
            {
                if (qFuzzyCompare(fullSpring.displacementX, -2.0))
                    fullSpring.displacementX = tempx;

                if (fullSpring.springDeadX == 0.0)
                    fullSpring.springDeadX = tempSpringDeadX;

                // Use largest found width for spring
                // mode dimensions.
                fullSpring.width = qMax(infoX.width, fullSpring.width);
            }

            if (infoY.relative)
            {
                if (qFuzzyCompare(relativeSpring.displacementY, -2.0))
                    relativeSpring.displacementY = tempy;

                relativeSpring.relative = true;

                // Use largest found height for spring
                // mode dimensions.
                relativeSpring.height = qMax(infoX.height, relativeSpring.height);
            } else
            {
                if (qFuzzyCompare(fullSpring.displacementY, -2.0))
                    fullSpring.displacementY = tempy;

                if (fullSpring.springDeadY == 0.0)
                    fullSpring.springDeadY = tempSpringDeadY;

                // Use largest found height for spring
                // mode dimensions.
                fullSpring.height = qMax(infoX.height, fullSpring.height);
            }

            if ((!qFuzzyCompare(relativeSpring.displacementX, -2.0) && !qFuzzyCompare(relativeSpring.displacementY, -2.0)) &&
                (!qFuzzyCompare(fullSpring.displacementX, -2.0) && !qFuzzyCompare(fullSpring.displacementY, -2.0)))
            {
                complete = true;
            } else if (((relativeSpring.springDeadX != 0.0) && (relativeSpring.springDeadY != 0.0)) &&
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
        } else
        {
            if (!hasFutureSpringEvents(JoyButton::getPendingMouseButtons()))
            {
                if (fullSpring.springDeadX != 0.0)
                    fullSpring.displacementX = fullSpring.springDeadX;

                if (fullSpring.springDeadY != 0.0)
                    fullSpring.displacementY = fullSpring.springDeadY;

                sendSpringEvent(&fullSpring, nullptr, &realMouseX, &realMouseY);
            } else
            {
                sendSpringEvent(&fullSpring, nullptr, &realMouseX, &realMouseY);
            }
        }

        movedX = realMouseX;
        movedY = realMouseY;
        hasMoved = true;
    }

    // Check if mouse event timer should use idle time.
    if (pendingMouseButtons->length() == 0)
    {
        staticMouseEventTimer->start(idleMouseRefrRate);
    } else
    {
        if (staticMouseEventTimer->interval() != mouseRefreshRate)
            // Restore intended QTimer interval.
            staticMouseEventTimer->start(mouseRefreshRate);
    }

    springXSpeeds->clear();
    springYSpeeds->clear();
}

void JoyButton::keyPressEvent()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
        } else
        {
            createDeskEvent();
        }
    } else
    {
        createDeskTimer.stop();
        startTimerOverrun(getPreferredKeyPressTime(), &keyPressHold, &keyPressTimer, true);
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
    QListIterator<JoyButtonSlot *> tempiter(*getAssignedSlots());

    // Move iterator to start of cycle.
    if (previousCycle != nullptr)
        tempiter.findNext(previousCycle);

    while (tempiter.hasNext())
    {
        JoyButtonSlot *slot = tempiter.next();

        if ((slot->getSlotMode() == JoyButtonSlot::JoyPause) || (slot->getSlotMode() == JoyButtonSlot::JoyRelease))
        {
            result = true;
            tempiter.toBack();
        } else if (slot->getSlotMode() == JoyButtonSlot::JoyCycle)
        {
            result = false;
            tempiter.toBack();
        }
    }

    return result;
}

SetJoystick *JoyButton::getParentSet()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_parentSet;
}

void JoyButton::checkForPressedSetChange()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (!isButtonPressedQueue.isEmpty() && !whileHeldStatus)
    {
        bool tempButtonPressed = isButtonPressedQueue.last();
        bool tempFinalIgnoreSetsState = ignoreSetQueue.last();

        if (tempButtonPressed && !tempFinalIgnoreSetsState && (setSelectionCondition == SetChangeWhileHeld) &&
            (currentRelease == nullptr))
        {
            setChangeTimer.start(0);
            quitEvent = true;
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

    if ((currentKeyPress != nullptr) && (currentKeyPress->getSlotCode() > 0))
    {
        return currentKeyPress->getSlotCode();
    } else if ((m_parentSet != nullptr) && (m_parentSet->getInputDevice()->getDeviceKeyPressTime() > 0))
    {
        return m_parentSet->getInputDevice()->getDeviceKeyPressTime();
    }

    return GlobalVariables::InputDevice::NUMBER_JOYSETS;
}

void JoyButton::setCycleResetTime(int interval)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (interval >= GlobalVariables::JoyButton::MINCYCLERESETTIME)
        cycleResetInterval =
            qBound(GlobalVariables::JoyButton::MINCYCLERESETTIME, interval, GlobalVariables::JoyButton::MAXCYCLERESETTIME);
    else
        cycleResetActive = false;

    emit propertyUpdated();
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

    connect(this, &JoyButton::slotsChanged, m_parentSet->getInputDevice(), &InputDevice::profileEdited);
    connect(this, &JoyButton::propertyUpdated, m_parentSet->getInputDevice(), &InputDevice::profileEdited);
}

void JoyButton::disconnectPropertyUpdatedConnections()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    disconnect(this, &JoyButton::slotsChanged, nullptr, nullptr);
    disconnect(this, &JoyButton::propertyUpdated, m_parentSet->getInputDevice(), &InputDevice::profileEdited);
}

/**
 * @brief Change initial settings used for mouse event timer being used by
 *     the application.
 */
void JoyButton::establishMouseTimerConnections()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (staticMouseEventTimer.timerType() != Qt::PreciseTimer)
        staticMouseEventTimer.setTimerType(Qt::PreciseTimer);

    // Only one connection will be made for each.
    connect(&staticMouseEventTimer, &QTimer::timeout, &mouseHelper, &JoyButtonMouseHelper::mouseEvent, Qt::UniqueConnection);

    if (staticMouseEventTimer.interval() != GlobalVariables::JoyButton::IDLEMOUSEREFRESHRATE)
        staticMouseEventTimer.setInterval(GlobalVariables::JoyButton::IDLEMOUSEREFRESHRATE);
}

void JoyButton::setSpringRelativeStatus(bool value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (value != relativeSpring)
    {
        if (value)
            setSpringDeadCircleMultiplier(0);

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
    QListIterator<JoyButtonSlot *> iter(*getAssignedSlots());

    while (iter.hasNext())
    {
        JoyButtonSlot *slot = iter.next();
        JoyButtonSlot *newslot = new JoyButtonSlot(slot, destButton);
        destButton->insertAssignedSlot(newslot, false);
    }

    assignmentsLock.unlock();

    destButton->m_toggle = m_toggle;
    destButton->turboInterval = turboInterval;
    destButton->m_useTurbo = m_useTurbo;
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
    destButton->m_easingDuration = m_easingDuration;
    destButton->extraAccelerationEnabled = extraAccelerationEnabled;
    destButton->extraAccelerationMultiplier = extraAccelerationMultiplier;
    destButton->minMouseDistanceAccelThreshold = minMouseDistanceAccelThreshold;
    destButton->maxMouseDistanceAccelThreshold = maxMouseDistanceAccelThreshold;
    destButton->startAccelMultiplier = startAccelMultiplier;
    destButton->springDeadCircleMultiplier = springDeadCircleMultiplier;
    destButton->extraAccelCurve = extraAccelCurve;

    destButton->buildActiveZoneSummaryString();

    if (!destButton->isDefault())
        emit propertyUpdated();
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
int JoyButton::calculateFinalMouseSpeed(JoyMouseCurve curve, int value, const float joyspeed)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int result = joyspeed * value;

    switch (curve)
    {
    case QuadraticExtremeCurve:
    case EasingQuadraticCurve:
    case EasingCubicCurve: {
        result *= 1.5;
        break;
    }
    default: {
        break;
    }
    }

    return result;
}

void JoyButton::setEasingDuration(double value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((value >= GlobalVariables::JoyButton::MINIMUMEASINGDURATION) &&
        (value <= GlobalVariables::JoyButton::MAXIMUMEASINGDURATION) && (!qFuzzyCompare(value, m_easingDuration)))
    {
        m_easingDuration = value;
        emit propertyUpdated();
    }
}

double JoyButton::getEasingDuration()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_easingDuration;
}

JoyButtonMouseHelper *JoyButton::getMouseHelper()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return &mouseHelper;
}

/**
 * @brief Get the list of buttons that have a pending mouse movement event.
 * @return QList<JoyButton*>*
 */
QList<JoyButton *> *JoyButton::getPendingMouseButtons()
{
    // qInstallMessageHandler(MessageHandler::myMessageOutput);

    return &pendingMouseButtons;
}

QList<JoyButton::mouseCursorInfo> *JoyButton::getCursorXSpeeds() { return &cursorXSpeeds; }

QList<JoyButton::mouseCursorInfo> *JoyButton::getCursorYSpeeds() { return &cursorYSpeeds; }

QList<PadderCommon::springModeInfo> *JoyButton::getSpringXSpeeds() { return &springXSpeeds; }

QList<PadderCommon::springModeInfo> *JoyButton::getSpringYSpeeds() { return &springYSpeeds; }

QTimer *JoyButton::getStaticMouseEventTimer() { return &staticMouseEventTimer; }

QTime *JoyButton::getTestOldMouseTime() { return &testOldMouseTime; }

bool JoyButton::hasCursorEvents(QList<JoyButton::mouseCursorInfo> *cursorXSpeedsList,
                                QList<JoyButton::mouseCursorInfo> *cursorYSpeedsList)
{
    //  qInstallMessageHandler(MessageHandler::myMessageOutput);

    return (cursorXSpeedsList->length() != 0) || (cursorYSpeedsList->length() != 0);
}

bool JoyButton::hasSpringEvents(QList<PadderCommon::springModeInfo> *springXSpeedsList,
                                QList<PadderCommon::springModeInfo> *springYSpeedsList)
{
    // qInstallMessageHandler(MessageHandler::myMessageOutput);

    return (springXSpeedsList->length() != 0) || (springYSpeedsList->length() != 0);
}

/**
 * @brief Set the weight modifier to use for mouse smoothing.
 * @param Weight modifier in the range of 0.0 - 1.0.
 */
void JoyButton::setWeightModifier(double modifier, double maxWeightModifier, double &weightModifier)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((modifier >= 0.0) && (modifier <= maxWeightModifier))
        weightModifier = modifier;
}

/**
 * @brief Set mouse history buffer size used for mouse smoothing.
 * @param Mouse history buffer size
 */
void JoyButton::setMouseHistorySize(int size, int maxMouseHistSize, int &mouseHistSize, QList<double> *mouseHistoryX,
                                    QList<double> *mouseHistoryY)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((size >= 1) && (size <= maxMouseHistSize))
    {
        mouseHistoryX->clear();
        mouseHistoryY->clear();
        mouseHistSize = size;
    }
}

/**
 * @brief Set the mouse refresh rate when a mouse slot is active.
 * @param Refresh rate in ms.
 */
void JoyButton::setMouseRefreshRate(int refresh, int &mouseRefreshRate, int idleMouseRefrRate,
                                    JoyButtonMouseHelper *mouseHelper, QList<double> *mouseHistoryX,
                                    QList<double> *mouseHistoryY, QTime *testOldMouseTime, QTimer *staticMouseEventTimer)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((refresh >= 1) && (refresh <= 16))
    {
        mouseRefreshRate = refresh;

        if (staticMouseEventTimer->isActive())
        {
            testOldMouseTime->restart();

            if (staticMouseEventTimer->interval() != idleMouseRefrRate && staticMouseEventTimer->interval() != 0)
            {
                QMetaObject::invokeMethod(staticMouseEventTimer, "start", Q_ARG(int, mouseRefreshRate));
            } else
            {
                // Restart QTimer to keep QTimer in line with QTime
                QMetaObject::invokeMethod(staticMouseEventTimer, "start", Q_ARG(int, idleMouseRefrRate));
            }

            // Clear current mouse history
            mouseHistoryX->clear();
            mouseHistoryY->clear();
        } else
        {
            staticMouseEventTimer->setInterval(idleMouseRefrRate);
        }

        mouseHelper->carryMouseRefreshRateUpdate(mouseRefreshRate);
    }
}

/**
 * @brief Set the gamepad poll rate to be used in the application.
 * @param Poll rate in ms.
 */

void JoyButton::setGamepadRefreshRate(int refresh, int &gamepadRefreshRate, JoyButtonMouseHelper *mouseHelper)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((refresh >= 1) && (refresh <= 16))
    {
        gamepadRefreshRate = refresh;
        mouseHelper->carryGamePollRateUpdate(gamepadRefreshRate);
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
    case JoyButtonSlot::JoySetChange: {
        setUseTurbo(false);
        break;
    }
    default: {
        break;
    }
    }
}

void JoyButton::resetProperties()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    qDebug() << "all current slots and previous slots ale cleared";

    resetAllProperties();
}

void JoyButton::resetAllProperties()
{
    resetSlotsProp(true);

    actionName.clear();
    m_toggle = GlobalVariables::JoyButton::DEFAULTTOGGLE;
    turboInterval = GlobalVariables::JoyButton::DEFAULTTURBOINTERVAL;
    currentTurboMode = NormalTurbo;
    m_useTurbo = GlobalVariables::JoyButton::DEFAULTUSETURBO;
    isDown = false;
    toggleActiveState = false;
    m_useTurbo = false;
    mouseSpeedX = GlobalVariables::JoyButton::DEFAULTMOUSESPEEDX;
    mouseSpeedY = GlobalVariables::JoyButton::DEFAULTMOUSESPEEDY;
    wheelSpeedX = GlobalVariables::JoyButton::DEFAULTWHEELX;
    wheelSpeedY = GlobalVariables::JoyButton::DEFAULTWHEELY;
    mouseMode = DEFAULTMOUSEMODE;
    mouseCurve = DEFAULTMOUSECURVE;
    springWidth = GlobalVariables::JoyButton::DEFAULTSPRINGWIDTH;
    springHeight = GlobalVariables::JoyButton::DEFAULTSPRINGHEIGHT;
    sensitivity = GlobalVariables::JoyButton::DEFAULTSENSITIVITY;
    setSelection = GlobalVariables::JoyButton::DEFAULTSETSELECTION;
    setSelectionCondition = DEFAULTSETCONDITION;
    m_ignoresets = false;
    ignoreEvents = false;
    whileHeldStatus = false;
    buttonName.clear();
    actionName.clear();
    cycleResetActive = GlobalVariables::JoyButton::DEFAULTCYCLERESETACTIVE;
    cycleResetInterval = GlobalVariables::JoyButton::DEFAULTCYCLERESET;
    relativeSpring = GlobalVariables::JoyButton::DEFAULTRELATIVESPRING;
    lastDistance = 0.0;
    lastMouseDistance = 0.0;
    currentMouseDistance = 0.0;
    updateMouseParams(false, false, 0.0);
    restartAccelParams(false, false, true);
    lastWheelVerticalDistance = 0.0;
    lastWheelHorizontalDistance = 0.0;
    tempTurboInterval = 0;
    currentTurboMode = DEFAULTTURBOMODE;
    m_easingDuration = GlobalVariables::JoyButton::DEFAULTEASINGDURATION;
    springDeadCircleMultiplier = GlobalVariables::JoyButton::DEFAULTSPRINGRELEASERADIUS;

    updatePendingParams(false, false, false);
    lockForWritedString(activeZoneString, tr("[NO KEY]"));
}

bool JoyButton::isModifierButton()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return false;
}

void JoyButton::resetActiveButtonMouseDistances(JoyButtonMouseHelper *mouseHelper)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    mouseHelper->resetButtonMouseDistances();
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
    } else
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
        // emit propertyUpdated();
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

void JoyButton::setSpringModeScreen(int screen, int &springModeScreen)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (screen >= -1)
        springModeScreen = screen;
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

bool JoyButton::hasFutureSpringEvents(QList<JoyButton *> *pendingMouseButtons)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = false;
    QListIterator<JoyButton *> iter(*pendingMouseButtons);

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

int JoyButton::getSpringDeadCircleMultiplier() { return springDeadCircleMultiplier; }

double JoyButton::getCurrentSpringDeadCircle() { return (springDeadCircleMultiplier * 0.01); }

void JoyButton::restartLastMouseTime(QTime *testOldMouseTime) { testOldMouseTime->restart(); }

void JoyButton::setStaticMouseThread(QThread *thread, QTimer *staticMouseEventTimer, QTime *testOldMouseTime,
                                     int idleMouseRefrRate, JoyButtonMouseHelper *mouseHelper)
{
    int oldInterval = staticMouseEventTimer->interval();

    if (oldInterval == 0)
        oldInterval = idleMouseRefrRate;

    staticMouseEventTimer->moveToThread(thread);
    mouseHelper->moveToThread(thread);

    QMetaObject::invokeMethod(staticMouseEventTimer, "start", Q_ARG(int, oldInterval));

    testOldMouseTime->start();
}

void JoyButton::indirectStaticMouseThread(QThread *thread, QTimer *staticMouseEventTimer, JoyButtonMouseHelper *mouseHelper)
{
    QMetaObject::invokeMethod(staticMouseEventTimer, "stop");
    QMetaObject::invokeMethod(mouseHelper, "changeThread", Q_ARG(QThread *, thread));
}

bool JoyButton::shouldInvokeMouseEvents(QList<JoyButton *> *pendingMouseButtons, QTimer *staticMouseEventTimer,
                                        QTime *testOldMouseTime)
{
    bool result = false;

    if ((staticMouseEventTimer->interval() == 0) && (pendingMouseButtons->size() > 0) && staticMouseEventTimer->isActive())
        result = true;
    else if ((testOldMouseTime->elapsed() >= staticMouseEventTimer->interval()) && (pendingMouseButtons->size() > 0) &&
             staticMouseEventTimer->isActive())
        result = true;

    return result;
}

void JoyButton::invokeMouseEvents(JoyButtonMouseHelper *mouseHelper) { mouseHelper->mouseEvent(); }

bool JoyButton::hasActiveSlots() { return !getActiveSlots().isEmpty(); }

void JoyButton::setExtraAccelerationCurve(JoyExtraAccelerationCurve curve)
{
    extraAccelCurve = curve;
    emit propertyUpdated();
}

JoyButton::JoyExtraAccelerationCurve JoyButton::getExtraAccelerationCurve() { return extraAccelCurve; }

void JoyButton::copyExtraAccelerationState(JoyButton *srcButton)
{
    this->currentAccelMulti = srcButton->currentAccelMulti;
    this->oldAccelMulti = srcButton->oldAccelMulti;
    this->accelTravel = srcButton->accelTravel;

    this->startingAccelerationDistance = srcButton->startingAccelerationDistance;
    this->lastAccelerationDistance = srcButton->lastAccelerationDistance;
    this->lastMouseDistance = srcButton->lastMouseDistance;

    this->accelExtraDurationTime.setHMS(srcButton->accelExtraDurationTime.hour(), srcButton->accelExtraDurationTime.minute(),
                                        srcButton->accelExtraDurationTime.second(),
                                        srcButton->accelExtraDurationTime.msec());

    updateMouseParams((srcButton->lastMouseDistance != 0.0), srcButton->updateStartingMouseDistance,
                      srcButton->updateOldAccelMulti);
}

void JoyButton::setUpdateInitAccel(bool state) { this->updateInitAccelValues = state; }

QList<JoyButtonSlot *> &JoyButton::getAssignmentsLocal() { return assignments; }

QList<JoyButtonSlot *> &JoyButton::getActiveSlotsLocal() { return activeSlots; }
