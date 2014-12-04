//#include <QDebug>
#include <QString>
#include <cmath>

#include "joycontrolstick.h"
#include "joycontrolstickmodifierbutton.h"

const QString JoyControlStickModifierButton::xmlName = "stickmodifierbutton";

JoyControlStickModifierButton::JoyControlStickModifierButton(JoyControlStick *stick, int originset, SetJoystick *parentSet, QObject *parent) :
    JoyButton(0, originset, parentSet, parent)
{
    this->stick = stick;
}

QString JoyControlStickModifierButton::getPartialName(bool forceFullFormat, bool displayNames)
{
    QString temp = stick->getPartialName(forceFullFormat, displayNames);

    temp.append(": ");

    if (!buttonName.isEmpty() && displayNames)
    {
        if (forceFullFormat)
        {
            temp.append(tr("Modifier")).append(" ");
        }

        temp.append(buttonName);
    }
    else if (!defaultButtonName.isEmpty())
    {
        if (forceFullFormat)
        {
            temp.append(tr("Modifier")).append(" ");
        }

        temp.append(defaultButtonName);
    }
    else
    {
        temp.append(tr("Modifier"));
    }
    return temp;
}

QString JoyControlStickModifierButton::getXmlName()
{
    return this->xmlName;
}

/**
 * @brief Get the distance that an element is away from its assigned
 *     dead zone
 * @return Normalized distance away from dead zone
 */
double JoyControlStickModifierButton::getDistanceFromDeadZone()
{
    return stick->calculateDirectionalDistance();
}

/**
 * @brief Get the distance factor that should be used for mouse movement
 * @return Distance factor that should be used for mouse movement
 */
double JoyControlStickModifierButton::getMouseDistanceFromDeadZone()
{
    return getDistanceFromDeadZone();
}

void JoyControlStickModifierButton::setChangeSetCondition(SetChangeCondition condition, bool passive)
{
    Q_UNUSED(condition);
    Q_UNUSED(passive);
}

int JoyControlStickModifierButton::getRealJoyNumber()
{
    return index;
}

JoyControlStick* JoyControlStickModifierButton::getStick()
{
    return stick;
}

/**
 * @brief Activate a turbo event on a JoyControlStickButton.
 */
void JoyControlStickModifierButton::turboEvent()
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
                 getMouseDistanceFromDeadZone() >= 1.0)
        {
            if (isKeyPressed)
            {
                changeState = false;
                if (!turboTimer.isActive() || turboTimer.interval() != 5)
                {
                    turboTimer.start(5);
                }

                turboHold.restart();
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
                if (currentTurboMode == GradientTurbo)
                {
                    tempInterval2 = (int)floor((getMouseDistanceFromDeadZone() * turboInterval) + 0.5);
                }
                else
                {
                    tempInterval2 = (int)floor((turboInterval * 0.5) + 0.5);
                }
            }
            else
            {
                if (currentTurboMode == GradientTurbo)
                {
                    tempInterval2 = (int)floor(((1 - getMouseDistanceFromDeadZone()) * turboInterval) + 0.5);
                }
                else
                {
                    double distance = getMouseDistanceFromDeadZone();
                    if (distance > 0.0)
                    {
                        tempInterval2 = (int)floor(((turboInterval / getMouseDistanceFromDeadZone()) * 0.5) + 0.5);
                    }
                    else
                    {
                        tempInterval2 = 0;
                    }
                }
            }

            if (turboHold.elapsed() < tempInterval2)
            {
                // Still some valid time left. Continue current action with
                // remaining time left.
                tempTurboInterval = tempInterval2 - turboHold.elapsed();
                int timerInterval = qMin(tempTurboInterval, 5);
                if (!turboTimer.isActive() || turboTimer.interval() != timerInterval)
                {
                    turboTimer.start(timerInterval);
                }

                turboHold.restart();
                changeState = false;
                lastDistance = getMouseDistanceFromDeadZone();
                //qDebug() << "diff tmpTurbo press: " << QString::number(tempTurboInterval);
                //qDebug() << "diff timer press: " << QString::number(timerInterval);
            }
            else
            {
                // Elapsed time is greater than new interval. Change state.
                //if (isKeyPressed)
                //{
                //    checkmate = turboHold.elapsed();
                //}
                changeState = true;
                //qDebug() << "YOU GOT CHANGE";
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

                    int timerInterval = qMin(tempTurboInterval, 5);
                    //qDebug() << "tmpTurbo press: " << QString::number(tempTurboInterval);
                    //qDebug() << "timer press: " << QString::number(timerInterval);
                    if (turboTimer.interval() != timerInterval)
                    {
                        turboTimer.start(timerInterval);
                    }

                    turboHold.restart();
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
                        double distance = getMouseDistanceFromDeadZone();
                        if (distance > 0.0)
                        {
                            tempTurboInterval = (int)floor(((turboInterval / getMouseDistanceFromDeadZone()) * 0.5) + 0.5);
                        }
                        else
                        {
                            tempTurboInterval = 0;
                        }
                    }

                    int timerInterval = qMin(tempTurboInterval, 5);
                    //qDebug() << "tmpTurbo release: " << QString::number(tempTurboInterval);
                    //qDebug() << "timer release: " << QString::number(timerInterval);
                    if (turboTimer.interval() != timerInterval)
                    {
                        turboTimer.start(timerInterval);
                    }

                    turboHold.restart();
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
void JoyControlStickModifierButton::setTurboMode(TurboMode mode)
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
bool JoyControlStickModifierButton::isPartRealAxis()
{
    return true;
}

bool JoyControlStickModifierButton::isModifierButton()
{
    return true;
}
