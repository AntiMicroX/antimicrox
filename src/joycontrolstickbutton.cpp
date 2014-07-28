#include <QDebug>
#include <cmath>

#include "joycontrolstickbutton.h"
#include "joycontrolstick.h"
#include "event.h"

const QString JoyControlStickButton::xmlName = "stickbutton";

JoyControlStickButton::JoyControlStickButton(JoyControlStick *stick, int index, int originset, SetJoystick *parentSet, QObject *parent) :
    JoyButton(index, originset, parentSet, parent)
{
    this->stick = stick;
}

JoyControlStickButton::JoyControlStickButton(JoyControlStick *stick, JoyStickDirectionsType::JoyStickDirections index, int originset, SetJoystick *parentSet, QObject *parent) :
    JoyButton((int)index, originset, parentSet, parent)
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
        label.append(tr("Up")).append("+").append(tr("Right"));
    }
    else if (index == JoyControlStick::StickRightDown)
    {
        label.append(tr("Down")).append("+").append(tr("Right"));
    }

    return label;
}

QString JoyControlStickButton::getPartialName(bool forceFullFormat, bool displayNames)
{
    QString temp = stick->getPartialName(forceFullFormat, displayNames);

    temp.append(": ");

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
        temp.append(tr("Button")).append(" ");
        temp.append(getDirectionName());
    }
    return temp;
}

QString JoyControlStickButton::getXmlName()
{
    return this->xmlName;
}

/**
 * @brief Get the distance that an element is away from its assigned
 *     dead zone
 * @return Normalized distance away from dead zone
 */
double JoyControlStickButton::getDistanceFromDeadZone()
{
    return stick->calculateDirectionalDistance();
}

/**
 * @brief Get the distance factor that should be used for mouse movement
 * @return Distance factor that should be used for mouse movement
 */
double JoyControlStickButton::getMouseDistanceFromDeadZone()
{
    return stick->calculateMouseDirectionalDistance(this);
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

JoyControlStick* JoyControlStickButton::getStick()
{
    return stick;
}

JoyStickDirectionsType::JoyStickDirections JoyControlStickButton::getDirection()
{
    return (JoyStickDirectionsType::JoyStickDirections)index;
}

void JoyControlStickButton::turboEvent()
{
    double diff = abs(getMouseDistanceFromDeadZone() - lastDistance);

    bool changeState = false;
    if (!turboTimer.isActive() && !isButtonPressed)
    {
        changeState = true;
    }
    else if (getMouseDistanceFromDeadZone() >= 1.0 && isKeyPressed)
    {
        changeState = false;
        turboTimer.start(5);
        turboHold.start();
    }
    else if (turboHold.isNull() || turboHold.elapsed() > tempTurboInterval)
    {
        changeState = true;
    }
    else if (diff >= 0.1)
    {
        if (isKeyPressed)
        {
            isKeyPressed = !isKeyPressed;
        }
        changeState = true;
    }

    if (changeState)
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
                tempTurboInterval = (int)floor((getMouseDistanceFromDeadZone() * turboInterval) + 0.5);
                int timerInterval = qMin(tempTurboInterval, 5);
                //qDebug() << "tmpTurbo press: " << QString::number(tempTurboInterval);
                //qDebug() << "timer press: " << QString::number(timerInterval);
                turboTimer.start(timerInterval);
                turboHold.start();
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
                tempTurboInterval = (int)floor(((1 - getMouseDistanceFromDeadZone()) * turboInterval) + 0.5);
                int timerInterval = qMin(tempTurboInterval, 5);
                //qDebug() << "tmpTurbo release: " << QString::number(tempTurboInterval);
                //qDebug() << "timer release: " << QString::number(timerInterval);
                turboTimer.start(timerInterval);
                turboHold.start();
            }

        }
    }
}
