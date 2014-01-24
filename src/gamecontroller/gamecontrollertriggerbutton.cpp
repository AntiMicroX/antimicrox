#include "gamecontrollertriggerbutton.h"

const QString GameControllerTriggerButton::xmlName = "triggerbutton";

GameControllerTriggerButton::GameControllerTriggerButton(JoyAxis *axis, int index, int originset, SetJoystick *parentSet, QObject *parent) :
    JoyAxisButton(axis, index, originset, parentSet, parent)
{
}

QString GameControllerTriggerButton::getXmlName()
{
    return this->xmlName;
}
