//#include <QDebug>
#include <QHashIterator>
#include <QStringList>
#include <cmath>

#include "joycontrolstick.h"
#include "inputdevice.h"
//#include "antkeymapper.h"

// Define Pi here.
const double JoyControlStick::PI = acos(-1.0);

// Set default values used for stick properties.
const int JoyControlStick::DEFAULTDEADZONE = 8000;
const int JoyControlStick::DEFAULTMAXZONE = JoyAxis::AXISMAXZONE;
const int JoyControlStick::DEFAULTDIAGONALRANGE = 45;
const JoyControlStick::JoyMode JoyControlStick::DEFAULTMODE = JoyControlStick::StandardMode;
const double JoyControlStick::DEFAULTCIRCLE = 0.0;
const unsigned int JoyControlStick::DEFAULTSTICKDELAY = 0;

JoyControlStick::JoyControlStick(JoyAxis *axis1, JoyAxis *axis2, int index, int originset, QObject *parent) :
    QObject(parent)
{
    this->axisX = axis1;
    this->axisX->setControlStick(this);
    this->axisY = axis2;
    this->axisY->setControlStick(this);

    this->index = index;
    this->originset = originset;
    this->modifierButton = 0;
    reset();

    populateButtons();

    directionDelayTimer.setSingleShot(true);
    connect(&directionDelayTimer, SIGNAL(timeout()), this, SLOT(stickDirectionChangeEvent()));
}

JoyControlStick::~JoyControlStick()
{
    axisX->removeControlStick(false);
    axisY->removeControlStick(false);

    deleteButtons();
}

