#include <QDebug>
#include <QHashIterator>
#include <QStringList>
#include <cmath>

#include "joycontrolstick.h"

const double JoyControlStick::PI = acos(-1.0);

JoyControlStick::JoyControlStick(JoyAxis *axis1, JoyAxis *axis2, int index, int originset, QObject *parent) :
    QObject(parent)
{
    this->axisX = axis1;
    this->axisX->setControlStick(this);
    this->axisY = axis2;
    this->axisY->setControlStick(this);

    this->index = index;
    this->originset = originset;
    reset();

    populateButtons();
}

JoyControlStick::~JoyControlStick()
{
    axisX->removeControlStick();
    axisY->removeControlStick();

    deleteButtons();
}

void JoyControlStick::joyEvent(bool ignoresets)
{
    safezone = !inDeadZone();

    if (safezone && !isActive)
    {
        isActive = true;
        emit active(axisX->getCurrentRawValue(), axisY->getCurrentRawValue());
        createDeskEvent(ignoresets);
    }
    else if (!safezone && isActive)
    {
        isActive = false;
        currentDirection = StickCentered;
        emit released(axisX->getCurrentRawValue(), axisY->getCurrentRawValue());

        createDeskEvent(ignoresets);
    }
    else if (isActive)
    {
        createDeskEvent(ignoresets);
    }

    emit moved(axisX->getCurrentRawValue(), axisY->getCurrentRawValue());
}

bool JoyControlStick::inDeadZone()
{
    int axis1Value = axisX->getCurrentRawValue();
    int axis2Value = axisY->getCurrentRawValue();

    unsigned int squareDist = (unsigned int)(axis1Value*axis1Value) + (unsigned int)(axis2Value*axis2Value);

    return squareDist <= (unsigned int)(deadZone*deadZone);
}

