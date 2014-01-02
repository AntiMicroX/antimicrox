#include "gamecontrollertriggerbutton.h"

const QString GameControllerTriggerButton::xmlName = "triggerbutton";

GameControllerTriggerButton::GameControllerTriggerButton(JoyAxis *axis, int index, int originset, QObject *parent) :
    JoyAxisButton(axis, index, originset, parent)
{
}

QString GameControllerTriggerButton::getXmlName()
{
    return this->xmlName;
}
