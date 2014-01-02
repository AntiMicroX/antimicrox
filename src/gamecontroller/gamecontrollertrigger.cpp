#include "gamecontrollertrigger.h"

GameControllerTrigger::GameControllerTrigger(int index, int originset, QObject *parent) :
    JoyAxis(index, originset, parent)
{
    this->setThrottle(JoyAxis::PositiveHalfThrottle);
}

bool GameControllerTrigger::isDefault()
{
    bool value = true;
    value = value && (deadZone == AXISDEADZONE);
    value = value && (maxZoneValue == AXISMAXZONE);
    value = value && (throttle == PositiveHalfThrottle);
    value = value && (paxisbutton->isDefault());
    value = value && (naxisbutton->isDefault());
    return value;
}