void JoyControlStick::populateButtons()
{
    JoyControlStickButton *button = new JoyControlStickButton (this, StickUp, originset, getParentSet(), this);
    buttons.insert(StickUp, button);

    button = new JoyControlStickButton (this, StickDown, originset, getParentSet(), this);
    buttons.insert(StickDown, button);

    button = new JoyControlStickButton(this, StickLeft, originset, getParentSet(), this);
    buttons.insert(StickLeft, button);

    button = new JoyControlStickButton(this, StickRight, originset, getParentSet(), this);
    buttons.insert(StickRight, button);

    button = new JoyControlStickButton(this, StickLeftUp, originset, getParentSet(), this);
    buttons.insert(StickLeftUp, button);

    button = new JoyControlStickButton(this, StickLeftDown, originset, getParentSet(), this);
    buttons.insert(StickLeftDown, button);

    button = new JoyControlStickButton(this, StickRightDown, originset, getParentSet(), this);
    buttons.insert(StickRightDown, button);

    button = new JoyControlStickButton(this, StickRightUp, originset, getParentSet(), this);
    buttons.insert(StickRightUp, button);
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
    JoyControlStickButton *eventbutton3 = 0;

    if (safezone)
    {
        double bearing = calculateBearing();

        QList<int> anglesList = getDiagonalZoneAngles();
        int initialLeft = anglesList.value(0);
        int initialRight = anglesList.value(1);
        int upRightInitial = anglesList.value(2);
        int rightInitial = anglesList.value(3);
        int downRightInitial = anglesList.value(4);
        int downInitial = anglesList.value(5);
        int downLeftInitial = anglesList.value(6);
        int leftInitial = anglesList.value(7);
        int upLeftInitial = anglesList.value(8);

        bearing = floor(bearing + 0.5);
        if (bearing <= initialRight || bearing >= initialLeft)
        {
            currentDirection = StickUp;
            eventbutton2 = buttons.value(StickUp);
        }
        else if (bearing >= upRightInitial && bearing < rightInitial)
        {
            currentDirection = StickRightUp;
            if (currentMode == EightWayMode && buttons.contains(StickRightUp))
            {
                eventbutton3 = buttons.value(StickRightUp);
            }
            else
            {
                eventbutton1 = buttons.value(StickRight);
                eventbutton2 = buttons.value(StickUp);
            }
        }
        else if (bearing >= rightInitial && bearing < downRightInitial)
        {
            currentDirection = StickRight;
            eventbutton1 = buttons.value(StickRight);
        }
        else if (bearing >= downRightInitial && bearing < downInitial)
        {
            currentDirection = StickRightDown;
            if (currentMode == EightWayMode && buttons.contains(StickRightDown))
            {
                eventbutton3 = buttons.value(StickRightDown);
            }
            else
            {
                eventbutton1 = buttons.value(StickRight);
                eventbutton2 = buttons.value(StickDown);
            }
        }
        else if (bearing >= downInitial && bearing < downLeftInitial)
        {
            currentDirection = StickDown;
            eventbutton2 = buttons.value(StickDown);
        }
        else if (bearing >= downLeftInitial && bearing < leftInitial)
        {
            currentDirection = StickLeftDown;
            if (currentMode == EightWayMode && buttons.contains(StickLeftDown))
            {
                eventbutton3 = buttons.value(StickLeftDown);
            }
            else
            {
                eventbutton1 = buttons.value(StickLeft);
                eventbutton2 = buttons.value(StickDown);
            }
        }
        else if (bearing >= leftInitial && bearing < upLeftInitial)
        {
            currentDirection = StickLeft;
            eventbutton1 = buttons.value(StickLeft);
        }
        else if (bearing >= upLeftInitial && bearing < initialLeft)
        {
            currentDirection = StickLeftUp;
            if (currentMode == EightWayMode && buttons.contains(StickLeftUp))
            {
                eventbutton3 = buttons.value(StickLeftUp);
            }
            else
            {
                eventbutton1 = buttons.value(StickLeft);
                eventbutton2 = buttons.value(StickUp);
            }
        }
    }

    // Release any currently active stick buttons
    if (!eventbutton1 && activeButton1)
    {
        // Currently in deadzone. Disable currently active button.
        performButtonRelease(activeButton1, ignoresets);
    }
    else if (eventbutton1 && activeButton1 && eventbutton1 != activeButton1)
    {
        // Deadzone skipped. Button for new event is not the currently
        // active button. Disable the active button.
        performButtonRelease(activeButton1, ignoresets);
    }

    if (!eventbutton2 && activeButton2)
    {
        // Currently in deadzone. Disable currently active button.
        performButtonRelease(activeButton2, ignoresets);
    }
    else if (eventbutton2 && activeButton2 && eventbutton2 != activeButton2)
    {
        // Deadzone skipped. Button for new event is not the currently
        // active button. Disable the active button.
        performButtonRelease(activeButton2, ignoresets);
    }

    if (!eventbutton3 && activeButton3)
    {
        // Currently in deadzone. Disable currently active button.
        performButtonRelease(activeButton3, ignoresets);
    }
    else if (eventbutton3 && activeButton3 && eventbutton3 != activeButton3)
    {
        // Deadzone skipped. Button for new event is not the currently
        // active button. Disable the active button.
        performButtonRelease(activeButton3, ignoresets);
    }

    // Enable stick buttons
    if (eventbutton1 && !activeButton1)
    {
        // There is no active button. Call joyEvent and set current
        // button as active button
        performButtonPress(eventbutton1, activeButton1, ignoresets);
    }
    else if (eventbutton1 && activeButton1 && eventbutton1 == activeButton1)
    {
        // Button is currently active. Just pass current value
        performButtonPress(eventbutton1, activeButton1, ignoresets);
    }

    if (eventbutton2 && !activeButton2)
    {
        // There is no active button. Call joyEvent and set current
        // button as active button
        performButtonPress(eventbutton2, activeButton2, ignoresets);
    }
    else if (eventbutton2 && activeButton2 && eventbutton2 == activeButton2)
    {
        // Button is currently active. Just pass current value
        performButtonPress(eventbutton2, activeButton2, ignoresets);
    }

    if (eventbutton3 && !activeButton3)
    {
        // There is no active button. Call joyEvent and set current
        // button as active button
        performButtonPress(eventbutton3, activeButton3, ignoresets);
    }
    else if (eventbutton3 && activeButton3 && eventbutton3 == activeButton3)
    {
        // Button is currently active. Just pass current value
        performButtonPress(eventbutton3, activeButton3, ignoresets);
    }

    /*if (eventbutton2 || activeButton2)
    {
        changeButtonEvent(eventbutton2, activeButton2, ignoresets);
    }

    if (eventbutton1 || activeButton1)
    {
        changeButtonEvent(eventbutton1, activeButton1, ignoresets);
    }

    if (eventbutton3 || activeButton3)
    {
        changeButtonEvent(eventbutton3, activeButton3, ignoresets);
    }*/
}

