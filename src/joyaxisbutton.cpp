#include <QDebug>
#include <cmath>

#include "joyaxisbutton.h"
#include "joyaxis.h"
#include "event.h"

const QString JoyAxisButton::xmlName = "axisbutton";

JoyAxisButton::JoyAxisButton(JoyAxis *axis, int index, int originset, QObject *parent) :
    JoyButton(index, originset, parent)
{
    this->axis = axis;
}

QString JoyAxisButton::getPartialName()
{
    QString buttontype;
    if (index == 0)
    {
        buttontype = tr("Negative");
    }
    else if (index == 1)
    {
        buttontype = tr("Positive");
    }
    else
    {
        buttontype = tr("Unknown");
    }

    return QString(tr("Axis ")).append(QString::number(axis->getRealJoyIndex())).append(": ")
            .append(tr("Button")).append(" ").append(buttontype);
}

QString JoyAxisButton::getXmlName()
{
    return this->xmlName;
}

void JoyAxisButton::setChangeSetCondition(SetChangeCondition condition, bool passive)
{
    if (condition != setSelectionCondition && !passive)
    {
        if (condition == SetChangeWhileHeld || condition == SetChangeTwoWay)
        {
            // Set new condition
            emit setAssignmentChanged(index, this->axis->getIndex(), setSelection, condition);
            //emit setAssignmentChanged(index, setSelection, condition);
        }
        else if (setSelectionCondition == SetChangeWhileHeld || setSelectionCondition == SetChangeTwoWay)
        {
            // Remove old condition
            emit setAssignmentChanged(index, this->axis->getIndex(), setSelection, SetChangeDisabled);
            //emit setAssignmentChanged(index, setSelection, SetChangeDisabled);
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

/*void JoyAxisButton::mouseEvent()
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
            double difference = axis->calculateNormalizedAxisPlacement();
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
            QTimer::singleShot(5, this, SLOT(mouseEvent()));
        }
        else
        {
            buttonslot->setDistance(0.0);
            mouseInterval->restart();
        }
    }
}*/

double JoyAxisButton::getDistanceFromDeadZone()
{
    return axis->getDistanceFromDeadZone();
}
