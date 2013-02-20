#include <cmath>

#include "joyaxisbutton.h"
#include "joyaxis.h"
#include "event.h"

const QString JoyAxisButton::xmlName = "axisbutton";

JoyAxisButton::JoyAxisButton(JoyAxis *axis, QObject *parent) :
    JoyButton(parent)
{
    this->axis = axis;
}

JoyAxisButton::JoyAxisButton(JoyAxis *axis, int index, QObject *parent) :
    JoyButton(parent)
{
    this->axis = axis;
    this->index = index;
}

QString JoyAxisButton::getXmlName()
{
    return this->xmlName;
}

void JoyAxisButton::mouseEvent(JoyButtonSlot *buttonslot)
{
    QTime* mouseInterval = buttonslot->getMouseInterval();
    int mouse1 = 0;
    int mouse2 = 0;
    double sumDist = buttonslot->getDistance();

    int mousemode = buttonslot->getSlotCode();
    int mousespeed;

    double difference = axis->calculateNormalizedAxisPlacement();
    sumDist += difference;
    int distance = (int)floor(sumDist + 0.5);

    if (mousemode == JoyButtonSlot::MouseRight)
    {
        mouse1 = distance;
        mousespeed = mouseSpeedX;
    }
    else if (mousemode == JoyButtonSlot::MouseLeft)
    {
        mouse1 = -distance;
        mousespeed = mouseSpeedX;
    }
    else if (mousemode == JoyButtonSlot::MouseDown)
    {
        mouse2 = distance;
        mousespeed = mouseSpeedY;
    }
    else if (mousemode == JoyButtonSlot::MouseUp)
    {
        mouse2 = -distance;
        mousespeed = mouseSpeedY;
    }

    if (isButtonPressed && mouseInterval->elapsed() >= (1000.0/(mousespeed*JoyButtonSlot::JOYSPEED)))
    {
        if (sumDist >= 1.0)
        {
            sendevent(mouse1, mouse2);
            sumDist = 0.0;
        }

        buttonslot->setDistance(sumDist);
        mouseInterval->restart();
    }

    if (isButtonPressed)
    {
        QMetaObject::invokeMethod(this, "mouseEvent", Qt::QueuedConnection, Q_ARG(JoyButtonSlot*, buttonslot));
    }
}
