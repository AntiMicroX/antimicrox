#include <QDebug>
#include <QStringList>
#include <cmath>

#include "joybutton.h"
#include "vdpad.h"
#include "event.h"

const QString JoyButton::xmlName = "button";
const int JoyButton::ENABLEDTURBODEFAULT = 100;
const double JoyButton::SMOOTHINGFACTOR = 0.85;
const double JoyButton::DEFAULTMOUSESPEEDMOD = 1.0;
double JoyButton::mouseSpeedModifier = JoyButton::DEFAULTMOUSESPEEDMOD;
QList<JoyButtonSlot*> JoyButton::mouseSpeedModList;

JoyButton::JoyButton(int index, int originset, QObject *parent) :
    QObject(parent)
{
    vdpad = 0;
    slotiter = 0;
    connect(&pauseTimer, SIGNAL(timeout()), this, SLOT(pauseEvent()));
    connect(&pauseWaitTimer, SIGNAL(timeout()), this, SLOT(pauseWaitEvent()));
    connect(&holdTimer, SIGNAL(timeout()), this, SLOT(holdEvent()));
    connect(&createDeskTimer, SIGNAL(timeout()), this, SLOT(waitForDeskEvent()));
    connect(&releaseDeskTimer, SIGNAL(timeout()), this, SLOT(waitForReleaseDeskEvent()));
    connect(&mouseEventTimer, SIGNAL(timeout()), this, SLOT(mouseEvent()));
    connect(&turboTimer, SIGNAL(timeout()), this, SLOT(turboEvent()));
    connect(&mouseWheelEventTimer, SIGNAL(timeout()), this, SLOT(wheelEvent()));

    this->reset();
    this->index = index;
    this->originset = originset;

    quitEvent = true;
}

JoyButton::~JoyButton()
{
    if (!isButtonPressedQueue.isEmpty() && isButtonPressedQueue.last())
    {
        emit released(index);
    }

    reset();
}

void JoyButton::joyEvent(bool pressed, bool ignoresets)
{
    if (this->vdpad)
    {
        if (pressed != isButtonPressed)
        {
            isButtonPressed = pressed;
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
                emit clicked(index);
            }
            else
            {
                emit released(index);
            }

            bool activePress = pressed;

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
                    buttonHold.restart();
                    buttonHeldRelease.restart();
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
                }
            }
            // Toogle is enabled and a controller button change has occurred.
            // Switch to a different distance zone if appropriate
            else if (toggle && !activePress && isButtonPressed)
            {
                bool releasedCalled = distanceEvent();
                if (releasedCalled)
                {
                    buttonHold.restart();
                    buttonHeldRelease.restart();
                    //createDeskTimer.start(0);
                    waitForDeskEvent();
                }
            }
            else if (isButtonPressed && activePress)
            {
                buttonHold.restart();
                buttonHeldRelease.restart();
                //createDeskTimer.start(0);
                waitForDeskEvent();
            }
            else if (!isButtonPressed && !activePress)
            {
                //releaseDeskTimer.start(0);
                waitForReleaseDeskEvent();
            }
        }
        else if (!useTurbo && isButtonPressed)
        {
            bool releasedCalled = distanceEvent();
            if (releasedCalled)
            {
                buttonHold.restart();
                buttonHeldRelease.restart();
                //createDeskTimer.start(0);
                waitForDeskEvent();
            }
        }
    }
}

int JoyButton::getJoyNumber()
{
    return index;
}

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
    }
}

void JoyButton::setTurboInterval(int interval)
{
    if (interval >= 10 && interval != this->turboInterval)
    {
        this->turboInterval = interval;
        emit turboIntervalChanged(interval);
    }
    else if (interval < 10 && interval != this->turboInterval)
    {
        interval = 0;
        this->setUseTurbo(false);
        this->turboInterval = interval;
        emit turboIntervalChanged(interval);
    }
}

