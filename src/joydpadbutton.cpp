#include "joydpadbutton.h"
#include "joydpad.h"
#include "event.h"

const QString JoyDPadButton::xmlName = "dpadbutton";

// Initially, qualify direction as the button's index
JoyDPadButton::JoyDPadButton(int direction, int originset, JoyDPad* dpad, SetJoystick *parentSet, QObject *parent) :
    JoyButton(direction, originset, parentSet, parent)
{
    this->direction = direction;
    this->dpad = dpad;
}

QString JoyDPadButton::getDirectionName()
{
    QString label = QString ();
    if (direction == DpadUp)
    {
        label.append(tr("Up"));
    }
    else if (direction == DpadDown)
    {
        label.append(tr("Down"));
    }
    else if (direction == DpadLeft)
    {
        label.append(tr("Left"));
    }
    else if (direction == DpadRight)
    {
        label.append(tr("Right"));
    }
    else if (direction == DpadLeftUp)
    {
        label.append(tr("Up")).append("+").append(tr("Left"));
    }
    else if (direction == DpadLeftDown)
    {
        label.append(tr("Down")).append("+").append(tr("Left"));
    }
    else if (direction == DpadRightUp)
    {
        label.append(tr("Up")).append("+").append(tr("Right"));
    }
    else if (direction == DpadRightDown)
    {
        label.append(tr("Down")).append("+").append(tr("Right"));
    }

    return label;
}

QString JoyDPadButton::getXmlName()
{
    return this->xmlName;
}

int JoyDPadButton::getRealJoyNumber()
{
    return index;
}

QString JoyDPadButton::getPartialName(bool forceFullFormat, bool displayNames)
{
    QString temp = dpad->getName().append(" - ");
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
        temp.append(tr("Button")).append(" ");
        temp.append(getDirectionName());
    }
    return temp;
}


void JoyDPadButton::reset()
{
    JoyButton::reset();
}

void JoyDPadButton::reset(int index)
{
    Q_UNUSED(index);
    reset();
}

void JoyDPadButton::setChangeSetCondition(SetChangeCondition condition, bool passive)
{
    if (condition != setSelectionCondition && !passive)
    {
        if (condition == SetChangeWhileHeld || condition == SetChangeTwoWay)
        {
            // Set new condition
            emit setAssignmentChanged(index, this->dpad->getJoyNumber(), setSelection, condition);
            //emit setAssignmentChanged(index, setSelection, condition);
        }
        else if (setSelectionCondition == SetChangeWhileHeld || setSelectionCondition == SetChangeTwoWay)
        {
            // Remove old condition
            emit setAssignmentChanged(index, this->dpad->getJoyNumber(), setSelection, SetChangeDisabled);
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

JoyDPad* JoyDPadButton::getDPad()
{
    return dpad;
}

int JoyDPadButton::getDirection()
{
    return direction;
}
