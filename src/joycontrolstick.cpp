#include <QDebug>
#include <QHashIterator>
#include <math.h>

#include "joycontrolstick.h"

double JoyControlStick::PI = acos(-1.0);

JoyControlStick::JoyControlStick(JoyAxis *axis1, JoyAxis *axis2, int index, int originset, QObject *parent) :
    QObject(parent)
{
    /*
    this->axis1 = axis1;
    this->axis2 = axis2;
    this->originset = originset;
    deadZone = 8000;
    maxZone = 30000;
    diagonalRange = 0;
    isActive = false;
    activeButton1 = 0;
    activeButton2 = 0;
    safezone = false;
    this->index = index;
    currentDirection = StickCentered;
    */

    this->axis1 = axis1;
    this->axis2 = axis2;
    this->index = index;
    this->originset = originset;
    reset();

    populateButtons();
}

JoyControlStick::~JoyControlStick()
{
    deleteButtons();
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
        currentDirection = StickCentered;
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

    unsigned int squareDist = (axis1Value*axis1Value) + (axis2Value*axis2Value);

    return squareDist <= (deadZone*deadZone);
}

void JoyControlStick::populateButtons()
{
    JoyControlStickButton *button = new JoyControlStickButton (this, StickUp, originset, this);
    //button->setAssignedSlot(31);
    //button->setAssignedSlot(25);
    buttons.insert(StickUp, button);

    button = new JoyControlStickButton (this, StickDown, originset, this);
    //button->setAssignedSlot(45);
    //button->setAssignedSlot(39);
    buttons.insert(StickDown, button);

    button = new JoyControlStickButton(this, StickLeft, originset, this);
    //button->setAssignedSlot(44);
    //button->setAssignedSlot(38);
    buttons.insert(StickLeft, button);

    button = new JoyControlStickButton(this, StickRight, originset, this);
    //button->setAssignedSlot(46);
    //button->setAssignedSlot(40);
    buttons.insert(StickRight, button);
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
        //*
        //int diagonalAngle = (int)floor((deadZone / (double)JoyAxis::AXISMAX) * 100);
        //int diagonalAngle = diagonalRange;
        int diagonalAngle = 45;
        /*
        if (diagonalAngle > 89)
        {
            diagonalAngle = 89;
        }
        //*/

        int cardinalAngle = (360 - (diagonalAngle * 4)) / 4;

        int initialLeft = 360 - (int)((cardinalAngle - 1) / 2);
        int initialRight = (int)((cardinalAngle - 1)/ 2);
        if ((cardinalAngle - 1) % 2 != 0)
        {
            initialLeft = 360 - (cardinalAngle / 2);
            initialRight = (cardinalAngle / 2) - 1;
        }

        int upRightInitial = initialRight + 1;
        int rightInitial = upRightInitial + diagonalAngle ;
        int downRightInitial = rightInitial + cardinalAngle;
        int downInitial = downRightInitial + diagonalAngle;
        int downLeftInitial = downInitial + cardinalAngle;
        int leftInitial = downLeftInitial + diagonalAngle;
        int upLeftInitial = leftInitial + cardinalAngle;

        //*
        double initialBearing = bearing;
        bearing = round(bearing);
        if (bearing <= initialRight || bearing >= initialLeft)
        {
            currentDirection = StickUp;
            eventbutton2 = buttons.value(StickUp);
        }
        else if (bearing >= upRightInitial && bearing < rightInitial)
        {
            currentDirection = StickRightUp;
            eventbutton1 = buttons.value(StickRight);
            eventbutton2 = buttons.value(StickUp);
        }
        else if (bearing >= rightInitial && bearing < downRightInitial)
        {
            currentDirection = StickRight;
            eventbutton1 = buttons.value(StickRight);
        }
        else if (bearing >= downRightInitial && bearing < downInitial)
        {
            currentDirection = StickRightDown;
            eventbutton1 = buttons.value(StickRight);
            eventbutton2 = buttons.value(StickDown);
        }
        else if (bearing >= downInitial && bearing < downLeftInitial)
        {
            currentDirection = StickDown;
            eventbutton2 = buttons.value(StickDown);
        }
        else if (bearing >= downLeftInitial && bearing < leftInitial)
        {
            currentDirection = StickLeftDown;
            eventbutton1 = buttons.value(StickLeft);
            eventbutton2 = buttons.value(StickDown);
        }
        else if (bearing >= leftInitial && bearing < upLeftInitial)
        {
            currentDirection = StickLeft;
            eventbutton1 = buttons.value(StickLeft);
        }
        else if (bearing >= upLeftInitial && bearing < initialLeft)
        {
            currentDirection = StickLeftUp;
            eventbutton1 = buttons.value(StickLeft);
            eventbutton2 = buttons.value(StickUp);
        }
        //*/

        /*
        int pedaltothemetal = (int)round(bearing/45.0);
        int direction = ((int)round(bearing/45.0)) % 8;
        direction += 1;
        currentDirection = (JoyStickDirections)direction;
        //*/

        /*
        if (direction == StickUp)
        {
            eventbutton2 = buttons.value(StickUp);
        }
        else if (direction == StickRightUp)
        {
            eventbutton1 = buttons.value(StickRight);
            eventbutton2 = buttons.value(StickUp);
        }
        else if (direction == StickRight)
        {
            eventbutton1 = buttons.value(StickRight);
        }
        else if (direction  == StickRightDown)
        {
            eventbutton1 = buttons.value(StickRight);
            eventbutton2 = buttons.value(StickDown);
        }
        else if (direction == StickDown)
        {
            eventbutton2 = buttons.value(StickDown);
        }
        else if (direction == StickLeftDown)
        {
            eventbutton1 = buttons.value(StickLeft);
            eventbutton2 = buttons.value(StickDown);
        }
        else if (direction == StickLeft)
        {
            eventbutton1 = buttons.value(StickLeft);
        }
        else if (direction == StickLeftUp)
        {
            eventbutton1 = buttons.value(StickLeft);
            eventbutton2 = buttons.value(StickUp);
        }
        //*/
    }

    if (eventbutton2 || activeButton2)
    {
        changeButtonEvent(eventbutton2, activeButton2, ignoresets);
    }

    if (eventbutton1 || activeButton1)
    {
        changeButtonEvent(eventbutton1, activeButton1, ignoresets);
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
        qDebug() << "KLJDLJKDL: " << activebutton->getName() << endl;
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

void JoyControlStick::setIndex(int index)
{
    this->index = index;
}

int JoyControlStick::getIndex()
{
    return index;
}

int JoyControlStick::getRealJoyIndex()
{
    return index+1;
}

QString JoyControlStick::getName()
{
    QString label(tr("Stick"));
    label.append(" ").append(QString::number(getRealJoyIndex()));
    return label;
}

int JoyControlStick::getMaxZone()
{
    return maxZone;
}

int JoyControlStick::getCurrentlyAssignedSet()
{
    return originset;
}

void JoyControlStick::reset()
{
    deadZone = 8000;
    maxZone = JoyAxis::AXISMAXZONE;
    diagonalRange = 0;
    isActive = false;

    /*if (activeButton1)
    {
        activeButton1->reset();
    }
    activeButton1 = 0;

    if (activeButton2)
    {
        activeButton2->reset();
    }

    activeButton2 = 0;*/

    activeButton1 = 0;
    activeButton2 = 0;
    safezone = false;
    currentDirection = StickCentered;
    resetButtons();
}

void JoyControlStick::setDeadZone(int value)
{
    value = abs(value);
    if (value > JoyAxis::AXISMAX)
    {
        value = JoyAxis::AXISMAX;
    }

    deadZone = value;
}

void JoyControlStick::setMaxZone(int value)
{
    value = abs(value);
    if (value >= JoyAxis::AXISMAX)
    {
        value = JoyAxis::AXISMAX;
    }

    maxZone = value;
}

void JoyControlStick::setDiagonalRange(int value)
{
    if (value < 1)
    {
        value = 1;
    }
    else if (value > 99)
    {
        value = 99;
    }

    diagonalRange = value;
}

void JoyControlStick::refreshButtons()
{
    deleteButtons();
    populateButtons();
}

void JoyControlStick::deleteButtons()
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

void JoyControlStick::readConfig(QXmlStreamReader *xml)
{

}

void JoyControlStick::writeConfig(QXmlStreamWriter *xml)
{

}

void JoyControlStick::resetButtons()
{
    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyButton *button = iter.next().value();
        if (button)
        {
            button->reset();
        }
    }
}