double JoyControlStick::calculateBearing()
{
    double finalAngle = 0.0;

    if (axisX->getCurrentRawValue() == 0 && axisY->getCurrentRawValue() == 0)
    {
        finalAngle = 0.0;
    }
    else
    {
        double temp1 = axisX->getCurrentRawValue() / (double)maxZone;
        double temp2 = axisY->getCurrentRawValue() / (double)maxZone;

        double angle = (atan2(temp1, -temp2) * 180) / PI;

        if (axisX->getCurrentRawValue() >= 0 && axisY->getCurrentRawValue() <= 0)
        {
            // NE Quadrant
            finalAngle = angle;
        }
        else if (axisX->getCurrentRawValue() >= 0 && axisY->getCurrentRawValue() >= 0)
        {
            // SE Quadrant (angle will be positive)
            finalAngle = angle;
        }
        else if (axisX->getCurrentRawValue() <= 0 && axisY->getCurrentRawValue() >= 0)
        {
            // SW Quadrant (angle will be negative)
            finalAngle = 360.0 + angle;
        }
        else if (axisX->getCurrentRawValue() <= 0 && axisY->getCurrentRawValue() <= 0)
        {
            // NW Quadrant (angle will be negative)
            finalAngle = 360.0 + angle;
        }
    }

    return finalAngle;
}

