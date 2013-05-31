#include <QDebug>
#include <QHashIterator>
#include <math.h>

#include "joycontrolstick.h"

double JoyControlStick::PI = acos(-1.0);

JoyControlStick::JoyControlStick(JoyAxis *axis1, JoyAxis *axis2, int originset, QObject *parent) :
    QObject(parent)
{
    this->axis1 = axis1;
    this->axis2 = axis2;
    this->originset = originset;
    deadZone = 8000;
    diagonalRange = 0;
    isActive = false;
    activeButton1 = 0;
    activeButton2 = 0;
    safezone = false;
    maxZone = 0;

    populateButtons();
}

JoyControlStick::~JoyControlStick()
{
    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyButton *button = iter.next().value();
        if (button)
        {
            delete button;
            button = 0;
        }
    }

    buttons.clear();
}

void JoyControlStick::joyEvent(bool ignoresets)
{
    safezone = !inDeadZone();

    if (safezone && !isActive)
    {
        isActive = true;
        emit active(axis1->getCurrentRawValue(), axis2->getCurrentRawValue());
        createDeskEvent(ignoresets);
    }
    else if (!safezone && isActive)
    {
        isActive = false;
        emit released(axis1->getCurrentRawValue(), axis2->getCurrentRawValue());

        createDeskEvent(ignoresets);
    }
    else if (isActive)
    {
        createDeskEvent(ignoresets);
    }

    emit moved(axis1->getCurrentRawValue(), axis2->getCurrentRawValue());
}

bool JoyControlStick::inDeadZone()
{
    int axis1Value = axis1->getCurrentRawValue();
    int axis2Value = axis2->getCurrentRawValue();

    unsigned int square_dist = (axis1Value*axis1Value) + (axis2Value*axis2Value);

    return square_dist < (deadZone*deadZone);
}

void JoyControlStick::populateButtons()
{
    JoyControlStickButton *button = new JoyControlStickButton (this, (int)StickUp, originset, this);
    button->setAssignedSlot(31);
    buttons.insert(StickUp, button);

    button = new JoyControlStickButton (this, (int)StickDown, originset, this);
    button->setAssignedSlot(45);
    buttons.insert(StickDown, button);

    button = new JoyControlStickButton(this, (int)StickRight, originset, this);
    button->setAssignedSlot(46);
    buttons.insert(StickRight, button);

    button = new JoyControlStickButton(this, (int)StickLeft, originset, this);
    button->setAssignedSlot(44);
    buttons.insert(StickLeft, button);
}

int JoyControlStick::getDeadZone()
{
    return deadZone;
}

int JoyControlStick::getDiagonalRange()
{
    return diagonalRange;
}

void JoyControlStick::createDeskEvent(bool ignoresets)
{
    JoyControlStickButton *eventbutton1 = 0;
    JoyControlStickButton *eventbutton2 = 0;

    if (safezone)
    {
        double bearing = calculateBearing();
        int direction = ((int)round(bearing/45.0)) % 8;

        if (direction == 0)
        {
            eventbutton2 = buttons.value(StickUp);
        }
        else if (direction == 1)
        {
            eventbutton1 = buttons.value(StickRight);
            eventbutton2 = buttons.value(StickUp);
        }
        else if (direction == 2)
        {
            eventbutton1 = buttons.value(StickRight);
        }
        else if (direction  == 3)
        {
            eventbutton1 = buttons.value(StickRight);
            eventbutton2 = buttons.value(StickDown);
        }
        else if (direction == 4)
        {
            eventbutton2 = buttons.value(StickDown);
        }
        else if (direction == 5)
        {
            eventbutton1 = buttons.value(StickLeft);
            eventbutton2 = buttons.value(StickDown);
        }
        else if (direction == 6)
        {
            eventbutton1 = buttons.value(StickLeft);
        }
        else if (direction == 7)
        {
            eventbutton1 = buttons.value(StickLeft);
            eventbutton2 = buttons.value(StickUp);
        }
    }

    if (eventbutton1 || activeButton1)
    {
        changeButtonEvent(eventbutton1, activeButton1, ignoresets);
    }

    if (eventbutton2 || activeButton2)
    {
        changeButtonEvent(eventbutton2, activeButton2, ignoresets);
    }
}

double JoyControlStick::calculateBearing()
{
    double finalAngle = 0.0;

    if (axis1->getCurrentRawValue() == 0 && axis2->getCurrentRawValue() == 0)
    {
        finalAngle = 0.0;
    }
    else
    {
        double temp1 = axis1->getCurrentRawValue() / (double)axis1->AXISMAXZONE;
        double temp2 = axis2->getCurrentRawValue() / (double)axis2->AXISMAXZONE;
        double fuck = atan2(temp2, temp1);
        double braburner = atan2(temp1, -temp2);
        double angle = (atan2(temp1, -temp2) * 180) / PI;

        if (axis1->getCurrentRawValue() >= 0 && axis2->getCurrentRawValue() <= 0)
        {
            // NE Quadrant
            finalAngle = angle;
        }
        else if (axis1->getCurrentRawValue() >= 0 && axis2->getCurrentRawValue() >= 0)
        {
            // SE Quadrant (angle will be positive)
            finalAngle = angle;
        }
        else if (axis1->getCurrentRawValue() <= 0 && axis2->getCurrentRawValue() >= 0)
        {
            // SW Quadrant (angle will be negative)
            finalAngle = 360.0 + angle;
        }
        else if (axis1->getCurrentRawValue() <= 0 && axis2->getCurrentRawValue() <= 0)
        {
            // NW Quadrant (angle will be negative)
            finalAngle = 360.0 + angle;
        }
    }

    return finalAngle;
}

void JoyControlStick::changeButtonEvent(JoyControlStickButton *eventbutton, JoyControlStickButton *&activebutton, bool ignoresets)
{
    if (eventbutton && !activebutton)
    {
        // There is no active button. Call joyEvent and set current
        // button as active button
        eventbutton->joyEvent(true, ignoresets);
        activebutton = eventbutton;
    }
    else if (!eventbutton && activebutton)
    {
        // Currently in deadzone. Disable currently active button.
        activebutton->joyEvent(false, ignoresets);
        activebutton = 0;
        qDebug() << "DISABLE EVENT: " << isActive << endl;
        qDebug() << "AXIS1: " << axis1->getCurrentRawValue() << endl;
        qDebug() << "AXIS2: " << axis2->getCurrentRawValue() << endl;
    }
    else if (eventbutton && activebutton && eventbutton == activebutton)
    {
        //Button is currently active. Just pass current value
        eventbutton->joyEvent(true, ignoresets);
    }
    else if (eventbutton && activebutton && eventbutton != activebutton)
    {
        // Deadzone skipped. Button for new event is not the currently
        // active button. Disable the active button before enabling
        // the new button
        activebutton->joyEvent(false, ignoresets);
        eventbutton->joyEvent(true, ignoresets);
        activebutton = eventbutton;
    }
}

double JoyControlStick::getDistanceFromDeadZone()
{
    double distance = 0.0;

    int axis1Value = axis1->getCurrentRawValue();
    int axis2Value = axis2->getCurrentRawValue();

    unsigned int square_dist = (axis1Value*axis1Value) + (axis2Value*axis2Value);

    distance = (sqrt(square_dist) - deadZone)/(double)(JoyAxis::AXISMAXZONE - deadZone);
    if (distance > 1.0)
    {
        distance = 1.0;
    }
    else if (distance < 0.0)
    {
        distance = 0.0;
    }

    return distance;
}
