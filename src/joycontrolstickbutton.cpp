#include <math.h>

#include "joycontrolstickbutton.h"
#include "joycontrolstick.h"
#include "event.h"

const QString JoyControlStickButton::xmlName = "stickbutton";

JoyControlStickButton::JoyControlStickButton(JoyControlStick *stick, int index, int originset, QObject *parent) :
    JoyButton(index, originset, parent)
{
    this->stick = stick;
}

JoyControlStickButton::JoyControlStickButton(JoyControlStick *stick, JoyStickDirectionsType::JoyStickDirections index, int originset, QObject *parent) :
    JoyButton((int)index, originset, parent)
{
    this->stick = stick;
}

QString JoyControlStickButton::getDirectionName()
{
    QString label = QString();
    if (index == JoyControlStick::StickUp)
    {
        label.append(tr("Up"));
    }
    else if (index == JoyControlStick::StickDown)
    {
        label.append(tr("Down"));
    }
    else if (index == JoyControlStick::StickLeft)
    {
        label.append(tr("Left"));
    }
    else if (index == JoyControlStick::StickRight)
    {
        label.append(tr("Right"));
    }
    else if (index == JoyControlStick::StickLeftUp)
    {
        label.append(tr("Up")).append("+").append(tr("Left"));
    }
    else if (index == JoyControlStick::StickLeftDown)
    {
        label.append(tr("Down")).append("+").append(tr("Left"));
    }
    else if (index == JoyControlStick::StickRightUp)
    {
        label.append(tr("Up")).append("+").append(tr("Left"));
    }
    else if (index == JoyControlStick::StickRightDown)
    {
        label.append(tr("Down")).append("+").append(tr("Right"));
    }

    return label;
}

QString JoyControlStickButton::getPartialName()
{
    return stick->getName().append(" - ").append(getDirectionName());
}

QString JoyControlStickButton::getXmlName()
{
    return this->xmlName;
}

double JoyControlStickButton::getDistanceFromDeadZone()
{
    //return stick->getDistanceFromDeadZone();
    return stick->calculateDirectionalDistance(this);
}

void JoyControlStickButton::setChangeSetCondition(SetChangeCondition condition, bool passive)
{
    if (condition != setSelectionCondition && !passive)
    {
        if (condition == SetChangeWhileHeld || condition == SetChangeTwoWay)
        {
            // Set new condition
            emit setAssignmentChanged(index, this->stick->getIndex(), setSelection, condition);
        }
        else if (setSelectionCondition == SetChangeWhileHeld || setSelectionCondition == SetChangeTwoWay)
        {
            // Remove old condition
            emit setAssignmentChanged(index, this->stick->getIndex(), setSelection, SetChangeDisabled);
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

int JoyControlStickButton::getRealJoyNumber()
{
    return index;
}

/*void JoyControlStickButton::mouseEvent()
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
        int mousespeed = 0;
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
            //double difference = stick->calculateDirectionalDistance(this);
            //double difference = getDistanceFromDeadZone();
            int mouse1 = 0;
            int mouse2 = 0;
            double sumDist = buttonslot->getMouseDistance();

            if (mousemode == JoyButtonSlot::MouseRight)
            {
                sumDist += difference * (mousespeed * JoyButtonSlot::JOYSPEED * timeElapsed) / 1000.0;
                int distance = (int)floor(sumDist + 0.5);
                mouse1 = distance;
            }
            else if (mousemode == JoyButtonSlot::MouseLeft)
            {
                sumDist += difference * (mousespeed * JoyButtonSlot::JOYSPEED * timeElapsed) / 1000.0;
                int distance = (int)floor(sumDist + 0.5);
                mouse1 = -distance;
            }
            else if (mousemode == JoyButtonSlot::MouseDown)
            {
                sumDist += difference * (mousespeed * JoyButtonSlot::JOYSPEED * timeElapsed) / 1000.0;
                int distance = (int)floor(sumDist + 0.5);
                mouse2 = distance;
            }
            else if (mousemode == JoyButtonSlot::MouseUp)
            {
                sumDist += difference * (mousespeed * JoyButtonSlot::JOYSPEED * timeElapsed) / 1000.0;
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
            //QTimer::singleShot(5, this, SLOT(mouseEvent()));
            if (!mouseEventTimer.isActive())
            {
                mouseEventTimer.start(5);
            }
        }
        else
        {
            buttonslot->setDistance(0.0);
            mouseInterval->restart();
            mouseEventTimer.stop();
        }
    }
}
*/