double JoyControlStick::getDistanceFromDeadZone()
{
    double distance = 0.0;

    int axis1Value = axisX->getCurrentRawValue();
    int axis2Value = axisY->getCurrentRawValue();

    unsigned int square_dist = (unsigned int)(axis1Value*axis1Value) + (unsigned int)(axis2Value*axis2Value);

    distance = (sqrt(square_dist) - deadZone)/(double)(maxZone - deadZone);
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

double JoyControlStick::calculateXDistanceFromDeadZone()
{
    double distance = 0.0;

    int axis1Value = axisX->getCurrentRawValue();

    double relativeAngle = calculateBearing();
    if (relativeAngle > 180)
    {
        relativeAngle = relativeAngle - 180;
    }

    int deadX = (int)floor(deadZone * sin(relativeAngle * PI / 180.0) + 0.5);
    distance = (abs(axis1Value) - deadX)/(double)(maxZone - deadX);
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

double JoyControlStick::calculateYDistanceFromDeadZone()
{
    double distance = 0.0;

    int axis2Value = axisY->getCurrentRawValue();

    double relativeAngle = calculateBearing();
    if (relativeAngle > 180)
    {
        relativeAngle = relativeAngle - 180;
    }

    int deadY = abs(floor(deadZone * cos(relativeAngle * PI / 180.0)) + 0.5);
    distance = (abs(axis2Value) - deadY)/(double)(maxZone - deadY);
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

double JoyControlStick::getAbsoluteDistance()
{
    double distance = 0.0;

    int axis1Value = axisX->getCurrentRawValue();
    int axis2Value = axisY->getCurrentRawValue();

    unsigned int square_dist = (unsigned int)(axis1Value*axis1Value) + (unsigned int)(axis2Value*axis2Value);

    distance = sqrt(square_dist);
    if (distance > JoyAxis::AXISMAX)
    {
        distance = JoyAxis::AXISMAX;
    }
    else if (distance < 0.0)
    {
        distance = 0.0;
    }

    return distance;
}

double JoyControlStick::getNormalizedAbsoluteDistance()
{
    double distance = 0.0;

    int axis1Value = axisX->getCurrentRawValue();
    int axis2Value = axisY->getCurrentRawValue();

    unsigned int square_dist = (unsigned int)(axis1Value*axis1Value) + (unsigned int)(axis2Value*axis2Value);

    distance = sqrt(square_dist)/(double)(maxZone);
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

QString JoyControlStick::getName(bool forceFullFormat, bool displayNames)
{
    QString label = getPartialName(forceFullFormat, displayNames);

    label.append(": ");
    QStringList tempList;
    if (buttons.contains(StickUp))
    {
        JoyControlStickButton *button = buttons.value(StickUp);
        if (!button->getButtonName().isEmpty())
        {
            tempList.append(button->getButtonName());
        }
        else
        {
            tempList.append(button->getSlotsSummary());
        }
    }

    if (buttons.contains(StickLeft))
    {
        JoyControlStickButton *button = buttons.value(StickLeft);
        if (!button->getButtonName().isEmpty())
        {
            tempList.append(button->getButtonName());
        }
        else
        {
            tempList.append(button->getSlotsSummary());
        }
    }

    if (buttons.contains(StickDown))
    {
        JoyControlStickButton *button = buttons.value(StickDown);
        if (!button->getButtonName().isEmpty())
        {
            tempList.append(button->getButtonName());
        }
        else
        {
            tempList.append(button->getSlotsSummary());
        }
    }

    if (buttons.contains(StickRight))
    {
        JoyControlStickButton *button = buttons.value(StickRight);
        if (!button->getButtonName().isEmpty())
        {
            tempList.append(button->getButtonName());
        }
        else
        {
            tempList.append(button->getSlotsSummary());
        }
    }

    label.append(tempList.join(", "));
    return label;
}

QString JoyControlStick::getPartialName(bool forceFullFormat, bool displayNames)
{
    QString label;

    if (!stickName.isEmpty() && displayNames)
    {
        if (forceFullFormat)
        {
            label.append(tr("Stick")).append(" ");
        }

        label.append(stickName);
    }
    else if (!defaultStickName.isEmpty())
    {
        if (forceFullFormat)
        {
            label.append(tr("Stick")).append(" ");
        }

        label.append(defaultStickName);
    }
    else
    {
        label.append(tr("Stick")).append(" ");
        label.append(QString::number(getRealJoyIndex()));
    }

    return label;
}

void JoyControlStick::setDefaultStickName(QString tempname)
{
    defaultStickName = tempname;
    emit stickNameChanged();
}

QString JoyControlStick::getDefaultStickName()
{
    return defaultStickName;
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
    diagonalRange = 45;
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
    activeButton3 = 0;
    safezone = false;
    currentDirection = StickCentered;
    currentMode = StandardMode;
    stickName.clear();
    resetButtons();
}

void JoyControlStick::setDeadZone(int value)
{
    value = abs(value);
    if (value > JoyAxis::AXISMAX)
    {
        value = JoyAxis::AXISMAX;
    }

    if (value != deadZone && value < maxZone)
    {
        deadZone = value;
        emit deadZoneChanged(value);
    }
}

void JoyControlStick::setMaxZone(int value)
{
    value = abs(value);
    if (value >= JoyAxis::AXISMAX)
    {
        value = JoyAxis::AXISMAX;
    }

    if (value != maxZone && value > deadZone)
    {
        maxZone = value;
        emit maxZoneChanged(value);
    }
}

void JoyControlStick::setDiagonalRange(int value)
{
    if (value < 1)
    {
        value = 1;
    }
    else if (value > 89)
    {
        value = 89;
    }

    if (value != diagonalRange)
    {
        diagonalRange = value;
        emit diagonalRangeChanged(value);
    }
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
    if (xml->isStartElement() && xml->name() == "stick")
    {
        xml->readNextStartElement();

        while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != "stick"))
        {
            if (xml->name() == "deadZone" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setDeadZone(tempchoice);
            }
            else if (xml->name() == "maxZone" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setMaxZone(tempchoice);
            }
            else if (xml->name() == "diagonalRange" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setDiagonalRange(tempchoice);
            }
            else if (xml->name() == "mode" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                if (temptext == "eight-way")
                {
                    this->setJoyMode(EightWayMode);
                }
            }
            else if (xml->name() == JoyControlStickButton::xmlName && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyControlStickButton *button = buttons.value((JoyStickDirections)index);
                if (button)
                {
                    button->readConfig(xml);
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else
            {
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }
    }
}

void JoyControlStick::writeConfig(QXmlStreamWriter *xml)
{
    if (!isDefault())
    {
        xml->writeStartElement("stick");
        xml->writeAttribute("index", QString::number(index+1));
        xml->writeTextElement("deadZone", QString::number(deadZone));
        xml->writeTextElement("maxZone", QString::number(maxZone));
        xml->writeTextElement("diagonalRange", QString::number(diagonalRange));
        if (currentMode == EightWayMode)
        {
            xml->writeTextElement("mode", "eight-way");
        }

        QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(buttons);
        while (iter.hasNext())
        {
            JoyControlStickButton *button = iter.next().value();
            button->writeConfig(xml);
        }

        xml->writeEndElement();
    }
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
    JoyControlStickButton *button = buttons.value(direction);
    return button;
}

double JoyControlStick::calculateNormalizedAxis1Placement()
{
    return axisX->calculateNormalizedAxisPlacement();
}

double JoyControlStick::calculateNormalizedAxis2Placement()
{
    return axisY->calculateNormalizedAxisPlacement();
}

double JoyControlStick::calculateDirectionalDistance(JoyControlStickButton *button, JoyButton::JoyMouseMovementMode mouseMode)
{
    Q_UNUSED(mouseMode);

    double finalDistance = 0.0;

    if (currentDirection == StickUp)
    {
        finalDistance = calculateYDistanceFromDeadZone();
    }
    else if (currentDirection == StickRightUp)
    {
        if (activeButton1 && activeButton1 == button)
        {
            finalDistance = calculateXDistanceFromDeadZone();
        }
        else if (activeButton2 && activeButton2 == button)
        {
            finalDistance = calculateYDistanceFromDeadZone();
        }
        else if (activeButton3 && activeButton3 == button)
        {
            double radius = getDistanceFromDeadZone();
            double bearing = calculateBearing();
            int relativeBearing = (int)floor(bearing + 0.5) % 90;
            //bearing = round(bearing) % 90;
            int diagonalAngle = relativeBearing;
            if (relativeBearing > 45)
            {
                diagonalAngle = 90 - relativeBearing;
            }

            finalDistance = radius * (diagonalAngle / 45.0);
        }
    }
    else if (currentDirection == StickRight)
    {
        finalDistance = calculateXDistanceFromDeadZone();
    }
    else if (currentDirection  == StickRightDown)
    {
        if (activeButton1 && activeButton1 == button)
        {
            finalDistance = calculateXDistanceFromDeadZone();
        }
        else if (activeButton2 && activeButton2 == button)
        {
            finalDistance = calculateYDistanceFromDeadZone();
        }
        else if (activeButton3 && activeButton3 == button)
        {
            double radius = getDistanceFromDeadZone();
            double bearing = calculateBearing();
            int relativeBearing = (int)floor(bearing + 0.5) % 90;
            //bearing = round(bearing) % 90;
            int diagonalAngle = relativeBearing;
            if (relativeBearing > 45)
            {
                diagonalAngle = 90 - relativeBearing;
            }

            finalDistance = radius * (diagonalAngle / 45.0);
        }
    }
    else if (currentDirection == StickDown)
    {
        finalDistance = calculateYDistanceFromDeadZone();
    }
    else if (currentDirection == StickLeftDown)
    {
        if (activeButton1 && activeButton1 == button)
        {
            finalDistance = calculateXDistanceFromDeadZone();
        }
        else if (activeButton2 && activeButton2 == button)
        {
            finalDistance = calculateYDistanceFromDeadZone();
        }
        else if (activeButton3 && activeButton3 == button)
        {
            double radius = getDistanceFromDeadZone();
            double bearing = calculateBearing();
            int relativeBearing = (int)floor(bearing + 0.5) % 90;
            //bearing = round(bearing) % 90;
            int diagonalAngle = relativeBearing;
            if (relativeBearing > 45)
            {
                diagonalAngle = 90 - relativeBearing;
            }

            finalDistance = radius * (diagonalAngle / 45.0);
        }
    }
    else if (currentDirection == StickLeft)
    {
        finalDistance = calculateXDistanceFromDeadZone();
    }
    else if (currentDirection == StickLeftUp)
    {
        if (activeButton1 && activeButton1 == button)
        {
            finalDistance = calculateXDistanceFromDeadZone();
        }
        else if (activeButton2 && activeButton2 == button)
        {
            finalDistance = calculateYDistanceFromDeadZone();
        }
        else if (activeButton3 && activeButton3 == button)
        {
            double radius = getDistanceFromDeadZone();
            double bearing = calculateBearing();
            int relativeBearing = (int)floor(bearing + 0.5) % 90;
            //bearing = round(bearing) % 90;
            int diagonalAngle = relativeBearing;
            if (relativeBearing > 45)
            {
                diagonalAngle = 90 - relativeBearing;
            }

            finalDistance = radius * (diagonalAngle / 45.0);
        }
    }

    return finalDistance;
}

JoyControlStick::JoyStickDirections JoyControlStick::getCurrentDirection()
{
    return currentDirection;
}

int JoyControlStick::getXCoordinate()
{
    return axisX->getCurrentRawValue();
}

int JoyControlStick::getYCoordinate()
{
    return axisY->getCurrentRawValue();
}

QList<int> JoyControlStick::getDiagonalZoneAngles()
{
    QList<int> anglesList;

    int diagonalAngle = diagonalRange;

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

    anglesList.append(initialLeft);
    anglesList.append(initialRight);
    anglesList.append(upRightInitial);
    anglesList.append(rightInitial);
    anglesList.append(downRightInitial);
    anglesList.append(downInitial);
    anglesList.append(downLeftInitial);
    anglesList.append(leftInitial);
    anglesList.append(upLeftInitial);

    return anglesList;
}

QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton*>* JoyControlStick::getButtons()
{
    return &buttons;
}

JoyAxis* JoyControlStick::getAxisX()
{
    return axisX;
}

JoyAxis* JoyControlStick::getAxisY()
{
    return axisY;
}

void JoyControlStick::replaceXAxis(JoyAxis *axis)
{
    if (axis->getParentSet() == axisY->getParentSet())
    {
        axisX->removeControlStick();
        this->axisX = axis;
        this->axisX->setControlStick(this);
    }
}

void JoyControlStick::replaceYAxis(JoyAxis *axis)
{
    if (axis->getParentSet() == axisX->getParentSet())
    {
        axisY->removeControlStick();
        this->axisY = axis;
        this->axisY->setControlStick(this);
    }
}

void JoyControlStick::replaceAxes(JoyAxis *axisX, JoyAxis *axisY)
{
    if (axisX->getParentSet() == axisY->getParentSet())
    {
        this->axisX->removeControlStick();
        this->axisY->removeControlStick();

        this->axisX = axisX;
        this->axisY = axisY;

        this->axisX->setControlStick(this);
        this->axisY->setControlStick(this);
    }
}

void JoyControlStick::setJoyMode(JoyMode mode)
{
    currentMode = mode;
}

JoyControlStick::JoyMode JoyControlStick::getJoyMode()
{
    return currentMode;
}

void JoyControlStick::releaseButtonEvents()
{
    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->joyEvent(false, true);
    }
}

bool JoyControlStick::isDefault()
{
    bool value = true;
    value = value && (deadZone == 8000);
    value = value && (maxZone == JoyAxis::AXISMAXZONE);
    value = value && (diagonalRange == 45);
    value = value && (currentMode == StandardMode);
    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        value = value && (button->isDefault());
    }
    return value;
}

void JoyControlStick::setButtonsMouseMode(JoyButton::JoyMouseMovementMode mode)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setMouseMode(mode);
    }
}

bool JoyControlStick::hasSameButtonsMouseMode()
{
    bool result = true;

    JoyButton::JoyMouseMovementMode initialMode = JoyButton::MouseCursor;
    QHash<JoyStickDirections, JoyControlStickButton*> temphash;
    temphash.insert(StickUp, buttons.value(StickUp));
    temphash.insert(StickDown, buttons.value(StickDown));
    temphash.insert(StickLeft, buttons.value(StickLeft));
    temphash.insert(StickRight, buttons.value(StickRight));
    if (currentMode == EightWayMode)
    {
        temphash.insert(StickLeftUp, buttons.value(StickLeftUp));
        temphash.insert(StickRightUp, buttons.value(StickRightUp));
        temphash.insert(StickRightDown, buttons.value(StickRightDown));
        temphash.insert(StickLeftDown, buttons.value(StickLeftDown));
    }

    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(temphash);
    while (iter.hasNext())
    {
        if (!iter.hasPrevious())
        {
            JoyControlStickButton *button = iter.next().value();
            initialMode = button->getMouseMode();
        }
        else
        {
            JoyControlStickButton *button = iter.next().value();
            JoyButton::JoyMouseMovementMode temp = button->getMouseMode();
            if (temp != initialMode)
            {
                result = false;
                iter.toBack();
            }
        }
    }

    return result;
}

JoyButton::JoyMouseMovementMode JoyControlStick::getButtonsPresetMouseMode()
{
    JoyButton::JoyMouseMovementMode resultMode = JoyButton::MouseCursor;

    QHash<JoyStickDirections, JoyControlStickButton*> temphash;
    temphash.insert(StickUp, buttons.value(StickUp));
    temphash.insert(StickDown, buttons.value(StickDown));
    temphash.insert(StickLeft, buttons.value(StickLeft));
    temphash.insert(StickRight, buttons.value(StickRight));
    if (currentMode == EightWayMode)
    {
        temphash.insert(StickLeftUp, buttons.value(StickLeftUp));
        temphash.insert(StickRightUp, buttons.value(StickRightUp));
        temphash.insert(StickRightDown, buttons.value(StickRightDown));
        temphash.insert(StickLeftDown, buttons.value(StickLeftDown));
    }

    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(temphash);
    while (iter.hasNext())
    {
        if (!iter.hasPrevious())
        {
            JoyControlStickButton *button = iter.next().value();
            resultMode = button->getMouseMode();
        }
        else
        {
            JoyControlStickButton *button = iter.next().value();
            JoyButton::JoyMouseMovementMode temp = button->getMouseMode();
            if (temp != resultMode)
            {
                resultMode = JoyButton::MouseCursor;
                iter.toBack();
            }
        }
    }

    return resultMode;
}

void JoyControlStick::setButtonsMouseCurve(JoyButton::JoyMouseCurve mouseCurve)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setMouseCurve(mouseCurve);
    }
}