void JoyButton::reset()
{
    turboTimer.stop();
    pauseTimer.stop();
    pauseWaitTimer.stop();
    createDeskTimer.stop();
    releaseDeskTimer.stop();
    mouseEventTimer.stop();
    holdTimer.stop();
    mouseWheelEventTimer.stop();

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

    currentCycle = 0;
    previousCycle = 0;
    currentPause = 0;
    currentHold = 0;
    currentDistance = 0;
    currentRawValue = 0;
    currentMouseEvent = 0;
    currentRelease = 0;
    currentWheelEvent = 0;

    isKeyPressed = isButtonPressed = false;
    toggle = false;
    turboInterval = 0;
    isDown = false;
    toggleActiveState = false;
    useTurbo = false;
    mouseSpeedX = 50;
    mouseSpeedY = 50;
    wheelSpeed = 20;
    mouseMode = MouseCursor;
    mouseCurve = LinearCurve;
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
            turboTimer.start(10);
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
            turboTimer.start(turboInterval - 10);
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
                    pauseTimer.stop();
                    pauseWaitTimer.stop();
                    holdTimer.stop();

                    // Release stuff
                    releaseActiveSlots();
                    currentPause = currentHold = 0;

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
                    pauseTimer.stop();
                    pauseWaitTimer.stop();
                    holdTimer.stop();

                    // Release stuff
                    releaseActiveSlots();
                    currentPause = currentHold = 0;

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

    if (!isButtonPressedQueue.isEmpty())
    {
        bool tempButtonPressed = isButtonPressedQueue.last();
        bool tempFinalIgnoreSetsState = ignoreSetQueue.last();
        if (tempButtonPressed && !tempFinalIgnoreSetsState && setSelectionCondition == SetChangeWhileHeld)
        {
            QTimer::singleShot(0, this, SLOT(checkForSetChange()));
            quitEvent = true;
        }
    }

    if (!quitEvent)
    {
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
        else if (!currentPause && !currentHold)
        {
            quitEvent = true;
        }
    }
}

