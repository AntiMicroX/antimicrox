//#include <QDebug>
#include <cmath>

#include "joyaxisbutton.h"
#include "joyaxis.h"
#include "event.h"

const QString JoyAxisButton::xmlName = "axisbutton";

JoyAxisButton::JoyAxisButton(JoyAxis *axis, int index, int originset, SetJoystick *parentSet, QObject *parent) :
    JoyButton(index, originset, parentSet, parent)
{
    this->axis = axis;
}

QString JoyAxisButton::getPartialName(bool forceFullFormat, bool displayNames)
{
    QString temp = QString(axis->getPartialName(forceFullFormat, displayNames));
    temp.append(": ");

    if (!buttonName.isEmpty() && displayNames)
    {
        if (forceFullFormat)
        {
            temp.append(tr("Button")).append(" ");
        }
        temp.append(buttonName);
    }
    else if (!defaultButtonName.isEmpty() && displayNames)
    {
        if (forceFullFormat)
        {
            temp.append(tr("Button")).append(" ");
        }
        temp.append(defaultButtonName);
    }
    else
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

        temp.append(tr("Button")).append(" ").append(buttontype);
    }

    return temp;
}

QString JoyAxisButton::getXmlName()
{
    return this->xmlName;
}

void JoyAxisButton::setChangeSetCondition(SetChangeCondition condition, bool passive)
{
    SetChangeCondition oldCondition = setSelectionCondition;

    if (condition != setSelectionCondition && !passive)
    {
        if (condition == SetChangeWhileHeld || condition == SetChangeTwoWay)
        {
            // Set new condition
            emit setAssignmentChanged(index, this->axis->getIndex(), setSelection, condition);
        }
        else if (setSelectionCondition == SetChangeWhileHeld || setSelectionCondition == SetChangeTwoWay)
        {
            // Remove old condition
            emit setAssignmentChanged(index, this->axis->getIndex(), setSelection, SetChangeDisabled);
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

/**
 * @brief Get the distance that an element is away from its assigned
 *     dead zone
 * @return Normalized distance away from dead zone
 */
double JoyAxisButton::getDistanceFromDeadZone()
{
    return axis->getDistanceFromDeadZone();
}

/**
 * @brief Get the distance factor that should be used for mouse movement
 * @return Distance factor that should be used for mouse movement
 */
double JoyAxisButton::getMouseDistanceFromDeadZone()
{
    return this->getDistanceFromDeadZone();
}

void JoyAxisButton::setVDPad(VDPad *vdpad)
{
    if (axis->isPartControlStick())
    {
        axis->removeControlStick();
    }

    JoyButton::setVDPad(vdpad);
}

JoyAxis* JoyAxisButton::getAxis()
{
    return this->axis;
}

void JoyAxisButton::turboEvent()
{
    if (currentTurboMode == NormalTurbo)
    {
        JoyButton::turboEvent();
    }
    else if (currentTurboMode == GradientTurbo || currentTurboMode == PulseTurbo)
    {
        double diff = fabs(getMouseDistanceFromDeadZone() - lastDistance);
        //qDebug() << "DIFF: " << QString::number(diff);

        bool changeState = false;
        int checkmate = 0;
        if (!turboTimer.isActive() && !isButtonPressed)
        {
            changeState = true;
        }
        else if (currentTurboMode == GradientTurbo && diff > 0 &&
                 getMouseDistanceFromDeadZone() >= 1.0 && isKeyPressed)
        {
            if (isKeyPressed)
            {
                changeState = false;
                turboTimer.start(5);
                turboHold.start();
                lastDistance = 1.0;
            }
            else
            {
                changeState = true;
            }
        }
        else if (turboHold.isNull() || lastDistance == 0.0 || turboHold.elapsed() > tempTurboInterval)
        {
            changeState = true;
        }
        else if (diff >= 0.1)
        {
            int tempInterval2 = 0;

            if (isKeyPressed)
            {
                //tempInterval2 = (int)floor((getMouseDistanceFromDeadZone() * turboInterval) + 0.5);
                if (currentTurboMode == GradientTurbo)
                {
                    tempInterval2 = (int)floor((getMouseDistanceFromDeadZone() * turboInterval) + 0.5);
                }
                else
                {
                    tempInterval2 = (int)floor((turboInterval * 0.5) + 0.5);
                }
            }
            //else
            //{
            //    tempInterval2 = (int)floor((lastDistance * turboInterval) + 0.5);
                //tempInterval2 = (int)floor(((1 - getMouseDistanceFromDeadZone()) * turboInterval) + 0.5);
            //}

            if (isKeyPressed && turboHold.elapsed() < tempInterval2)
            {
                // Still some valid time left. Continue current action with
                // remaining time left.
                tempTurboInterval = tempInterval2 - turboHold.elapsed();
                int timerInterval = qMin(tempTurboInterval, 5);
                turboTimer.start(timerInterval);
                turboHold.start();
                changeState = false;
                lastDistance = getMouseDistanceFromDeadZone();
                //qDebug() << "diff tmpTurbo press: " << QString::number(tempTurboInterval);
                //qDebug() << "diff timer press: " << QString::number(timerInterval);
            }
            //else if (!isKeyPressed && turboHold.elapsed() < tempInterval2)
            //{
            //    checkmate = turboHold.elapsed();
            //    changeState = true;
            //}
            //else if (!isKeyPressed && turboHold.elapsed() < tempInterval2)
            //{
            //    changeState = true;
            //}
            else
            {
                // Elapsed time is greater than new interval. Change state.
                //if (isKeyPressed)
                //{
                //    isKeyPressed = !isKeyPressed;
                //}
                //qDebug() << "AND THAT'S THE BOTTOM LINE";
                if (isKeyPressed)
                {
                    checkmate = turboHold.elapsed();
                }
                changeState = true;
            }
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
                    if (currentTurboMode == GradientTurbo)
                    {
                        tempTurboInterval = (int)floor((getMouseDistanceFromDeadZone() * turboInterval) + 0.5);
                    }
                    else
                    {
                        tempTurboInterval = (int)floor((turboInterval * 0.5) + 0.5);
                    }

                    //if (checkmate > 0 && tempTurboInterval > checkmate)
                    //{
                    //    tempTurboInterval = tempTurboInterval - checkmate;
                    //}
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
                    if (currentTurboMode == GradientTurbo)
                    {
                        tempTurboInterval = (int)floor(((1 - getMouseDistanceFromDeadZone()) * turboInterval) + 0.5);
                    }
                    else
                    {
                        tempTurboInterval = (int)floor(((turboInterval / getMouseDistanceFromDeadZone()) * 0.5) + 0.5);
                    }

                    int timerInterval = qMin(tempTurboInterval, 5);
                    //qDebug() << "tmpTurbo release: " << QString::number(tempTurboInterval);
                    //qDebug() << "timer release: " << QString::number(timerInterval);
                    turboTimer.start(timerInterval);
                    turboHold.start();
                }

            }

            lastDistance = getMouseDistanceFromDeadZone();
        }

        checkmate = 0;
    }
}

/**
 * @brief Set the turbo mode that the button should use
 * @param Mode that should be used
 */
void JoyAxisButton::setTurboMode(TurboMode mode)
{
    if (isPartRealAxis())
    {
        currentTurboMode = mode;
    }
}

/**
 * @brief Check if button should be considered a part of a real controller
 *     axis. Needed for some dialogs so the program won't have to resort to
 *     type checking.
 * @return Status of being part of a real controller axis
 */
bool JoyAxisButton::isPartRealAxis()
{
    return true;
}