bool JoyControlStick::hasSameButtonsMouseCurve()
{
    bool result = true;

    JoyButton::JoyMouseCurve initialCurve = JoyButton::LinearCurve;
    QHash<JoyStickDirections, JoyControlStickButton*> temphash;
    temphash.insert(StickUp, buttons.value(StickUp));
    temphash.insert(StickDown, buttons.value(StickDown));
    temphash.insert(StickLeft, buttons.value(StickLeft));
    temphash.insert(StickRight, buttons.value(StickRight));
    if (currentMode == EightWayMode)
    {
        temphash.insert(StickLeftUp, buttons.value(StickLeftUp));
        temphash.insert(StickRightUp, buttons.value(StickRightUp));
        temphash.insert(StickRightDown, buttons.value(StickRightDown));
        temphash.insert(StickLeftDown, buttons.value(StickLeftDown));
    }

    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(temphash);
    while (iter.hasNext())
    {
        if (!iter.hasPrevious())
        {
            JoyControlStickButton *button = iter.next().value();
            initialCurve = button->getMouseCurve();
        }
        else
        {
            JoyControlStickButton *button = iter.next().value();
            JoyButton::JoyMouseCurve temp = button->getMouseCurve();
            if (temp != initialCurve)
            {
                result = false;
                iter.toBack();
            }
        }
    }

    return result;
}

