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
        label.append(tr("Up")).append("+").append(tr("Right"));
    }
    else if (index == JoyControlStick::StickRightDown)
    {
        label.append(tr("Down")).append("+").append(tr("Right"));
    }

    return label;
}

QString JoyControlStickButton::getPartialName()
{
    return QString(tr("Stick %1").arg(QString::number(stick->getRealJoyIndex()))).append(": ").append(getDirectionName());
}

QString JoyControlStickButton::getXmlName()
{
    return this->xmlName;
}

double JoyControlStickButton::getDistanceFromDeadZone()
{
    return stick->calculateDirectionalDistance(this);
    //return stick->getNormalizedAbsoluteDistance();
}

double JoyControlStickButton::getSpringDistanceFromDeadZone()
{
    return stick->calculateDirectionalDistance(this, JoyButton::MouseSpring);
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
