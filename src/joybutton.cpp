//#include <QDebug>
#include <QStringList>
#include <cmath>

#include "setjoystick.h"
#include "inputdevice.h"
#include "joybutton.h"
#include "vdpad.h"
#include "event.h"

const QString JoyButton::xmlName = "button";

// Set default values for many properties.
const int JoyButton::ENABLEDTURBODEFAULT = 100;
const double JoyButton::SMOOTHINGFACTOR = 0.85;
const double JoyButton::DEFAULTMOUSESPEEDMOD = 1.0;
double JoyButton::mouseSpeedModifier = JoyButton::DEFAULTMOUSESPEEDMOD;
const unsigned int JoyButton::DEFAULTKEYREPEATDELAY = 600; // 600 ms
const unsigned int JoyButton::DEFAULTKEYREPEATRATE = 40; // 40 ms. 25 times per second
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
const bool JoyButton::DEFAULTSMOOTHING = false;
const int JoyButton::DEFAULTWHEELX = 20;
const int JoyButton::DEFAULTWHEELY = 20;
const bool JoyButton::DEFAULTCYCLERESETACTIVE = false;
const int JoyButton::DEFAULTCYCLERESET = 0;
const bool JoyButton::DEFAULTRELATIVESPRING = false;
const JoyButton::TurboMode JoyButton::DEFAULTTURBOMODE = JoyButton::NormalTurbo;
const double JoyButton::DEFAULTEASINGDURATION = 0.5;
const double JoyButton::MINIMUMEASINGDURATION = 0.2;

const int JoyButton::DEFAULTMOUSEHISTORYSIZE = 10;
const double JoyButton::DEFAULTWEIGHTMODIFIER = 0.2;
const int JoyButton::MAXIMUMMOUSEHISTORYSIZE = 30;
const double JoyButton::MAXIMUMWEIGHTMODIFIER = 1.0;
const int JoyButton::MAXIMUMMOUSEREFRESHRATE = 16;
const int JoyButton::IDLEMOUSEREFRESHRATE = JoyButton::MAXIMUMMOUSEREFRESHRATE;

// Keep references to active keys and mouse buttons.
QHash<unsigned int, int> JoyButton::activeKeys;
QHash<unsigned int, int> JoyButton::activeMouseButtons;
JoyButtonSlot* JoyButton::lastActiveKey = 0;

// Keep track of active Mouse Speed Mod slots.
QList<JoyButtonSlot*> JoyButton::mouseSpeedModList;

// Lists used for cursor mode calculations.
QList<JoyButton::mouseCursorInfo> JoyButton::cursorXSpeeds;
QList<JoyButton::mouseCursorInfo> JoyButton::cursorYSpeeds;

// Lists used for spring mode calculations.
QList<PadderCommon::springModeInfo> JoyButton::springXSpeeds;
QList<PadderCommon::springModeInfo> JoyButton::springYSpeeds;

// Keeps timestamp of last mouse event.
QTime JoyButton::lastMouseTime;

// Helper object to have a single mouse event for all JoyButton
// instances.
JoyButtonMouseHelper JoyButton::mouseHelper;

QTimer JoyButton::staticMouseEventTimer;
QList<JoyButton*> JoyButton::pendingMouseButtons;

QList<double> JoyButton::mouseHistoryX;
QList<double> JoyButton::mouseHistoryY;

double JoyButton::cursorRemainderX = 0.0;
double JoyButton::cursorRemainderY = 0.0;

double JoyButton::weightModifier = 0;
int JoyButton::mouseHistorySize = 1;

int JoyButton::mouseRefreshRate = 5;


JoyButton::JoyButton(int index, int originset, SetJoystick *parentSet, QObject *parent) :
    QObject(parent)
{
    vdpad = 0;
    slotiter = 0;
    setChangeTimer.setSingleShot(true);
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
    connect(&keyRepeatTimer, SIGNAL(timeout()), this, SLOT(repeatKeysEvent()));

    establishMouseTimerConnections();

    this->reset();
    this->index = index;
    this->originset = originset;

    quitEvent = true;
}

JoyButton::~JoyButton()
{
    reset();
}

void JoyButton::joyEvent(bool pressed, bool ignoresets)
{
    if (this->vdpad)
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

            this->vdpad->joyEvent(pressed, ignoresets);
        }
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
                //qDebug() << "PRESS STARTED: " << QTime::currentTime().toString("hh:mm:ss.zzz");
                emit clicked(index);
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
                        cycleResetHold.elapsed() >= cycleResetInterval && slotiter)
                    {
                        slotiter->toFront();
                        currentCycle = 0;
                        previousCycle = 0;
                    }

                    buttonHold.restart();
                    buttonHeldRelease.restart();
                    keyPressHold.restart();
                    cycleResetHold.restart();
                    turboTimer.start();
                    turboEvent();
                }
                else if (!isButtonPressed && !activePress && turboTimer.isActive())
                {
                    turboTimer.stop();
                    if (isKeyPressed)
                    {
                        turboEvent();
                        //QTimer::singleShot(0, this, SLOT(turboEvent()));
                    }
                    else
                    {
                        lastDistance = getMouseDistanceFromDeadZone();
                    }
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
                    //createDeskTimer.start(0);
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
                    cycleResetHold.elapsed() >= cycleResetInterval && slotiter)
                {
                    slotiter->toFront();
                    currentCycle = 0;
                    previousCycle = 0;
                }

                buttonHold.restart();
                buttonHeldRelease.restart();
                cycleResetHold.restart();
                keyPressHold.restart();
                //createDeskTimer.start(0);
                releaseDeskTimer.stop();

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
                waitForReleaseDeskEvent();
            }
        }
        else if (!useTurbo && isButtonPressed)
        {
            if (!setChangeTimer.isActive())
            {
                bool releasedCalled = distanceEvent();
                if (releasedCalled)
                {
                    quitEvent = true;
                    buttonHold.restart();
                    buttonHeldRelease.restart();
                    keyPressHold.restart();
                    //createDeskTimer.start(0);
                    releaseDeskTimer.stop();
                    if (!keyPressTimer.isActive())
                    {
                        waitForDeskEvent();
                    }
                }
            }
        }
    }
}

/**
 * @brief Get 0 indexed number of button
 * @return 0 indexed button index number
 */
int JoyButton::getJoyNumber()
{
    return index;
}

/**
 * @brief Get a 1 indexed number of button
 * @return 1 indexed button index number
 */
int JoyButton::getRealJoyNumber()
{
    return index + 1;
}

void JoyButton::setJoyNumber(int index)
{
    this->index = index;
}

void JoyButton::setToggle(bool toggle)
{
    if (toggle != this->toggle)
    {
        this->toggle = toggle;
        emit toggleChanged(toggle);
        emit propertyUpdated();
    }
}

void JoyButton::setTurboInterval(int interval)
{
    if (interval >= 10 && interval != this->turboInterval)
    {
        this->turboInterval = interval;
        emit turboIntervalChanged(interval);
        emit propertyUpdated();
    }
    else if (interval < 10 && interval != this->turboInterval)
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
    keyRepeatTimer.stop();

    if (slotiter)
    {
        delete slotiter;
        slotiter = 0;
    }

    releaseDeskEvent(true);
    clearAssignedSlots();

    isButtonPressedQueue.clear();
    ignoreSetQueue.clear();
    mouseEventQueue.clear();
    mouseWheelVerticalEventQueue.clear();
    mouseWheelHorizontalEventQueue.clear();

    currentCycle = 0;
    previousCycle = 0;
    currentPause = 0;
    currentHold = 0;
    currentDistance = 0;
    currentRawValue = 0;
    currentMouseEvent = 0;
    currentRelease = 0;
    currentWheelVerticalEvent = 0;
    currentWheelHorizontalEvent = 0;
    currentKeyPress = 0;
    currentDelay = 0;

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
    smoothing = false;
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
    lastWheelVerticalDistance = 0.0;
    lastWheelHorizontalDistance = 0.0;
    tempTurboInterval = 0;
    //currentTurboMode = GradientTurbo;
    currentTurboMode = DEFAULTTURBOMODE;
    easingDuration = DEFAULTEASINGDURATION;
}

void JoyButton::reset(int index)
{
    JoyButton::reset();
    this->index = index;
}

bool JoyButton::getToggleState()
{
    return toggle;
}

int JoyButton::getTurboInterval()
{
    return turboInterval;
}

void JoyButton::turboEvent()
{
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
                //turboTimer.start(10);
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
                //turboTimer.start(turboInterval - 10);
            }

        }

    }
}

