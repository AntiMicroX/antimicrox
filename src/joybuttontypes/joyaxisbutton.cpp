//#include <QDebug>
#include <cmath>

#include "joyaxisbutton.h"
#include "joyaxis.h"
#include "event.h"

const QString JoyAxisButton::xmlName = "axisbutton";

JoyAxisButton::JoyAxisButton(JoyAxis *axis, int index, int originset, SetJoystick *parentSet, QObject *parent) :
    JoyGradientButton(index, originset, parentSet, parent)
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

double JoyAxisButton::getLastMouseDistanceFromDeadZone()
{
    double distance = 0.0;
    if (axis->getAxisButtonByValue(axis->getLastKnownThrottleValue()) == this)
    {
        distance = axis->getDistanceFromDeadZone(axis->getLastKnownThrottleValue());
    }

    return distance;
}
