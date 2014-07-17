#include <QDebug>

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
