#include <QDebug>
#include <QEventLoop>
#include <cmath>

#include "joybutton.h"
#include "event.h"

const QString JoyButton::xmlName = "button";

JoyButton::JoyButton(QObject *parent) :
    QObject(parent)
{
    slotiter = 0;

    this->reset();
    index = 0;
    originset = 0;
    /*currentPause = 0;
    currentHold = 0;
    currentCycle = 0;
    currentDistance = 0;*/

    quitEvent = true;
}

JoyButton::JoyButton(int index, int originset, QObject *parent) :
    QObject(parent)
{
    slotiter = 0;

    this->reset();
    this->index = index;
    this->originset = originset;
    /*currentPause = 0;
    currentHold = 0;
    currentCycle = 0;
    currentDistance = 0;*/

    quitEvent = true;
}

void JoyButton::joyEvent(bool pressed, bool ignoresets)
{
    buttonMutex.lock();

    if (toggle && pressed && (pressed != isDown))
    {
        this->ignoresets = ignoresets;
        isButtonPressed = !isButtonPressed;
        isDown = true;
        emit clicked(index);
        //createDeskEvent();

        ignoreSetQueue.enqueue(ignoresets);
        isButtonPressedQueue.enqueue(isButtonPressed);

        if (isButtonPressed)
        {
            buttonHold.restart();
            QTimer::singleShot(0, this, SLOT(waitForDeskEvent()));
        }
        else
        {
            QTimer::singleShot(0, this, SLOT(waitForReleaseDeskEvent()));
        }

    }
    else if (toggle && !pressed && isDown)
    {
        isDown = false;
        emit released(index);
    }

    else if (!toggle && (pressed != isButtonPressed))
    {
        if (pressed)
        {
            emit clicked(index);
        }
        else
        {
            emit released(index);
        }

        this->ignoresets = ignoresets;
        isButtonPressed = pressed;

        ignoreSetQueue.enqueue(ignoresets);
        isButtonPressedQueue.enqueue(isButtonPressed);

        if (useTurbo && isButtonPressed)
        {
            connect(&timer, SIGNAL(timeout()), this, SLOT(turboEvent()));
            timer.start();
        }
        else if (useTurbo && !isButtonPressed)
        {
            timer.stop();
            disconnect(&timer, SIGNAL(timeout()), 0, 0);
            if (isKeyPressed)
            {
                QTimer::singleShot(0, this, SLOT(turboEvent()));
            }
        }
        else if (isButtonPressed)
        //if (isButtonPressed)
        {
            buttonHold.restart();
            QTimer::singleShot(0, this, SLOT(waitForDeskEvent()));
        }
        else
        {
            QTimer::singleShot(0, this, SLOT(waitForReleaseDeskEvent()));
        }
    }

    buttonMutex.unlock();
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
    this->toggle = toggle;
}

void JoyButton::setTurboInterval(int interval)
{
    this->turboInterval = interval;
}

void JoyButton::reset()
{
    timer.stop();

    if (slotiter)
    {
        delete slotiter;
        slotiter = 0;
    }
    releaseDeskEvent();
    assignments.clear();

    isButtonPressedQueue.clear();
    ignoreSetQueue.clear();
    mouseEventQueue.clear();

    currentCycle = 0;
    currentPause = 0;
    currentHold = 0;
    currentDistance = 0;
    currentRawValue = 0;
    currentMouseEvent = 0;

    isKeyPressed = isButtonPressed = false;
    toggle = false;
    turboInterval = 0;
    isDown = false;
    useTurbo = false;
    mouseSpeedX = 50;
    mouseSpeedY = 50;
    setSelection = -1;
    setSelectionCondition = SetChangeDisabled;
    ignoresets = false;
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
        createDeskEvent();
        isKeyPressed = true;
        timer.start(100);
    }
    else
    {
        releaseDeskEvent();
        isKeyPressed = false;
        timer.start(turboInterval - 100);
    }
    /*QListIterator<JoyButtonSlot*> iter(assignments);
    if (!isKeyPressed)
    {
        while(iter.hasNext())
        {
            JoyButtonSlot *slot = iter.next();
            int tempcode = slot->getSlotCode();
            JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();
            sendevent(tempcode, true, mode);
        }
        isKeyPressed = true;
        timer.start(100);
    }
    else
    {
        while(iter.hasNext())
        {
            JoyButtonSlot *slot = iter.next();
            int tempcode = slot->getSlotCode();
            JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();
            sendevent(tempcode, false, mode);
        }
        isKeyPressed = false;
        timer.start(turboInterval - 100);
    }*/
}

