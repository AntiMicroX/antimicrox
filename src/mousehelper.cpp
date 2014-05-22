#include "mousehelper.h"

MouseHelper::MouseHelper(QObject *parent) :
    QObject(parent)
{
    springMouseMoving = false;
    previousCursorLocation[0] = 0;
    previousCursorLocation[1] = 0;
    pivotPoint[0] = -1;
    pivotPoint[1] = -1;
    mouseTimer.setSingleShot(true);
    QObject::connect(&mouseTimer, SIGNAL(timeout()), this, SLOT(resetSpringMouseMoving()));
}

void MouseHelper::resetSpringMouseMoving()
{
    springMouseMoving = false;
}