void JoyButton::activateSlots()
{
    if (slotiter)
    {
        bool exit = false;

        while (slotiter->hasNext() && !exit)
        {
            JoyButtonSlot *slot = slotiter->next();
            int tempcode = slot->getSlotCode();
            JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();

            if (mode == JoyButtonSlot::JoyKeyboard)
            {
                sendevent(tempcode, true, mode);
                activeSlots.append(slot);
            }
            else if (mode == JoyButtonSlot::JoyMouseButton)
            {
                if (tempcode != JoyButtonSlot::MouseWheelUp &&
                    tempcode != JoyButtonSlot::MouseWheelDown)
                {
                    sendevent(tempcode, true, mode);
                    activeSlots.append(slot);
                }
                else
                {
                    slot->getMouseInterval()->restart();
                    currentWheelEvent = slot;
                    activeSlots.append(slot);
                    wheelEvent();
                    currentWheelEvent = 0;
                }

            }
            else if (mode == JoyButtonSlot::JoyMouseMovement)
            {
                slot->getMouseInterval()->restart();
                currentMouseEvent = slot;
                activeSlots.append(slot);
                mouseEvent();
                currentMouseEvent = 0;
            }
            else if (mode == JoyButtonSlot::JoyPause)
            {
                currentPause = slot;
                pauseHold.restart();
                pauseTimer.start(0);
                exit = true;
            }
            else if (mode == JoyButtonSlot::JoyHold)
            {
                currentHold = slot;
                holdTimer.start(0);
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
                else
                {
                    currentRelease = 0;
                    exit = true;
                }
            }
            else if (mode == JoyButtonSlot::JoyMouseSpeedMod)
            {
                mouseSpeedModifier = tempcode * 0.01;
                mouseSpeedModList.append(slot);
                activeSlots.append(slot);
            }
        }
    }
}

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
            QTime* mouseInterval = buttonslot->getMouseInterval();

            int mousedirection = buttonslot->getSlotCode();
            JoyButton::JoyMouseMovementMode mousemode = getMouseMode();
            int mousespeed = 0;
            int timeElapsed = mouseInterval->elapsed();

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

                    double difference = getDistanceFromDeadZone();
                    int mouse1 = 0;
                    int mouse2 = 0;
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
                        default:
                        {
                            break;
                        }
                    }

                    int distance = 0;
                    difference = (mouseSpeedModifier == 1.0) ? difference : (difference * mouseSpeedModifier);

                    if (mousedirection == JoyButtonSlot::MouseRight)
                    {
                        sumDist += difference * (mousespeed * JoyButtonSlot::JOYSPEED * timeElapsed) * 0.001;
                        distance = (int)floor(sumDist + 0.5);
                        mouse1 = distance;
                    }
                    else if (mousedirection == JoyButtonSlot::MouseLeft)
                    {
                        sumDist += difference * (mousespeed * JoyButtonSlot::JOYSPEED * timeElapsed) * 0.001;
                        distance = (int)floor(sumDist + 0.5);
                        mouse1 = -distance;
                    }
                    else if (mousedirection == JoyButtonSlot::MouseDown)
                    {
                        sumDist += difference * (mousespeed * JoyButtonSlot::JOYSPEED * timeElapsed) * 0.001;
                        distance = (int)floor(sumDist + 0.5);
                        mouse2 = distance;
                    }
                    else if (mousedirection == JoyButtonSlot::MouseUp)
                    {
                        sumDist += difference * (mousespeed * JoyButtonSlot::JOYSPEED * timeElapsed) * 0.001;
                        distance = (int)floor(sumDist + 0.5);
                        mouse2 = -distance;
                    }

                    if (distance >= 1)
                    {
                        sendevent(mouse1, mouse2);
                        sumDist -= distance;
                        if (smoothing)
                        {
                            sumDist *= SMOOTHINGFACTOR;
                        }
                        mouseInterval->restart();
                        mouseEventTimer.stop();
                    }

                    buttonslot->setDistance(sumDist);
                }
                else if (mousemode == JoyButton::MouseSpring)
                {
                    double mouse1 = -2.0;
                    double mouse2 = -2.0;
                    double difference = getDistanceFromDeadZone();
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
                    sendSpringEvent(mouse1, mouse2, springWidth, springHeight);
                    mouseInterval->restart();
                    mouseEventTimer.stop();
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

            if (!mouseEventTimer.isActive())
            {
                mouseEventTimer.start(5);
            }
        }
        else
        {
            mouseEventTimer.stop();
        }
    }
    else
    {
        mouseEventTimer.stop();
    }
}