JoyButton::JoyMouseCurve JoyControlStick::getButtonsPresetMouseCurve()
{
    JoyButton::JoyMouseCurve resultCurve = JoyButton::LinearCurve;

    QHash<JoyStickDirections, JoyControlStickButton*> temphash;
    temphash.insert(StickUp, buttons.value(StickUp));
    temphash.insert(StickDown, buttons.value(StickDown));
    temphash.insert(StickLeft, buttons.value(StickLeft));
    temphash.insert(StickRight, buttons.value(StickRight));
    if (currentMode == EightWayMode)
    {
        temphash.insert(StickLeftUp, buttons.value(StickLeftUp));
        temphash.insert(StickRightUp, buttons.value(StickRightUp));
        temphash.insert(StickRightDown, buttons.value(StickRightDown));
        temphash.insert(StickLeftDown, buttons.value(StickLeftDown));
    }

    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(temphash);
    while (iter.hasNext())
    {
        if (!iter.hasPrevious())
        {
            JoyControlStickButton *button = iter.next().value();
            resultCurve = button->getMouseCurve();
        }
        else
        {
            JoyControlStickButton *button = iter.next().value();
            JoyButton::JoyMouseCurve temp = button->getMouseCurve();
            if (temp != resultCurve)
            {
                resultCurve = JoyButton::LinearCurve;
                iter.toBack();
            }
        }
    }

    return resultCurve;
}

