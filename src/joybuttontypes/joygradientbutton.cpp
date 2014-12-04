//#include <QDebug>
#include <cmath>

#include "joygradientbutton.h"

JoyGradientButton::JoyGradientButton(int index, int originset, SetJoystick *parentSet, QObject *parent) :
    JoyButton(index, originset, parentSet, parent)
{
}

/**
 * @brief Activate a turbo event on a JoyControlStickButton.
 */
void JoyGradientButton::turboEvent()
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