JoyButton::~JoyButton()
{
    timer.stop();

    if (slotiter)
    {
        delete slotiter;
        slotiter = 0;
    }
}

void JoyButton::createDeskEvent()
{
    buttonMutex.lock();

    if (!slotiter)
    {
        slotiter = new QListIterator<JoyButtonSlot*> (assignments);
        quitEvent = false;
    }
    else if (currentCycle)
    {
        quitEvent = false;
        currentCycle = 0;
    }

    bool exit = false;

    while (slotiter->hasNext() && !exit)
    {
        JoyButtonSlot *slot = 0;
        slot = slotiter->next();
        int tempcode = slot->getSlotCode();
        JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();

        if (mode == JoyButtonSlot::JoyKeyboard || mode == JoyButtonSlot::JoyMouseButton)
        {
            sendevent(tempcode, true, mode);
            activeSlots.append(slot);
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
            QTimer::singleShot(0, this, SLOT(pauseEvent()));
            exit = true;
        }
        else if (mode == JoyButtonSlot::JoyHold)
        {
            currentHold = slot;
            QTimer::singleShot(0, this, SLOT(holdEvent()));
            exit = true;
        }
        else if (mode == JoyButtonSlot::JoyCycle)
        {
            currentCycle = slot;
            exit = true;
        }
        else if (mode == JoyButtonSlot::JoyDistance)
        {

        }
    }

    if (!slotiter->hasNext())
    {
        delete slotiter;
        slotiter = 0;

        quitEvent = true;
        currentCycle = 0;

        if (!isButtonPressedQueue.isEmpty())
        {
            bool tempButtonPressed = isButtonPressedQueue.last();
            if (tempButtonPressed && setSelectionCondition == SetChangeWhileHeld)
            {
                QTimer::singleShot(0, this, SLOT(checkForSetChange()));
            }
        }
    }
    else if (currentCycle)
    {
        quitEvent = true;

        if (!isButtonPressedQueue.isEmpty())
        {
            bool tempButtonPressed = isButtonPressedQueue.last();
            if (tempButtonPressed && setSelectionCondition == SetChangeWhileHeld)
            {
                QTimer::singleShot(0, this, SLOT(checkForSetChange()));
            }
        }
    }

    buttonMutex.unlock();
}

