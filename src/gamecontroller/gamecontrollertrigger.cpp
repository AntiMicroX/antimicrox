#include "gamecontrollertrigger.h"

GameControllerTrigger::GameControllerTrigger(int index, int originset, QObject *parent) :
    JoyAxis(index, originset, parent)
{
    this->setThrottle(JoyAxis::PositiveHalfThrottle);
}

bool GameControllerTrigger::isDefault()
{
    bool temp = JoyAxis::isDefault();
    temp = temp && (throttle == PositiveHalfThrottle);
    return temp;
}