void JoyButton::wheelEvent()
{
    JoyButtonSlot *buttonslot = 0;
    if (currentWheelEvent)
    {
        buttonslot = currentWheelEvent;
    }

    if (buttonslot && wheelSpeed != 0)
    {
        bool isActive = activeSlots.contains(buttonslot);
        if (isActive)
        {
            sendevent(buttonslot->getSlotCode(), true, buttonslot->getSlotMode());
            sendevent(buttonslot->getSlotCode(), false, buttonslot->getSlotMode());
            mouseWheelEventQueue.enqueue(buttonslot);
            mouseWheelEventTimer.start(1000 / wheelSpeed);
        }
        else
        {
            mouseWheelEventTimer.stop();
        }
    }
    else if (!mouseWheelEventQueue.isEmpty() && wheelSpeed != 0)
    {
        QQueue<JoyButtonSlot*> tempQueue;
        while (!mouseWheelEventQueue.isEmpty())
        {
            buttonslot = mouseWheelEventQueue.dequeue();
            bool isActive = activeSlots.contains(buttonslot);
            if (isActive)
            {
                sendevent(buttonslot->getSlotCode(), true, buttonslot->getSlotMode());
                sendevent(buttonslot->getSlotCode(), false, buttonslot->getSlotMode());
                tempQueue.enqueue(buttonslot);
            }
        }

        if (!tempQueue.isEmpty())
        {
            mouseWheelEventQueue = tempQueue;
            mouseWheelEventTimer.start(1000 / wheelSpeed);
        }
        else
        {
            mouseWheelEventTimer.stop();
        }
    }
    else
    {
        mouseWheelEventTimer.stop();
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
        //reset();

        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != getXmlName()))
        {
            if (xml->name() == "toggle" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                if (temptext == "true")
                {
                    this->setToggle(true);
                }
            }
            else if (xml->name() == "turbointerval" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setTurboInterval(tempchoice);
            }
            else if (xml->name() == "useturbo" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                if (temptext == "true")
                {
                    this->setUseTurbo(true);
                }
            }
            else if (xml->name() == "mousespeedx" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setMouseSpeedX(tempchoice);
            }
            else if (xml->name() == "mousespeedy" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setMouseSpeedY(tempchoice);
            }
            else if (xml->name() == "slots" && xml->isStartElement())
            {
                xml->readNextStartElement();
                while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != "slots"))
                {
                    if (xml->name() == "slot" && xml->isStartElement())
                    {
                        JoyButtonSlot *buttonslot = new JoyButtonSlot();
                        buttonslot->readConfig(xml);
                        setAssignedSlot(buttonslot->getSlotCode(), buttonslot->getSlotMode());
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
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                if (tempchoice >= 1 && tempchoice <= 8)
                {
                    this->setChangeSetSelection(tempchoice - 1);
                }
            }
            else if (xml->name() == "setselectcondition" && xml->isStartElement())
            {
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
            }
            else if (xml->name() == "mousespringwidth" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                setSpringWidth(tempchoice);
            }
            else if (xml->name() == "mousespringheight" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                setSpringHeight(tempchoice);
            }
            else if (xml->name() == "mousesensitivity" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                double tempchoice = temptext.toDouble();
                setSensitivity(tempchoice);
            }
            else if (xml->name() == "mousesmoothing" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                if (temptext == "true")
                {
                    setSmoothing(true);
                }
            }
            else if (xml->name() == "actionname" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                if (!temptext.isEmpty())
                {
                    setActionName(temptext);
                }
            }
            else if (xml->name() == "wheelspeed" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                setWheelSpeed(tempchoice);
            }
            else
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

        xml->writeTextElement("toggle", toggle ? "true" : "false");
        xml->writeTextElement("turbointerval", QString::number(turboInterval));
        xml->writeTextElement("useturbo", useTurbo ? "true" : "false");
        xml->writeTextElement("mousespeedx", QString::number(mouseSpeedX));
        xml->writeTextElement("mousespeedy", QString::number(mouseSpeedY));

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

        xml->writeTextElement("mousesmoothing", smoothing ? "true" : "false");
        xml->writeTextElement("wheelspeed", QString::number(wheelSpeed));

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

        xml->writeStartElement("slots");
        QListIterator<JoyButtonSlot*> iter(assignments);
        while (iter.hasNext())
        {
            JoyButtonSlot *buttonslot = iter.next();
            buttonslot->writeConfig(xml);
        }
        xml->writeEndElement();

        xml->writeEndElement();
    }
}

QString JoyButton::getName(bool forceFullFormat)
{
    QString newlabel = getPartialName(forceFullFormat);
    newlabel.append(": ");
    if (!actionName.isEmpty())
    {
        newlabel.append(actionName);
    }
    else
    {
        newlabel.append(getSlotsSummary());
    }
    return newlabel;
}

QString JoyButton::getPartialName(bool forceFullFormat)
{
    QString temp;
    if (!buttonName.isEmpty())
    {
        if (forceFullFormat)
        {
            temp.append(tr("Button")).append(" ");
        }
        temp.append(buttonName);
    }
    else
    {
        temp.append(tr("Button")).append(" ").append(QString::number(getRealJoyNumber()));
    }

    return temp;
}

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

            if (i > 4)
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