JoyControlStickButton* JoyControlStick::getDirectionButton(JoyStickDirections direction)
{
    JoyControlStickButton *button = 0;
    switch (direction)
    {
        case StickUp:
        case StickDown:
        case StickLeft:
        case StickRight:
        {
            button = buttons.value(direction);
            break;
        }
        default:
        {
            break;
        }
    }

    return button;
}

double JoyControlStick::calculateNormalizedAxis1Placement()
{
    return axis1->calculateNormalizedAxisPlacement();
}

double JoyControlStick::calculateNormalizedAxis2Placement()
{
    return axis2->calculateNormalizedAxisPlacement();
}

double JoyControlStick::calculateDirectionalDistance(JoyControlStickButton *button)
{
    /*
    double finalDistance = 0.0;

    double bearing = calculateBearing();
    int direction = ((int)round(bearing/45.0)) % 8;

    if (direction == 0)
    {
        finalDistance = axis1->getCurrentRawValue() / (double)JoyAxis::AXISMAXZONE;
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

    return finalDistance;
    //*/
    double finalDistance = 0.0;

    if (currentDirection == StickUp)
    {
        //finalDistance = axis2->getCurrentRawValue() / (double)JoyAxis::AXISMAXZONE;
        finalDistance = axis2->getAbsoluteAxisPlacement();
    }
    else if (currentDirection == StickRightUp)
    {
        if (activeButton1 && activeButton1 == button)
        {
            finalDistance = axis1->getAbsoluteAxisPlacement();
        }
        else if (activeButton2 && activeButton2 == button)
        {
            finalDistance = axis2->getAbsoluteAxisPlacement();
        }
    }
    else if (currentDirection == StickRight)
    {
        finalDistance = axis1->getAbsoluteAxisPlacement();
    }
    else if (currentDirection  == StickRightDown)
    {
        if (activeButton1 && activeButton1 == button)
        {
            finalDistance = axis1->getAbsoluteAxisPlacement();
        }
        else if (activeButton2 && activeButton2 == button)
        {
            finalDistance = axis2->getAbsoluteAxisPlacement();
        }
    }
    else if (currentDirection == StickDown)
    {
        finalDistance = axis2->getAbsoluteAxisPlacement();
    }
    else if (currentDirection == StickLeftDown)
    {
        if (activeButton1 && activeButton1 == button)
        {
            finalDistance = axis1->getAbsoluteAxisPlacement();
        }
        else if (activeButton2 && activeButton2 == button)
        {
            finalDistance = axis2->getAbsoluteAxisPlacement();
        }
    }
    else if (currentDirection == StickLeft)
    {
        finalDistance = axis1->getAbsoluteAxisPlacement();
    }
    else if (currentDirection == StickLeftUp)
    {
        if (activeButton1 && activeButton1 == button)
        {
            finalDistance = axis1->getAbsoluteAxisPlacement();
        }
        else if (activeButton2 && activeButton2 == button)
        {
            finalDistance = axis2->getAbsoluteAxisPlacement();
        }
    }

    return finalDistance;
}

JoyControlStick::JoyStickDirections JoyControlStick::getCurrentDirection()
{
    return currentDirection;
}