bool JoyButton::distanceEvent()
{
    bool released = false;

    if (slotiter)
    {
        bool distanceFound = containsDistanceSlots();

        if (distanceFound)
        {
            double currentDistance = getDistanceFromDeadZone();
            double tempDistance = 0.0;
            JoyButtonSlot *previousDistanceSlot = 0;
            QListIterator<JoyButtonSlot*> iter(assignments);
            if (previousCycle)
            {
                iter.findNext(previousCycle);
            }

            while (iter.hasNext())
            {
                JoyButtonSlot *slot = iter.next();
                int tempcode = slot->getSlotCode();
                if (slot->getSlotMode() == JoyButtonSlot::JoyDistance)
                {
                    tempDistance += tempcode / 100.0;

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
                if (this->currentDistance)
                {
                    // Distance slot is currently active.
                    // Release slots, return iterator to
                    // the front, and nullify currentDistance
                    pauseWaitTimer.stop();
                    holdTimer.stop();

                    // Release stuff
                    releaseActiveSlots();
                    currentPause = currentHold = 0;
                    //quitEvent = true;

                    slotiter->toFront();
                    if (previousCycle)
                    {
                        slotiter->findNext(previousCycle);
                    }

                    this->currentDistance = 0;
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
                    currentPause = currentHold = 0;
                    //quitEvent = true;

                    slotiter->toFront();
                    if (previousCycle)
                    {
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
    quitEvent = false;

    if (!slotiter)
    {
        slotiter = new QListIterator<JoyButtonSlot*> (assignments);
        distanceEvent();
    }
    else if (!slotiter->hasPrevious())
    {
        distanceEvent();
    }
    else if (currentCycle)
    {
        currentCycle = 0;
        distanceEvent();
    }

    activateSlots();

    if (currentCycle)
    {
        quitEvent = true;
    }
    else if (!currentPause && !currentHold && !keyPressTimer.isActive())
    {
        quitEvent = true;
    }
}

void JoyButton::activateSlots()
{
    if (slotiter)
    {
        bool exit = false;
        //bool delaySequence = checkForDelaySequence();
        bool delaySequence = false;

        while (slotiter->hasNext() && !exit)
        {
            JoyButtonSlot *slot = slotiter->next();
            int tempcode = slot->getSlotCode();
            JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();

            if (mode == JoyButtonSlot::JoyKeyboard)
            {
                sendevent(slot, true);
                //qDebug() << "PRESS FINISHED: " << QTime::currentTime().toString("hh:mm:ss.zzz");
                activeSlots.append(slot);
                int oldvalue = activeKeys.value(tempcode, 0) + 1;
                activeKeys.insert(tempcode, oldvalue);
                if (!slot->isModifierKey())
                {
                    lastActiveKey = slot;
                }
            }
            else if (mode == JoyButtonSlot::JoyMouseButton)
            {
                if (tempcode == JoyButtonSlot::MouseWheelUp ||
                    tempcode == JoyButtonSlot::MouseWheelDown)
                {
                    slot->getMouseInterval()->restart();
                    wheelVerticalTime.restart();
                    currentWheelVerticalEvent = slot;
                    activeSlots.append(slot);
                    wheelEventVertical();
                    currentWheelVerticalEvent = 0;
                }
                else if (tempcode == JoyButtonSlot::MouseWheelLeft ||
                         tempcode == JoyButtonSlot::MouseWheelRight)
                {
                    slot->getMouseInterval()->restart();
                    wheelHorizontalTime.restart();
                    currentWheelHorizontalEvent = slot;
                    activeSlots.append(slot);
                    wheelEventHorizontal();
                    currentWheelHorizontalEvent = 0;
                }
                else
                {
                    sendevent(slot, true);
                    activeSlots.append(slot);
                    int oldvalue = activeMouseButtons.value(tempcode, 0) + 1;
                    activeMouseButtons.insert(tempcode, oldvalue);
                }
            }
            else if (mode == JoyButtonSlot::JoyMouseMovement)
            {
                slot->getMouseInterval()->restart();
                //currentMouseEvent = slot;
                activeSlots.append(slot);
                //mouseEventQueue.enqueue(slot);
                //mouseEvent();
                pendingMouseButtons.append(this);
                mouseEventQueue.enqueue(slot);
                //currentMouseEvent = 0;

                // Temporarily lower timer interval. Helps improve mouse control
                // precision on the lower end of an axis.
                if (!staticMouseEventTimer.isActive() || staticMouseEventTimer.interval() != 0)
                {
                    lastMouseTime.restart();
                    staticMouseEventTimer.start(0);
                }
            }
            else if (mode == JoyButtonSlot::JoyPause)
            {
                if (!activeSlots.isEmpty())
                {
                    if (slotiter->hasPrevious())
                    {
                        slotiter->previous();
                    }
                    delaySequence = true;
                    exit = true;
                }
                // Segment can be ignored on a 0 interval pause
                else if (tempcode > 0)
                {
                    currentPause = slot;
                    pauseHold.restart();
                    inpauseHold.restart();
                    pauseWaitTimer.start(0);
                    exit = true;
                }
            }
            else if (mode == JoyButtonSlot::JoyHold)
            {
                currentHold = slot;
                holdTimer.start(0);
                exit = true;
            }
            else if (mode == JoyButtonSlot::JoyDelay)
            {
                currentDelay = slot;
                buttonDelay.restart();
                delayTimer.start(0);
                exit = true;
            }
            else if (mode == JoyButtonSlot::JoyCycle)
            {
                currentCycle = slot;
                exit = true;
            }
            else if (mode == JoyButtonSlot::JoyDistance)
            {
                exit = true;
            }
            else if (mode == JoyButtonSlot::JoyRelease)
            {
                if (!currentRelease)
                {
                    findReleaseEventEnd();
                }
                /*else
                {
                    currentRelease = 0;
                    exit = true;
                }*/

                else if (currentRelease && activeSlots.isEmpty())
                {
                    //currentRelease = 0;
                    exit = true;
                }
                else if (currentRelease && !activeSlots.isEmpty())
                {
                    if (slotiter->hasPrevious())
                    {
                        slotiter->previous();
                    }
                    delaySequence = true;
                    exit = true;
                }
            }
            else if (mode == JoyButtonSlot::JoyMouseSpeedMod)
            {
                mouseSpeedModifier = tempcode * 0.01;
                mouseSpeedModList.append(slot);
                activeSlots.append(slot);
            }
            else if (mode == JoyButtonSlot::JoyKeyPress)
            {
                if (activeSlots.isEmpty())
                {
                    delaySequence = true;
                    //currentKeyPress = new JoyButtonSlot(2000, JoyButtonSlot::JoyKeyPress, this);
                    currentKeyPress = slot;
                }
                else
                {
                    if (slotiter->hasPrevious())
                    {
                        slotiter->previous();
                    }
                    delaySequence = true;
                    exit = true;
                }
            }
        }

        if (delaySequence && !activeSlots.isEmpty())
        {
            keyPressHold.restart();
            keyPressEvent();
        }

#ifdef Q_OS_WIN
        else if (lastActiveKey && activeSlots.contains(lastActiveKey) &&
                 !useTurbo)
        {
            InputDevice *device = getParentSet()->getInputDevice();
            if (device->isKeyRepeatEnabled())
            {
                keyRepeatTimer.start(device->getKeyRepeatDelay());
            }
        }
#endif

        emit activeZoneChanged();
    }
}

/**
 * @brief Calculate mouse movement coordinates for mouse movement slots
 *     currently active for this button.
 */
void JoyButton::mouseEvent()
{
    JoyButtonSlot *buttonslot = 0;
    bool singleShot = false;
    if (currentMouseEvent)
    {
        buttonslot = currentMouseEvent;
        singleShot = true;
    }

    if (buttonslot || !mouseEventQueue.isEmpty())
    {
        QQueue<JoyButtonSlot*> tempQueue;

        if (!buttonslot)
        {
            buttonslot = mouseEventQueue.dequeue();
        }

        while (buttonslot)
        {
            QElapsedTimer* mouseInterval = buttonslot->getMouseInterval();

            int mousedirection = buttonslot->getSlotCode();
            JoyButton::JoyMouseMovementMode mousemode = getMouseMode();
            int mousespeed = 0;
            int timeElapsed = mouseInterval->elapsed();
            int nanoTimeElapsed = mouseInterval->nsecsElapsed();

            bool isActive = activeSlots.contains(buttonslot);
            if (isActive)
            {
                if (mousemode == JoyButton::MouseCursor)
                {
                    if (mousedirection == JoyButtonSlot::MouseRight)
                    {
                        mousespeed = mouseSpeedX;
                    }
                    else if (mousedirection == JoyButtonSlot::MouseLeft)
                    {
                        mousespeed = mouseSpeedX;
                    }
                    else if (mousedirection == JoyButtonSlot::MouseDown)
                    {
                        mousespeed = mouseSpeedY;
                    }
                    else if (mousedirection == JoyButtonSlot::MouseUp)
                    {
                        mousespeed = mouseSpeedY;
                    }

                    double difference = getMouseDistanceFromDeadZone();
                    //qDebug() << "RIGHT DIFF: " << difference;
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
                                // Perform Quadratic acceleration.
                                difference = difference * difference;
                            }
                            else if (temp <= 0.8)
                            {
                                // Perform Linear accleration with an appropriate
                                // offset.
                                difference = difference - 0.24;
                            }
                            else if (temp > 0.8)
                            {
                                // Perform mouse acceleration. Make up the difference
                                // due to the previous two segments. Maxes out at 1.0.
                                difference = (difference * 2.2) - 1.2;
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
                                // Perform Quadratic acceleration.
                                difference = difference * difference;

                                if (buttonslot->isEasingActive())
                                {
                                    buttonslot->setEasingStatus(false);
                                    buttonslot->getEasingTime()->restart();
                                    //qDebug() << "QUITTING";
                                }
                            }
                            else if (temp <= 0.8)
                            {
                                // Perform Linear accleration with an appropriate
                                // offset.
                                difference = difference - 0.24;

                                if (buttonslot->isEasingActive())
                                {
                                    buttonslot->setEasingStatus(false);
                                    buttonslot->getEasingTime()->restart();
                                    //qDebug() << "QUITTING";
                                }
                            }
                            else if (temp > 0.8)
                            {
                                unsigned int easingElapsed = buttonslot->getEasingTime()->elapsed();
                                double easingDuration = this->easingDuration; // Time in seconds
                                //qDebug() << "TEMP: " << temp;
                                if (!buttonslot->isEasingActive())
                                {
                                    buttonslot->setEasingStatus(true);
                                    buttonslot->getEasingTime()->restart();
                                    easingElapsed = timeElapsed;
                                }

                                if ((easingElapsed * .001) < easingDuration)
                                {
                                    difference = ((easingElapsed * .001) / easingDuration);
                                    if (currentCurve == EasingQuadraticCurve)
                                    {
                                        // Range 0.8 - 1.0
                                        difference = (1.0 - 0.8) * difference * difference + 0.8;
                                    }
                                    else
                                    {
                                        // Range 0.8 - 1.0
                                        difference = (1.0 - 0.8) * (difference * difference
                                                     * difference) + 0.8;
                                    }

                                    //qDebug() << "TIME: " << easingDuration;
                                }
                                else
                                {
                                    difference = 1.0;
                                }

                                //difference = difference * 7.2 - 5.2; // Range 0.56 - 2.0
                                difference = difference * 4.7 - 3.2; // Range 0.56 - 1.5
                            }
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }

                    double distance = 0;
                    difference = (mouseSpeedModifier == 1.0) ? difference : (difference * mouseSpeedModifier);
                    //sumDist += difference * (mousespeed * JoyButtonSlot::JOYSPEED * timeElapsed) * 0.001;
                    sumDist = difference * (nanoTimeElapsed * 0.000000001) * mousespeed * JoyButtonSlot::JOYSPEED;
                    //distance = (int)floor(sumDist + 0.5);
                    //distance = (int)floor(sumDist);
                    distance = sumDist;
                    //qDebug() << "DISTANCE: " << distance;

                    //double prevDistance = buttonslot->getPreviousDistance();
                    //qDebug() << "PREV: " << prevDistance;
                    //if (smoothing)
                    //{
                        //distance = (int)floor((distance - prevDistance) * 0.1 + prevDistance);
                        //qDebug() << "NOW: " << distance;
                    //}

                    if (mousedirection == JoyButtonSlot::MouseRight)
                    {
                        mouse1 = distance;
                    }
                    else if (mousedirection == JoyButtonSlot::MouseLeft)
                    {
                        mouse1 = -distance;
                    }
                    else if (mousedirection == JoyButtonSlot::MouseDown)
                    {
                        mouse2 = distance;
                    }
                    else if (mousedirection == JoyButtonSlot::MouseUp)
                    {
                        mouse2 = -distance;
                    }

                    /*if (timeElapsed > 5)
                    {
                        //qDebug() << "ENCOUNTERED:" << timeElapsed;
                    }
                    */

                    //if (distance >= 1)
                    //{
                        mouseCursorInfo infoX;
                        infoX.code = mouse1;
                        infoX.slot = buttonslot;
                        cursorXSpeeds.append(infoX);

                        mouseCursorInfo infoY;
                        infoY.code = mouse2;
                        infoY.slot = buttonslot;
                        cursorYSpeeds.append(infoY);

                        //sendevent(mouse1, mouse2);
                        //sumDist -= distance;
                        sumDist = 0;
                        //buttonslot->setPreviousDistance(distance);

                        //if (smoothing)
                        //{
                        //    sumDist *= 0.5;
                        //}
                    //}
                    /*else
                    {
                        buttonslot->setPreviousDistance(0.0);
                    }
                    */

                    buttonslot->setDistance(sumDist);
                }
                else if (mousemode == JoyButton::MouseSpring)
                {
                    double mouse1 = -2.0;
                    double mouse2 = -2.0;
                    double difference = getMouseDistanceFromDeadZone();
                    double sumDist = buttonslot->getMouseDistance();

                    if (mousedirection == JoyButtonSlot::MouseRight)
                    {
                        mouse1 = difference;
                    }
                    else if (mousedirection == JoyButtonSlot::MouseLeft)
                    {
                        mouse1 = -difference;
                    }
                    else if (mousedirection == JoyButtonSlot::MouseDown)
                    {
                        mouse2 = difference;
                    }
                    else if (mousedirection == JoyButtonSlot::MouseUp)
                    {
                        mouse2 = -difference;
                    }

                    double tempdiff = (difference >= 0.0) ? difference : -difference;
                    double change = sumDist - tempdiff;
                    change = (change >= 0.0) ? change : -change;

                    //springXSpeeds.append(mouse1);
                    //springYSpeeds.append(mouse2);

                    PadderCommon::springModeInfo infoX;
                    infoX.displacementX = mouse1;
                    infoX.width = springWidth;
                    infoX.height = springHeight;
                    infoX.relative = relativeSpring;
                    springXSpeeds.append(infoX);

                    PadderCommon::springModeInfo infoY;
                    infoY.displacementY = mouse2;
                    infoY.width = springWidth;
                    infoY.height = springHeight;
                    infoY.relative = relativeSpring;
                    springYSpeeds.append(infoY);

                    //sendSpringEvent(mouse1, mouse2, springWidth, springHeight);
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
    JoyButtonSlot *buttonslot = 0;
    bool activateEvent = false;

    int tempInterval = 0;
    double diff = fabs(getMouseDistanceFromDeadZone() - lastWheelVerticalDistance);
    int oldInterval = 0;

    if (wheelSpeedY != 0)
    {
        if (lastWheelVerticalDistance > 0)
        {
            oldInterval = 1000 / wheelSpeedY / lastWheelVerticalDistance;
        }
        else
        {
            oldInterval = 1000 / wheelSpeedY / 0.01;
        }
    }

    if (currentWheelVerticalEvent)
    {
        buttonslot = currentWheelVerticalEvent;
        activateEvent = true;
    }

    if (!activateEvent)
    {
        if (!mouseWheelVerticalEventTimer.isActive())
        {
            activateEvent = true;
        }
        else if (wheelVerticalTime.elapsed() > oldInterval)
        {
            activateEvent = true;
        }
        else if (diff >= 0.1 && wheelSpeedY != 0)
        {
            tempInterval = static_cast<int>(1000 / wheelSpeedY / getMouseDistanceFromDeadZone());
            if (wheelVerticalTime.elapsed() < tempInterval)
            {
                // Still some valid time left. Continue current action with
                // remaining time left.
                tempInterval = tempInterval - wheelVerticalTime.elapsed();
                tempInterval = qMin(tempInterval, 5);
                if (!mouseWheelVerticalEventTimer.isActive() || mouseWheelVerticalEventTimer.interval() != tempInterval)
                {
                    mouseWheelVerticalEventTimer.start(tempInterval);
                }
            }
            else
            {
                // Elapsed time is greater than new interval. Change state.
                activateEvent = true;
            }
        }
    }

    if (buttonslot && wheelSpeedY != 0)
    {
        bool isActive = activeSlots.contains(buttonslot);
        if (isActive && activateEvent)
        {
            sendevent(buttonslot, true);
            sendevent(buttonslot, false);
            mouseWheelVerticalEventQueue.enqueue(buttonslot);
            tempInterval = static_cast<int>(1000 / wheelSpeedY / getMouseDistanceFromDeadZone());
            tempInterval = qMin(tempInterval, 5);
            if (!mouseWheelVerticalEventTimer.isActive() || mouseWheelVerticalEventTimer.interval() != tempInterval)
            {
                mouseWheelVerticalEventTimer.start(tempInterval);
            }
        }
        else if (!isActive)
        {
            mouseWheelVerticalEventTimer.stop();
        }
    }
    else if (!mouseWheelVerticalEventQueue.isEmpty() && wheelSpeedY != 0)
    {
        QQueue<JoyButtonSlot*> tempQueue;
        while (!mouseWheelVerticalEventQueue.isEmpty())
        {
            buttonslot = mouseWheelVerticalEventQueue.dequeue();
            bool isActive = activeSlots.contains(buttonslot);
            if (isActive && activateEvent)
            {
                sendevent(buttonslot, true);
                sendevent(buttonslot, false);
                tempQueue.enqueue(buttonslot);
            }
            else if (isActive)
            {
                tempQueue.enqueue(buttonslot);
            }
        }

        if (!tempQueue.isEmpty())
        {
            mouseWheelVerticalEventQueue = tempQueue;
            tempInterval = static_cast<int>(1000 / wheelSpeedY / getMouseDistanceFromDeadZone());
            tempInterval = qMin(tempInterval, 5);
            if (!mouseWheelVerticalEventTimer.isActive() || mouseWheelVerticalEventTimer.interval() != tempInterval)
            {
                mouseWheelVerticalEventTimer.start(tempInterval);
            }
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

    if (activateEvent)
    {
        wheelVerticalTime.restart();
        lastWheelVerticalDistance = getMouseDistanceFromDeadZone();
    }
}

void JoyButton::wheelEventHorizontal()
{
    JoyButtonSlot *buttonslot = 0;
    bool activateEvent = false;

    int tempInterval = 0;
    double diff = fabs(getMouseDistanceFromDeadZone() - lastWheelHorizontalDistance);
    int oldInterval = 0;

    if (wheelSpeedX != 0)
    {
        if (lastWheelHorizontalDistance > 0)
        {
            oldInterval = 1000 / wheelSpeedX / lastWheelHorizontalDistance;
        }
        else
        {
            oldInterval = 1000 / wheelSpeedX / 0.01;
        }
    }

    if (currentWheelHorizontalEvent)
    {
        buttonslot = currentWheelHorizontalEvent;
        activateEvent = true;
    }

    if (!activateEvent)
    {
        if (!mouseWheelHorizontalEventTimer.isActive())
        {
            activateEvent = true;
        }
        else if (wheelHorizontalTime.elapsed() > oldInterval)
        {
            activateEvent = true;
        }
        else if (diff >= 0.1 && wheelSpeedX != 0)
        {
            tempInterval = static_cast<int>(1000 / wheelSpeedX / getMouseDistanceFromDeadZone());
            if (wheelHorizontalTime.elapsed() < tempInterval)
            {
                // Still some valid time left. Continue current action with
                // remaining time left.
                tempInterval = tempInterval - wheelHorizontalTime.elapsed();
                tempInterval = qMin(tempInterval, 5);
                if (!mouseWheelHorizontalEventTimer.isActive() || mouseWheelHorizontalEventTimer.interval() != tempInterval)
                {
                    mouseWheelHorizontalEventTimer.start(tempInterval);
                }
            }
            else
            {
                // Elapsed time is greater than new interval. Change state.
                activateEvent = true;
            }
        }
    }

    if (buttonslot && wheelSpeedX != 0)
    {
        bool isActive = activeSlots.contains(buttonslot);
        if (isActive && activateEvent)
        {
            sendevent(buttonslot, true);
            sendevent(buttonslot, false);
            mouseWheelHorizontalEventQueue.enqueue(buttonslot);
            tempInterval = static_cast<int>(1000 / wheelSpeedX / getMouseDistanceFromDeadZone());
            tempInterval = qMin(tempInterval, 5);

            if (!mouseWheelHorizontalEventTimer.isActive() || mouseWheelVerticalEventTimer.interval() != tempInterval)
            {
                mouseWheelHorizontalEventTimer.start(tempInterval);
            }
        }
        else if (!isActive)
        {
            mouseWheelHorizontalEventTimer.stop();
        }
    }
    else if (!mouseWheelHorizontalEventQueue.isEmpty() && wheelSpeedX != 0)
    {
        QQueue<JoyButtonSlot*> tempQueue;
        while (!mouseWheelHorizontalEventQueue.isEmpty())
        {
            buttonslot = mouseWheelHorizontalEventQueue.dequeue();
            bool isActive = activeSlots.contains(buttonslot);
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
            tempInterval = static_cast<int>(1000 / wheelSpeedX / getMouseDistanceFromDeadZone());
            tempInterval = qMin(tempInterval, 5);

            if (!mouseWheelHorizontalEventTimer.isActive() || mouseWheelVerticalEventTimer.interval() != tempInterval)
            {
                mouseWheelHorizontalEventTimer.start(tempInterval);
            }
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

    if (activateEvent)
    {
        wheelHorizontalTime.restart();
        lastWheelHorizontalDistance = getMouseDistanceFromDeadZone();
    }
}

void JoyButton::setUseTurbo(bool useTurbo)
{
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

            if (this->useTurbo && this->turboInterval == 0)
            {
                this->setTurboInterval(ENABLEDTURBODEFAULT);
            }
        }
    }
}

bool JoyButton::isUsingTurbo()
{
    return useTurbo;
}

QString JoyButton::getXmlName()
{
    return this->xmlName;
}

void JoyButton::readConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && xml->name() == getXmlName())
    {
        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != getXmlName()))
        {
            bool found = readButtonConfig(xml);
            if (!found)
            {
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }
    }
}

void JoyButton::writeConfig(QXmlStreamWriter *xml)
{
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

        /*if (smoothing != DEFAULTSMOOTHING)
        {
            xml->writeTextElement("mousesmoothing", smoothing ? "true" : "false");
        }
        */

        if (wheelSpeedX != DEFAULTWHEELX)
        {
            xml->writeTextElement("wheelspeedx", QString::number(wheelSpeedX));
        }

        if (wheelSpeedY != DEFAULTWHEELY)
        {
            xml->writeTextElement("wheelspeedy", QString::number(wheelSpeedY));
        }

        if (setSelectionCondition != SetChangeDisabled)
        {
            xml->writeTextElement("setselect", QString::number(setSelection+1));

            QString temptext;
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

        if (!actionName.isEmpty())
        {
            xml->writeTextElement("actionname", actionName);
        }

        if (cycleResetActive)
        {
            xml->writeTextElement("cycleresetactive", "true");
        }

        if (cycleResetInterval >= 10)
        {
            xml->writeTextElement("cycleresetinterval", QString::number(cycleResetInterval));
        }

        if (relativeSpring == true)
        {
            xml->writeTextElement("relativespring", "true");
        }

        if (easingDuration > DEFAULTEASINGDURATION)
        {
            xml->writeTextElement("easingduration", QString::number(easingDuration));
        }

        // Write information about assigned slots.
        if (!assignments.isEmpty())
        {
            xml->writeStartElement("slots");
            QListIterator<JoyButtonSlot*> iter(assignments);
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
    bool found = false;

    if (xml->name() == "toggle" && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        if (temptext == "true")
        {
            this->setToggle(true);
        }
    }
    else if (xml->name() == "turbointerval" && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        this->setTurboInterval(tempchoice);
    }
    else if (xml->name() == "turbomode" && xml->isStartElement())
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
    else if (xml->name() == "useturbo" && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        if (temptext == "true")
        {
            this->setUseTurbo(true);
        }
    }
    else if (xml->name() == "mousespeedx" && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        this->setMouseSpeedX(tempchoice);
    }
    else if (xml->name() == "mousespeedy" && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        this->setMouseSpeedY(tempchoice);
    }
    else if (xml->name() == "cycleresetactive" && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        if (temptext == "true")
        {
            this->setCycleResetStatus(true);
        }
    }
    else if (xml->name() == "cycleresetinterval" && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        unsigned int tempchoice = temptext.toInt();
        if (tempchoice >= 10)
        {
            this->setCycleResetTime(tempchoice);
        }
    }
    else if (xml->name() == "slots" && xml->isStartElement())
    {
        found = true;
        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != "slots"))
        {
            if (xml->name() == "slot" && xml->isStartElement())
            {
                JoyButtonSlot *buttonslot = new JoyButtonSlot(this);
                buttonslot->readConfig(xml);
                //setAssignedSlot(buttonslot->getSlotCode(), buttonslot->getSlotMode());
                bool inserted = setAssignedSlot(buttonslot);
                if (!inserted)
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
    else if (xml->name() == "setselect" && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        if (tempchoice >= 1 && tempchoice <= 8)
        {
            this->setChangeSetSelection(tempchoice - 1);
        }
    }
    else if (xml->name() == "setselectcondition" && xml->isStartElement())
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
            this->setChangeSetCondition(tempcondition);
        }
    }
    else if (xml->name() == "mousemode" && xml->isStartElement())
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
    else if (xml->name() == "mouseacceleration" && xml->isStartElement())
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
    else if (xml->name() == "mousespringwidth" && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        setSpringWidth(tempchoice);
    }
    else if (xml->name() == "mousespringheight" && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        setSpringHeight(tempchoice);
    }
    else if (xml->name() == "mousesensitivity" && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        double tempchoice = temptext.toDouble();
        setSensitivity(tempchoice);
    }
    /*else if (xml->name() == "mousesmoothing" && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        if (temptext == "true")
        {
            setSmoothing(true);
        }
    }
    */
    else if (xml->name() == "actionname" && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        if (!temptext.isEmpty())
        {
            setActionName(temptext);
        }
    }
    else if (xml->name() == "wheelspeedx" && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        setWheelSpeedX(tempchoice);
    }
    else if (xml->name() == "wheelspeedy" && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        setWheelSpeedY(tempchoice);
    }
    else if (xml->name() == "relativespring" && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        if (temptext == "true")
        {
            this->setSpringRelativeStatus(true);
        }
    }
    else if (xml->name() == "easingduration" && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        double tempchoice = temptext.toDouble();
        setEasingDuration(tempchoice);
    }

    return found;
}

QString JoyButton::getName(bool forceFullFormat, bool displayNames)
{
    QString newlabel = getPartialName(forceFullFormat, displayNames);
    newlabel.append(": ");
    if (!actionName.isEmpty() && displayNames)
    {
        newlabel.append(actionName);
    }
    else
    {
        newlabel.append(getActiveZoneSummary());
    }
    return newlabel;
}

QString JoyButton::getPartialName(bool forceFullFormat, bool displayNames)
{
    QString temp;
    if (!buttonName.isEmpty() && displayNames)
    {
        if (forceFullFormat)
        {
            temp.append(tr("Button")).append(" ");
        }
        temp.append(buttonName);
    }
    else if (!defaultButtonName.isEmpty())
    {
        if (forceFullFormat)
        {
            temp.append(tr("Button")).append(" ");
        }
        temp.append(defaultButtonName);
    }
    else
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
    QString newlabel;
    int slotCount = assignments.size();

    if (slotCount > 0)
    {
        QListIterator<JoyButtonSlot*> iter(assignments);
        QStringList stringlist;

        int i = 0;
        while (iter.hasNext())
        {
            JoyButtonSlot *slot = iter.next();
            stringlist.append(slot->getSlotString());
            i++;

            if (i > 4 && iter.hasNext())
            {
                stringlist.append(" ...");
                iter.toBack();
            }
        }

        newlabel = stringlist.join(", ");
    }
    else
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
    QString newlabel;
    QListIterator<JoyButtonSlot*> activeSlotsIter(activeSlots);
    QListIterator<JoyButtonSlot*> assignmentsIter(assignments);
    QListIterator<JoyButtonSlot*> *iter = 0;
    bool slotsActive = !activeSlots.isEmpty();
    if (slotsActive)
    {
        iter = &activeSlotsIter;
    }
    else
    {
        iter = &assignmentsIter;
        if (previousCycle)
        {
            iter->findNext(previousCycle);
        }
    }

    QStringList stringlist;
    int i = 0;

    if (setSelectionCondition == SetChangeOneWay)
    {
        newlabel.append(tr("[Set %1 1W]").arg(setSelection+1));
        if (iter->hasNext())
        {
            newlabel.append(" ");
        }
    }
    else if (setSelectionCondition == SetChangeTwoWay)
    {
        newlabel = newlabel.append(tr("[Set %1 2W]").arg(setSelection+1));
        if (iter->hasNext())
        {
           newlabel.append(" ");
        }
    }

    if (setSelectionCondition == SetChangeWhileHeld)
    {
        newlabel.append(tr("[Set %1 WH]").arg(setSelection+1));
    }
    else if (iter->hasNext())
    {
        bool behindHold = false;
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
                    if (!slotsActive && i == 0)
                    {
                        // If button is not active and first slot is a hold,
                        // keep processing slots but take note of the hold.
                        behindHold = true;
                    }
                    else
                    {
                        // Move iter to back so loop will end.
                        iter->toBack();
                    }

                    break;
                }
                default:
                {
                    iter->toBack();
                    break;
                }
            }

            if (i > 4 && iter->hasNext())
            {
                stringlist.append(" ...");
                iter->toBack();
            }
        }

        newlabel.append(stringlist.join(", "));
    }
    else if (setSelectionCondition == SetChangeDisabled)
    {
        newlabel.append(tr("[NO KEY]"));
    }

    return newlabel;
}

/**
 * @brief Generate a string representing all the currently assigned slots for
 *     a button
 * @return String representing all assigned slots for a button
 */
QString JoyButton::getSlotsString()
{
    QString label;

    if (assignments.size() > 0)
    {
        QListIterator<JoyButtonSlot*> iter(assignments);
        QStringList stringlist;

        while (iter.hasNext())
        {
            JoyButtonSlot *slot = iter.next();
            stringlist.append(slot->getSlotString());
        }

        label = stringlist.join(", ");
    }
    else
    {
        label = label.append(tr("[NO KEY]"));
    }

    return label;
}

void JoyButton::setCustomName(QString name)
{
    customName = name;
}

QString JoyButton::getCustomName()
{
    return customName;
}

bool JoyButton::setAssignedSlot(int code, JoyButtonSlot::JoySlotInputAction mode)
{
    bool slotInserted = false;
    JoyButtonSlot *slot = new JoyButtonSlot(code, mode, this);
    if (slot->getSlotMode() == JoyButtonSlot::JoyDistance)
    {
        if (slot->getSlotCode() >= 1 && slot->getSlotCode() <= 100)
        {
            double tempDistance = getTotalSlotDistance(slot);
            if (tempDistance <= 1.0)
            {
                assignments.append(slot);
                slotInserted = true;
            }
        }
    }
    else if (slot->getSlotCode() >= 0)
    {
        assignments.append(slot);
        slotInserted = true;
    }

    if (slotInserted)
    {
        if (slot->getSlotMode() == JoyButtonSlot::JoyPause ||
            slot->getSlotMode() == JoyButtonSlot::JoyHold ||
            slot->getSlotMode() == JoyButtonSlot::JoyDistance ||
            slot->getSlotMode() == JoyButtonSlot::JoyRelease
           )
        {
            setUseTurbo(false);
        }

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

bool JoyButton::setAssignedSlot(int code, unsigned int alias, JoyButtonSlot::JoySlotInputAction mode)
{
    bool slotInserted = false;
    JoyButtonSlot *slot = new JoyButtonSlot(code, alias, mode, this);
    if (slot->getSlotMode() == JoyButtonSlot::JoyDistance)
    {
        if (slot->getSlotCode() >= 1 && slot->getSlotCode() <= 100)
        {
            double tempDistance = getTotalSlotDistance(slot);
            if (tempDistance <= 1.0)
            {
                assignments.append(slot);
                slotInserted = true;
            }
        }
    }
    else if (slot->getSlotCode() >= 0)
    {
        assignments.append(slot);
        slotInserted = true;
    }

    if (slotInserted)
    {
        if (slot->getSlotMode() == JoyButtonSlot::JoyPause ||
            slot->getSlotMode() == JoyButtonSlot::JoyHold ||
            slot->getSlotMode() == JoyButtonSlot::JoyDistance ||
            slot->getSlotMode() == JoyButtonSlot::JoyRelease
           )
        {
            setUseTurbo(false);
        }

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

bool JoyButton::setAssignedSlot(int code, unsigned int alias, int index, JoyButtonSlot::JoySlotInputAction mode)
{
    bool permitSlot = true;

    JoyButtonSlot *slot = new JoyButtonSlot(code, alias, mode, this);
    if (slot->getSlotMode() == JoyButtonSlot::JoyDistance)
    {
        if (slot->getSlotCode() >= 1 && slot->getSlotCode() <= 100)
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
        if (index >= 0 && index < assignments.count())
        {
            // Insert slot and move existing slots.
            assignments.replace(index, slot);
        }
        else if (index >= assignments.count())
        {
            // Append code into a new slot
            assignments.append(slot);
        }

        if (slot->getSlotMode() == JoyButtonSlot::JoyPause ||
            slot->getSlotMode() == JoyButtonSlot::JoyHold ||
            slot->getSlotMode() == JoyButtonSlot::JoyDistance ||
            slot->getSlotMode() == JoyButtonSlot::JoyRelease
           )
        {
            setUseTurbo(false);
        }

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

bool JoyButton::insertAssignedSlot(int code, unsigned int alias, int index, JoyButtonSlot::JoySlotInputAction mode)
{
    bool permitSlot = true;

    JoyButtonSlot *slot = new JoyButtonSlot(code, alias, mode, this);
    if (slot->getSlotMode() == JoyButtonSlot::JoyDistance)
    {
        if (slot->getSlotCode() >= 1 && slot->getSlotCode() <= 100)
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
        if (index >= 0 && index < assignments.count())
        {
            // Slot already exists. Override code and place into desired slot
            assignments.insert(index, slot);
        }
        else if (index >= assignments.count())
        {
            // Append code into a new slot
            assignments.append(slot);
        }

        if (slot->getSlotMode() == JoyButtonSlot::JoyPause ||
            slot->getSlotMode() == JoyButtonSlot::JoyHold ||
            slot->getSlotMode() == JoyButtonSlot::JoyDistance ||
            slot->getSlotMode() == JoyButtonSlot::JoyRelease
           )
        {
            setUseTurbo(false);
        }

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

bool JoyButton::setAssignedSlot(JoyButtonSlot *newslot)
{
    bool slotInserted = false;

    if (newslot->getSlotMode() == JoyButtonSlot::JoyDistance)
    {
        if (newslot->getSlotCode() >= 1 && newslot->getSlotCode() <= 100)
        {
            double tempDistance = getTotalSlotDistance(newslot);
            if (tempDistance <= 1.0)
            {
                assignments.append(newslot);
                slotInserted = true;
            }
        }
    }
    else if (newslot->getSlotCode() >= 0)
    {
        assignments.append(newslot);
        slotInserted = true;
    }

    if (slotInserted)
    {
        if (newslot->getSlotMode() == JoyButtonSlot::JoyPause ||
            newslot->getSlotMode() == JoyButtonSlot::JoyHold ||
            newslot->getSlotMode() == JoyButtonSlot::JoyDistance ||
            newslot->getSlotMode() == JoyButtonSlot::JoyRelease
           )
        {
            setUseTurbo(false);
        }

        emit slotsChanged();
    }

    return slotInserted;
}

QList<JoyButtonSlot*>* JoyButton::getAssignedSlots()
{
    QList<JoyButtonSlot*> *newassign = new QList<JoyButtonSlot*> (assignments);
    return newassign;
}

void JoyButton::setMouseSpeedX(int speed)
{
    if (speed >= 1 && speed <= 300)
    {
        mouseSpeedX = speed;
        emit propertyUpdated();
    }
}

int JoyButton::getMouseSpeedX()
{
    return mouseSpeedX;
}

void JoyButton::setMouseSpeedY(int speed)
{
    if (speed >= 1 && speed <= 300)
    {
        mouseSpeedY = speed;
        emit propertyUpdated();
    }
}

int JoyButton::getMouseSpeedY()
{
    return mouseSpeedY;
}

void JoyButton::setChangeSetSelection(int index)
{
    if (index >= -1 && index <= 7)
    {
        setSelection = index;
        emit propertyUpdated();
    }
}

int JoyButton::getSetSelection()
{
    return setSelection;
}

void JoyButton::setChangeSetCondition(SetChangeCondition condition, bool passive)
{
    SetChangeCondition oldCondition = setSelectionCondition;

    if (condition != setSelectionCondition && !passive)
    {
        if (condition == SetChangeWhileHeld || condition == SetChangeTwoWay)
        {
            // Set new condition
            emit setAssignmentChanged(index, setSelection, condition);
        }
        else if (setSelectionCondition == SetChangeWhileHeld || setSelectionCondition == SetChangeTwoWay)
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
        emit propertyUpdated();
    }
}

JoyButton::SetChangeCondition JoyButton::getChangeSetCondition()
{
    return setSelectionCondition;
}

bool JoyButton::getButtonState()
{
    return isButtonPressed;
}

int JoyButton::getOriginSet()
{
    return originset;
}

void JoyButton::pauseWaitEvent()
{
    if (currentPause)
    {
        if (!isButtonPressedQueue.isEmpty() && createDeskTimer.isActive())
        {
            if (slotiter)
            {
                slotiter->toBack();

                bool lastIgnoreSetState = ignoreSetQueue.last();
                bool lastIsButtonPressed = isButtonPressedQueue.last();
                ignoreSetQueue.clear();
                isButtonPressedQueue.clear();

                ignoreSetQueue.enqueue(lastIgnoreSetState);
                isButtonPressedQueue.enqueue(lastIsButtonPressed);
                currentPause = 0;
                //JoyButtonSlot *oldCurrentRelease = currentRelease;
                currentRelease = 0;
                //createDeskTimer.stop();
                releaseDeskTimer.stop();
                pauseWaitTimer.stop();

                slotiter->toFront();
                if (previousCycle)
                {
                    slotiter->findNext(previousCycle);
                }
                quitEvent = true;
                keyPressHold.restart();
                //waitForDeskEvent();
                // Assuming that if this is the case, a pause from
                // a release slot was previously active. setChangeTimer
                // should not be active at this point.
                //if (oldCurrentRelease && setChangeTimer.isActive())
                //{
                //    setChangeTimer.stop();
                //}
            }
        }
    }

    if (currentPause)
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
            currentPause = 0;
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
    if (!ignoreSetQueue.isEmpty() && !isButtonPressedQueue.isEmpty())
    {
        bool tempFinalState = isButtonPressedQueue.last();
        bool tempFinalIgnoreSetsState = ignoreSetQueue.last();

        if (!tempFinalIgnoreSetsState)
        {
            if (!tempFinalState && setSelectionCondition == SetChangeOneWay && setSelection > -1)
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
            else if (!tempFinalState && setSelectionCondition == SetChangeTwoWay && setSelection > -1)
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
            else if (setSelectionCondition == SetChangeWhileHeld && setSelection > -1)
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
        /*else
        {
            createDeskTimer.start(0);
            releaseDeskTimer.stop();
            keyDelayHold.restart();
        }*/
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
    bool result = false;

    QListIterator<JoyButtonSlot*> tempiter(assignments);
    while (tempiter.hasNext())
    {
        JoyButtonSlot *slot = tempiter.next();
        JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();
        if (mode == JoyButtonSlot::JoyPause ||
            mode == JoyButtonSlot::JoyHold ||
            mode == JoyButtonSlot::JoyDistance
           )
        {
            result = true;
            tempiter.toBack();
        }
    }

    return result;
}

void JoyButton::holdEvent()
{
    if (currentHold)
    {
        bool currentlyPressed = false;
        if (!isButtonPressedQueue.isEmpty())
        {
            currentlyPressed = isButtonPressedQueue.last();
        }

        // Activate hold event
        if (currentlyPressed && buttonHold.elapsed() > currentHold->getSlotCode())
        {
            releaseActiveSlots();
            currentHold = 0;
            holdTimer.stop();
            buttonHold.restart();
            createDeskEvent();
        }
        // Elapsed time has not occurred
        else if (currentlyPressed)
        {
            unsigned int holdTime = currentHold->getSlotCode();
            int proposedInterval = holdTime - buttonHold.elapsed();
            proposedInterval = proposedInterval > 0 ? proposedInterval : 0;
            int newTimerInterval = qMin(10, proposedInterval);
            holdTimer.start(newTimerInterval);
        }
        // Pre-emptive release
        else
        {
            currentHold = 0;
            holdTimer.stop();

            if (slotiter)
            {
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
    if (currentDelay)
    {
        bool currentlyPressed = false;
        if (!isButtonPressedQueue.isEmpty())
        {
            currentlyPressed = isButtonPressedQueue.last();
        }

        // Delay time has elapsed. Continue processing slots.
        if (currentDelay && buttonDelay.elapsed() > currentDelay->getSlotCode())
        {
            currentDelay = 0;
            delayTimer.stop();
            buttonDelay.restart();
            createDeskEvent();
        }
        // Elapsed time has not occurred
        else if (currentlyPressed)
        {
            unsigned int delayTime = currentDelay->getSlotCode();
            int proposedInterval = delayTime - buttonDelay.elapsed();
            proposedInterval = proposedInterval > 0 ? proposedInterval : 0;
            int newTimerInterval = qMin(10, proposedInterval);
            delayTimer.start(newTimerInterval);
        }
        // Pre-emptive release
        else
        {
            currentDelay = 0;
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
    quitEvent = false;

    pauseWaitTimer.stop();
    holdTimer.stop();
    createDeskTimer.stop();
    keyPressTimer.stop();
    delayTimer.stop();
    keyRepeatTimer.stop();

    releaseActiveSlots();
    if (!isButtonPressedQueue.isEmpty() && !currentRelease)
    {
        releaseSlotEvent();
    }
    else if (currentRelease)
    {
        currentRelease = 0;
    }

    if (!skipsetchange && setSelectionCondition != SetChangeDisabled &&
        !isButtonPressedQueue.isEmpty() && !currentRelease)
    {
        bool tempButtonPressed = isButtonPressedQueue.last();
        bool tempFinalIgnoreSetsState = ignoreSetQueue.last();

        if (!tempButtonPressed && !tempFinalIgnoreSetsState)
        {
            if (setSelectionCondition == SetChangeWhileHeld && whileHeldStatus)
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
            if (tempFinalState || currentRelease)
            {
                isButtonPressedQueue.enqueue(tempFinalState);
            }
        }

        if (!ignoreSetQueue.isEmpty())
        {
            bool tempFinalIgnoreSetsState = ignoreSetQueue.last();
            ignoreSetQueue.clear();
            if (tempFinalState || currentRelease)
            {
                ignoreSetQueue.enqueue(tempFinalIgnoreSetsState);
            }
        }
    }

    if (!currentRelease)
    {
        if (slotiter && !slotiter->hasNext())
        {
            // At the end of the list of assignments.
            currentCycle = 0;
            previousCycle = 0;
            slotiter->toFront();
        }
        else if (slotiter && slotiter->hasNext() && currentCycle)
        {
            // Cycle at the end of a segment.
            slotiter->toFront();
            slotiter->findNext(currentCycle);
        }
        else if (slotiter && slotiter->hasPrevious() && slotiter->hasNext() && !currentCycle)
        {
            // Check if there is a cycle action slot after
            // current slot. Useful after dealing with pause
            // actions.
            JoyButtonSlot *tempslot = 0;
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
            if (!currentCycle)
            {
                slotiter->toFront();
                previousCycle = 0;
            }
        }

        if (currentCycle)
        {
            previousCycle = currentCycle;
            currentCycle = 0;
        }
        else if (slotiter && slotiter->hasNext() && containsReleaseSlots())
        {
            currentCycle = 0;
            previousCycle = 0;
            slotiter->toFront();
        }

        this->currentDistance = 0;
        this->currentKeyPress = 0;
        quitEvent = true;

        emit activeZoneChanged();
    }
}

/**
 * @brief Get the distance that an element is away from its assigned
 *     dead zone
 * @return Normalized distance away from dead zone
 */
double JoyButton::getDistanceFromDeadZone()
{
    double distance = 0.0;
    if (isButtonPressed)
    {
        distance = 1.0;
    }

    return distance;
}

/**
 * @brief Get the distance factor that should be used for mouse movement
 * @return Distance factor that should be used for mouse movement
 */
double JoyButton::getMouseDistanceFromDeadZone()
{
    return this->getDistanceFromDeadZone();
}

double JoyButton::getTotalSlotDistance(JoyButtonSlot *slot)
{
    double tempDistance = 0.0;

    QListIterator<JoyButtonSlot*> iter(assignments);
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
    bool result = false;
    QListIterator<JoyButtonSlot*> iter(assignments);
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
    QListIterator<JoyButtonSlot*> iter(assignments);
    while (iter.hasNext())
    {
        JoyButtonSlot *slot = iter.next();
        if (slot)
        {
            delete slot;
            slot = 0;
        }
    }

    assignments.clear();
    if (signalEmit)
    {
        emit slotsChanged();
    }
}

void JoyButton::removeAssignedSlot(int index)
{    
    if (index >= 0 && index < assignments.size())
    {
        JoyButtonSlot *slot = assignments.takeAt(index);
        if (slot)
        {
            delete slot;
            slot = 0;
        }

        emit slotsChanged();
    }
}

void JoyButton::clearSlotsEventReset(bool clearSignalEmit)
{
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
    keyRepeatTimer.stop();

    if (slotiter)
    {
        delete slotiter;
        slotiter = 0;
    }

    releaseDeskEvent(true);
    clearAssignedSlots(clearSignalEmit);

    isButtonPressedQueue.clear();
    ignoreSetQueue.clear();
    mouseEventQueue.clear();
    mouseWheelVerticalEventQueue.clear();
    mouseWheelHorizontalEventQueue.clear();

    currentCycle = 0;
    previousCycle = 0;
    currentPause = 0;
    currentHold = 0;
    currentDistance = 0;
    currentRawValue = 0;
    currentMouseEvent = 0;
    currentRelease = 0;
    currentWheelVerticalEvent = 0;
    currentWheelHorizontalEvent = 0;
    currentKeyPress = 0;
    currentDelay = 0;

    isKeyPressed = isButtonPressed = false;

    quitEvent = true;
}

void JoyButton::eventReset()
{
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
    keyRepeatTimer.stop();

    if (slotiter)
    {
        delete slotiter;
        slotiter = 0;
    }

    isButtonPressedQueue.clear();
    ignoreSetQueue.clear();
    mouseEventQueue.clear();
    mouseWheelVerticalEventQueue.clear();
    mouseWheelHorizontalEventQueue.clear();

    currentCycle = 0;
    previousCycle = 0;
    currentPause = 0;
    currentHold = 0;
    currentDistance = 0;
    currentRawValue = 0;
    currentMouseEvent = 0;
    currentRelease = 0;
    currentWheelVerticalEvent = 0;
    currentWheelHorizontalEvent = 0;
    currentKeyPress = 0;
    currentDelay = 0;

    isKeyPressed = isButtonPressed = false;

    releaseActiveSlots();

    quitEvent = true;
}

void JoyButton::releaseActiveSlots()
{
    if (!activeSlots.isEmpty())
    {
        QListIterator<JoyButtonSlot*> iter(activeSlots);

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
                }
                else
                {
                    activeKeys.insert(tempcode, referencecount);
                }

                if (lastActiveKey == slot)
                {
                    lastActiveKey = 0;
                }
            }
            else if (mode == JoyButtonSlot::JoyMouseButton)
            {
                if (tempcode != JoyButtonSlot::MouseWheelUp &&
                    tempcode != JoyButtonSlot::MouseWheelDown &&
                    tempcode != JoyButtonSlot::MouseWheelLeft &&
                    tempcode != JoyButtonSlot::MouseWheelRight)
                {
                    int referenceount = activeMouseButtons.value(tempcode, 1) - 1;
                    if (referenceount <= 0)
                    {
                        sendevent(slot, false);
                        activeMouseButtons.remove(tempcode);
                    }
                    else
                    {
                        activeMouseButtons.insert(tempcode, referenceount);
                    }
                }
                else if (tempcode == JoyButtonSlot::MouseWheelUp ||
                         tempcode == JoyButtonSlot::MouseWheelDown)
                {
                    mouseWheelVerticalEventQueue.removeAll(slot);
                }
                else if (tempcode == JoyButtonSlot::MouseWheelLeft ||
                         tempcode == JoyButtonSlot::MouseWheelRight)
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
                    unsigned int i = cursorXSpeeds.length();

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
                    double mouse1 = (tempcode == JoyButtonSlot::MouseLeft ||
                                     tempcode == JoyButtonSlot::MouseRight) ? 0.0 : -2.0;
                    double mouse2 = (tempcode == JoyButtonSlot::MouseUp ||
                                     tempcode == JoyButtonSlot::MouseDown) ? 0.0 : -2.0;

                    PadderCommon::springModeInfo infoX;
                    infoX.displacementX = mouse1;
                    infoX.displacementY = -2.0;
                    infoX.width = springWidth;
                    infoX.height = springHeight;
                    infoX.relative = relativeSpring;
                    springXSpeeds.append(infoX);

                    PadderCommon::springModeInfo infoY;
                    infoY.displacementX = -2.0;
                    infoY.displacementY = mouse2;
                    infoY.width = springWidth;
                    infoY.height = springHeight;
                    infoY.relative = relativeSpring;
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
        }

        activeSlots.clear();

        currentMouseEvent = 0;
        if (!mouseEventQueue.isEmpty())
        {
            mouseEventQueue.clear();
        }

        pendingMouseButtons.removeAll(this);
        currentWheelVerticalEvent = 0;
        currentWheelHorizontalEvent = 0;
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

        // Check if mouse event timer should be stopped.
        // Only need to check one list from cursor speeds and spring speeds
        // since the correspond Y lists will be the same size.
        if (pendingMouseButtons.length() == 0 && cursorXSpeeds.length() == 0 &&
            springXSpeeds.length() == 0)
        {
            lastMouseTime.restart();
            staticMouseEventTimer.start(IDLEMOUSEREFRESHRATE);
            /*staticMouseEventTimer.stop();
            mouseHistoryX.clear();
            mouseHistoryY.clear();
            for (int i=0; i < 10; i++)
            {
                mouseHistoryX.append(0);
            }
            for (int i=0; i < 10; i++)
            {
                mouseHistoryY.append(0);
            }
            */
            cursorRemainderX = 0;
            cursorRemainderY = 0;
        }


    }
}

bool JoyButton::containsReleaseSlots()
{
    bool result = false;
    QListIterator<JoyButtonSlot*> iter(assignments);
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
    JoyButtonSlot *temp = 0;

    int timeElapsed = buttonHeldRelease.elapsed();
    int tempElapsed = 0;

    if (containsReleaseSlots())
    {
        QListIterator<JoyButtonSlot*> iter(assignments);
        if (previousCycle)
        {
            iter.findNext(previousCycle);
        }

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

        if (temp && slotiter)
        {
            slotiter->toFront();
            slotiter->findNext(temp);
            currentRelease = temp;

            activateSlots();
            if (!keyPressTimer.isActive() && !pauseWaitTimer.isActive())
            {
                releaseActiveSlots();
                currentRelease = 0;
            }

            // Stop hold timer here to be sure that
            // a hold timer that could be activated
            // during a release event is stopped.
            holdTimer.stop();
            if (currentHold)
            {
                currentHold = 0;
            }
        }
    }
}

void JoyButton::findReleaseEventEnd()
{
    bool found = false;
    while (!found && slotiter->hasNext())
    {
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
    }
}

void JoyButton::findHoldEventEnd()
{
    bool found = false;

    while (!found && slotiter->hasNext())
    {
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
    }
}

void JoyButton::setVDPad(VDPad *vdpad)
{
    joyEvent(false, true);
    this->vdpad = vdpad;
    emit propertyUpdated();
}

bool JoyButton::isPartVDPad()
{
    return (this->vdpad != 0);
}

VDPad* JoyButton::getVDPad()
{
    return this->vdpad;
}

void JoyButton::removeVDPad()
{
    this->vdpad = 0;
    emit propertyUpdated();
}

/**
 * @brief Check if button properties are at their default values
 * @return Status of possible property edits
 */
bool JoyButton::isDefault()
{
    bool value = true;
    value = value && (toggle == DEFAULTTOGGLE);
    value = value && (turboInterval == DEFAULTTURBOINTERVAL);
    value = value && (currentTurboMode == NormalTurbo);
    value = value && (useTurbo == DEFAULTUSETURBO);
    value = value && (mouseSpeedX == DEFAULTMOUSESPEEDX);
    value = value && (mouseSpeedY == DEFAULTMOUSESPEEDY);
    value = value && (setSelection == DEFAULTSETSELECTION);
    value = value && (setSelectionCondition == DEFAULTSETCONDITION);
    value = value && (assignments.isEmpty());
    value = value && (mouseMode == DEFAULTMOUSEMODE);
    value = value && (mouseCurve == DEFAULTMOUSECURVE);
    value = value && (springWidth == DEFAULTSPRINGWIDTH);
    value = value && (springHeight == DEFAULTSPRINGHEIGHT);
    value = value && (sensitivity == DEFAULTSENSITIVITY);
    value = value && (smoothing == DEFAULTSMOOTHING);
    value = value && (actionName.isEmpty());
    //value = value && (buttonName.isEmpty());
    value = value && (wheelSpeedX == DEFAULTWHEELX);
    value = value && (wheelSpeedY == DEFAULTWHEELY);
    value = value && (cycleResetActive == DEFAULTCYCLERESETACTIVE);
    value = value && (cycleResetInterval == DEFAULTCYCLERESET);
    value = value && (relativeSpring == DEFAULTRELATIVESPRING);
    value = value && (easingDuration == DEFAULTEASINGDURATION);
    return value;
}

void JoyButton::setIgnoreEventState(bool ignore)
{
    ignoreEvents = ignore;
}

bool JoyButton::getIgnoreEventState()
{
    return ignoreEvents;
}

void JoyButton::setMouseMode(JoyMouseMovementMode mousemode)
{
    this->mouseMode = mousemode;
    emit propertyUpdated();
}

JoyButton::JoyMouseMovementMode JoyButton::getMouseMode()
{
    return mouseMode;
}

void JoyButton::setMouseCurve(JoyMouseCurve selectedCurve)
{
    mouseCurve = selectedCurve;
    emit propertyUpdated();
}

JoyButton::JoyMouseCurve JoyButton::getMouseCurve()
{
    return mouseCurve;
}

void JoyButton::setSpringWidth(int value)
{
    if (value >= 0)
    {
        springWidth = value;
        emit propertyUpdated();
    }
}

int JoyButton::getSpringWidth()
{
    return springWidth;
}

void JoyButton::setSpringHeight(int value)
{
    if (springHeight >= 0)
    {
        springHeight = value;
        emit propertyUpdated();
    }
}

int JoyButton::getSpringHeight()
{
    return springHeight;
}

void JoyButton::setSensitivity(double value)
{
    if (value >= 0.001 && value <= 1000)
    {
        sensitivity = value;
        emit propertyUpdated();
    }
}

double JoyButton::getSensitivity()
{
    return sensitivity;
}

void JoyButton::setSmoothing(bool enabled)
{
    smoothing = enabled;
    emit propertyUpdated();
}

bool JoyButton::isSmoothingEnabled()
{
    return smoothing;
}

bool JoyButton::getWhileHeldStatus()
{
    return whileHeldStatus;
}

void JoyButton::setWhileHeldStatus(bool status)
{
    whileHeldStatus = status;
}

void JoyButton::setActionName(QString tempName)
{
    if (tempName.length() <= 50 && tempName != actionName)
    {
        actionName = tempName;
        emit actionNameChanged();
        emit propertyUpdated();
    }
}

QString JoyButton::getActionName()
{
    return actionName;
}

void JoyButton::setButtonName(QString tempName)
{
    if (tempName.length() <= 20 && tempName != buttonName)
    {
        buttonName = tempName;
        emit buttonNameChanged();
        emit propertyUpdated();
    }
}

QString JoyButton::getButtonName()
{
    return buttonName;
}

void JoyButton::setWheelSpeedX(int speed)
{
    if (speed >= 1 && speed <= 100)
    {
        wheelSpeedX = speed;
        emit propertyUpdated();
    }
}

void JoyButton::setWheelSpeedY(int speed)
{
    if (speed >= 1 && speed <= 100)
    {
        wheelSpeedY = speed;
        emit propertyUpdated();
    }
}

int JoyButton::getWheelSpeedX()
{
    return wheelSpeedX;
}

int JoyButton::getWheelSpeedY()
{
    return wheelSpeedY;
}

void JoyButton::setDefaultButtonName(QString tempname)
{
    defaultButtonName = tempname;
}

QString JoyButton::getDefaultButtonName()
{
    return defaultButtonName;
}

/**
 * @brief Take cursor mouse information provided by all buttons and
 *     send a cursor mode mouse event to the display server.
 */
void JoyButton::moveMouseCursor(int &movedX, int &movedY, int &movedElapsed)
{
    movedX = 0;
    movedY = 0;
    double finalx = 0.0;
    double finaly = 0.0;
    int elapsedTime = lastMouseTime.elapsed();
    movedElapsed = lastMouseTime.elapsed();

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
    if (cursorXSpeeds.length() == cursorYSpeeds.length() &&
        cursorXSpeeds.length() > 0)
    {
        int queueLength = cursorXSpeeds.length();
        for (int i=0; i < queueLength; i++)
        {
            mouseCursorInfo infoX = cursorXSpeeds.takeFirst();
            mouseCursorInfo infoY = cursorYSpeeds.takeFirst();
            finalx += infoX.code;
            finaly += infoY.code;

            infoX.slot->getMouseInterval()->restart();
            infoY.slot->getMouseInterval()->restart();
        }

        finalx += cursorRemainderX;
        mouseHistoryX.prepend(finalx);
        finaly += cursorRemainderY;
        mouseHistoryY.prepend(finaly);

        cursorRemainderX = 0;
        cursorRemainderY = 0;

        double adjustedX = 0;
        double adjustedY = 0;

        QListIterator<double> iterX(mouseHistoryX);
        double currentWeight = 1.0;
        double weightModifier = JoyButton::weightModifier;
        double finalWeight = 0.0;

        //qDebug() << "OG: " << finalx;
        while (iterX.hasNext())
        {
            double temp = iterX.next();
            //qDebug() << "SAMPLE: " << temp;
            adjustedX += temp * currentWeight;
            finalWeight += currentWeight;
            currentWeight *= weightModifier;
        }

        //qDebug();
        //qDebug() << "X TO THE Z: " << adjustedX;
        if (fabs(adjustedX) > 0)
        {
            adjustedX = adjustedX / (double)finalWeight;

            if (adjustedX > 0)
            {
                //adjustedX = adjustedX + remainderX;
                double oldX = adjustedX;
                adjustedX = (int)floor(adjustedX);
                cursorRemainderX = oldX - adjustedX;
            }
            else
            {
                //adjustedX = adjustedX + remainderX;
                double oldX = adjustedX;
                adjustedX = (int)ceil(adjustedX);
                //adjustedX = (int)(adjustedX - 0.5);
                cursorRemainderX = oldX - adjustedX;
            }

        }
        //qDebug() << "FINAL X TO THE Z: " << adjustedX;

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
            adjustedY = adjustedY / (double)finalWeight;
            if (adjustedY > 0)
            {
                //adjustedY = adjustedY + remainderY;
                double oldY = adjustedY;
                adjustedY = (int)floor(adjustedY);
                cursorRemainderY = oldY - adjustedY;
            }
            else
            {
                //adjustedY = adjustedY + remainderY;
                double oldY = adjustedY;
                adjustedY = (int)ceil(adjustedY);
                cursorRemainderY = oldY - adjustedY;
            }
        }

        sendevent(adjustedX, adjustedY);
        //qDebug() << "FINAL X: " << finalx;
        //qDebug() << "FINAL Y: " << finaly;
        //qDebug() << "ELAPSED: " << elapsedTime;
        //qDebug() << "REMAINDER X: " << remainderX;

        movedX = (int)adjustedX;
        movedY = (int)adjustedY;
        movedElapsed = elapsedTime;
    }
    else
    {
        mouseHistoryX.prepend(0);
        mouseHistoryY.prepend(0);
    }

    lastMouseTime.restart();

    // Check if mouse event timer should be stopped.
    if (pendingMouseButtons.length() == 0)
    {
        staticMouseEventTimer.start(IDLEMOUSEREFRESHRATE);
        //staticMouseEventTimer.stop();
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
    PadderCommon::springModeInfo fullSpring = {
        -2.0, -2.0, 0, 0, false
    };
    PadderCommon::springModeInfo relativeSpring = {
        -2.0, -2.0, 0, 0, false
    };

    int realMouseX = movedX = 0;
    int realMouseY = movedY = 0;
    hasMoved = false;

    if (springXSpeeds.length() == springYSpeeds.length() &&
        springXSpeeds.length() > 0)
    {
        int queueLength = springXSpeeds.length();
        bool complete = false;
        for (int i=queueLength-1; i >= 0 && !complete; i--)
        {
            double tempx = -2.0;
            double tempy = -2.0;

            PadderCommon::springModeInfo infoX;
            PadderCommon::springModeInfo infoY;

            infoX = springXSpeeds.takeLast();
            infoY = springYSpeeds.takeLast();

            tempx = infoX.displacementX;
            tempy = infoY.displacementY;

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
                // Use largest found height for spring
                // mode dimensions.
                fullSpring.height = qMax(infoX.height, fullSpring.height);
            }

            if ((relativeSpring.displacementX != -2.0 && relativeSpring.displacementY != -2.0) &&
                (fullSpring.displacementX != -2.0 && fullSpring.displacementY != -2.0))
            {
                complete = true;
            }
        }

        if (relativeSpring.relative)
        {
            sendSpringEvent(&fullSpring, &relativeSpring, &realMouseX, &realMouseY);
        }
        else
        {
            sendSpringEvent(&fullSpring, 0, &realMouseX, &realMouseY);
        }

        movedX = realMouseX;
        movedY = realMouseY;
        hasMoved = true;

    }

    lastMouseTime.restart();

    // Check if mouse event timer should be stopped.
    if (pendingMouseButtons.length() == 0)
    {
        staticMouseEventTimer.start(IDLEMOUSEREFRESHRATE);
        //staticMouseEventTimer.stop();
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
    //qDebug() << "RADIO EDIT: " << keyDelayHold.elapsed();
    if (keyPressTimer.isActive() && keyPressHold.elapsed() >= getPreferredKeyPressTime())
    {
        currentKeyPress = 0;

        keyPressTimer.stop();
        keyPressHold.restart();
        releaseActiveSlots();

        createDeskTimer.stop();

        if (currentRelease)
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
        //releaseDeskTimer.stop();

        unsigned int preferredDelay = getPreferredKeyPressTime();
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
    bool result = false;

    QListIterator<JoyButtonSlot*> tempiter(assignments);

    // Move iterator to start of cycle.
    if (previousCycle)
    {
        tempiter.findNext(previousCycle);
    }

    while (tempiter.hasNext())
    {
        JoyButtonSlot *slot = tempiter.next();
        JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();
        if (mode == JoyButtonSlot::JoyPause || mode == JoyButtonSlot::JoyRelease)
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
    return parentSet;
}

void JoyButton::checkForPressedSetChange()
{
    if (!isButtonPressedQueue.isEmpty())
    {
        bool tempButtonPressed = isButtonPressedQueue.last();
        bool tempFinalIgnoreSetsState = ignoreSetQueue.last();

        if (!whileHeldStatus)
        {
            if (tempButtonPressed && !tempFinalIgnoreSetsState &&
                setSelectionCondition == SetChangeWhileHeld && !currentRelease)
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
unsigned int JoyButton::getPreferredKeyPressTime()
{
    unsigned int tempPressTime = InputDevice::DEFAULTKEYPRESSTIME;
    if (currentKeyPress && currentKeyPress->getSlotCode() > 0)
    {
        tempPressTime = (unsigned int)currentKeyPress->getSlotCode();
    }
    else if (parentSet->getInputDevice()->getDeviceKeyPressTime() > 0)
    {
        tempPressTime = parentSet->getInputDevice()->getDeviceKeyPressTime();
    }

    return tempPressTime;
}

void JoyButton::setCycleResetTime(unsigned int interval)
{
    if (interval >= 10)
    {
        unsigned int ceiling = 6000;
        unsigned int temp = qMax(interval, ceiling);
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

unsigned int JoyButton::getCycleResetTime()
{
    return cycleResetInterval;
}

void JoyButton::setCycleResetStatus(bool enabled)
{
    cycleResetActive = enabled;
    emit propertyUpdated();
}

bool JoyButton::isCycleResetActive()
{
    return cycleResetActive;
}

/**
 * @brief Send key event needed to fake key repeating in Windows.
 *     Method is not used in Linux as key repeating is taken care of
 *     by X instead.
 */
void JoyButton::repeatKeysEvent()
{
    if (!activeSlots.isEmpty() && lastActiveKey && activeSlots.contains(lastActiveKey))
    {
        JoyButtonSlot *slot = lastActiveKey;

        // Send another key press to fake a key repeat
        sendevent(slot);

        InputDevice *device = getParentSet()->getInputDevice();
        keyRepeatTimer.start(device->getKeyRepeatRate());
    }
    else
    {
        keyRepeatTimer.stop();
    }
}

void JoyButton::establishPropertyUpdatedConnections()
{
    connect(this, SIGNAL(slotsChanged()), parentSet->getInputDevice(), SLOT(profileEdited()));
    connect(this, SIGNAL(propertyUpdated()), parentSet->getInputDevice(), SLOT(profileEdited()));
}

void JoyButton::disconnectPropertyUpdatedConnections()
{
    disconnect(this, SIGNAL(slotsChanged()), 0, 0);
    disconnect(this, SIGNAL(propertyUpdated()), parentSet->getInputDevice(), SLOT(profileEdited()));
}

void JoyButton::establishMouseTimerConnections()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (staticMouseEventTimer.timerType() != Qt::PreciseTimer)
    {
        staticMouseEventTimer.setTimerType(Qt::PreciseTimer);
    }
#endif

    // Only one connection will be made for each.
    connect(&staticMouseEventTimer, SIGNAL(timeout()), &mouseHelper, SLOT(mouseEvent()), Qt::UniqueConnection);
    lastMouseTime.start();
    staticMouseEventTimer.start(IDLEMOUSEREFRESHRATE);
}

void JoyButton::setSpringRelativeStatus(bool value)
{
    relativeSpring = value;
}

bool JoyButton::isRelativeSpring()
{
    return relativeSpring;
}

/**
 * @brief Copy assignments and properties from one button to another.
 *     Used for set copying.
 * @param Button instance that should be modified.
 */
void JoyButton::copyAssignments(JoyButton *destButton)
{
    destButton->eventReset();
    destButton->assignments.clear();
    QListIterator<JoyButtonSlot*> iter(assignments);
    while (iter.hasNext())
    {
        JoyButtonSlot *slot = iter.next();
        JoyButtonSlot *newslot = new JoyButtonSlot(slot, destButton);
        destButton->setAssignedSlot(newslot);
        //destButton->assignments.append(slot);
    }

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
    destButton->smoothing = smoothing;
    //destButton->setSelection = setSelection;
    //destButton->setSelectionCondition = setSelectionCondition;
    destButton->buttonName = buttonName;
    destButton->actionName = actionName;
    destButton->cycleResetActive = cycleResetActive;
    destButton->cycleResetInterval = cycleResetInterval;
    destButton->relativeSpring = relativeSpring;
    destButton->currentTurboMode = currentTurboMode;
    destButton->easingDuration = easingDuration;
}

/**
 * @brief Set the turbo mode that the button should use
 * @param Mode that should be used
 */
void JoyButton::setTurboMode(TurboMode mode)
{
    currentTurboMode = mode;
}

/**
 * @brief Get currently assigned turbo mode
 * @return Currently assigned turbo mode
 */
JoyButton::TurboMode JoyButton::getTurboMode()
{
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
    }

    return result;
}

void JoyButton::setEasingDuration(double value)
{
    if (value >= MINIMUMEASINGDURATION && value <= 2.0 &&
        value != easingDuration)
    {
        easingDuration = value;
        emit propertyUpdated();
    }
}

double JoyButton::getEasingDuration()
{
    return easingDuration;
}

JoyButtonMouseHelper* JoyButton::getMouseHelper()
{
    return &mouseHelper;
}

QList<JoyButton*>* JoyButton::getPendingMouseButtons()
{
    return &pendingMouseButtons;
}

bool JoyButton::hasCursorEvents()
{
    return (cursorXSpeeds.length() != 0) || (cursorYSpeeds.length() != 0);
}

bool JoyButton::hasSpringEvents()
{
    return (springXSpeeds.length() != 0) || (springYSpeeds.length() != 0);
}

double JoyButton::getWeightModifier()
{
    return weightModifier;
}

void JoyButton::setWeightModifier(double modifier)
{
    if (modifier >= 0.0 && modifier <= MAXIMUMWEIGHTMODIFIER)
    {
        weightModifier = modifier;
    }
}

int JoyButton::getMouseHistorySize()
{
    return mouseHistorySize;
}

void JoyButton::setMouseHistorySize(int size)
{
    if (size >= 1 && size <= MAXIMUMMOUSEHISTORYSIZE)
    {
        mouseHistoryX.clear();
        mouseHistoryY.clear();

        mouseHistorySize = size;
    }
}


int JoyButton::getMouseRefreshRate()
{
    return mouseRefreshRate;
}

void JoyButton::setMouseRefreshRate(int refresh)
{
    if (refresh >= 1 && refresh <= 16)
    {
        mouseRefreshRate = refresh;
        if (staticMouseEventTimer.isActive())
        {
            lastMouseTime.restart();
            staticMouseEventTimer.start(mouseRefreshRate);

            // Clear current mouse history
            mouseHistoryX.clear();
            mouseHistoryY.clear();
        }
    }
}