void JoyButton::setAssignedSlot(int code, JoyButtonSlot::JoySlotInputAction mode)
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
    else if (slot->getSlotCode() > 0)
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
}

void JoyButton::setAssignedSlot(int code, int index, JoyButtonSlot::JoySlotInputAction mode)
{
    bool permitSlot = true;

    JoyButtonSlot *slot = new JoyButtonSlot(code, mode, this);
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
    else if (slot->getSlotCode() <= 0)
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
    }
}

int JoyButton::getMouseSpeedY()
{
    return mouseSpeedY;
}

void JoyButton::setChangeSetSelection(int index)
{
    if (index >= 0 && index <= 7)
    {
        setSelection = index;
    }
}

int JoyButton::getSetSelection()
{
    return setSelection;
}

void JoyButton::setChangeSetCondition(SetChangeCondition condition, bool passive)
{
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

void JoyButton::pauseEvent()
{
    if (currentPause)
    {
        if (pauseHold.elapsed() > 100)
        {
            releaseActiveSlots();
            inpauseHold.restart();
            pauseTimer.stop();
            pauseWaitTimer.start(0);
        }
        else
        {
            pauseTimer.start(10);
        }
    }
    else
    {
        pauseTimer.stop();
        pauseWaitTimer.stop();
    }
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
                currentRelease = 0;
                createDeskTimer.stop();
                releaseDeskTimer.stop();
                pauseWaitTimer.stop();

                slotiter->toFront();
                if (previousCycle)
                {
                    slotiter->findNext(previousCycle);
                }
                quitEvent = true;
                waitForDeskEvent();
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
            pauseWaitTimer.start(10);
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
            createDeskTimer.stop();
        }
        createDeskEvent();
    }
    else if (!createDeskTimer.isActive())
    {
#ifdef Q_CC_MSVC
        createDeskTimer.start(1);
#else
        createDeskTimer.start(0);
#endif
    }
}

void JoyButton::waitForReleaseDeskEvent()
{
    if (quitEvent)
    {
        if (releaseDeskTimer.isActive())
        {
            releaseDeskTimer.stop();
        }
        releaseDeskEvent();
    }
    else if (!releaseDeskTimer.isActive())
    {
#ifdef Q_CC_MSVC
        releaseDeskTimer.start(1);
#else
        releaseDeskTimer.start(0);
#endif
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
            holdTimer.start(10);
        }
        // Pre-emptive release
        else
        {
            holdTimer.stop();

            if (slotiter)
            {
                findHoldEventEnd();
                currentHold = 0;
                createDeskEvent();
            }
        }
    }
    else
    {
        holdTimer.stop();
    }
}