void JoyControlStick::setButtonsSpringWidth(int value)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setSpringWidth(value);
    }
}

void JoyControlStick::setButtonsSpringHeight(int value)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setSpringHeight(value);
    }
}

int JoyControlStick::getButtonsPresetSpringWidth()
{
    int presetSpringWidth = 0;

    QHash<JoyStickDirections, JoyControlStickButton*> temphash;
    temphash.insert(StickUp, buttons.value(StickUp));
    temphash.insert(StickDown, buttons.value(StickDown));
    temphash.insert(StickLeft, buttons.value(StickLeft));
    temphash.insert(StickRight, buttons.value(StickRight));
    if (currentMode == EightWayMode)
    {
        temphash.insert(StickLeftUp, buttons.value(StickLeftUp));
        temphash.insert(StickRightUp, buttons.value(StickRightUp));
        temphash.insert(StickRightDown, buttons.value(StickRightDown));
        temphash.insert(StickLeftDown, buttons.value(StickLeftDown));
    }

    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(temphash);
    while (iter.hasNext())
    {
        if (!iter.hasPrevious())
        {
            JoyControlStickButton *button = iter.next().value();
            presetSpringWidth = button->getSpringWidth();
        }
        else
        {
            JoyControlStickButton *button = iter.next().value();
            int temp = button->getSpringWidth();
            if (temp != presetSpringWidth)
            {
                presetSpringWidth = 0;
                iter.toBack();
            }
        }
    }

    return presetSpringWidth;
}

int JoyControlStick::getButtonsPresetSpringHeight()
{
    int presetSpringHeight = 0;

    QHash<JoyStickDirections, JoyControlStickButton*> temphash;
    temphash.insert(StickUp, buttons.value(StickUp));
    temphash.insert(StickDown, buttons.value(StickDown));
    temphash.insert(StickLeft, buttons.value(StickLeft));
    temphash.insert(StickRight, buttons.value(StickRight));
    if (currentMode == EightWayMode)
    {
        temphash.insert(StickLeftUp, buttons.value(StickLeftUp));
        temphash.insert(StickRightUp, buttons.value(StickRightUp));
        temphash.insert(StickRightDown, buttons.value(StickRightDown));
        temphash.insert(StickLeftDown, buttons.value(StickLeftDown));
    }

    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(temphash);
    while (iter.hasNext())
    {
        if (!iter.hasPrevious())
        {
            JoyControlStickButton *button = iter.next().value();
            presetSpringHeight = button->getSpringHeight();
        }
        else
        {
            JoyControlStickButton *button = iter.next().value();
            int temp = button->getSpringHeight();
            if (temp != presetSpringHeight)
            {
                presetSpringHeight = 0;
                iter.toBack();
            }
        }
    }

    return presetSpringHeight;
}