void JoyControlStick::joyEvent(bool ignoresets)
{
    safezone = !inDeadZone();

    if (safezone && !isActive)
    {
        isActive = true;
        emit active(axisX->getCurrentRawValue(), axisY->getCurrentRawValue());
        if (ignoresets || stickDelay == 0)
        {
            if (directionDelayTimer.isActive())
            {
                directionDelayTimer.stop();
            }

            createDeskEvent(ignoresets);
        }
        else
        {
            //createDeskEvent(ignoresets);
            //directionDelayTimer.start(stickDelay);
            if (!directionDelayTimer.isActive())
            {
                directionDelayTimer.start(stickDelay);
            }
        }

        //createDeskEvent(ignoresets);
    }
    else if (!safezone && isActive)
    {
        isActive = false;
        //currentDirection = StickCentered;
        emit released(axisX->getCurrentRawValue(), axisY->getCurrentRawValue());
        //directionDelayTimer.stop();
        //createDeskEvent(ignoresets);
        if (ignoresets || stickDelay == 0)
        {
            if (directionDelayTimer.isActive())
            {
                directionDelayTimer.stop();
            }

            createDeskEvent(ignoresets);
        }
        else
        {
            //createDeskEvent(ignoresets);
            //directionDelayTimer.start(stickDelay);
            if (!directionDelayTimer.isActive())
            {
                directionDelayTimer.start(stickDelay);
            }
        }
    }
    else if (isActive)
    {
        if (ignoresets || stickDelay == 0)
        {
            if (directionDelayTimer.isActive())
            {
                directionDelayTimer.stop();
            }

            createDeskEvent(ignoresets);
        }
        else
        {
            JoyStickDirections pendingDirection = calculateStickDirection();
            if (currentDirection != pendingDirection)
            {
                if (!directionDelayTimer.isActive())
                {
                    directionDelayTimer.start(stickDelay);
                }
            }
            else
            {
                if (directionDelayTimer.isActive())
                {
                    directionDelayTimer.stop();
                }

                createDeskEvent(ignoresets);
            }
        }

        //createDeskEvent(ignoresets);
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

    modifierButton = new JoyControlStickModifierButton(this, originset, getParentSet(), this);
    //modifierButton->setAssignedSlot(60, JoyButtonSlot::JoyDistance);
    //modifierButton->setAssignedSlot(AntKeyMapper::getInstance()->returnVirtualKey(Qt::Key_Shift), Qt::Key_Shift);
}

int JoyControlStick::getDeadZone()
{
    return deadZone;
}

int JoyControlStick::getDiagonalRange()
{
    return diagonalRange;
}

/**
 * @brief Find the position of the two stick axes, deactivate no longer used
 *     stick direction button and then activate direction buttons for new
 *     direction.
 * @param Should set changing operations be ignored. Necessary in the middle
 *     of a set change.
 */
void JoyControlStick::createDeskEvent(bool ignoresets)
{
    JoyControlStickButton *eventbutton1 = 0;
    JoyControlStickButton *eventbutton2 = 0;
    JoyControlStickButton *eventbutton3 = 0;

    if (safezone)
    {
        if (currentMode == StandardMode)
        {
            determineStandardModeEvent(eventbutton1, eventbutton2);
        }
        else if (currentMode == EightWayMode)
        {
            determineEightWayModeEvent(eventbutton1, eventbutton2, eventbutton3);
        }
        else if (currentMode == FourWayCardinal)
        {
            determineFourWayCardinalEvent(eventbutton1, eventbutton2);
        }
        else if (currentMode == FourWayDiagonal)
        {
            determineFourWayDiagonalEvent(eventbutton3);
        }
    }
    else
    {
        currentDirection = StickCentered;
    }

    /*
     * Release any currently active stick buttons.
     */
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

    if (safezone)
    {
        // Activate modifier button before activating directional buttons.
        modifierButton->joyEvent(true, ignoresets);
    }
    else
    {
        // Release modifier button after releasing directional buttons.
        modifierButton->joyEvent(false, ignoresets);
    }

    /*
     * Enable stick buttons.
     */
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
}

/**
 * @brief Calculate the bearing (in degrees) corresponding to the current
 *    position of the X and Y axes of a stick.
 * @return Bearing (in degrees)
 */
double JoyControlStick::calculateBearing()
{
    double finalAngle = 0.0;

    if (axisX->getCurrentRawValue() == 0 && axisY->getCurrentRawValue() == 0)
    {
        finalAngle = 0.0;
    }
    else
    {
        double temp1 = axisX->getCurrentRawValue();
        double temp2 = axisY->getCurrentRawValue();

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

/**
 * @brief Get radial distance of the stick position past the assigned dead zone.
 * @return Distance percentage in the range of 0.0 - 1.0.
 */
double JoyControlStick::getDistanceFromDeadZone()
{
    double distance = 0.0;

    int axis1Value = axisX->getCurrentRawValue();
    int axis2Value = axisY->getCurrentRawValue();

    double angle2 = atan2(axis1Value, -axis2Value);
    double ang_sin = sin(angle2);
    double ang_cos = cos(angle2);

    unsigned int squared_dist = (unsigned int)(axis1Value*axis1Value) + (unsigned int)(axis2Value*axis2Value);
    unsigned int dist = sqrt(squared_dist);

    double squareStickFull = qMin(ang_sin ? 1/fabs(ang_sin) : 2, ang_cos ? 1/fabs(ang_cos) : 2);
    double circle = this->circle;
    double circleStickFull = (squareStickFull - 1) * circle + 1;
    double alternateStickFullValue = circleStickFull * JoyAxis::AXISMAX;

    int adjustedDist = circleStickFull > 1.0 ? (dist / alternateStickFullValue) * JoyAxis::AXISMAX : dist;
    int adjustedDeadZone = circleStickFull > 1.0 ? (deadZone / alternateStickFullValue) * JoyAxis::AXISMAX : deadZone;

    distance = (adjustedDist - adjustedDeadZone)/(double)(maxZone - adjustedDeadZone);
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

/**
 * @brief Get square distance of the X axis past the assigned dead zone.
 * @return Distance percentage in the range of 0.0 - 1.0.
 */
double JoyControlStick::calculateXDistanceFromDeadZone()
{
    double distance = 0.0;

    int axis1Value = axisX->getCurrentRawValue();
    int axis2Value = axisY->getCurrentRawValue();
    //unsigned int square_dist = (unsigned int)(axis1Value*axis1Value) + (unsigned int)(axis2Value*axis2Value);

    double angle2 = atan2(axis1Value, -axis2Value);
    double ang_sin = sin(angle2);
    double ang_cos = cos(angle2);

    int deadX = abs((int)floor(deadZone * ang_sin + 0.5));
    int axis1ValueCircleFull = (int)floor(JoyAxis::AXISMAX * fabs(ang_sin) + 0.5);
    double squareStickFull = qMin(ang_sin ? 1/fabs(ang_sin) : 2, ang_cos ? 1/fabs(ang_cos) : 2);
    double circle = this->circle;
    double circleStickFull = (squareStickFull - 1) * circle + 1;
    double alternateStickFullValue = circleStickFull * abs(axis1ValueCircleFull);

    int adjustedAxis1Value = circleStickFull > 1.0 ? (int)floor((axis1Value / alternateStickFullValue) * abs(axis1ValueCircleFull) + 0.5) : axis1Value;
    int adjustedDeadXZone = circleStickFull > 1.0 ? (int)floor((deadX / alternateStickFullValue) * abs(axis1ValueCircleFull) + 0.5) : deadX;

    distance = (abs(adjustedAxis1Value) - adjustedDeadXZone)/(double)(maxZone - adjustedDeadXZone);
    if (distance > 1.0)
    {
        distance = 1.0;
    }
    else if (distance < 0.0)
    {
        distance = 0.0;
    }

    /*qDebug() << "CIRCLE: " << circle;
    qDebug() << "CIRCLE FULL: " << circleStickFull;
    qDebug() << "OLD X: " << axis1Value;
    qDebug() << "ADJUSTED X: " << adjustedAxis1Value;
    qDebug() << "FULL CIRCLE X: " << axis1ValueCircleFull;
    qDebug() << "LIVING DEAD GIRL: " << adjustedDeadXZone;
    qDebug() << "GOING THE DISTANCE: " << distance;
    qDebug();
    */

    return distance;
}

/**
 * @brief Get square distance of the Y axis past the assigned dead zone.
 * @return Distance percentage in the range of 0.0 - 1.0.
 */
double JoyControlStick::calculateYDistanceFromDeadZone()
{
    double distance = 0.0;

    int axis1Value = axisX->getCurrentRawValue();
    int axis2Value = axisY->getCurrentRawValue();
    //unsigned int square_dist = (unsigned int)(axis1Value*axis1Value) + (unsigned int)(axis2Value*axis2Value);

    double angle2 = atan2(axis1Value, -axis2Value);
    double ang_sin = sin(angle2);
    double ang_cos = cos(angle2);

    int deadY = abs(floor(deadZone * ang_cos) + 0.5);
    int axis2ValueCircleFull = (int)floor(JoyAxis::AXISMAX * fabs(ang_cos) + 0.5);
    double squareStickFull = qMin(ang_sin ? 1/fabs(ang_sin) : 2, ang_cos ? 1/fabs(ang_cos) : 2);
    double circle = this->circle;
    double circleStickFull = (squareStickFull - 1) * circle + 1;
    double alternateStickFullValue = circleStickFull * abs(axis2ValueCircleFull);

    int adjustedAxis2Value = circleStickFull > 1.0 ? (int)floor((axis2Value / alternateStickFullValue) * abs(axis2ValueCircleFull) + 0.5) : axis2Value;
    int adjustedDeadYZone = circleStickFull > 1.0 ? (int)floor((deadY / alternateStickFullValue) * abs(axis2ValueCircleFull) + 0.5) : deadY;

    distance = (abs(adjustedAxis2Value) - adjustedDeadYZone)/(double)(maxZone - adjustedDeadYZone);
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

double JoyControlStick::getAbsoluteRawDistance()
{
    double distance = 0.0;

    int axis1Value = axisX->getCurrentRawValue();
    int axis2Value = axisY->getCurrentRawValue();

    unsigned int square_dist = (unsigned int)(axis1Value*axis1Value) + (unsigned int)(axis2Value*axis2Value);

    distance = sqrt(square_dist);
    /*if (distance > JoyAxis::AXISMAX)
    {
        distance = JoyAxis::AXISMAX;
    }
    else if (distance < 0.0)
    {
        distance = 0.0;
    }*/

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
    circle = DEFAULTCIRCLE;
    stickDelay = DEFAULTSTICKDELAY;
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
        emit propertyUpdated();
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
        emit propertyUpdated();
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
        emit propertyUpdated();
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

    if (modifierButton)
    {
        delete modifierButton;
        modifierButton = 0;
    }
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
                else if (temptext == "four-way")
                {
                    this->setJoyMode(FourWayCardinal);
                }
                else if (temptext == "diagonal")
                {
                    this->setJoyMode(FourWayDiagonal);
                }
            }
            else if (xml->name() == "squareStick" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                if (tempchoice > 0 && tempchoice <= 100)
                {
                    this->setCircleAdjust(tempchoice / 100.0);
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
            else if (xml->name() == JoyControlStickModifierButton::xmlName && xml->isStartElement())
            {
                modifierButton->readConfig(xml);
            }
            else if (xml->name() == "stickDelay" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setStickDelay(tempchoice);
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

        if (deadZone != DEFAULTDEADZONE)
        {
            xml->writeTextElement("deadZone", QString::number(deadZone));
        }

        if (maxZone != DEFAULTMAXZONE)
        {
            xml->writeTextElement("maxZone", QString::number(maxZone));
        }

        if (currentMode == StandardMode || currentMode == EightWayMode)
        {
            if (diagonalRange != DEFAULTDIAGONALRANGE)
            {
                xml->writeTextElement("diagonalRange", QString::number(diagonalRange));
            }
        }

        if (currentMode == EightWayMode)
        {
            xml->writeTextElement("mode", "eight-way");
        }
        else if (currentMode == FourWayCardinal)
        {
            xml->writeTextElement("mode", "four-way");
        }
        else if (currentMode == FourWayDiagonal)
        {
            xml->writeTextElement("mode", "diagonal");
        }

        if (circle > DEFAULTCIRCLE)
        {
            xml->writeTextElement("squareStick", QString::number(circle * 100));
        }

        if (stickDelay > DEFAULTSTICKDELAY)
        {
            xml->writeTextElement("stickDelay", QString::number(stickDelay));
        }

        QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(buttons);
        while (iter.hasNext())
        {
            JoyControlStickButton *button = iter.next().value();
            button->writeConfig(xml);
        }

        if (!modifierButton->isDefault())
        {
            modifierButton->writeConfig(xml);
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

    if (modifierButton)
    {
        modifierButton->reset();
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

/**
 * @brief Used to calculate the distance value that should be used for mouse
 *     movement
 *
 * @param button
 * @return Distance factor that should be used for mouse movement
 */
double JoyControlStick::calculateMouseDirectionalDistance(JoyControlStickButton *button)
{
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

/**
 * @brief Used to calculate the distance value that should be used for keyboard
 *     events and distance slots
 * @return Distance factor that should be used for keyboard events and
 *     distance slots
 */
double JoyControlStick::calculateDirectionalDistance()
{
    double finalDistance = 0.0;

    if (currentDirection == StickUp)
    {
        finalDistance = calculateYDistanceFromDeadZone();
    }
    else if (currentDirection == StickRightUp)
    {
        finalDistance = getDistanceFromDeadZone();
    }
    else if (currentDirection == StickRight)
    {
        finalDistance = calculateXDistanceFromDeadZone();
    }
    else if (currentDirection  == StickRightDown)
    {
        finalDistance = getDistanceFromDeadZone();
    }
    else if (currentDirection == StickDown)
    {
        finalDistance = calculateYDistanceFromDeadZone();
    }
    else if (currentDirection == StickLeftDown)
    {
        finalDistance = getDistanceFromDeadZone();
    }
    else if (currentDirection == StickLeft)
    {
        finalDistance = calculateXDistanceFromDeadZone();
    }
    else if (currentDirection == StickLeftUp)
    {
        finalDistance = getDistanceFromDeadZone();
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

int JoyControlStick::getCircleXCoordinate()
{
    int value = axisX->getCurrentRawValue();
    if (this->circle > 0.0)
    {
        value = calculateCircleXValue(value);
    }

    return value;
}

int JoyControlStick::getCircleYCoordinate()
{
    int value = axisY->getCurrentRawValue();
    if (this->circle > 0.0)
    {
        value = calculateCircleYValue(value);
    }

    return value;
}

int JoyControlStick::calculateCircleXValue(int rawXValue)
{
    int value = rawXValue;
    if (this->circle > 0.0)
    {
        int axis1Value = axisX->getCurrentRawValue();
        int axis2Value = axisY->getCurrentRawValue();

        double angle2 = atan2(axis1Value, -axis2Value);
        double ang_sin = sin(angle2);
        double ang_cos = cos(angle2);

        int axisXValueCircleFull = (int)floor(JoyAxis::AXISMAX * fabs(ang_sin) + 0.5);
        double squareStickFull = qMin(ang_sin ? 1/fabs(ang_sin) : 2, ang_cos ? 1/fabs(ang_cos) : 2);
        double circle = this->circle;
        double circleStickFull = (squareStickFull - 1) * circle + 1;
        double alternateStickFullValue = circleStickFull * abs(axisXValueCircleFull);

        value = circleStickFull > 1.0 ? (int)floor((rawXValue / alternateStickFullValue) * abs(axisXValueCircleFull) + 0.5) : value;
    }

    return value;
}

int JoyControlStick::calculateCircleYValue(int rawYValue)
{
    int value = rawYValue;
    if (this->circle > 0.0)
    {
        int axis1Value = axisX->getCurrentRawValue();
        int axis2Value = axisY->getCurrentRawValue();

        double angle2 = atan2(axis1Value, -axis2Value);
        double ang_sin = sin(angle2);
        double ang_cos = cos(angle2);

        int axisYValueCircleFull = (int)floor(JoyAxis::AXISMAX * fabs(ang_cos) + 0.5);
        double squareStickFull = qMin(ang_sin ? 1/fabs(ang_sin) : 2, ang_cos ? 1/fabs(ang_cos) : 2);
        double circle = this->circle;
        double circleStickFull = (squareStickFull - 1) * circle + 1;
        double alternateStickFullValue = circleStickFull * abs(axisYValueCircleFull);

        value = circleStickFull > 1.0 ? (int)floor((rawYValue / alternateStickFullValue) * abs(axisYValueCircleFull) + 0.5) : value;
    }

    return value;
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
    int rightInitial = upRightInitial + diagonalAngle;
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

QList<int> JoyControlStick::getFourWayCardinalZoneAngles()
{
    QList<int> anglesList;

    int zoneRange = 90;

    int rightInitial = 45;
    int downInitial = rightInitial + zoneRange;
    int leftInitial = downInitial + zoneRange;
    int upInitial = leftInitial + zoneRange;

    anglesList.append(rightInitial);
    anglesList.append(downInitial);
    anglesList.append(leftInitial);
    anglesList.append(upInitial);
    return anglesList;
}

QList<int> JoyControlStick::getFourWayDiagonalZoneAngles()
{
    QList<int> anglesList;

    int zoneRange = 90;

    int upRightInitial = 0;
    int downRightInitial = zoneRange;
    int downLeftInitial = downRightInitial + zoneRange;
    int upLeftInitial = downLeftInitial + zoneRange;

    anglesList.append(upRightInitial);
    anglesList.append(downRightInitial);
    anglesList.append(downLeftInitial);
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
    emit joyModeChanged();
    emit propertyUpdated();
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
    value = value && (deadZone == DEFAULTDEADZONE);
    value = value && (maxZone == DEFAULTMAXZONE);
    value = value && (diagonalRange == DEFAULTDIAGONALRANGE);
    value = value && (currentMode == DEFAULTMODE);
    value = value && (circle == DEFAULTCIRCLE);
    value = value && (stickDelay == DEFAULTSTICKDELAY);

    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        value = value && (button->isDefault());
    }

    if (modifierButton)
    {
        value = value && modifierButton->isDefault();
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

    QHash<JoyStickDirections, JoyControlStickButton*> temphash = getApplicableButtons();
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

    QHash<JoyStickDirections, JoyControlStickButton*> temphash = getApplicableButtons();
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

    QHash<JoyStickDirections, JoyControlStickButton*> temphash = getApplicableButtons();
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

    QHash<JoyStickDirections, JoyControlStickButton*> temphash = getApplicableButtons();
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

    QHash<JoyStickDirections, JoyControlStickButton*> temphash = getApplicableButtons();
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

    QHash<JoyStickDirections, JoyControlStickButton*> temphash = getApplicableButtons();
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

    QHash<JoyStickDirections, JoyControlStickButton*> temphash = getApplicableButtons();
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

    if (currentMode == StandardMode || currentMode == EightWayMode ||
        currentMode == FourWayCardinal)
    {
        temphash.insert(StickUp, buttons.value(StickUp));
        temphash.insert(StickDown, buttons.value(StickDown));
        temphash.insert(StickLeft, buttons.value(StickLeft));
        temphash.insert(StickRight, buttons.value(StickRight));
    }

    if (currentMode == EightWayMode || currentMode == FourWayDiagonal)
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
        emit propertyUpdated();
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

void JoyControlStick::determineStandardModeEvent(JoyControlStickButton *&eventbutton1, JoyControlStickButton *&eventbutton2)
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
}

void JoyControlStick::determineEightWayModeEvent(JoyControlStickButton *&eventbutton1, JoyControlStickButton *&eventbutton2, JoyControlStickButton *&eventbutton3)
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
        eventbutton3 = buttons.value(StickRightUp);
    }
    else if (bearing >= rightInitial && bearing < downRightInitial)
    {
        currentDirection = StickRight;
        eventbutton1 = buttons.value(StickRight);
    }
    else if (bearing >= downRightInitial && bearing < downInitial)
    {
        currentDirection = StickRightDown;
        eventbutton3 = buttons.value(StickRightDown);
    }
    else if (bearing >= downInitial && bearing < downLeftInitial)
    {
        currentDirection = StickDown;
        eventbutton2 = buttons.value(StickDown);
    }
    else if (bearing >= downLeftInitial && bearing < leftInitial)
    {
        currentDirection = StickLeftDown;
        eventbutton3 = buttons.value(StickLeftDown);
    }
    else if (bearing >= leftInitial && bearing < upLeftInitial)
    {
        currentDirection = StickLeft;
        eventbutton1 = buttons.value(StickLeft);
    }
    else if (bearing >= upLeftInitial && bearing < initialLeft)
    {
        currentDirection = StickLeftUp;
        eventbutton3 = buttons.value(StickLeftUp);
    }
}

void JoyControlStick::determineFourWayCardinalEvent(JoyControlStickButton *&eventbutton1, JoyControlStickButton *&eventbutton2)
{
    double bearing = calculateBearing();

    QList<int> anglesList = getFourWayCardinalZoneAngles();
    int rightInitial = anglesList.value(0);
    int downInitial = anglesList.value(1);
    int leftInitial = anglesList.value(2);
    int upInitial = anglesList.value(3);

    bearing = floor(bearing + 0.5);
    if (bearing < rightInitial || bearing >= upInitial)
    {
        currentDirection = StickUp;
        eventbutton2 = buttons.value(StickUp);
    }
    else if (bearing >= rightInitial && bearing < downInitial)
    {
        currentDirection = StickRight;
        eventbutton1 = buttons.value(StickRight);
    }
    else if (bearing >= downInitial && bearing < leftInitial)
    {
        currentDirection = StickDown;
        eventbutton2 = buttons.value(StickDown);
    }
    else if (bearing >= leftInitial && bearing < upInitial)
    {
        currentDirection = StickLeft;
        eventbutton1 = buttons.value(StickLeft);
    }
}

void JoyControlStick::determineFourWayDiagonalEvent(JoyControlStickButton *&eventbutton3)
{
    double bearing = calculateBearing();

    QList<int> anglesList = getFourWayDiagonalZoneAngles();
    int upRightInitial = anglesList.value(0);
    int downRightInitial = anglesList.value(1);
    int downLeftInitial = anglesList.value(2);
    int upLeftInitial = anglesList.value(3);

    bearing = floor(bearing + 0.5);
    if (bearing >= upRightInitial && bearing < downRightInitial)
    {
        currentDirection = StickRightUp;
        eventbutton3 = buttons.value(StickRightUp);
    }
    else if (bearing >= downRightInitial && bearing < downLeftInitial)
    {
        currentDirection = StickRightDown;
        eventbutton3 = buttons.value(StickRightDown);
    }
    else if (bearing >= downLeftInitial && bearing < upLeftInitial)
    {
        currentDirection = StickLeftDown;
        eventbutton3 = buttons.value(StickLeftDown);
    }
    else if (bearing >= upLeftInitial)
    {
        currentDirection = StickLeftUp;
        eventbutton3 = buttons.value(StickLeftUp);
    }
}

/**
 * @brief Find the current stick direction based on a Standard mode stick.
 * @return Current direction the stick is positioned.
 */
JoyControlStick::JoyStickDirections JoyControlStick::determineStandardModeDirection()
{
    JoyStickDirections result = StickCentered;

    double bearing = calculateBearing();
    bearing = floor(bearing + 0.5);

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

    if (bearing <= initialRight || bearing >= initialLeft)
    {
        result = StickUp;
    }
    else if (bearing >= upRightInitial && bearing < rightInitial)
    {
        result = StickRightUp;
    }
    else if (bearing >= rightInitial && bearing < downRightInitial)
    {
        result = StickRight;
    }
    else if (bearing >= downRightInitial && bearing < downInitial)
    {
        result = StickRightDown;
    }
    else if (bearing >= downInitial && bearing < downLeftInitial)
    {
        result = StickDown;
    }
    else if (bearing >= downLeftInitial && bearing < leftInitial)
    {
        result = StickLeftDown;
    }
    else if (bearing >= leftInitial && bearing < upLeftInitial)
    {
        result = StickLeft;
    }
    else if (bearing >= upLeftInitial && bearing < initialLeft)
    {
        result = StickLeftUp;
    }

    return result;
}

/**
 * @brief Find the current stick direction based on a Eight Way mode stick.
 * @return Current direction the stick is positioned.
 */
JoyControlStick::JoyStickDirections JoyControlStick::determineEightWayModeDirection()
{
    return determineStandardModeDirection();
}

/**
 * @brief Find the current stick direction based on a Four Way Cardinal mode
 *     stick.
 * @return Current direction the stick is positioned.
 */
JoyControlStick::JoyStickDirections JoyControlStick::determineFourWayCardinalDirection()
{
    JoyStickDirections result = StickCentered;

    double bearing = calculateBearing();
    bearing = floor(bearing + 0.5);

    QList<int> anglesList = getFourWayCardinalZoneAngles();
    int rightInitial = anglesList.value(0);
    int downInitial = anglesList.value(1);
    int leftInitial = anglesList.value(2);
    int upInitial = anglesList.value(3);

    if (bearing < rightInitial || bearing >= upInitial)
    {
        result = StickUp;
    }
    else if (bearing >= rightInitial && bearing < downInitial)
    {
        result = StickRight;
    }
    else if (bearing >= downInitial && bearing < leftInitial)
    {
        result = StickDown;
    }
    else if (bearing >= leftInitial && bearing < upInitial)
    {
        result = StickLeft;
    }

    return result;
}

/**
 * @brief Find the current stick direction based on a Four Way Diagonal mode
 *     stick.
 * @return Current direction the stick is positioned.
 */
JoyControlStick::JoyStickDirections JoyControlStick::determineFourWayDiagonalDirection()
{
    JoyStickDirections result = StickCentered;

    double bearing = calculateBearing();
    bearing = floor(bearing + 0.5);

    QList<int> anglesList = getFourWayDiagonalZoneAngles();
    int upRightInitial = anglesList.value(0);
    int downRightInitial = anglesList.value(1);
    int downLeftInitial = anglesList.value(2);
    int upLeftInitial = anglesList.value(3);

    if (bearing >= upRightInitial && bearing < downRightInitial)
    {
        result = StickRightUp;
    }
    else if (bearing >= downRightInitial && bearing < downLeftInitial)
    {
        result = StickRightDown;
    }
    else if (bearing >= downLeftInitial && bearing < upLeftInitial)
    {
        result = StickLeftDown;
    }
    else if (bearing >= upLeftInitial)
    {
        result = StickLeftUp;
    }

    return result;
}

/**
 * @brief Calculate the current direction of the stick based on the values
 *     of the X and Y axes and the current mode of the stick.
 * @return Current direction the stick is positioned.
 */
JoyControlStick::JoyStickDirections JoyControlStick::calculateStickDirection()
{
    JoyStickDirections result = StickCentered;

    if (currentMode == StandardMode)
    {
        result = determineStandardModeDirection();
    }
    else if (currentMode == EightWayMode)
    {
        result = determineEightWayModeDirection();
    }
    else if (currentMode == FourWayCardinal)
    {
        result = determineFourWayCardinalDirection();
    }
    else if (currentMode == FourWayDiagonal)
    {
        result = determineFourWayDiagonalDirection();
    }

    return result;
}

void JoyControlStick::establishPropertyUpdatedConnection()
{
    connect(this, SIGNAL(propertyUpdated()), getParentSet()->getInputDevice(), SLOT(profileEdited()));
}

void JoyControlStick::disconnectPropertyUpdatedConnection()
{
    disconnect(this, SIGNAL(propertyUpdated()), getParentSet()->getInputDevice(), SLOT(profileEdited()));
}

/**
 * @brief Check all stick buttons and see if any have slots assigned.
 * @return Status of whether any stick button has a slot assigned.
 */
bool JoyControlStick::hasSlotsAssigned()
{
    bool hasSlots = false;

    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyButton *button = iter.next().value();
        if (button)
        {
            if (button->getAssignedSlots()->count() > 0)
            {
                hasSlots = true;
                iter.toBack();
            }
        }
    }

    return hasSlots;
}

void JoyControlStick::setButtonsSpringRelativeStatus(bool value)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setSpringRelativeStatus(value);
    }
}

bool JoyControlStick::isRelativeSpring()
{
    bool relative = false;

    QHash<JoyStickDirections, JoyControlStickButton*> temphash = getApplicableButtons();
    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(temphash);
    while (iter.hasNext())
    {
        if (!iter.hasPrevious())
        {
            JoyControlStickButton *button = iter.next().value();
            relative = button->isRelativeSpring();
        }
        else
        {
            JoyControlStickButton *button = iter.next().value();
            bool temp = button->isRelativeSpring();
            if (temp != relative)
            {
                relative = false;
                iter.toBack();
            }
        }
    }

    return relative;
}

/**
 * @brief Copy slots from all stick buttons and properties from a stick
 *     onto another.
 * @param JoyControlStick object to be modified.
 */
void JoyControlStick::copyAssignments(JoyControlStick *destStick)
{
    destStick->reset();
    destStick->deadZone = deadZone;
    destStick->maxZone = maxZone;
    destStick->diagonalRange = diagonalRange;
    destStick->currentDirection = currentDirection;
    destStick->currentMode = currentMode;
    destStick->stickName = stickName;
    destStick->circle = circle;
    destStick->stickDelay = stickDelay;

    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(destStick->buttons);
    while (iter.hasNext())
    {
        JoyControlStickButton *destButton = iter.next().value();
        if (destButton)
        {
            JoyControlStickButton *sourceButton = buttons.value(destButton->getDirection());
            if (sourceButton)
            {
                sourceButton->copyAssignments(destButton);
            }
        }
    }

    JoyControlStickModifierButton *destModifierButton = destStick->getModifierButton();
    if (modifierButton && destModifierButton)
    {
        modifierButton->copyAssignments(destModifierButton);
    }
}

/**
 * @brief Set the percentage of the outer square that should be ignored
 *     when performing the final axis calculations.
 * @param Percentage represented by the range of 0.0 - 1.0.
 */
void JoyControlStick::setCircleAdjust(double circle)
{
    if (circle >= 0.0 && circle <= 1.0)
    {
        this->circle = circle;
        emit circleAdjustChange(circle);
        emit propertyUpdated();
    }
}

/**
 * @brief Get the current percentage of the outer square that should be ignored
 *     when performing the final axis calculations.
 * @return Percentage represented by the range of 0.0 - 1.0.
 */
double JoyControlStick::getCircleAdjust()
{
    return circle;
}

/**
 * @brief Slot called when directionDelayTimer has timed out. The method will
 *     call createDeskEvent.
 */
void JoyControlStick::stickDirectionChangeEvent()
{
    createDeskEvent();
}

void JoyControlStick::setStickDelay(int value)
{
    if ((value >= 10 && value <= 1000) || (value == 0))
    {
        this->stickDelay = value;
        emit stickDelayChanged(value);
        emit propertyUpdated();
    }
}

unsigned int JoyControlStick::getStickDelay()
{
    return stickDelay;
}

void JoyControlStick::setButtonsEasingDuration(double value)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setEasingDuration(value);
    }
}

double JoyControlStick::getButtonsEasingDuration()
{
    double result = JoyButton::DEFAULTEASINGDURATION;

    QHash<JoyStickDirections, JoyControlStickButton*> temphash = getApplicableButtons();
    QHashIterator<JoyStickDirections, JoyControlStickButton*> iter(temphash);
    while (iter.hasNext())
    {
        if (!iter.hasPrevious())
        {
            JoyControlStickButton *button = iter.next().value();
            result = button->getEasingDuration();
        }
        else
        {
            JoyControlStickButton *button = iter.next().value();
            double temp = button->getEasingDuration();
            if (temp != result)
            {
                result = JoyButton::DEFAULTEASINGDURATION;
                iter.toBack();
            }
        }
    }

    return result;
}

JoyControlStickModifierButton *JoyControlStick::getModifierButton()
{
    return modifierButton;
}
