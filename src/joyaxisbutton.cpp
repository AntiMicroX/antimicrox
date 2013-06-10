#include <QDebug>

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

double JoyAxisButton::getDistanceFromDeadZone()
{
    return axis->getDistanceFromDeadZone();
}