void JoyButton::mouseEvent()
{
    JoyButtonSlot *buttonslot = 0;
    if (currentMouseEvent)
    {
        buttonslot = currentMouseEvent;
    }
    else if (!mouseEventQueue.isEmpty())
    {
        buttonslot = mouseEventQueue.dequeue();
    }

    if (buttonslot)
    {
        QTime* mouseInterval = buttonslot->getMouseInterval();

        int mousemode = buttonslot->getSlotCode();
        int mousespeed;
        int timeElapsed = mouseInterval->elapsed();

        if (mousemode == JoyButtonSlot::MouseRight)
        {
            mousespeed = mouseSpeedX;
        }
        else if (mousemode == JoyButtonSlot::MouseLeft)
        {
            mousespeed = mouseSpeedX;
        }
        else if (mousemode == JoyButtonSlot::MouseDown)
        {
            mousespeed = mouseSpeedY;
        }
        else if (mousemode == JoyButtonSlot::MouseUp)
        {
            mousespeed = mouseSpeedY;
        }

        bool isActive = activeSlots.contains(buttonslot);
        if (isActive && timeElapsed >= 5)
        {
            int mouse1 = 0;
            int mouse2 = 0;
            double sumDist = buttonslot->getDistance();

            if (mousemode == JoyButtonSlot::MouseRight)
            {
                sumDist += (mousespeed * JoyButtonSlot::JOYSPEED * timeElapsed) / 1000.0;
                int distance = (int)floor(sumDist + 0.5);
                mouse1 = distance;
            }
            else if (mousemode == JoyButtonSlot::MouseLeft)
            {
                sumDist += (mousespeed * JoyButtonSlot::JOYSPEED * timeElapsed) / 1000.0;
                int distance = (int)floor(sumDist + 0.5);
                mouse1 = -distance;
            }
            else if (mousemode == JoyButtonSlot::MouseDown)
            {
                sumDist += (mousespeed * JoyButtonSlot::JOYSPEED * timeElapsed) / 1000.0;
                int distance = (int)floor(sumDist + 0.5);
                mouse2 = distance;
            }
            else if (mousemode == JoyButtonSlot::MouseUp)
            {
                sumDist += (mousespeed * JoyButtonSlot::JOYSPEED * timeElapsed) / 1000.0;
                int distance = (int)floor(sumDist + 0.5);
                mouse2 = -distance;
            }

            if (sumDist < 1.0)
            {
                buttonslot->setDistance(sumDist);
            }
            else if (sumDist >= 1.0)
            {
                sendevent(mouse1, mouse2);
                sumDist = 0.0;

                buttonslot->setDistance(sumDist);
            }

            mouseInterval->restart();
        }

        if (isActive)
        {
            mouseEventQueue.enqueue(buttonslot);
            QTimer::singleShot(5, this, SLOT(mouseEvent()));
        }
        else
        {
            buttonslot->setDistance(0.0);
            mouseInterval->restart();
        }
    }


}

void JoyButton::setUseTurbo(bool useTurbo)
{
    this->useTurbo = useTurbo;
    if (this->useTurbo && this->containsSequence())
    {
        this->useTurbo = false;
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
        reset();

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
                        this->assignments.append(buttonslot);
                    }
                    xml->readNextStartElement();
                }
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
    xml->writeStartElement(getXmlName());
    xml->writeAttribute("index", QString::number(getRealJoyNumber()));

    xml->writeTextElement("toggle", toggle ? "true" : "false");
    xml->writeTextElement("turbointerval", QString::number(turboInterval));
    xml->writeTextElement("useturbo", useTurbo ? "true" : "false");
    xml->writeTextElement("mousespeedx", QString::number(mouseSpeedX));
    xml->writeTextElement("mousespeedy", QString::number(mouseSpeedY));

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

QString JoyButton::getName()
{
    QString newlabel = getPartialName();
    newlabel = newlabel.append(": ").append(getSlotsSummary());
    return newlabel;
}

QString JoyButton::getPartialName()
{
    return QString("Button ").append(QString::number(getRealJoyNumber()));
}

QString JoyButton::getSlotsSummary()
{
    QString newlabel;
    int slotCount = assignments.count();

    if (slotCount > 0)
    {
        JoyButtonSlot *slot = assignments.first();
        int code = slot->getSlotCode();
        if (slot->getSlotMode() == JoyButtonSlot::JoyKeyboard)
        {
            newlabel = newlabel.append(keycodeToKey(code).toUpper());
        }
        else if (slot->getSlotMode() == JoyButtonSlot::JoyMouseButton)
        {
            newlabel = newlabel.append("Mouse ").append(QString::number(code));
        }
        else if (slot->getSlotMode() == JoyButtonSlot::JoyMouseMovement)
        {
            newlabel.append(slot->movementString());
        }
        else if (slot->getSlotMode() == JoyButtonSlot::JoyPause)
        {
            newlabel.append("Pause ").append(QString::number(slot->getSlotCode() / 1000.0, 'g', 3));
        }
        else if (slot->getSlotMode() == JoyButtonSlot::JoyHold)
        {
            newlabel.append("Hold ").append(QString::number(slot->getSlotCode() / 1000.0, 'g', 3));
        }
        else if (slot->getSlotMode() == JoyButtonSlot::JoyCycle)
        {
            newlabel.append("Cycle");
        }

        if (slotCount > 1)
        {
            newlabel = newlabel.append(" ...");
        }
    }
    else
    {
        newlabel = newlabel.append("[NO KEY]");
    }

    return newlabel;
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
    JoyButtonSlot *slot = new JoyButtonSlot(code, mode);
    assignments.append(slot);
    if (slot->getSlotMode() == JoyButtonSlot::JoyPause || slot->getSlotMode() == JoyButtonSlot::JoyHold)
    {
        setUseTurbo(false);
    }
}