void JoyControlStick::setButtonsSensitivity(double value)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setSensitivity(value);
    }
}

double JoyControlStick::getButtonsPresetSensitivity()
{
    double presetSensitivity = 1.0;

    QHash<JoyStickDirections, JoyControlStickButton*> temphash;
    temphash.insert(StickUp, buttons.value(StickUp));
    temphash.insert(StickDown, buttons.value(StickDown));
    temphash.insert(StickLeft, buttons.value(StickLeft));
    temphash.insert(StickRight, buttons.value(StickRight));
    if (currentMode == EightWayMode)
    {
        temphash.insert(StickLeftUp, buttons.value(StickLeftUp));
        temphash.insert(StickRightUp, buttons.value(StickRightUp));
        temphash.insert(StickRightDown, buttons.value(StickRightDown));
        temphash.insert(StickLeftDown, buttons.value(StickLeftDown));
    }

    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(temphash);
    while (iter.hasNext())
    {
        if (!iter.hasPrevious())
        {
            JoyControlStickButton *button = iter.next().value();
            presetSensitivity = button->getSensitivity();
        }
        else
        {
            JoyControlStickButton *button = iter.next().value();
            double temp = button->getSensitivity();
            if (temp != presetSensitivity)
            {
                presetSensitivity = 1.0;
                iter.toBack();
            }
        }
    }

    return presetSensitivity;
}

QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton*> JoyControlStick::getApplicableButtons()
{
    QHash<JoyStickDirections, JoyControlStickButton*> temphash;
    temphash.insert(StickUp, buttons.value(StickUp));
    temphash.insert(StickDown, buttons.value(StickDown));
    temphash.insert(StickLeft, buttons.value(StickLeft));
    temphash.insert(StickRight, buttons.value(StickRight));
    if (currentMode == EightWayMode)
    {
        temphash.insert(StickLeftUp, buttons.value(StickLeftUp));
        temphash.insert(StickRightUp, buttons.value(StickRightUp));
        temphash.insert(StickRightDown, buttons.value(StickRightDown));
        temphash.insert(StickLeftDown, buttons.value(StickLeftDown));
    }

    return temphash;
}

void JoyControlStick::setButtonsSmoothing(bool enabled)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setSmoothing(enabled);
    }
}

bool JoyControlStick::getButtonsPresetSmoothing()
{
    bool presetSmoothing = false;

    QHash<JoyStickDirections, JoyControlStickButton*> temphash = getApplicableButtons();
    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(temphash);
    while (iter.hasNext())
    {
        if (!iter.hasPrevious())
        {
            JoyControlStickButton *button = iter.next().value();
            presetSmoothing = button->isSmoothingEnabled();
        }
        else
        {
            JoyControlStickButton *button = iter.next().value();
            bool temp = button->isSmoothingEnabled();
            if (temp != presetSmoothing)
            {
                presetSmoothing = false;
                iter.toBack();
            }
        }
    }

    return presetSmoothing;
}

void JoyControlStick::setStickName(QString tempName)
{
    if (tempName.length() <= 20 && tempName != stickName)
    {
        stickName = tempName;
        emit stickNameChanged();
    }
}

QString JoyControlStick::getStickName()
{
    return stickName;
}

void JoyControlStick::setButtonsWheelSpeedX(int value)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setWheelSpeedX(value);
    }
}

void JoyControlStick::setButtonsWheelSpeedY(int value)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setWheelSpeedY(value);
    }
}

SetJoystick* JoyControlStick::getParentSet()
{
    SetJoystick *temp = 0;
    if (axisX)
    {
        temp = axisX->getParentSet();
    }
    else if (axisY)
    {
        temp = axisY->getParentSet();
    }
    return temp;
}

void JoyControlStick::performButtonPress(JoyControlStickButton *eventbutton, JoyControlStickButton *&activebutton, bool ignoresets)
{
    eventbutton->joyEvent(true, ignoresets);
    activebutton = eventbutton;
}

void JoyControlStick::performButtonRelease(JoyControlStickButton *&eventbutton, bool ignoresets)
{
    eventbutton->joyEvent(false, ignoresets);
    eventbutton = 0;
}