void JoyButton::releaseDeskEvent(bool skipsetchange)
{
    quitEvent = false;

    pauseTimer.stop();
    pauseWaitTimer.stop();
    holdTimer.stop();

    releaseActiveSlots();
    if (!isButtonPressedQueue.isEmpty() && !currentRelease)
    {
        releaseSlotEvent();
    }
    else if (currentRelease)
    {
        currentRelease = 0;
    }

    if (!skipsetchange && !isButtonPressedQueue.isEmpty() && !currentRelease)
    {
        bool tempButtonPressed = isButtonPressedQueue.last();
        if (!tempButtonPressed)
        {
            QTimer::singleShot(0, this, SLOT(checkForSetChange()));
            // If createDeskTimer is currently active,
            // a rapid press was detected before a release
            // started. Restart timer so checkForSetChange
            // happens first
            if (createDeskTimer.isActive())
            {
                createDeskTimer.start();
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
        currentRelease = 0;
        quitEvent = true;
    }
    else
    {
        createDeskTimer.stop();
    }
}

double JoyButton::getDistanceFromDeadZone()
{
    double distance = 0.0;
    if (isButtonPressed)
    {
        distance = 1.0;
    }

    return distance;
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

void JoyButton::clearAssignedSlots()
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
    emit slotsChanged();
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

void JoyButton::clearSlotsEventReset()
{
    turboTimer.stop();
    pauseTimer.stop();
    pauseWaitTimer.stop();
    createDeskTimer.stop();
    releaseDeskTimer.stop();
    mouseEventTimer.stop();
    holdTimer.stop();

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

    currentCycle = 0;
    previousCycle = 0;
    currentPause = 0;
    currentHold = 0;
    currentDistance = 0;
    currentRawValue = 0;
    currentMouseEvent = 0;

    isKeyPressed = isButtonPressed = false;
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
                sendevent(tempcode, false, mode);
            }
            else if (mode == JoyButtonSlot::JoyMouseButton)
            {
                if (tempcode != JoyButtonSlot::MouseWheelUp &&
                    tempcode != JoyButtonSlot::MouseWheelDown)
                {
                    sendevent(tempcode, false, mode);
                }

                slot->setDistance(0.0);
                slot->getMouseInterval()->restart();
            }
            else if (mode == JoyButtonSlot::JoyMouseMovement)
            {
                JoyMouseMovementMode mousemode = getMouseMode();
                if (mousemode == JoyButton::MouseSpring)
                {
                    double mouse1 = (tempcode == JoyButtonSlot::MouseLeft ||
                                     tempcode == JoyButtonSlot::MouseRight) ? 0.0 : -2.0;
                    double mouse2 = (tempcode == JoyButtonSlot::MouseUp ||
                                     tempcode == JoyButtonSlot::MouseDown) ? 0.0 : -2.0;
                    sendSpringEvent(mouse1, mouse2);
                }
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

        mouseEventTimer.stop();
        currentMouseEvent = 0;
        if (!mouseEventQueue.isEmpty())
        {
            mouseEventQueue.clear();
        }

        currentWheelEvent = 0;
        mouseWheelEventTimer.stop();
        if (!mouseWheelEventQueue.isEmpty())
        {
            mouseWheelEventQueue.clear();
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
            releaseActiveSlots();

            if (currentRelease)
            {
                if (!pauseTimer.isActive())
                {
                    currentRelease = 0;
                }
            }

            // Stop hold timer here to be sure that
            // a hold timer that could be activated
            // during a release event is stopped.
            holdTimer.stop();
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
    this->vdpad = vdpad;
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
}

bool JoyButton::isDefault()
{
    bool value = true;
    value = value && (toggle == false);
    value = value && (turboInterval == 0);
    value = value && (useTurbo == false);
    value = value && (mouseSpeedX == 50);
    value = value && (mouseSpeedY == 50);
    value = value && (setSelection == -1);
    value = value && (setSelectionCondition == SetChangeDisabled);
    value = value && (assignments.isEmpty());
    value = value && (mouseMode == MouseCursor);
    value = value && (mouseCurve == LinearCurve);
    value = value && (springWidth == 0);
    value = value && (springHeight == 0);
    value = value && (sensitivity == 1.0);
    value = value && (smoothing == false);
    value = value && (actionName.isEmpty());
    value = value && (buttonName.isEmpty());
    value = value && (wheelSpeed != 20);
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
}

JoyButton::JoyMouseMovementMode JoyButton::getMouseMode()
{
    return mouseMode;
}

void JoyButton::setMouseCurve(JoyMouseCurve selectedCurve)
{
    mouseCurve = selectedCurve;
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
    }
}

double JoyButton::getSensitivity()
{
    return sensitivity;
}

void JoyButton::setSmoothing(bool enabled)
{
    smoothing = enabled;
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
    }
}

QString JoyButton::getButtonName()
{
    return buttonName;
}

void JoyButton::setWheelSpeed(int speed)
{
    if (speed >= 1 && speed <= 100)
    {
        wheelSpeed = speed;
    }
}

int JoyButton::getWheelSpeed()
{
    return wheelSpeed;
}