void JoyButton::setAssignedSlot(int code, int index, JoyButtonSlot::JoySlotInputAction mode)
{
    JoyButtonSlot *slot = new JoyButtonSlot(code, mode);
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

    if (slot->getSlotMode() == JoyButtonSlot::JoyPause || slot->getSlotMode() == JoyButtonSlot::JoyHold)
    {
        setUseTurbo(false);
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
    setSelection = index;
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

void JoyButton::release()
{

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
            QListIterator<JoyButtonSlot*> iter(activeSlots);
            while (iter.hasNext())
            {
                JoyButtonSlot *slot = iter.next();
                int tempcode = slot->getSlotCode();
                JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();
                if (mode == JoyButtonSlot::JoyKeyboard)
                {
                    sendevent(tempcode, false, mode);
                }
            }

            activeSlots.clear();
            QTimer::singleShot(0, this, SLOT(pauseWaitEvent()));
            inpauseHold.restart();
        }
        else
        {
            QTimer::singleShot(10, this, SLOT(pauseEvent()));
        }
    }
}

void JoyButton::pauseWaitEvent()
{
    if (currentPause)
    {

        if (!isButtonPressedQueue.isEmpty() && isButtonPressedQueue.size() > 2)
        {
            if (slotiter)
            {
                slotiter->toBack();

                bool lastIgnoreSetState = ignoreSetQueue.last();
                bool lastIsButtonPressed = isButtonPressedQueue.last();
                ignoreSetQueue.clear();
                isButtonPressedQueue.clear();

                createDeskEvent();
                ignoreSetQueue.enqueue(lastIgnoreSetState);
                isButtonPressedQueue.enqueue(lastIsButtonPressed);
                currentPause = 0;
            }
        }
    }

    if (currentPause)
    {
        if (inpauseHold.elapsed() < currentPause->getSlotCode())
        {
            QTimer::singleShot(10, this, SLOT(pauseWaitEvent()));
        }
        else
        {
            QTimer::singleShot(0, this, SLOT(createDeskEvent()));
            currentPause = 0;
        }
    }
}

void JoyButton::checkForSetChange()
{
    if (!ignoreSetQueue.isEmpty() && !isButtonPressedQueue.isEmpty())
    {
        bool tempFinalState = isButtonPressedQueue.last();
        bool tempFinalIgnoreSetsState = ignoreSetQueue.last();
        bool tempButtonPressed = isButtonPressedQueue.dequeue();

        if (!tempFinalIgnoreSetsState)
        {
            if (!tempFinalState && setSelectionCondition == SetChangeOneWay && setSelection > -1)
            {
                emit setChangeActivated(setSelection);
            }
            else if (!tempFinalState && setSelectionCondition == SetChangeTwoWay && setSelection > -1)
            {
                emit setChangeActivated(setSelection);
            }
            else if ((tempButtonPressed == tempFinalState) && setSelectionCondition == SetChangeWhileHeld && setSelection > -1)
            {
                emit setChangeActivated(setSelection);
            }
        }

        ignoreSetQueue.clear();
        isButtonPressedQueue.clear();
    }
}

void JoyButton::waitForDeskEvent()
{
    if (!quitEvent)
    {
        QTimer::singleShot(0, this, SLOT(waitForDeskEvent()));
    }
    else
    {
        createDeskEvent();
    }
}

void JoyButton::waitForReleaseDeskEvent()
{
    if (!quitEvent && !isButtonPressedQueue.isEmpty() && !isButtonPressedQueue.last())
    {
        QTimer::singleShot(0, this, SLOT(waitForReleaseDeskEvent()));
    }
    else
    {
        releaseDeskEvent();
    }
}

bool JoyButton::containsSequence()
{
    bool result = false;

    QListIterator<JoyButtonSlot*> tempiter(assignments);
    while (tempiter.hasNext() && !result)
    {
        JoyButtonSlot *slot = tempiter.next();
        JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();
        if (mode == JoyButtonSlot::JoyPause || mode == JoyButtonSlot::JoyHold)
        {
            result = true;
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
            QListIterator<JoyButtonSlot*> iter(activeSlots);
            while (iter.hasNext())
            {
                JoyButtonSlot *slot = iter.next();
                int tempcode = slot->getSlotCode();
                JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();
                if (mode == JoyButtonSlot::JoyKeyboard || mode == JoyButtonSlot::JoyMouseButton)
                {
                    sendevent(tempcode, false, mode);
                }
            }

            activeSlots.clear();
            QTimer::singleShot(0, this, SLOT(createDeskEvent()));
            currentHold = 0;
            buttonHold.restart();
        }
        // Elapsed time has not occurred
        else if (currentlyPressed)
        {
            QTimer::singleShot(10, this, SLOT(holdEvent()));
        }
        // Pre-emptive release
        else
        {
            if (slotiter)
            {
                slotiter->toBack();
                currentHold = 0;
                createDeskEvent();
            }
        }
    }
}

void JoyButton::releaseDeskEvent()
{
    buttonMutex.lock();

    quitEvent = false;

    if (!activeSlots.isEmpty())
    {
        QListIterator<JoyButtonSlot*> iter(activeSlots);

        while (iter.hasNext())
        {
            JoyButtonSlot *slot = iter.next();
            int tempcode = slot->getSlotCode();
            JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();

            if (mode == JoyButtonSlot::JoyKeyboard || mode == JoyButtonSlot::JoyMouseButton)
            {
                sendevent(tempcode, false, mode);
            }
        }

        activeSlots.clear();
    }


    if (!isButtonPressedQueue.isEmpty())
    {
        bool tempButtonPressed = isButtonPressedQueue.last();
        if (!tempButtonPressed)
        {
            QTimer::singleShot(0, this, SLOT(checkForSetChange()));
        }
    }

    quitEvent = true;

    buttonMutex.unlock();
}

void JoyButton::distanceEvent()
{
    if (currentDistance)
    {
        bool currentlyPressed = false;
        if (!isButtonPressedQueue.isEmpty())
        {
            currentlyPressed = isButtonPressedQueue.last();
        }

        // Activate hold event
        if (currentlyPressed && currentRawValue > 20000)
        {
            QListIterator<JoyButtonSlot*> iter(activeSlots);
            while (iter.hasNext())
            {
                JoyButtonSlot *slot = iter.next();
                int tempcode = slot->getSlotCode();
                JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();
                if (mode == JoyButtonSlot::JoyKeyboard || mode == JoyButtonSlot::JoyMouseButton)
                {
                    sendevent(tempcode, false, mode);
                }
            }

            activeSlots.clear();
            QTimer::singleShot(0, this, SLOT(createDeskEvent()));
            currentDistance = 0;
            //buttonHold.restart();
        }
        // Elapsed time has not occurred
        else if (currentlyPressed)
        {
            QTimer::singleShot(0, this, SLOT(distanceEvent()));
        }
        // Pre-emptive release
        else
        {
            if (slotiter)
            {
                slotiter->toBack();
                currentDistance = 0;
                createDeskEvent();
            }
        }
    }
}
