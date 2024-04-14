/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "joycontrolstick.h"

#include "globalvariables.h"
#include "inputdevice.h"
#include "joyaxis.h"
#include "joybuttontypes/joycontrolstickbutton.h"
#include "joybuttontypes/joycontrolstickmodifierbutton.h"
#include "xml/joybuttonxml.h"

#include <QDebug>
#include <QHashIterator>
#include <QLabel>
#include <QPointer>
#include <QStringList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
//#include <QtTest/QTest>

#include <math.h>

const JoyControlStick::JoyMode JoyControlStick::DEFAULTMODE = JoyControlStick::StandardMode;

JoyControlStick::JoyControlStick(JoyAxis *axis1, JoyAxis *axis2, int index, int originset, QObject *parent)
    : QObject(parent)
{
    this->axisX = axis1;
    this->axisX->setControlStick(this);
    this->axisY = axis2;
    this->axisY->setControlStick(this);

    this->index = index;
    this->originset = originset;
    this->modifierButton = nullptr;

    reset();
    populateStickBtns();
    directionDelayTimer.setSingleShot(true);

    connect(&directionDelayTimer, &QTimer::timeout, this, &JoyControlStick::stickDirectionChangeEvent);
}

JoyControlStick::~JoyControlStick()
{
    if (!axisX.isNull())
        axisX->removeControlStick(false);
    if (!axisY.isNull())
        axisY->removeControlStick(false);
}

/**
 * @brief Take the input value for the two axes that make up a stick and
 *   activate the proper event based on the current values.
 * @param Should set changing routines be ignored.
 */
void JoyControlStick::joyEvent(bool ignoresets)
{
    safezone = !inDeadZone();

    if (safezone && !isActive)
    {
        isActive = true;
        emit active(axisX->getCurrentRawValue(), axisY->getCurrentRawValue());

        if (ignoresets || (stickDelay == 0))
        {
            if (directionDelayTimer.isActive())
                directionDelayTimer.stop();

            createDeskEvent(ignoresets);
        } else
        {
            if (!directionDelayTimer.isActive())
                directionDelayTimer.start(stickDelay);
        }
    } else if (!safezone && isActive)
    {
        isActive = false;
        emit released(axisX->getCurrentRawValue(), axisY->getCurrentRawValue());

        if (ignoresets || (stickDelay == 0))
        {
            if (directionDelayTimer.isActive())
                directionDelayTimer.stop();

            createDeskEvent(ignoresets);
        } else
        {
            if (!directionDelayTimer.isActive())
                directionDelayTimer.start(stickDelay);
        }
    } else if (isActive)
    {
        if (ignoresets || (stickDelay == 0))
        {
            if (directionDelayTimer.isActive())
                directionDelayTimer.stop();

            createDeskEvent(ignoresets);
        } else
        {
            JoyStickDirections pendingDirection = calculateStickDirection();

            if (currentDirection != pendingDirection)
            {
                if (!directionDelayTimer.isActive())
                    directionDelayTimer.start(stickDelay);
            } else
            {
                if (directionDelayTimer.isActive())
                    directionDelayTimer.stop();

                createDeskEvent(ignoresets);
            }
        }
    }

    emit moved(axisX->getCurrentRawValue(), axisY->getCurrentRawValue());

    pendingStickEvent = false;
}

/**
 * @brief Check the current stick position to see if it lies in
 *   the assigned dead zone.
 * @return If stick position is in the assigned dead zone
 */
bool JoyControlStick::inDeadZone()
{
    int axis1Value = axisX->getCurrentRawValue();
    int axis2Value = axisY->getCurrentRawValue();

    int squareDist = (axis1Value * axis1Value) + (axis2Value * axis2Value);

    return squareDist <= (deadZone * deadZone);
}

/**
 * @brief Populate the virtual buttons assigned to an analog stick.
 */
void JoyControlStick::populateButtons() { populateStickBtns(); }

void JoyControlStick::populateStickBtns()
{
    JoyControlStickButton *button = new JoyControlStickButton(this, StickUp, originset, getParentSet(), this);
    buttons.insert(StickUp, button);

    button = new JoyControlStickButton(this, StickDown, originset, getParentSet(), this);
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
}

/**
 * @brief Get the assigned dead zone value.
 * @return Assigned dead zone value
 */
int JoyControlStick::getDeadZone() { return deadZone; }

/**
 * @brief Get the assigned diagonal range value.
 * @return Assigned diagonal range.
 */
int JoyControlStick::getDiagonalRange() { return diagonalRange; }

/**
 * @brief Find the position of the two stick axes, deactivate no longer used
 *   stick direction button and then activate direction buttons for new
 *   direction.
 * @param Should set changing operations be ignored. Necessary in the middle
 *   of a set change.
 */
void JoyControlStick::createDeskEvent(bool ignoresets)
{
    JoyControlStickButton *eventbutton1 = nullptr;
    JoyControlStickButton *eventbutton2 = nullptr;
    JoyControlStickButton *eventbutton3 = nullptr;

    if (safezone)
    {
        switch (currentMode)
        {
        case StandardMode:
            determineStandardModeEvent(eventbutton1, eventbutton2);
            break;

        case EightWayMode:
            determineEightWayModeEvent(eventbutton1, eventbutton2, eventbutton3);
            break;

        case FourWayCardinal:
            determineFourWayCardinalEvent(eventbutton1, eventbutton2);
            break;

        case FourWayDiagonal:
            determineFourWayDiagonalEvent(eventbutton3);
            break;
        }
    } else
    {
        currentDirection = StickCentered;
    }

    /*
     * Release any currently active stick buttons.
     */
    if ((eventbutton1 == nullptr) && (activeButton1 != nullptr))
    {
        // Currently in deadzone. Disable currently active button.
        performButtonRelease(activeButton1, ignoresets);
    } else if ((eventbutton1 != nullptr) && (activeButton1 != nullptr) && (eventbutton1 != activeButton1))
    {
        // Deadzone skipped. Button for new event is not the currently
        // active button. Disable the active button.
        performButtonRelease(activeButton1, ignoresets);
    }

    if ((eventbutton2 == nullptr) && (activeButton2 != nullptr))
    {
        // Currently in deadzone. Disable currently active button.
        performButtonRelease(activeButton2, ignoresets);
    } else if ((eventbutton2 != nullptr) && (activeButton2 != nullptr) && (eventbutton2 != activeButton2))
    {
        // Deadzone skipped. Button for new event is not the currently
        // active button. Disable the active button.
        performButtonRelease(activeButton2, ignoresets);
    }

    if ((eventbutton3 == nullptr) && (activeButton3 != nullptr))
    {
        // Currently in deadzone. Disable currently active button.
        performButtonRelease(activeButton3, ignoresets);
    } else if ((eventbutton3 != nullptr) && (activeButton3 != nullptr) && (eventbutton3 != activeButton3))
    {
        // Deadzone skipped. Button for new event is not the currently
        // active button. Disable the active button.
        performButtonRelease(activeButton3, ignoresets);
    }

    // Activate modifier button before activating directional buttons.
    // Value from the new stick event will be used to determine
    // distance events.
    // Release modifier button after releasing directional buttons.
    double distance = getAbsoluteRawDistance();
    if (m_modifier_zone_inverted)
    {
        if (safezone && distance < m_modifier_zone)
            modifierButton->joyEvent(true, ignoresets);
        else
            modifierButton->joyEvent(false, ignoresets);
    } else
    {
        if (safezone && distance > m_modifier_zone)
            modifierButton->joyEvent(true, ignoresets);
        else
            modifierButton->joyEvent(false, ignoresets);
    }

    /*
     * Enable stick buttons.
     */
    if ((eventbutton1 != nullptr) && (activeButton1 == nullptr))
    {
        // There is no active button. Call joyEvent and set current
        // button as active button
        performButtonPress(eventbutton1, activeButton1, ignoresets);
    } else if ((eventbutton1 != nullptr) && (activeButton1 != nullptr) && (eventbutton1 == activeButton1))
    {
        // Button is currently active. Just pass current value
        performButtonPress(eventbutton1, activeButton1, ignoresets);
    }

    if ((eventbutton2 != nullptr) && (activeButton2 == nullptr))
    {
        // There is no active button. Call joyEvent and set current
        // button as active button
        performButtonPress(eventbutton2, activeButton2, ignoresets);
    } else if ((eventbutton2 != nullptr) && (activeButton2 != nullptr) && (eventbutton2 == activeButton2))
    {
        // Button is currently active. Just pass current value
        performButtonPress(eventbutton2, activeButton2, ignoresets);
    }

    if ((eventbutton3 != nullptr) && (activeButton3 == nullptr))
    {
        // There is no active button. Call joyEvent and set current
        // button as active button
        performButtonPress(eventbutton3, activeButton3, ignoresets);
    } else if ((eventbutton3 != nullptr) && (activeButton3 != nullptr) && (eventbutton3 == activeButton3))
    {
        // Button is currently active. Just pass current value
        performButtonPress(eventbutton3, activeButton3, ignoresets);
    }
}

/**
 * @brief Calculate the bearing (in degrees) corresponding to the current
 *   position of the X and Y axes of a stick.
 * @return Bearing (in degrees)
 */
double JoyControlStick::calculateBearing()
{
    return calculateBearing(axisX->getCurrentRawValue(), axisY->getCurrentRawValue());
}

/**
 * @brief Calculate the bearing (in degrees) corresponding to the
 *   passed X and Y axes values associated with the stick.
 * @param X axis value
 * @param Y axis value
 * @return Bearing (in degrees)
 */
double JoyControlStick::calculateBearing(int axisXValue, int axisYValue)
{
    double finalAngle = 0.0;
    int axis1Value = axisXValue;
    int axis2Value = axisYValue;

    if ((axis1Value == 0) && (axis2Value == 0))
    {
        finalAngle = 0.0;
    } else
    {
        double temp1 = axis1Value;
        double temp2 = axis2Value;
        double angle = (atan2(temp1, -temp2) * 180) / GlobalVariables::JoyControlStick::PI;

        if ((axis1Value >= 0) && (axis2Value <= 0))
        {
            // NE Quadrant
            finalAngle = angle;
        } else if ((axis1Value >= 0) && (axis2Value >= 0))
        {
            // SE Quadrant (angle will be positive)
            finalAngle = angle;
        } else if ((axis1Value <= 0) && (axis2Value >= 0))
        {
            // SW Quadrant (angle will be negative)
            finalAngle = 360.0 + angle;
        } else if ((axis1Value <= 0) && (axis2Value <= 0))
        {
            // NW Quadrant (angle will be negative)
            finalAngle = 360.0 + angle;
        }
    }

    return finalAngle;
}

/**
 * @brief Get current radial distance of the stick position past the assigned
 *   dead zone.
 * @return Distance percentage in the range of 0.0 - 1.0.
 */
double JoyControlStick::getDistanceFromDeadZone()
{
    return getDistanceFromDeadZone(axisX->getCurrentRawValue(), axisY->getCurrentRawValue());
}

/**
 * @brief Get radial distance of the stick position past the assigned dead zone
 *   based on the passed X and Y axes values associated with the stick.
 * @param X axis value
 * @param Y axis value
 * @return Distance percentage in the range of 0.0 - 1.0.
 */
double JoyControlStick::getDistanceFromDeadZone(int axisXValue, int axisYValue)
{
    double distance = 0.0;

    int axis1Value = axisXValue;
    int axis2Value = axisYValue;

    double angle2 = atan2(axis1Value, -axis2Value);
    double ang_sin = sin(angle2);
    double ang_cos = cos(angle2);

    int squared_dist = (axis1Value * axis1Value) + (axis2Value * axis2Value);
    int dist = sqrt(squared_dist);

    double squareStickFullPhi = qMin((ang_sin != 0.0) ? 1 / fabs(ang_sin) : 2, ang_cos != 0.0 ? 1 / fabs(ang_cos) : 2);
    double circle = this->circle;
    double circleStickFull = (squareStickFullPhi - 1) * circle + 1;

    double adjustedDist = (circleStickFull > 1.0) ? (dist / circleStickFull) : dist;
    double adjustedDeadZone = (circleStickFull > 1.0) ? (deadZone / circleStickFull) : deadZone;

    distance = (adjustedDist - adjustedDeadZone) / (static_cast<double>(maxZone) - adjustedDeadZone);
    distance = qBound(0.0, distance, 1.0);
    return distance;
}

/**
 * @brief Get distance of the Y axis past the assigned dead zone.
 * @param Should interpolation be performed along the diagonal regions.
 * @return Distance percentage in the range of 0.0 - 1.0.
 */
double JoyControlStick::calculateYDistanceFromDeadZone(bool interpolate)
{
    return calculateYDistanceFromDeadZone(axisX->getCurrentRawValue(), axisY->getCurrentRawValue(), interpolate);
}

/**
 * @brief Get distance of the Y axis past the assigned dead zone based
 *   on the passed X and Y axis values for the analog stick.
 * @param X axis value
 * @param Y axis value
 * @param Should interpolation be performed along the diagonal regions.
 * @return Distance percentage in the range of 0.0 - 1.0.
 */
double JoyControlStick::calculateYDistanceFromDeadZone(int axisXValue, int axisYValue, bool interpolate)
{
    double distance = 0.0;

    int axis1Value = axisXValue;
    int axis2Value = axisYValue;

    double angle2 = atan2(axis1Value, -axis2Value);
    double ang_sin = sin(angle2);
    double ang_cos = cos(angle2);

    int deadY = abs(floor(deadZone * ang_cos + 0.5));

    double squareStickFullPhi = qMin(ang_sin != 0.0 ? 1 / fabs(ang_sin) : 2, ang_cos != 0.0 ? 1 / fabs(ang_cos) : 2);
    double circle = this->circle;
    double circleStickFull = (squareStickFullPhi - 1) * circle + 1;

    double adjustedAxis2Value = (circleStickFull > 1.0) ? (axis2Value / circleStickFull) : axis2Value;
    double adjustedDeadYZone = (circleStickFull > 1.0) ? (deadY / circleStickFull) : deadY;

    // Interpolation would return the correct value if diagonalRange is 90 but
    // the routine gets skipped to save time.
    if (interpolate && (diagonalRange < 90))
    {
        JoyStickDirections direction = calculateStickDirection(axis1Value, axis2Value);
        if ((direction == StickRightUp) || (direction == StickUp))
        {
            QList<double> tempangles = getDiagonalZoneAngles();

            double minangle = tempangles.at(1);
            double square_dist = getAbsoluteRawDistance(axis1Value, axis2Value);
            double mindeadY = fabs(square_dist * sin(minangle * GlobalVariables::JoyControlStick::PI / 180.0));
            double currentDeadY = qMax(adjustedDeadYZone, mindeadY);
            double maxRange = static_cast<double>(maxZone) - currentDeadY;
            double tempdist4 = 0.0;

            if (maxRange != 0.0)
                tempdist4 = (fabs(adjustedAxis2Value) - currentDeadY) / maxRange;

            distance = tempdist4;
        } else if ((direction == StickRightDown) || (direction == StickRight))
        {
            QList<double> tempfuck = getDiagonalZoneAngles();

            double minangle = tempfuck.at(4);
            double square_dist = getAbsoluteRawDistance(axis1Value, axis2Value);
            double mindeadY = fabs(square_dist * sin((minangle - 90.0) * GlobalVariables::JoyControlStick::PI / 180.0));
            double currentDeadY = qMax(adjustedDeadYZone, mindeadY);
            double maxRange = static_cast<double>(maxZone) - currentDeadY;
            double tempdist4 = 0.0;

            if (maxRange != 0.0)
                tempdist4 = (fabs(adjustedAxis2Value) - currentDeadY) / maxRange;

            distance = tempdist4;
        } else if ((direction == StickLeftDown) || (direction == StickDown))
        {
            QList<double> tempangles = getDiagonalZoneAngles();

            double minangle = tempangles.at(6);
            double square_dist = getAbsoluteRawDistance(axis1Value, axis2Value);
            double mindeadY = fabs(square_dist * sin((minangle - 180.0) * GlobalVariables::JoyControlStick::PI / 180.0));
            double currentDeadY = qMax(adjustedDeadYZone, mindeadY);
            double maxRange = static_cast<double>(maxZone) - currentDeadY;
            double tempdist4 = 0.0;

            if (maxRange != 0.0)
                tempdist4 = (fabs(adjustedAxis2Value) - currentDeadY) / maxRange;

            distance = tempdist4;
        } else if ((direction == StickLeftUp) || (direction == StickLeft))
        {
            QList<double> tempangles = getDiagonalZoneAngles();

            double minangle = tempangles.at(8);
            double square_dist = getAbsoluteRawDistance(axis1Value, axis2Value);
            double mindeadY = fabs(square_dist * sin((minangle - 270.0) * GlobalVariables::JoyControlStick::PI / 180.0));
            double currentDeadY = qMax(adjustedDeadYZone, mindeadY);
            double maxRange = static_cast<double>(maxZone) - currentDeadY;
            double tempdist4 = 0.0;

            if (maxRange != 0.0)
                tempdist4 = (fabs(adjustedAxis2Value) - currentDeadY) / maxRange;

            distance = tempdist4;
        } else
        {
            // Backup plan. Should not arrive here.
            double maxRange = static_cast<double>(maxZone) - adjustedDeadYZone;
            double tempdist4 = 0.0;

            if (maxRange != 0.0)
                tempdist4 = (fabs(adjustedAxis2Value) - adjustedDeadYZone) / maxRange;

            distance = tempdist4;
        }
    } else
    {
        // No interpolation desired or diagonal range is 90 degrees.
        double maxRange = static_cast<double>(maxZone) - adjustedDeadYZone;
        double tempdist4 = 0.0;

        if (maxRange != 0.0)
            tempdist4 = (fabs(adjustedAxis2Value) - adjustedDeadYZone) / maxRange;

        distance = tempdist4;
    }

    distance = qBound(0.0, distance, 1.0);
    return distance;
}

/**
 * @brief Get distance of the X axis past the assigned dead zone.
 * @param Should interpolation be performed along the diagonal regions.
 * @return Distance percentage in the range of 0.0 - 1.0.
 */
double JoyControlStick::calculateXDistanceFromDeadZone(bool interpolate)
{
    return calculateXDistanceFromDeadZone(axisX->getCurrentRawValue(), axisY->getCurrentRawValue(), interpolate);
}

/**
 * @brief Get distance of the X axis past the assigned dead zone based
 *   on the passed X and Y axis values for the analog stick.
 * @param X axis value
 * @param Y axis value
 * @param Should interpolation be performed along the diagonal regions.
 * @return Distance percentage in the range of 0.0 - 1.0.
 */
double JoyControlStick::calculateXDistanceFromDeadZone(int axisXValue, int axisYValue, bool interpolate)
{
    double distance = 0.0;

    int axis1Value = axisXValue;
    int axis2Value = axisYValue;

    double angle2 = atan2(axis1Value, -axis2Value);
    double ang_sin = sin(angle2);
    double ang_cos = cos(angle2);

    int deadX = abs(floor(deadZone * ang_sin + 0.5));
    double squareStickFullPhi = qMin(ang_sin != 0.0 ? 1 / fabs(ang_sin) : 2, ang_cos != 0.0 ? 1 / fabs(ang_cos) : 2);
    double circle = this->circle;
    double circleStickFull = (squareStickFullPhi - 1) * circle + 1;

    double adjustedAxis1Value = (circleStickFull > 1.0) ? (axis1Value / circleStickFull) : axis1Value;
    double adjustedDeadXZone = (circleStickFull > 1.0) ? (deadX / circleStickFull) : deadX;

    // Interpolation would return the correct value if diagonalRange is 90 but
    // the routine gets skipped to save time.
    if (interpolate && (diagonalRange < 90))
    {
        JoyStickDirections direction = calculateStickDirection(axis1Value, axis2Value);

        if ((direction == StickRightUp) || (direction == StickRight))
        {
            QList<double> tempangles = getDiagonalZoneAngles();

            double maxangle = tempangles.at(3);
            double square_dist = getAbsoluteRawDistance(axis1Value, axis2Value);
            double mindeadX = fabs(square_dist * cos(maxangle * GlobalVariables::JoyControlStick::PI / 180.0));
            double currentDeadX = qMax(mindeadX, adjustedDeadXZone);
            double maxRange = static_cast<double>(maxZone) - currentDeadX;
            double tempdist4 = 0.0;

            if (maxRange != 0.0)
                tempdist4 = (fabs(adjustedAxis1Value) - currentDeadX) / maxRange;

            distance = tempdist4;
        } else if ((direction == StickRightDown) || (direction == StickDown))
        {
            QList<double> tempangles = getDiagonalZoneAngles();

            double maxangle = tempangles.at(5);
            double square_dist = getAbsoluteRawDistance(axis1Value, axis2Value);
            double mindeadX = fabs(square_dist * cos((maxangle - 90.0) * GlobalVariables::JoyControlStick::PI / 180.0));
            double currentDeadX = qMax(mindeadX, adjustedDeadXZone);
            double maxRange = static_cast<double>(maxZone) - currentDeadX;
            double tempdist4 = 0.0;

            if (maxRange != 0.0)
                tempdist4 = (fabs(adjustedAxis1Value) - currentDeadX) / maxRange;

            distance = tempdist4;
        } else if ((direction == StickLeftDown) || (direction == StickLeft))
        {
            QList<double> tempangles = getDiagonalZoneAngles();

            double maxangle = tempangles.at(7);
            double square_dist = getAbsoluteRawDistance(axis1Value, axis2Value);
            double mindeadX = fabs(square_dist * cos((maxangle - 180.0) * GlobalVariables::JoyControlStick::PI / 180.0));
            double currentDeadX = qMax(mindeadX, adjustedDeadXZone);
            double maxRange = static_cast<double>(maxZone) - currentDeadX;
            double tempdist4 = 0.0;

            if (maxRange != 0.0)
                tempdist4 = (fabs(adjustedAxis1Value) - currentDeadX) / maxRange;

            distance = tempdist4;
        } else if ((direction == StickLeftUp) || (direction == StickUp))
        {
            QList<double> tempangles = getDiagonalZoneAngles();

            double maxangle = tempangles.at(1);
            double square_dist = getAbsoluteRawDistance(axis1Value, axis2Value);
            double mindeadX = fabs(square_dist * cos((maxangle - 270.0) * GlobalVariables::JoyControlStick::PI / 180.0));
            double currentDeadX = qMax(mindeadX, adjustedDeadXZone);
            double maxRange = static_cast<double>(maxZone) - currentDeadX;
            double tempdist4 = 0.0;

            if (maxRange != 0.0)
                tempdist4 = (fabs(adjustedAxis1Value) - currentDeadX) / maxRange;

            distance = tempdist4;
        } else
        {
            // Backup plan. Should not arrive here.
            double maxRange = static_cast<double>(maxZone) - adjustedDeadXZone;
            double tempdist4 = 0.0;

            if (maxRange != 0.0)
                tempdist4 = (fabs(adjustedAxis1Value) - adjustedDeadXZone) / maxRange;

            distance = tempdist4;
        }
    } else
    {
        // No interpolation desired or diagonal range is 90 degrees.
        double maxRange = static_cast<double>(maxZone) - adjustedDeadXZone;
        double tempdist4 = 0.0;

        if (maxRange != 0.0)
            tempdist4 = (fabs(adjustedAxis1Value) - adjustedDeadXZone) / maxRange;

        distance = tempdist4;
    }

    distance = qBound(0.0, distance, 1.0);
    return distance;
}

/**
 * @brief Get the raw radial distance of the stick. Values will be between 0 - 32,767.
 * @return Radial distance in the range of 0 - 32,767.
 */
double JoyControlStick::getAbsoluteRawDistance()
{
    double temp = getAbsoluteRawDistance(axisX->getCurrentRawValue(), axisY->getCurrentRawValue());
    return temp;
}

double JoyControlStick::getAbsoluteRawDistance(int axisXValue, int axisYValue)
{
    int axis1Value = axisXValue;
    int axis2Value = axisYValue;

    int square_dist = (axis1Value * axis1Value) + (axis2Value * axis2Value);

    return sqrt(square_dist);
}

double JoyControlStick::getNormalizedAbsoluteDistance()
{
    int axis1Value = axisX->getCurrentRawValue();
    int axis2Value = axisY->getCurrentRawValue();

    int square_dist = (axis1Value * axis1Value) + (axis2Value * axis2Value);

    double distance = sqrt(square_dist) / static_cast<double>(maxZone);

    if (distance > 1.0)
        distance = 1.0;
    else if (distance < 0.0)
        distance = 0.0;

    return distance;
}

double JoyControlStick::getRadialDistance(int axisXValue, int axisYValue)
{
    int axis1Value = axisXValue;
    int axis2Value = axisYValue;

    int square_dist = (axis1Value * axis1Value) + (axis2Value * axis2Value);

    double distance = sqrt(square_dist) / static_cast<double>(maxZone);

    if (distance > 1.0)
        distance = 1.0;
    else if (distance < 0.0)
        distance = 0.0;

    return distance;
}

void JoyControlStick::setIndex(int index) { this->index = index; }

int JoyControlStick::getIndex() { return index; }

int JoyControlStick::getRealJoyIndex() { return index + 1; }

QString JoyControlStick::getName(bool forceFullFormat, bool displayNames)
{
    QString label = getPartialName(forceFullFormat, displayNames);

    label.append(": ");
    QStringList tempList = QStringList();

    if (buttons.contains(StickUp))
    {
        JoyControlStickButton *button = buttons.value(StickUp);

        if (!button->getButtonName().isEmpty())
            tempList.append(button->getButtonName());
        else
            tempList.append(button->getSlotsSummary());
    }

    if (buttons.contains(StickLeft))
    {
        JoyControlStickButton *button = buttons.value(StickLeft);

        if (!button->getButtonName().isEmpty())
            tempList.append(button->getButtonName());
        else
            tempList.append(button->getSlotsSummary());
    }

    if (buttons.contains(StickDown))
    {
        JoyControlStickButton *button = buttons.value(StickDown);

        if (!button->getButtonName().isEmpty())
            tempList.append(button->getButtonName());
        else
            tempList.append(button->getSlotsSummary());
    }

    if (buttons.contains(StickRight))
    {
        JoyControlStickButton *button = buttons.value(StickRight);

        if (!button->getButtonName().isEmpty())
            tempList.append(button->getButtonName());
        else
            tempList.append(button->getSlotsSummary());
    }

    label.append(tempList.join(", "));
    return label;
}

QString JoyControlStick::getPartialName(bool forceFullFormat, bool displayNames)
{
    QString label = QString();

    if (!stickName.isEmpty() && displayNames)
    {
        if (forceFullFormat)
            label.append(tr("Stick")).append(" ");

        label.append(stickName);
    } else if (!defaultStickName.isEmpty())
    {
        if (forceFullFormat)
            label.append(tr("Stick")).append(" ");

        label.append(defaultStickName);
    } else
    {
        label.append(tr("Stick")).append(" ");
        label.append(QString::number(getRealJoyIndex()));
    }

    setStickName(label);
    return label;
}

void JoyControlStick::setDefaultStickName(QString tempname)
{
    defaultStickName = tempname;
    emit stickNameChanged();
}

QString JoyControlStick::getDefaultStickName() { return defaultStickName; }

int JoyControlStick::getMaxZone() { return maxZone; }

/**
 * @brief Returns the modifier zone of the stick
 */
int JoyControlStick::getModifierZone() const { return m_modifier_zone; }

/**
 * @brief Checks if the modifier zone of this stick is inverted
 * @returns True if the modifier zone is inverted, false otherwise
 */
bool JoyControlStick::getModifierZoneInverted() const { return m_modifier_zone_inverted; }

int JoyControlStick::getCurrentlyAssignedSet() { return originset; }

void JoyControlStick::reset()
{
    deadZone = GlobalVariables::JoyControlStick::DEFAULTDEADZONE;
    maxZone = GlobalVariables::JoyAxis::AXISMAXZONE;
    m_modifier_zone = GlobalVariables::JoyControlStick::DEFAULTMODIFIERZONE;
    m_modifier_zone_inverted = GlobalVariables::JoyControlStick::DEFAULTMODIFIERZONEINVERTED;
    diagonalRange = GlobalVariables::JoyControlStick::DEFAULTDIAGONALRANGE;
    isActive = false;
    pendingStickEvent = false;

    activeButton1 = nullptr;
    activeButton2 = nullptr;
    activeButton3 = nullptr;
    safezone = false;
    currentDirection = StickCentered;
    currentMode = StandardMode;
    stickName.clear();
    circle = GlobalVariables::JoyControlStick::DEFAULTCIRCLE;
    stickDelay = GlobalVariables::JoyControlStick::DEFAULTSTICKDELAY;

    resetButtons();
}

void JoyControlStick::setDeadZone(int value)
{
    value = std::min(abs(value), GlobalVariables::JoyAxis::AXISMAX);

    if ((value != deadZone) && (value <= maxZone))
    {
        deadZone = value;
        emit deadZoneChanged(value);
        emit propertyUpdated();
    }
}

void JoyControlStick::setMaxZone(int value)
{
    value = std::min(abs(value), GlobalVariables::JoyAxis::AXISMAX);

    if ((value != maxZone) && (value > deadZone))
    {
        maxZone = value;
        emit maxZoneChanged(value);
        emit propertyUpdated();
    }
}

/**
 * @brief Sets the modifier zone of the stick to the given value
 * @param[in] value New stick modifier zone
 */
void JoyControlStick::setModifierZone(int value)
{
    value = std::min(abs(value), GlobalVariables::JoyAxis::AXISMAX);

    if ((value != m_modifier_zone) && (value < maxZone) && (value > deadZone))
    {
        m_modifier_zone = value;
        emit modifierZoneChanged(value);
        emit propertyUpdated();
    }
}

/**
 * @brief Inverts the direction of the modifier zone of the stick.
 * @param[in] value True if the zone should be inverted, false otherwise.
 */
void JoyControlStick::setModifierZoneInverted(bool value)
{
    if (value != m_modifier_zone_inverted)
    {
        m_modifier_zone_inverted = value;
        emit modifierZoneChanged(m_modifier_zone);
        emit propertyUpdated();
    }
}

/**
 * @brief Check if the stick is calibrated
 * @returns True if both axes of the stick are calibrated, false otherwise.
 */
bool JoyControlStick::isCalibrated() const { return axisX->isCalibrated() && axisY->isCalibrated(); }

/**
 * @brief Resets the calibration of both stick axes back to uncalibrated state.
 */
void JoyControlStick::resetCalibration()
{
    axisX->resetCalibration();
    axisY->resetCalibration();
}

/**
 * @brief Reads the calibration values of both stick axes
 * @param[out] offsetX Offset value "a" for X axis
 * @param[out] gainX Gain value "b" for X axis
 * @param[out] offsetY Offset value "a" for Y axis
 * @param[out] gainY Gain value "b" for Y axis
 */
void JoyControlStick::getCalibration(double *offsetX, double *gainX, double *offsetY, double *gainY) const
{
    axisX->getCalibration(offsetX, gainX);
    axisY->getCalibration(offsetY, gainY);
}

/**
 * @brief Sets the axis calibration values and sets the calibration flag.
 *  Calibrated value is calculated by the formula "a+b*x".
 * @param[in] offsetX Offset value "a" for X axis
 * @param[in] gainX Gain value "b" for X axis
 * @param[in] offsetY Offset value "a" for Y axis
 * @param[in] gainY Gain value "b" for Y axis
 */
void JoyControlStick::setCalibration(double offsetX, double gainX, double offsetY, double gainY)
{
    axisX->setCalibration(offsetX, gainX);
    axisY->setCalibration(offsetY, gainY);
}

/**
 * @brief Set the diagonal range value for a stick.
 * @param Value between 1 - 90.
 */
void JoyControlStick::setDiagonalRange(int value)
{
    if (value < 1)
        value = 1;
    else if (value > 90)
        value = 90;

    if (value != diagonalRange)
    {
        diagonalRange = value;
        emit diagonalRangeChanged(value);
        emit propertyUpdated();
    }
}

/**
 * @brief Take a XML stream and set the stick and direction button properties
 *     according to the values contained within the stream.
 * @param QXmlStreamReader instance that will be used to read property values.
 */
void JoyControlStick::readConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && (xml->name().toString() == "stick"))
    {
        xml->readNextStartElement();

        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name().toString() != "stick")))
        {
            if ((xml->name().toString() == "deadZone") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setDeadZone(tempchoice);
            } else if ((xml->name().toString() == "maxZone") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setMaxZone(tempchoice);
            } else if ((xml->name().toString() == "modifierZone") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                setModifierZone(tempchoice);
            } else if ((xml->name().toString() == "modifierZoneInverted") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                setModifierZoneInverted(tempchoice);
            } else if ((xml->name().toString() == "diagonalRange") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setDiagonalRange(tempchoice);
            } else if ((xml->name().toString() == "mode") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();

                if (temptext == "eight-way")
                {
                    this->setJoyMode(EightWayMode);
                } else if (temptext == "four-way")
                {
                    this->setJoyMode(FourWayCardinal);
                } else if (temptext == "diagonal")
                {
                    this->setJoyMode(FourWayDiagonal);
                }
            } else if ((xml->name().toString() == "squareStick") && xml->isStartElement())
            {
                int tempchoice = xml->readElementText().toInt();

                if ((tempchoice > 0) && (tempchoice <= 100))
                    this->setCircleAdjust(tempchoice / 100.0);
            } else if ((xml->name().toString() == GlobalVariables::JoyControlStickButton::xmlName) && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyControlStickButton *button = buttons.value(static_cast<JoyStickDirections>(index));
                QPointer<JoyButtonXml> joyButtonXml = new JoyButtonXml(button);

                if (button != nullptr)
                    joyButtonXml->readConfig(xml);
                else
                    xml->skipCurrentElement();

                if (!joyButtonXml.isNull())
                    delete joyButtonXml;
            } else if ((xml->name().toString() == GlobalVariables::JoyControlStickModifierButton::xmlName) &&
                       xml->isStartElement())
            {
                JoyButtonXml *joyButtonXml = new JoyButtonXml(modifierButton);
                joyButtonXml->readConfig(xml);
            } else if ((xml->name().toString() == "stickDelay") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setStickDelay(tempchoice);
            } else
            {
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }
    }
}

/**
 * @brief Write the status of the properties of a stick and direction buttons
 *     to an XML stream.
 * @param QXmlStreamWriter instance that will be used to write a profile.
 */
void JoyControlStick::writeConfig(QXmlStreamWriter *xml)
{
    if (!isDefault())
    {
        xml->writeStartElement("stick");
        xml->writeAttribute("index", QString::number(index + 1));

        if (deadZone != GlobalVariables::JoyControlStick::DEFAULTDEADZONE)
            xml->writeTextElement("deadZone", QString::number(deadZone));

        if (maxZone != GlobalVariables::JoyControlStick::DEFAULTMAXZONE)
            xml->writeTextElement("maxZone", QString::number(maxZone));

        if (m_modifier_zone != GlobalVariables::JoyControlStick::DEFAULTMODIFIERZONE)
            xml->writeTextElement("modifierZone", QString::number(m_modifier_zone));

        if (m_modifier_zone_inverted != GlobalVariables::JoyControlStick::DEFAULTMODIFIERZONEINVERTED)
            xml->writeTextElement("modifierZoneInverted", QString::number(m_modifier_zone_inverted));

        if ((currentMode == StandardMode || currentMode == EightWayMode) &&
            (diagonalRange != GlobalVariables::JoyControlStick::DEFAULTDIAGONALRANGE))
        {
            xml->writeTextElement("diagonalRange", QString::number(diagonalRange));
        }

        switch (currentMode)
        {
        case EightWayMode:
            xml->writeTextElement("mode", "eight-way");
            break;

        case FourWayCardinal:
            xml->writeTextElement("mode", "four-way");
            break;

        case FourWayDiagonal:
            xml->writeTextElement("mode", "diagonal");
            break;

        default:

            break;
        }

        if (circle > GlobalVariables::JoyControlStick::DEFAULTCIRCLE)
            xml->writeTextElement("squareStick", QString::number(circle * 100));

        if (stickDelay > GlobalVariables::JoyControlStick::DEFAULTSTICKDELAY)
            xml->writeTextElement("stickDelay", QString::number(stickDelay));

        QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(buttons);

        while (iter.hasNext())
        {
            JoyControlStickButton *button = iter.next().value();
            JoyButtonXml *joyButtonXml = new JoyButtonXml(button);
            joyButtonXml->writeConfig(xml);
            delete joyButtonXml;
            joyButtonXml = nullptr;
        }

        if (!modifierButton->isDefault())
        {
            JoyButtonXml *joyButtonXmlModif = new JoyButtonXml(modifierButton);
            joyButtonXmlModif->writeConfig(xml);
        }

        xml->writeEndElement();
    }
}

/**
 * @brief Reset all the properties of the stick direction buttons and the
 *     stick modifier button.
 */
void JoyControlStick::resetButtons()
{
    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(buttons);

    while (iter.hasNext())
    {
        JoyButton *button = iter.next().value();

        if (button != nullptr)
            button->reset();
    }

    if (modifierButton != nullptr)
        modifierButton->reset();
}

/**
 * @brief Get a pointer to the stick direction button for the desired
 *     direction.
 * @param Value of the direction of the stick.
 * @return Pointer to the stick direction button for the stick
 *     direction.
 */
JoyControlStickButton *JoyControlStick::getDirectionButton(JoyStickDirections direction)
{
    JoyControlStickButton *button = buttons.value(direction);

    return button;
}

/**
 * @brief Used to calculate the distance value that should be used for mouse
 *     movement.
 * @param button
 * @return Distance factor that should be used for mouse movement
 */
double JoyControlStick::calculateMouseDirectionalDistance(JoyControlStickButton *button)
{
    double finalDistance = 0.0;

    switch (currentDirection)
    {
    case StickUp: {
        finalDistance = calculateYDistanceFromDeadZone(true);
        break;
    }
    case StickRightUp: {
        if ((activeButton1 != nullptr) && (activeButton1 == button))
        {
            finalDistance = calculateXDistanceFromDeadZone(true);
        } else if ((activeButton2 != nullptr) && (activeButton2 == button))
        {
            finalDistance = calculateYDistanceFromDeadZone(true);
        } else if ((activeButton3 != nullptr) && (activeButton3 == button))
        {
            finalDistance = calculateEightWayDiagonalDistanceFromDeadZone();
        }

        break;
    }
    case StickRight: {
        finalDistance = calculateXDistanceFromDeadZone(true);

        break;
    }
    case StickRightDown: {
        if ((activeButton1 != nullptr) && (activeButton1 == button))
        {
            finalDistance = calculateXDistanceFromDeadZone(true);
        } else if ((activeButton2 != nullptr) && (activeButton2 == button))
        {
            finalDistance = calculateYDistanceFromDeadZone(true);
        } else if ((activeButton3 != nullptr) && (activeButton3 == button))
        {
            finalDistance = calculateEightWayDiagonalDistanceFromDeadZone();
        }

        break;
    }
    case StickDown: {
        finalDistance = calculateYDistanceFromDeadZone(true);

        break;
    }
    case StickLeftDown: {
        if ((activeButton1 != nullptr) && (activeButton1 == button))
        {
            finalDistance = calculateXDistanceFromDeadZone(true);
        } else if ((activeButton2 != nullptr) && (activeButton2 == button))
        {
            finalDistance = calculateYDistanceFromDeadZone(true);
        } else if ((activeButton3 != nullptr) && (activeButton3 == button))
        {
            finalDistance = calculateEightWayDiagonalDistanceFromDeadZone();
        }

        break;
    }
    case StickLeft: {
        finalDistance = calculateXDistanceFromDeadZone(true);

        break;
    }
    case StickLeftUp: {
        if ((activeButton1 != nullptr) && (activeButton1 == button))
        {
            finalDistance = calculateXDistanceFromDeadZone(true);
        } else if ((activeButton2 != nullptr) && (activeButton2 == button))
        {
            finalDistance = calculateYDistanceFromDeadZone(true);
        } else if ((activeButton3 != nullptr) && (activeButton3 == button))
        {
            finalDistance = calculateEightWayDiagonalDistanceFromDeadZone();
        }

        break;
    }
    default:
        break;
    }

    return finalDistance;
}

double JoyControlStick::calculateLastMouseDirectionalDistance(JoyControlStickButton *button)
{
    double finalDistance = 0.0;

    JoyStickDirections direction =
        calculateStickDirection(axisX->getLastKnownThrottleValue(), axisY->getLastKnownThrottleValue());

    if ((direction == StickUp) && (button->getJoyNumber() == static_cast<int>(StickUp)))
    {
        if (axisY->getLastKnownThrottleValue() >= 0)
        {
            finalDistance = 0.0;
        } else
        {
            finalDistance =
                calculateYDistanceFromDeadZone(axisX->getLastKnownThrottleValue(), axisY->getLastKnownThrottleValue(), true);
        }
    } else if (direction == StickRightUp)
    {
        if (button->getJoyNumber() == static_cast<int>(StickRight))
        {
            if (axisX->getLastKnownThrottleValue() < 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateXDistanceFromDeadZone(axisX->getLastKnownThrottleValue(),
                                                               axisY->getLastKnownThrottleValue(), true);
            }
        } else if (button->getJoyNumber() == static_cast<int>(StickUp))
        {
            if (axisY->getLastKnownThrottleValue() >= 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateYDistanceFromDeadZone(axisX->getLastKnownThrottleValue(),
                                                               axisY->getLastKnownThrottleValue(), true);
            }
        } else if (button->getJoyNumber() == static_cast<int>(StickRightUp))
        {
            if ((axisX->getLastKnownThrottleValue() <= 0) || (axisY->getLastKnownThrottleValue() >= 0))
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateEightWayDiagonalDistanceFromDeadZone(axisX->getLastKnownThrottleValue(),
                                                                              axisY->getLastKnownThrottleValue());
            }
        }
    } else if ((direction == StickRight) && (button->getJoyNumber() == static_cast<int>(StickRight)))
    {
        if (axisX->getLastKnownThrottleValue() < 0)
        {
            finalDistance = 0.0;
        } else
        {
            finalDistance =
                calculateXDistanceFromDeadZone(axisX->getLastKnownThrottleValue(), axisY->getLastKnownThrottleValue(), true);
        }
    } else if (direction == StickRightDown)
    {
        if (button->getJoyNumber() == static_cast<int>(StickRight))
        {
            if (axisX->getLastKnownThrottleValue() < 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateXDistanceFromDeadZone(axisX->getLastKnownThrottleValue(),
                                                               axisY->getLastKnownThrottleValue(), true);
            }
        } else if (button->getJoyNumber() == static_cast<int>(StickDown))
        {
            if (axisY->getLastKnownThrottleValue() < 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateYDistanceFromDeadZone(axisX->getLastKnownThrottleValue(),
                                                               axisY->getLastKnownThrottleValue(), true);
            }
        } else if (button->getJoyNumber() == static_cast<int>(StickRightDown))
        {
            if ((axisX->getLastKnownThrottleValue() <= 0) || (axisY->getLastKnownThrottleValue() >= 0))
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateEightWayDiagonalDistanceFromDeadZone(axisX->getLastKnownThrottleValue(),
                                                                              axisY->getLastKnownThrottleValue());
            }
        }
    } else if (direction == StickDown && (button->getJoyNumber() == static_cast<int>(StickDown)))
    {
        if (axisY->getLastKnownThrottleValue() >= 0)
        {
            finalDistance = 0.0;
        } else
        {
            finalDistance =
                calculateYDistanceFromDeadZone(axisX->getLastKnownThrottleValue(), axisY->getLastKnownThrottleValue(), true);
        }
    } else if (direction == StickLeftDown)
    {
        if (button->getJoyNumber() == static_cast<int>(StickLeft))
        {
            if (axisX->getLastKnownThrottleValue() >= 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateXDistanceFromDeadZone(axisX->getLastKnownThrottleValue(),
                                                               axisY->getLastKnownThrottleValue(), true);
            }
        } else if (button->getJoyNumber() == static_cast<int>(StickDown))
        {
            if (axisY->getLastKnownThrottleValue() < 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateYDistanceFromDeadZone(axisX->getLastKnownThrottleValue(),
                                                               axisY->getLastKnownThrottleValue(), true);
            }
        } else if (button->getJoyNumber() == static_cast<int>(StickLeftDown))
        {
            if ((axisX->getLastKnownThrottleValue() >= 0) || (axisY->getLastKnownThrottleValue() <= 0))
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateEightWayDiagonalDistanceFromDeadZone(axisX->getLastKnownThrottleValue(),
                                                                              axisY->getLastKnownThrottleValue());
            }
        }
    } else if ((direction == StickLeft) && (button->getJoyNumber() == static_cast<int>(StickLeft)))
    {
        if (axisX->getLastKnownThrottleValue() >= 0)
        {
            finalDistance = 0.0;
        } else
        {
            finalDistance =
                calculateXDistanceFromDeadZone(axisX->getLastKnownThrottleValue(), axisY->getLastKnownThrottleValue(), true);
        }
    } else if (direction == StickLeftUp)
    {
        if (button->getJoyNumber() == static_cast<int>(StickLeft))
        {
            if (axisX->getLastKnownThrottleValue() >= 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateXDistanceFromDeadZone(axisX->getLastKnownThrottleValue(),
                                                               axisY->getLastKnownThrottleValue(), true);
            }
        } else if (button->getJoyNumber() == static_cast<int>(StickUp))
        {
            if (axisY->getLastKnownThrottleValue() >= 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateYDistanceFromDeadZone(axisX->getLastKnownThrottleValue(),
                                                               axisY->getLastKnownThrottleValue(), true);
            }
        } else if (button->getJoyNumber() == static_cast<int>(StickLeftUp))
        {
            if ((axisX->getLastKnownThrottleValue() >= 0) || (axisY->getLastKnownThrottleValue() >= 0))
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateEightWayDiagonalDistanceFromDeadZone(axisX->getLastKnownThrottleValue(),
                                                                              axisY->getLastKnownThrottleValue());
            }
        }
    }

    return finalDistance;
}

double JoyControlStick::calculateLastDirectionalDistance()
{
    double finalDistance = 0.0;

    JoyStickDirections direction =
        calculateStickDirection(axisX->getLastKnownThrottleValue(), axisY->getLastKnownThrottleValue());

    switch (direction)
    {
    case StickUp: {
        if (!(axisX->getLastKnownThrottleValue() >= 0))
        {
            finalDistance =
                calculateYDistanceFromDeadZone(axisX->getLastKnownThrottleValue(), axisY->getLastKnownThrottleValue());
        }

        break;
    }
    case StickRightUp: {
        if (!(axisY->getLastKnownThrottleValue() <= 0) && !(axisY->getLastKnownThrottleValue() >= 0))
        {
            finalDistance = calculateEightWayDiagonalDistanceFromDeadZone(axisX->getLastKnownThrottleValue(),
                                                                          axisY->getLastKnownThrottleValue());
        }

        break;
    }
    case StickRight: {
        if (!(axisX->getLastKnownThrottleValue() <= 0))
        {
            finalDistance =
                calculateXDistanceFromDeadZone(axisX->getLastKnownThrottleValue(), axisY->getLastKnownThrottleValue());
        }

        break;
    }
    case StickRightDown: {
        if (!(axisY->getLastKnownThrottleValue() <= 0) && !(axisY->getLastKnownThrottleValue() <= 0))
        {
            finalDistance = calculateEightWayDiagonalDistanceFromDeadZone(axisX->getLastKnownThrottleValue(),
                                                                          axisY->getLastKnownThrottleValue());
        }

        break;
    }
    case StickDown: {
        if (!(axisY->getLastKnownThrottleValue() <= 0))
        {
            finalDistance =
                calculateYDistanceFromDeadZone(axisX->getLastKnownThrottleValue(), axisY->getLastKnownThrottleValue());
        }

        break;
    }
    case StickLeftDown: {
        if (!(axisY->getLastKnownThrottleValue() >= 0) && !(axisY->getLastKnownThrottleValue() <= 0))
        {
            finalDistance = calculateEightWayDiagonalDistanceFromDeadZone(axisX->getLastKnownThrottleValue(),
                                                                          axisY->getLastKnownThrottleValue());
        }

        break;
    }
    case StickLeft: {
        if (!(axisX->getLastKnownThrottleValue() >= 0))
        {
            finalDistance =
                calculateXDistanceFromDeadZone(axisX->getLastKnownThrottleValue(), axisY->getLastKnownThrottleValue());
        }

        break;
    }
    case StickLeftUp: {
        if (!(axisY->getLastKnownThrottleValue() >= 0) && !(axisY->getLastKnownThrottleValue() >= 0))
        {
            finalDistance = calculateEightWayDiagonalDistanceFromDeadZone(axisX->getLastKnownThrottleValue(),
                                                                          axisY->getLastKnownThrottleValue());
        }

        break;
    }
    default:
        break;
    }

    return finalDistance;
}

double JoyControlStick::calculateLastAccelerationDirectionalDistance()
{
    double finalDistance = 0.0;

    switch (currentDirection)
    {
    case StickUp: {
        if (!(axisX->getLastKnownRawValue() >= 0))
            finalDistance = calculateYAxisDistance(axisY->getLastKnownRawValue());

        break;
    }
    case StickRightUp: {
        if (!(axisY->getLastKnownRawValue() <= 0) && !(axisY->getLastKnownRawValue() >= 0))
            finalDistance = calculateEightWayDiagonalDistance(axisX->getLastKnownRawValue(), axisY->getLastKnownRawValue());

        break;
    }
    case StickRight: {
        if (!(axisX->getLastKnownRawValue() <= 0))
            finalDistance = calculateXAxisDistance(axisX->getLastKnownRawValue());

        break;
    }
    case StickRightDown: {
        if (!(axisY->getLastKnownRawValue() <= 0) && !(axisY->getLastKnownRawValue() <= 0))
            finalDistance = calculateEightWayDiagonalDistance(axisX->getLastKnownRawValue(), axisY->getLastKnownRawValue());

        break;
    }
    case StickDown: {
        if (!(axisY->getLastKnownRawValue() <= 0))
            finalDistance = calculateYAxisDistance(axisY->getLastKnownRawValue());

        break;
    }
    case StickLeftDown: {
        if (!(axisY->getLastKnownRawValue() >= 0) && !(axisY->getLastKnownRawValue() <= 0))
            finalDistance = calculateEightWayDiagonalDistance(axisX->getLastKnownRawValue(), axisY->getLastKnownRawValue());

        break;
    }
    case StickLeft: {
        if (!(axisX->getLastKnownRawValue() >= 0))
            finalDistance = calculateXAxisDistance(axisX->getLastKnownRawValue());

        break;
    }
    case StickLeftUp: {
        if (!(axisY->getLastKnownRawValue() >= 0) && !(axisY->getLastKnownRawValue() >= 0))
            finalDistance = calculateEightWayDiagonalDistance(axisX->getLastKnownRawValue(), axisY->getLastKnownRawValue());

        break;
    }
    default:
        break;
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

    switch (currentDirection)
    {
    case StickUp: {
        finalDistance = calculateYDistanceFromDeadZone();
        break;
    }
    case StickRightUp: {
        finalDistance = getDistanceFromDeadZone();
        break;
    }
    case StickRight: {
        finalDistance = calculateXDistanceFromDeadZone();
        break;
    }
    case StickRightDown: {
        finalDistance = getDistanceFromDeadZone();
        break;
    }
    case StickDown: {
        finalDistance = calculateYDistanceFromDeadZone();
        break;
    }
    case StickLeftDown: {
        finalDistance = getDistanceFromDeadZone();
        break;
    }
    case StickLeft: {
        finalDistance = calculateXDistanceFromDeadZone();
        break;
    }
    case StickLeftUp: {
        finalDistance = getDistanceFromDeadZone();
        break;
    }
    default:
        break;
    }

    return finalDistance;
}

/**
 * @brief Get the value for the currently active stick direction.
 * @return Value of the corresponding active stick direction.
 */
JoyControlStick::JoyStickDirections JoyControlStick::getCurrentDirection() { return currentDirection; }

/**
 * @brief Get the value for the corresponding X axis.
 * @return X axis value.
 */
int JoyControlStick::getXCoordinate() { return axisX->getCurrentRawValue(); }

/**
 * @brief Get the value for the corresponding Y axis.
 * @return Y axis value.
 */
int JoyControlStick::getYCoordinate() { return axisY->getCurrentRawValue(); }

int JoyControlStick::getCircleXCoordinate()
{
    int axisXValue = axisX->getCurrentRawValue();
    int axisYValue = axisX->getCurrentRawValue();

    if (this->circle > 0.0)
        axisXValue = calculateCircleXValue(axisXValue, axisYValue);

    return axisXValue;
}

int JoyControlStick::getCircleYCoordinate()
{
    int axisXValue = axisX->getCurrentRawValue();
    int axisYValue = axisY->getCurrentRawValue();

    if (this->circle > 0.0)
        axisYValue = calculateCircleYValue(axisXValue, axisYValue);

    return axisYValue;
}

int JoyControlStick::calculateCircleXValue(int axisXValue, int axisYValue)
{
    int value = axisXValue;

    if (this->circle > 0.0)
    {
        int axis1Value = axisXValue;
        int axis2Value = axisYValue;

        double angle2 = atan2(axis1Value, -axis2Value);
        double ang_sin = sin(angle2);
        double ang_cos = cos(angle2);

        double squareStickFull = qMin(ang_sin != 0.0 ? 1 / fabs(ang_sin) : 2, ang_cos != 0.0 ? 1 / fabs(ang_cos) : 2);
        double circle = this->circle;
        double circleStickFull = (squareStickFull - 1) * circle + 1;

        value = (circleStickFull > 1.0) ? floor((axisXValue / circleStickFull) + 0.5) : value;
    }

    return value;
}

int JoyControlStick::calculateCircleYValue(int axisXValue, int axisYValue)
{
    int value = axisYValue;

    if (this->circle > 0.0)
    {
        int axis1Value = axisXValue;
        int axis2Value = axisYValue;

        double angle2 = atan2(axis1Value, -axis2Value);
        double ang_sin = sin(angle2);
        double ang_cos = cos(angle2);

        double squareStickFull = qMin(ang_sin != 0.0 ? 1 / fabs(ang_sin) : 2, ang_cos != 0.0 ? 1 / fabs(ang_cos) : 2);
        double circle = this->circle;
        double circleStickFull = (squareStickFull - 1) * circle + 1;

        value = (circleStickFull > 1.0) ? floor((axisYValue / circleStickFull) + 0.5) : value;
    }

    return value;
}

QList<double> JoyControlStick::getDiagonalZoneAngles()
{
    QList<double> anglesList;

    int diagonalAngle = diagonalRange;

    double cardinalAngle = (360 - (diagonalAngle * 4)) / 4.0;

    double initialLeft = 360 - ((cardinalAngle) / 2.0);
    double initialRight = ((cardinalAngle) / 2.0);

    double upRightInitial = initialRight;
    double rightInitial = upRightInitial + diagonalAngle;
    double downRightInitial = rightInitial + cardinalAngle;
    double downInitial = downRightInitial + diagonalAngle;
    double downLeftInitial = downInitial + cardinalAngle;
    double leftInitial = downLeftInitial + diagonalAngle;
    double upLeftInitial = leftInitial + cardinalAngle;

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

QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton *> *JoyControlStick::getButtons() { return &buttons; }

JoyAxis *JoyControlStick::getAxisX() { return axisX; }

JoyAxis *JoyControlStick::getAxisY() { return axisY; }

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

JoyControlStick::JoyMode JoyControlStick::getJoyMode() { return currentMode; }

void JoyControlStick::releaseButtonEvents()
{
    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(buttons);

    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->joyEvent(false, true);
    }
}

bool JoyControlStick::isDefault()
{
    bool value = true;
    value = value && (deadZone == GlobalVariables::JoyControlStick::DEFAULTDEADZONE);
    value = value && (maxZone == GlobalVariables::JoyControlStick::DEFAULTMAXZONE);
    value = value && (m_modifier_zone == GlobalVariables::JoyControlStick::DEFAULTMODIFIERZONE);
    value = value && (m_modifier_zone_inverted == GlobalVariables::JoyControlStick::DEFAULTMODIFIERZONEINVERTED);
    value = value && (diagonalRange == GlobalVariables::JoyControlStick::DEFAULTDIAGONALRANGE);
    value = value && (currentMode == DEFAULTMODE);
    value = value && qFuzzyCompare(circle, GlobalVariables::JoyControlStick::DEFAULTCIRCLE);
    value = value && (stickDelay == GlobalVariables::JoyControlStick::DEFAULTSTICKDELAY);

    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(buttons);

    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        value = value && (button->isDefault());
    }

    if (modifierButton != nullptr)
        value = value && modifierButton->isDefault();

    return value;
}

void JoyControlStick::setButtonsMouseMode(JoyButton::JoyMouseMovementMode mode)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(buttons);

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
    QHash<JoyStickDirections, JoyControlStickButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return result;

    JoyControlStickButton *button = iter.value();
    initialMode = button->getMouseMode();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        JoyButton::JoyMouseMovementMode temp = button->getMouseMode();

        if (temp != initialMode)
        {
            result = false;
            break;
        }
    }

    return result;
}

JoyButton::JoyMouseMovementMode JoyControlStick::getButtonsPresetMouseMode()
{
    JoyButton::JoyMouseMovementMode resultMode = JoyButton::MouseCursor;
    QHash<JoyStickDirections, JoyControlStickButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return resultMode;

    JoyControlStickButton *button = iter.value();
    resultMode = button->getMouseMode();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        JoyButton::JoyMouseMovementMode temp = button->getMouseMode();

        if (temp != resultMode)
        {
            resultMode = JoyButton::MouseCursor;
            break;
        }
    }

    return resultMode;
}

void JoyControlStick::setButtonsMouseCurve(JoyButton::JoyMouseCurve mouseCurve)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(buttons);

    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setMouseCurve(mouseCurve);
    }
}

bool JoyControlStick::hasSameButtonsMouseCurve()
{
    bool result = true;

    QHash<JoyStickDirections, JoyControlStickButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return result;

    JoyControlStickButton *button = iter.value();
    JoyButton::JoyMouseCurve initialCurve = button->getMouseCurve();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        JoyButton::JoyMouseCurve temp = button->getMouseCurve();

        if (temp != initialCurve)
        {
            result = false;
            break;
        }
    }

    return result;
}

JoyButton::JoyMouseCurve JoyControlStick::getButtonsPresetMouseCurve()
{
    JoyButton::JoyMouseCurve resultCurve = JoyButton::LinearCurve;
    QHash<JoyStickDirections, JoyControlStickButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return resultCurve;

    JoyControlStickButton *button = iter.value();
    resultCurve = button->getMouseCurve();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        JoyButton::JoyMouseCurve temp = button->getMouseCurve();

        if (temp != resultCurve)
        {
            resultCurve = JoyButton::LinearCurve;
            break;
        }
    }

    return resultCurve;
}

void JoyControlStick::setButtonsSpringWidth(int value)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(buttons);

    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setSpringWidth(value);
    }
}

void JoyControlStick::setButtonsSpringHeight(int value)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(buttons);

    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setSpringHeight(value);
    }
}

int JoyControlStick::getButtonsPresetSpringWidth()
{
    int presetSpringWidth = 0;

    QHash<JoyStickDirections, JoyControlStickButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return presetSpringWidth;

    JoyControlStickButton *button = iter.value();
    presetSpringWidth = button->getSpringWidth();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        int temp = button->getSpringWidth();

        if (temp != presetSpringWidth)
        {
            presetSpringWidth = 0;
            break;
        }
    }

    return presetSpringWidth;
}

int JoyControlStick::getButtonsPresetSpringHeight()
{
    int presetSpringHeight = 0;

    QHash<JoyStickDirections, JoyControlStickButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return presetSpringHeight;

    JoyControlStickButton *button = iter.value();
    presetSpringHeight = button->getSpringHeight();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        int temp = button->getSpringHeight();

        if (temp != presetSpringHeight)
        {
            presetSpringHeight = 0;
            break;
        }
    }

    return presetSpringHeight;
}

void JoyControlStick::setButtonsSensitivity(double value)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(buttons);

    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setSensitivity(value);
    }
}

double JoyControlStick::getButtonsPresetSensitivity()
{
    double presetSensitivity = 1.0;

    QHash<JoyStickDirections, JoyControlStickButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return presetSensitivity;

    JoyControlStickButton *button = iter.value();
    presetSensitivity = button->getSensitivity();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        double temp = button->getSensitivity();

        if (!qFuzzyCompare(temp, presetSensitivity))
        {
            presetSensitivity = 1.0;
            break;
        }
    }

    return presetSensitivity;
}

QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton *> JoyControlStick::getApplicableButtons()
{
    QHash<JoyStickDirections, JoyControlStickButton *> temphash;

    if ((currentMode == StandardMode) || (currentMode == EightWayMode) || (currentMode == FourWayCardinal))
    {
        temphash.insert(StickUp, buttons.value(StickUp));
        temphash.insert(StickDown, buttons.value(StickDown));
        temphash.insert(StickLeft, buttons.value(StickLeft));
        temphash.insert(StickRight, buttons.value(StickRight));
    }

    if ((currentMode == EightWayMode) || (currentMode == FourWayDiagonal))
    {
        temphash.insert(StickLeftUp, buttons.value(StickLeftUp));
        temphash.insert(StickRightUp, buttons.value(StickRightUp));
        temphash.insert(StickRightDown, buttons.value(StickRightDown));
        temphash.insert(StickLeftDown, buttons.value(StickLeftDown));
    }

    return temphash;
}

void JoyControlStick::setStickName(QString tempName)
{
    if ((tempName.length() <= 20) && (tempName != stickName))
    {
        stickName = tempName;
        emit stickNameChanged();
        // emit propertyUpdated();
    }
}

QString JoyControlStick::getStickName() { return stickName; }

void JoyControlStick::setButtonsWheelSpeedX(int value)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(buttons);

    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setWheelSpeed(value, 'X');
    }
}

void JoyControlStick::setButtonsWheelSpeedY(int value)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(buttons);

    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setWheelSpeed(value, 'Y');
    }
}

/**
 * @brief Get pointer to the set that a stick belongs to.
 * @return Pointer to the set that a stick belongs to.
 */
SetJoystick *JoyControlStick::getParentSet()
{
    SetJoystick *temp = nullptr;

    if (axisX != nullptr)
        temp = axisX->getParentSet();
    else if (axisY != nullptr)
        temp = axisY->getParentSet();

    return temp;
}

/**
 * @brief Activate a stick direction button.
 * @param Stick direction button that will be activated.
 * @param [out] Pointer to the currently active button.
 * @param Should set changing routines be ignored.
 */
void JoyControlStick::performButtonPress(JoyControlStickButton *eventbutton, JoyControlStickButton *&activebutton,
                                         bool ignoresets)
{
    activebutton = eventbutton;
    eventbutton->joyEvent(true, ignoresets);
}

/**
 * @brief Stick direction button to release.
 * @param Stick direction button that will be released.
 * @param Should set changing routines be ignored.
 */
void JoyControlStick::performButtonRelease(JoyControlStickButton *&eventbutton, bool ignoresets)
{
    eventbutton->joyEvent(false, ignoresets);
    eventbutton = nullptr;
}

/**
 * @brief Determine which stick direction buttons should be active for a
 *     standard mode stick.
 * @param [out] Pointer to an X axis stick direction button that should be active.
 * @param [out] Pointer to a Y axis stick direction button that should be active.
 */
void JoyControlStick::determineStandardModeEvent(JoyControlStickButton *&eventbutton1, JoyControlStickButton *&eventbutton2)
{
    double bearing = calculateBearing();

    QList<double> anglesList = getDiagonalZoneAngles();
    double initialLeft = anglesList.value(0);
    double initialRight = anglesList.value(1);
    double upRightInitial = anglesList.value(2);
    double rightInitial = anglesList.value(3);
    double downRightInitial = anglesList.value(4);
    double downInitial = anglesList.value(5);
    double downLeftInitial = anglesList.value(6);
    double leftInitial = anglesList.value(7);
    double upLeftInitial = anglesList.value(8);

    if ((bearing <= initialRight) || (bearing >= initialLeft))
    {
        currentDirection = StickUp;
        eventbutton2 = buttons.value(StickUp);
    } else if ((bearing >= upRightInitial) && (bearing < rightInitial))
    {
        currentDirection = StickRightUp;
        eventbutton1 = buttons.value(StickRight);
        eventbutton2 = buttons.value(StickUp);
    } else if ((bearing >= rightInitial) && (bearing < downRightInitial))
    {
        currentDirection = StickRight;
        eventbutton1 = buttons.value(StickRight);
    } else if ((bearing >= downRightInitial) && (bearing < downInitial))
    {
        currentDirection = StickRightDown;
        eventbutton1 = buttons.value(StickRight);
        eventbutton2 = buttons.value(StickDown);
    } else if ((bearing >= downInitial) && (bearing < downLeftInitial))
    {
        currentDirection = StickDown;
        eventbutton2 = buttons.value(StickDown);
    } else if ((bearing >= downLeftInitial) && (bearing < leftInitial))
    {
        currentDirection = StickLeftDown;
        eventbutton1 = buttons.value(StickLeft);
        eventbutton2 = buttons.value(StickDown);
    } else if ((bearing >= leftInitial) && (bearing < upLeftInitial))
    {
        currentDirection = StickLeft;
        eventbutton1 = buttons.value(StickLeft);
    } else if ((bearing >= upLeftInitial) && (bearing < initialLeft))
    {
        currentDirection = StickLeftUp;
        eventbutton1 = buttons.value(StickLeft);
        eventbutton2 = buttons.value(StickUp);
    }
}

/**
 * @brief Determine which stick direction button should be active for a 8 way
 *     mode stick.
 * @param [out] Pointer to an X axis stick direction button that should be active.
 * @param [out] Pointer to a Y axis stick direction button that should be active.
 * @param [out] Pointer to a diagonal stick direction button that should be active.
 */
void JoyControlStick::determineEightWayModeEvent(JoyControlStickButton *&eventbutton1, JoyControlStickButton *&eventbutton2,
                                                 JoyControlStickButton *&eventbutton3)
{
    double bearing = calculateBearing();

    QList<double> anglesList = getDiagonalZoneAngles();
    double initialLeft = anglesList.value(0);
    double initialRight = anglesList.value(1);
    double upRightInitial = anglesList.value(2);
    double rightInitial = anglesList.value(3);
    double downRightInitial = anglesList.value(4);
    double downInitial = anglesList.value(5);
    double downLeftInitial = anglesList.value(6);
    double leftInitial = anglesList.value(7);
    double upLeftInitial = anglesList.value(8);

    if ((bearing <= initialRight) || (bearing >= initialLeft))
    {
        currentDirection = StickUp;
        eventbutton2 = buttons.value(StickUp);
    } else if ((bearing >= upRightInitial) && (bearing < rightInitial))
    {
        currentDirection = StickRightUp;
        eventbutton3 = buttons.value(StickRightUp);
    } else if ((bearing >= rightInitial) && (bearing < downRightInitial))
    {
        currentDirection = StickRight;
        eventbutton1 = buttons.value(StickRight);
    } else if ((bearing >= downRightInitial) && (bearing < downInitial))
    {
        currentDirection = StickRightDown;
        eventbutton3 = buttons.value(StickRightDown);
    } else if ((bearing >= downInitial) && (bearing < downLeftInitial))
    {
        currentDirection = StickDown;
        eventbutton2 = buttons.value(StickDown);
    } else if ((bearing >= downLeftInitial) && (bearing < leftInitial))
    {
        currentDirection = StickLeftDown;
        eventbutton3 = buttons.value(StickLeftDown);
    } else if ((bearing >= leftInitial) && (bearing < upLeftInitial))
    {
        currentDirection = StickLeft;
        eventbutton1 = buttons.value(StickLeft);
    } else if ((bearing >= upLeftInitial) && (bearing < initialLeft))
    {
        currentDirection = StickLeftUp;
        eventbutton3 = buttons.value(StickLeftUp);
    }
}

/**
 * @brief Determine which cardinal stick direction button should be active
 *     when using a four way cardinal stick.
 * @param [out] Pointer to an X axis stick direction button that should be active.
 * @param [out] Pointer to a Y axis stick direction button that should be active.
 */
void JoyControlStick::determineFourWayCardinalEvent(JoyControlStickButton *&eventbutton1,
                                                    JoyControlStickButton *&eventbutton2)
{
    double bearing = calculateBearing();

    QList<int> anglesList = getFourWayCardinalZoneAngles();
    int rightInitial = anglesList.value(0);
    int downInitial = anglesList.value(1);
    int leftInitial = anglesList.value(2);
    int upInitial = anglesList.value(3);

    if ((bearing < rightInitial) || (bearing >= upInitial))
    {
        currentDirection = StickUp;
        eventbutton2 = buttons.value(StickUp);
    } else if ((bearing >= rightInitial) && (bearing < downInitial))
    {
        currentDirection = StickRight;
        eventbutton1 = buttons.value(StickRight);
    } else if ((bearing >= downInitial) && (bearing < leftInitial))
    {
        currentDirection = StickDown;
        eventbutton2 = buttons.value(StickDown);
    } else if ((bearing >= leftInitial) && (bearing < upInitial))
    {
        currentDirection = StickLeft;
        eventbutton1 = buttons.value(StickLeft);
    }
}

/**
 * @brief Determine which stick direction button should be active when using 4 way
 *     diagonal mode.
 * @param [out] pointer to a diagonal stick direction button that should be active.
 */
void JoyControlStick::determineFourWayDiagonalEvent(JoyControlStickButton *&eventbutton3)
{
    double bearing = calculateBearing();

    QList<int> anglesList = getFourWayDiagonalZoneAngles();
    int upRightInitial = anglesList.value(0);
    int downRightInitial = anglesList.value(1);
    int downLeftInitial = anglesList.value(2);
    int upLeftInitial = anglesList.value(3);

    if ((bearing >= upRightInitial) && (bearing < downRightInitial))
    {
        currentDirection = StickRightUp;
        eventbutton3 = buttons.value(StickRightUp);
    } else if ((bearing >= downRightInitial) && (bearing < downLeftInitial))
    {
        currentDirection = StickRightDown;
        eventbutton3 = buttons.value(StickRightDown);
    } else if ((bearing >= downLeftInitial) && (bearing < upLeftInitial))
    {
        currentDirection = StickLeftDown;
        eventbutton3 = buttons.value(StickLeftDown);
    } else if (bearing >= upLeftInitial)
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
    return determineStandardModeDirection(axisX->getCurrentRawValue(), axisY->getCurrentRawValue());
}

JoyControlStick::JoyStickDirections JoyControlStick::determineStandardModeDirection(int axisXValue, int axisYValue)
{
    JoyStickDirections result = StickCentered;

    double bearing = calculateBearing(axisXValue, axisYValue);

    QList<double> anglesList = getDiagonalZoneAngles();
    int initialLeft = anglesList.value(0);
    int initialRight = anglesList.value(1);
    int upRightInitial = anglesList.value(2);
    int rightInitial = anglesList.value(3);
    int downRightInitial = anglesList.value(4);
    int downInitial = anglesList.value(5);
    int downLeftInitial = anglesList.value(6);
    int leftInitial = anglesList.value(7);
    int upLeftInitial = anglesList.value(8);

    if ((bearing <= initialRight) || (bearing >= initialLeft))
    {
        result = StickUp;
    } else if ((bearing >= upRightInitial) && (bearing < rightInitial))
    {
        result = StickRightUp;
    } else if ((bearing >= rightInitial) && (bearing < downRightInitial))
    {
        result = StickRight;
    } else if ((bearing >= downRightInitial) && (bearing < downInitial))
    {
        result = StickRightDown;
    } else if ((bearing >= downInitial) && (bearing < downLeftInitial))
    {
        result = StickDown;
    } else if ((bearing >= downLeftInitial) && (bearing < leftInitial))
    {
        result = StickLeftDown;
    } else if ((bearing >= leftInitial) && (bearing < upLeftInitial))
    {
        result = StickLeft;
    } else if ((bearing >= upLeftInitial) && (bearing < initialLeft))
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
    return determineStandardModeDirection(axisX->getCurrentRawValue(), axisY->getCurrentRawValue());
}

JoyControlStick::JoyStickDirections JoyControlStick::determineEightWayModeDirection(int axisXValue, int axisYValue)
{
    return determineStandardModeDirection(axisXValue, axisYValue);
}

/**
 * @brief Find the current stick direction based on a Four Way Cardinal mode
 *     stick.
 * @return Current direction the stick is positioned.
 */
JoyControlStick::JoyStickDirections JoyControlStick::determineFourWayCardinalDirection()
{
    return determineFourWayCardinalDirection(axisX->getCurrentRawValue(), axisY->getCurrentRawValue());
}

JoyControlStick::JoyStickDirections JoyControlStick::determineFourWayCardinalDirection(int axisXValue, int axisYValue)
{
    JoyStickDirections result = StickCentered;

    double bearing = calculateBearing(axisXValue, axisYValue);

    QList<int> anglesList = getFourWayCardinalZoneAngles();
    int rightInitial = anglesList.value(0);
    int downInitial = anglesList.value(1);
    int leftInitial = anglesList.value(2);
    int upInitial = anglesList.value(3);

    if ((bearing < rightInitial) || (bearing >= upInitial))
    {
        result = StickUp;
    } else if ((bearing >= rightInitial) && (bearing < downInitial))
    {
        result = StickRight;
    } else if ((bearing >= downInitial) && (bearing < leftInitial))
    {
        result = StickDown;
    } else if ((bearing >= leftInitial) && (bearing < upInitial))
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
    return determineFourWayDiagonalDirection(axisX->getCurrentRawValue(), axisY->getCurrentRawValue());
}

JoyControlStick::JoyStickDirections JoyControlStick::determineFourWayDiagonalDirection(int axisXValue, int axisYValue)
{
    JoyStickDirections result = StickCentered;
    double bearing = calculateBearing(axisXValue, axisYValue);
    QList<int> anglesList = getFourWayDiagonalZoneAngles();
    int upRightInitial = anglesList.value(0);
    int downRightInitial = anglesList.value(1);
    int downLeftInitial = anglesList.value(2);
    int upLeftInitial = anglesList.value(3);

    if ((bearing >= upRightInitial) && (bearing < downRightInitial))
    {
        result = StickRightUp;
    } else if ((bearing >= downRightInitial) && (bearing < downLeftInitial))
    {
        result = StickRightDown;
    } else if ((bearing >= downLeftInitial) && (bearing < upLeftInitial))
    {
        result = StickLeftDown;
    } else if (bearing >= upLeftInitial)
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
    return calculateStickDirection(axisX->getCurrentRawValue(), axisY->getCurrentRawValue());
}

JoyControlStick::JoyStickDirections JoyControlStick::calculateStickDirection(int axisXValue, int axisYValue)
{
    JoyStickDirections result = StickCentered;

    switch (currentMode)
    {
    case StandardMode: {
        result = determineStandardModeDirection(axisXValue, axisYValue);
        break;
    }
    case EightWayMode: {
        result = determineEightWayModeDirection(axisXValue, axisYValue);
        break;
    }
    case FourWayCardinal: {
        result = determineFourWayCardinalDirection(axisXValue, axisYValue);
        break;
    }
    case FourWayDiagonal: {
        result = determineFourWayDiagonalDirection(axisXValue, axisYValue);
        break;
    }
    }

    return result;
}

void JoyControlStick::establishPropertyUpdatedConnection()
{
    connect(this, &JoyControlStick::propertyUpdated, getParentSet()->getInputDevice(), &InputDevice::profileEdited);
}

void JoyControlStick::disconnectPropertyUpdatedConnection()
{
    disconnect(this, &JoyControlStick::propertyUpdated, getParentSet()->getInputDevice(), &InputDevice::profileEdited);
}

/**
 * @brief Check all stick buttons and see if any have slots assigned.
 * @return Status of whether any stick button has a slot assigned.
 */
bool JoyControlStick::hasSlotsAssigned()
{
    bool hasSlots = false;
    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(buttons);

    while (iter.hasNext())
    {
        JoyButton *button = iter.next().value();

        if (button != nullptr)
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
    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(buttons);

    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setSpringRelativeStatus(value);
    }
}

bool JoyControlStick::isRelativeSpring()
{
    bool relative = false;

    QHash<JoyStickDirections, JoyControlStickButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return relative;

    JoyControlStickButton *button = iter.value();
    relative = button->isRelativeSpring();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        bool temp = button->isRelativeSpring();

        if (temp != relative)
        {
            relative = false;
            break;
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
    destStick->m_modifier_zone = m_modifier_zone;
    destStick->m_modifier_zone_inverted = m_modifier_zone_inverted;
    destStick->diagonalRange = diagonalRange;
    destStick->currentDirection = currentDirection;
    destStick->currentMode = currentMode;
    destStick->stickName = stickName;
    destStick->circle = circle;
    destStick->stickDelay = stickDelay;

    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(destStick->buttons);

    while (iter.hasNext())
    {
        JoyControlStickButton *destButton = iter.next().value();

        if (destButton != nullptr)
        {
            JoyControlStickButton *sourceButton = buttons.value(destButton->getDirection());

            if (sourceButton != nullptr)
                sourceButton->copyAssignments(destButton);
        }
    }

    JoyControlStickModifierButton *destModifierButton = destStick->getModifierButton();
    if ((modifierButton != nullptr) && (destModifierButton != nullptr))
    {
        modifierButton->copyAssignments(destModifierButton);
    }

    if (!destStick->isDefault())
        emit propertyUpdated();
}

/**
 * @brief Set the percentage of the outer square that should be ignored
 *     when performing the final axis calculations.
 * @param Percentage represented by the range of 0.0 - 1.0.
 */
void JoyControlStick::setCircleAdjust(double circle)
{
    if ((circle >= 0.0) && (circle <= 1.0))
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
double JoyControlStick::getCircleAdjust() { return circle; }

/**
 * @brief Slot called when directionDelayTimer has timed out. The method will
 *     call createDeskEvent.
 */
void JoyControlStick::stickDirectionChangeEvent() { createDeskEvent(); }

void JoyControlStick::setStickDelay(int value)
{
    if (((value >= 10) && (value <= 1000)) || (value == 0))
    {
        this->stickDelay = value;
        emit stickDelayChanged(value);
        emit propertyUpdated();
    }
}

int JoyControlStick::getStickDelay() { return stickDelay; }

void JoyControlStick::setButtonsEasingDuration(double value)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(buttons);

    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setEasingDuration(value);
    }
}

double JoyControlStick::getButtonsEasingDuration()
{
    double result = GlobalVariables::JoyButton::DEFAULTEASINGDURATION;
    QHash<JoyStickDirections, JoyControlStickButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return result;

    JoyControlStickButton *button = iter.value();
    result = button->getEasingDuration();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        double temp = button->getEasingDuration();

        if (!qFuzzyCompare(temp, result))
        {
            result = GlobalVariables::JoyButton::DEFAULTEASINGDURATION;
            break;
        }
    }

    return result;
}

JoyControlStickModifierButton *JoyControlStick::getModifierButton() { return modifierButton; }

void JoyControlStick::queueJoyEvent(bool ignoresets)
{
    Q_UNUSED(ignoresets)

    pendingStickEvent = true;
}

bool JoyControlStick::hasPendingEvent() { return pendingStickEvent; }

void JoyControlStick::activatePendingEvent()
{
    if (pendingStickEvent)
    {
        bool ignoresets = false;
        joyEvent(ignoresets);

        pendingStickEvent = false;
    }
}

void JoyControlStick::clearPendingEvent() { pendingStickEvent = false; }

void JoyControlStick::setButtonsExtraAccelerationStatus(bool enabled)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(getApplicableButtons());

    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();

        if (button != nullptr)
            button->setExtraAccelerationStatus(enabled);
    }
}

bool JoyControlStick::getButtonsExtraAccelerationStatus()
{
    bool result = false;

    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(getApplicableButtons());
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();

        if (button != nullptr)
        {
            bool temp = button->isExtraAccelerationEnabled();

            if (!temp)
            {
                result = false;
                iter.toBack();
            } else
            {
                result = temp;
            }
        }
    }

    return result;
}

void JoyControlStick::setButtonsExtraAccelerationMultiplier(double value)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(getApplicableButtons());
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();

        if (button != nullptr)
            button->setExtraAccelerationMultiplier(value);
    }
}

double JoyControlStick::getButtonsExtraAccelerationMultiplier()
{
    double result = GlobalVariables::JoyButton::DEFAULTEXTRACCELVALUE;
    QHash<JoyStickDirections, JoyControlStickButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return result;

    JoyControlStickButton *button = iter.value();
    if (button != nullptr)
        result = button->getExtraAccelerationMultiplier();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        if (button != nullptr)
        {
            double temp = button->getExtraAccelerationMultiplier();

            if (!qFuzzyCompare(temp, result))
            {
                result = GlobalVariables::JoyButton::DEFAULTEXTRACCELVALUE;
                break;
            }
        }
    }

    return result;
}

void JoyControlStick::setButtonsStartAccelerationMultiplier(double value)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(getApplicableButtons());

    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();

        if (button != nullptr)
            button->setStartAccelMultiplier(value);
    }
}

double JoyControlStick::getButtonsStartAccelerationMultiplier()
{
    double result = GlobalVariables::JoyButton::DEFAULTSTARTACCELMULTIPLIER;
    QHash<JoyStickDirections, JoyControlStickButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return result;

    JoyControlStickButton *button = iter.value();
    if (button != nullptr)
        result = button->getStartAccelMultiplier();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        if (button != nullptr)
        {
            double temp = button->getStartAccelMultiplier();

            if (!qFuzzyCompare(temp, result))
            {
                result = GlobalVariables::JoyButton::DEFAULTSTARTACCELMULTIPLIER;
                break;
            }
        }
    }

    return result;
}

void JoyControlStick::setButtonsMinAccelerationThreshold(double value)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(getApplicableButtons());

    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();

        if (button != nullptr)
            button->setMinAccelThreshold(value);
    }
}

double JoyControlStick::getButtonsMinAccelerationThreshold()
{
    double result = GlobalVariables::JoyButton::DEFAULTMINACCELTHRESHOLD;
    QHash<JoyStickDirections, JoyControlStickButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return result;

    JoyControlStickButton *button = iter.value();
    if (button != nullptr)
        result = button->getMinAccelThreshold();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        if (button != nullptr)
        {
            double temp = button->getMinAccelThreshold();

            if (!qFuzzyCompare(temp, result))
            {
                result = GlobalVariables::JoyButton::DEFAULTMINACCELTHRESHOLD;
                break;
            }
        }
    }

    return result;
}

void JoyControlStick::setButtonsMaxAccelerationThreshold(double value)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(getApplicableButtons());

    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();

        if (button != nullptr)
            button->setMaxAccelThreshold(value);
    }
}

double JoyControlStick::getButtonsMaxAccelerationThreshold()
{
    double result = GlobalVariables::JoyButton::DEFAULTMAXACCELTHRESHOLD;
    QHash<JoyStickDirections, JoyControlStickButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return result;

    JoyControlStickButton *button = iter.value();
    if (button != nullptr)
        result = button->getMaxAccelThreshold();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        if (button != nullptr)
        {
            double temp = button->getMaxAccelThreshold();

            if (!qFuzzyCompare(temp, result))
            {
                result = GlobalVariables::JoyButton::DEFAULTMAXACCELTHRESHOLD;
                break;
            }
        }
    }

    return result;
}

void JoyControlStick::setButtonsAccelerationExtraDuration(double value)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(getApplicableButtons());

    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();

        if (button != nullptr)
            button->setAccelExtraDuration(value);
    }
}

double JoyControlStick::getButtonsAccelerationEasingDuration()
{
    double result = GlobalVariables::JoyButton::DEFAULTACCELEASINGDURATION;
    QHash<JoyStickDirections, JoyControlStickButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return result;

    JoyControlStickButton *button = iter.value();
    if (button != nullptr)
        result = button->getAccelExtraDuration();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        if (button != nullptr)
        {
            double temp = button->getAccelExtraDuration();
            if (!qFuzzyCompare(temp, result))
            {
                result = GlobalVariables::JoyButton::DEFAULTACCELEASINGDURATION;
                break;
            }
        }
    }

    return result;
}

void JoyControlStick::setButtonsSpringDeadCircleMultiplier(int value)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(getApplicableButtons());
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        if (button != nullptr)
        {
            button->setSpringDeadCircleMultiplier(value);
        }
    }
}

int JoyControlStick::getButtonsSpringDeadCircleMultiplier()
{
    int result = GlobalVariables::JoyButton::DEFAULTSPRINGRELEASERADIUS;
    QHash<JoyStickDirections, JoyControlStickButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return result;

    JoyControlStickButton *button = iter.value();
    if (button != nullptr)
        result = button->getSpringDeadCircleMultiplier();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        if (button != nullptr)
        {
            int temp = button->getSpringDeadCircleMultiplier();
            if (temp != result)
            {
                result = GlobalVariables::JoyButton::DEFAULTSPRINGRELEASERADIUS;
                break;
            }
        }
    }

    return result;
}

double JoyControlStick::calculateAccelerationDistance(JoyControlStickButton *button)
{
    double finalDistance = 0.0;

    switch (currentDirection)
    {
    case StickUp: {
        if (axisY->getCurrentRawValue() >= 0)
        {
            finalDistance = 0.0;
        } else
        {
            finalDistance = calculateYAxisDistance(axisY->getCurrentRawValue());
        }

        break;
    }
    case StickRightUp: {
        switch (button->getJoyNumber())
        {
        case 1:

            if (axisY->getCurrentRawValue() >= 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateYAxisDistance(axisY->getCurrentRawValue());
            }

            break;

        case 2:

            if ((axisX->getCurrentRawValue() <= 0) || (axisY->getCurrentRawValue() >= 0))
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateEightWayDiagonalDistance(axisX->getCurrentRawValue(), axisY->getCurrentRawValue());
            }

            break;

        case 3:

            if (axisX->getCurrentRawValue() < 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateXAxisDistance(axisX->getCurrentRawValue());
            }

            break;
        }

        break;
    }
    case StickRight: {
        if (axisX->getCurrentRawValue() < 0)
        {
            finalDistance = 0.0;
        } else
        {
            finalDistance = calculateXAxisDistance(axisX->getCurrentRawValue());
        }

        break;
    }
    case StickRightDown: {
        switch (button->getJoyNumber())
        {
        case 3:

            if (axisX->getCurrentRawValue() < 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateXAxisDistance(axisX->getCurrentRawValue());
            }

            break;

        case 4:

            if ((axisX->getCurrentRawValue() <= 0) || (axisY->getCurrentRawValue() <= 0))
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateEightWayDiagonalDistance(axisX->getCurrentRawValue(), axisY->getCurrentRawValue());
            }

            break;

        case 5:

            if (axisY->getCurrentRawValue() < 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateYAxisDistance(axisY->getCurrentRawValue());
            }

            break;
        }

        break;
    }
    case StickDown: {
        if (axisY->getCurrentRawValue() <= 0)
        {
            finalDistance = 0.0;
        } else
        {
            finalDistance = calculateYAxisDistance(axisY->getCurrentRawValue());
        }

        break;
    }
    case StickLeftDown: {
        switch (button->getJoyNumber())
        {
        case 5:

            if (axisY->getCurrentRawValue() < 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateYAxisDistance(axisY->getCurrentRawValue());
            }

            break;

        case 6:

            if ((axisX->getCurrentRawValue() >= 0) || (axisY->getCurrentRawValue() <= 0))
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateEightWayDiagonalDistance(axisX->getCurrentRawValue(), axisY->getCurrentRawValue());
            }

            break;

        case 7:

            if (axisX->getCurrentRawValue() >= 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateXAxisDistance(axisX->getCurrentRawValue());
            }

            break;
        }

        break;
    }
    case StickLeft: {
        if (axisX->getCurrentRawValue() >= 0)
        {
            finalDistance = 0.0;
        } else
        {
            finalDistance = calculateXAxisDistance(axisX->getCurrentRawValue());
        }

        break;
    }
    case StickLeftUp: {
        switch (button->getJoyNumber())
        {
        case 1:

            if (axisY->getCurrentRawValue() >= 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateYAxisDistance(axisY->getCurrentRawValue());
            }

            break;

        case 7:

            if (axisX->getCurrentRawValue() >= 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateXAxisDistance(axisX->getCurrentRawValue());
            }

            break;

        case 8:

            if ((axisX->getCurrentRawValue() >= 0) || (axisY->getCurrentRawValue() >= 0))
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateEightWayDiagonalDistance(axisX->getCurrentRawValue(), axisY->getCurrentRawValue());
            }

            break;
        }

        break;
    }

    default:
        break;
    }

    return finalDistance;
}

// TODO: Maybe change method name.
double JoyControlStick::calculateLastAccelerationButtonDistance(JoyControlStickButton *button)
{
    double finalDistance = 0.0;

    switch (currentDirection)
    {
    case StickUp: {
        if (axisY->getLastKnownRawValue() >= 0)
        {
            finalDistance = 0.0;
        } else
        {
            finalDistance = calculateYAxisDistance(axisY->getLastKnownRawValue());
        }

        break;
    }
    case StickRightUp: {
        switch (button->getJoyNumber())
        {
        case StickRight: {
            if (axisX->getLastKnownRawValue() <= 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateXAxisDistance(axisX->getLastKnownRawValue());
            }

            break;
        }
        case StickUp: {
            if (axisY->getLastKnownRawValue() >= 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateYAxisDistance(axisY->getLastKnownRawValue());
            }

            break;
        }
        case StickRightUp: {
            if ((axisX->getLastKnownRawValue() <= 0) || (axisY->getLastKnownRawValue() >= 0))
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance =
                    calculateEightWayDiagonalDistance(axisX->getLastKnownRawValue(), axisY->getLastKnownRawValue());
            }

            break;
        }
        }

        break;
    }
    case StickRight: {
        if (axisX->getLastKnownRawValue() <= 0)
        {
            finalDistance = 0.0;
        } else
        {
            finalDistance = calculateXAxisDistance(axisX->getLastKnownRawValue());
        }

        break;
    }
    case StickRightDown: {
        switch (button->getJoyNumber())
        {
        case StickRight: {
            if (axisX->getLastKnownRawValue() <= 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateXAxisDistance(axisX->getLastKnownRawValue());
            }

            break;
        }
        case StickDown: {
            if (axisY->getLastKnownRawValue() <= 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateYAxisDistance(axisY->getLastKnownRawValue());
            }

            break;
        }
        case StickRightDown: {
            if ((axisX->getLastKnownRawValue() <= 0) || (axisY->getLastKnownRawValue() <= 0))
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance =
                    calculateEightWayDiagonalDistance(axisX->getLastKnownRawValue(), axisY->getLastKnownRawValue());
            }

            break;
        }
        }

        break;
    }
    case StickDown: {
        if (axisY->getLastKnownRawValue() <= 0)
        {
            finalDistance = 0.0;
        } else
        {
            finalDistance = calculateYAxisDistance(axisY->getLastKnownRawValue());
        }

        break;
    }
    case StickLeftDown: {
        switch (button->getJoyNumber())
        {
        case 7: {
            if (axisX->getLastKnownRawValue() >= 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateXAxisDistance(axisX->getLastKnownRawValue());
            }

            break;
        }
        case 5: {
            if (axisY->getLastKnownRawValue() <= 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateYAxisDistance(axisY->getLastKnownRawValue());
            }

            break;
        }
        case 6: {
            if ((axisX->getLastKnownRawValue() >= 0) || (axisY->getLastKnownRawValue() <= 0))
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance =
                    calculateEightWayDiagonalDistance(axisX->getLastKnownRawValue(), axisY->getLastKnownRawValue());
            }

            break;
        }
        }

        break;
    }
    case StickLeft: {
        if (axisX->getLastKnownRawValue() >= 0)
        {
            finalDistance = 0.0;
        } else
        {
            finalDistance = calculateXAxisDistance(axisX->getLastKnownRawValue());
        }

        break;
    }
    case StickLeftUp: {
        switch (button->getJoyNumber())
        {
        case 7: {
            if (axisX->getLastKnownRawValue() >= 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateXAxisDistance(axisX->getLastKnownRawValue());
            }

            break;
        }
        case 1: {
            if (axisY->getLastKnownRawValue() >= 0)
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance = calculateYAxisDistance(axisY->getLastKnownRawValue());
            }

            break;
        }
        case 8: {
            if ((axisX->getLastKnownRawValue() >= 0) || (axisY->getLastKnownRawValue() >= 0))
            {
                finalDistance = 0.0;
            } else
            {
                finalDistance =
                    calculateEightWayDiagonalDistance(axisX->getLastKnownRawValue(), axisY->getLastKnownRawValue());
            }

            break;
        }
        }

        break;
    }

    default:
        break;
    }

    return finalDistance;
}

double JoyControlStick::calculateXAxisDistance(int axisXValue)
{
    int axis1Value = axisXValue;
    double distance = axis1Value / static_cast<double>(maxZone);

    if (distance < -1.0)
        distance = -1.0;
    else if (distance > 1.0)
        distance = 1.0;

    qDebug() << "DISTANCE: " << distance;

    return distance;
}

double JoyControlStick::calculateYAxisDistance(int axisYValue)
{
    int axis2Value = axisYValue;
    double distance = axis2Value / static_cast<double>(maxZone);

    if (distance < -1.0)
        distance = -1.0;
    else if (distance > 1.0)
        distance = 1.0;

    return distance;
}

double JoyControlStick::calculateEightWayDiagonalDistanceFromDeadZone()
{
    return calculateEightWayDiagonalDistanceFromDeadZone(axisX->getCurrentRawValue(), axisY->getCurrentRawValue());
}

double JoyControlStick::calculateEightWayDiagonalDistanceFromDeadZone(int axisXValue, int axisYValue)
{
    double distance = 0.0;
    double radius = getDistanceFromDeadZone(axisXValue, axisYValue);
    double bearing = calculateBearing(axisXValue, axisYValue);
    int relativeBearing = static_cast<int>(bearing) % 90;
    int diagonalAngle = relativeBearing;

    if (relativeBearing > 45)
        diagonalAngle = 90 - relativeBearing;

    distance = radius * (diagonalAngle / 45.0);

    return distance;
}

double JoyControlStick::calculateEightWayDiagonalDistance(int axisXValue, int axisYValue)
{
    double distance = 0.0;
    double radius = getRadialDistance(axisXValue, axisYValue);
    double bearing = calculateBearing(axisXValue, axisYValue);
    int relativeBearing = static_cast<int>(bearing) % 90;
    int diagonalAngle = relativeBearing;

    if (relativeBearing > 45)
        diagonalAngle = 90 - relativeBearing;

    distance = radius * (diagonalAngle / 45.0);

    return distance;
}

double JoyControlStick::calculateXDiagonalDeadZone(int axisXValue, int axisYValue)
{
    double diagonalDeadZone = 0.0;
    JoyStickDirections direction = calculateStickDirection(axisXValue, axisYValue);

    if (diagonalRange < 90)
    {
        if ((direction == StickRightUp) || (direction == StickRight))
        {
            QList<double> tempangles = getDiagonalZoneAngles();

            double maxangle = tempangles.at(3);
            double mindeadX = fabs(deadZone * cos(maxangle * GlobalVariables::JoyControlStick::PI / 180.0));
            diagonalDeadZone = mindeadX;
        } else if ((direction == StickRightDown) || (direction == StickDown))
        {
            QList<double> tempangles = getDiagonalZoneAngles();

            double maxangle = tempangles.at(5);
            double mindeadX = fabs(deadZone * cos((maxangle - 90.0) * GlobalVariables::JoyControlStick::PI / 180.0));
            diagonalDeadZone = mindeadX;
        } else if ((direction == StickLeftDown) || (direction == StickLeft))
        {
            QList<double> tempangles = getDiagonalZoneAngles();

            double maxangle = tempangles.at(7);
            double mindeadX = fabs(deadZone * cos((maxangle - 180.0) * GlobalVariables::JoyControlStick::PI / 180.0));
            diagonalDeadZone = mindeadX;
        } else if ((direction == StickLeftUp) || (direction == StickUp))
        {
            QList<double> tempangles = getDiagonalZoneAngles();

            double maxangle = tempangles.at(1);
            double mindeadX = fabs(deadZone * cos((maxangle - 270.0) * GlobalVariables::JoyControlStick::PI / 180.0));
            diagonalDeadZone = mindeadX;
        } else
        {
            diagonalDeadZone = 0.0;
        }
    } else
    {
        diagonalDeadZone = 0.0;
    }

    return diagonalDeadZone;
}

double JoyControlStick::calculateYDiagonalDeadZone(int axisXValue, int axisYValue)
{
    double diagonalDeadZone = 0.0;
    JoyStickDirections direction = calculateStickDirection(axisXValue, axisYValue);

    if (diagonalRange < 90)
    {
        if ((direction == StickRightUp) || (direction == StickUp))
        {
            QList<double> tempangles = getDiagonalZoneAngles();

            double minangle = tempangles.at(1);
            double mindeadY = fabs(deadZone * sin(minangle * GlobalVariables::JoyControlStick::PI / 180.0));
            diagonalDeadZone = mindeadY;
        } else if ((direction == StickRightDown) || (direction == StickRight))
        {
            QList<double> tempfuck = getDiagonalZoneAngles();

            double minangle = tempfuck.at(4);
            double mindeadY = fabs(deadZone * sin((minangle - 90.0) * GlobalVariables::JoyControlStick::PI / 180.0));
            diagonalDeadZone = mindeadY;
        } else if ((direction == StickLeftDown) || (direction == StickDown))
        {
            QList<double> tempangles = getDiagonalZoneAngles();

            double minangle = tempangles.at(6);
            double mindeadY = fabs(deadZone * sin((minangle - 180.0) * GlobalVariables::JoyControlStick::PI / 180.0));
            diagonalDeadZone = mindeadY;
        } else if ((direction == StickLeftUp) || (direction == StickLeft))
        {
            QList<double> tempangles = getDiagonalZoneAngles();

            double minangle = tempangles.at(8);
            double mindeadY = fabs(deadZone * sin((minangle - 270.0) * GlobalVariables::JoyControlStick::PI / 180.0));
            diagonalDeadZone = mindeadY;
        } else
        {
            diagonalDeadZone = 0.0;
        }
    } else
    {
        diagonalDeadZone = 0.0;
    }

    return diagonalDeadZone;
}

double JoyControlStick::getSpringDeadCircleX()
{
    double result = 0.0;
    double angle2 = 0.0;
    int axis1Value = 0;
    int axis2Value = 0;

    if ((axisX->getCurrentRawValue() == 0) && (axisY->getCurrentRawValue() == 0))
    {
        // Stick moved back to absolute center. Use previously available values
        // to find stick angle.
        angle2 = atan2(axisX->getLastKnownRawValue(), -axisY->getLastKnownRawValue());
        axis1Value = axisX->getLastKnownRawValue();
        axis2Value = axisY->getLastKnownRawValue();
    } else
    {
        // Use current axis values to find stick angle.
        angle2 = atan2(axisX->getCurrentRawValue(), -axisY->getCurrentRawValue());
        axis1Value = axisX->getCurrentRawValue();
        axis2Value = axisY->getCurrentRawValue();
    }

    double ang_sin = sin(angle2);
    double ang_cos = cos(angle2);

    int deadX = abs(floor(deadZone * ang_sin + 0.5));
    double diagonalDeadX = calculateXDiagonalDeadZone(axis1Value, axis2Value);

    double squareStickFullPhi = qMin(ang_sin != 0.0 ? 1 / fabs(ang_sin) : 2, ang_cos != 0.0 ? 1 / fabs(ang_cos) : 2);
    double circle = this->circle;
    double circleStickFull = (squareStickFullPhi - 1) * circle + 1;

    double adjustedDeadXZone = circleStickFull > 1.0 ? (deadX / circleStickFull) : deadX;
    double finalDeadZoneX = adjustedDeadXZone - diagonalDeadX;
    double maxRange = static_cast<double>(deadZone) - diagonalDeadX;

    if (maxRange != 0.0)
        result = finalDeadZoneX / maxRange;

    return result;
}

double JoyControlStick::getSpringDeadCircleY()
{
    double result = 0.0;
    double angle2 = 0.0;
    int axis1Value = 0;
    int axis2Value = 0;

    if ((axisX->getCurrentRawValue() == 0) && (axisY->getCurrentRawValue() == 0))
    {
        // Stick moved back to absolute center. Use previously available values
        // to find stick angle.
        angle2 = atan2(axisX->getLastKnownRawValue(), -axisY->getLastKnownRawValue());
        axis1Value = axisX->getLastKnownRawValue();
        axis2Value = axisY->getLastKnownRawValue();
    } else
    {
        // Use current axis values to find stick angle.
        angle2 = atan2(axisX->getCurrentRawValue(), -axisY->getCurrentRawValue());
        axis1Value = axisX->getCurrentRawValue();
        axis2Value = axisY->getCurrentRawValue();
    }

    double ang_sin = sin(angle2);
    double ang_cos = cos(angle2);

    int deadY = abs(floor(deadZone * ang_cos + 0.5));
    double diagonalDeadY = calculateYDiagonalDeadZone(axis1Value, axis2Value);

    double squareStickFullPhi = qMin(ang_sin != 0.0 ? 1 / fabs(ang_sin) : 2, ang_cos != 0.0 ? 1 / fabs(ang_cos) : 2);
    double circle = this->circle;
    double circleStickFull = (squareStickFullPhi - 1) * circle + 1;

    double adjustedDeadYZone = (circleStickFull > 1.0) ? (deadY / circleStickFull) : deadY;
    double finalDeadZoneY = adjustedDeadYZone - diagonalDeadY;
    double maxRange = static_cast<double>(deadZone) - diagonalDeadY;

    if (maxRange != 0.0)
        result = finalDeadZoneY / maxRange;

    return result;
}

void JoyControlStick::setButtonsExtraAccelCurve(JoyButton::JoyExtraAccelerationCurve curve)
{
    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(getApplicableButtons());
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        if (button != nullptr)
        {
            button->setExtraAccelerationCurve(curve);
        }
    }
}

JoyButton::JoyExtraAccelerationCurve JoyControlStick::getButtonsExtraAccelerationCurve()
{
    JoyButton::JoyExtraAccelerationCurve result = JoyButton::LinearAccelCurve;
    QHash<JoyStickDirections, JoyControlStickButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return result;

    JoyControlStickButton *button = iter.value();
    if (button != nullptr)
        result = button->getExtraAccelerationCurve();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        if (button != nullptr)
        {
            JoyButton::JoyExtraAccelerationCurve temp = button->getExtraAccelerationCurve();
            if (temp != result)
            {
                result = JoyButton::LinearAccelCurve;
                break;
            }
        }
    }

    return result;
}

void JoyControlStick::setDirButtonsUpdateInitAccel(JoyControlStick::JoyStickDirections direction, bool state)
{
    QHash<JoyStickDirections, JoyControlStickButton *> apphash = getButtonsForDirection(direction);

    // QVERIFY(apphash.isEmpty() == false);

    QHashIterator<JoyStickDirections, JoyControlStickButton *> iter(apphash);

    // QVERIFY(iter.hasNext() == true);

    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        button->setUpdateInitAccel(state);
    }
}

QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton *>
JoyControlStick::getButtonsForDirection(JoyControlStick::JoyStickDirections direction)
{
    QHash<JoyStickDirections, JoyControlStickButton *> temphash;

    switch (currentMode)
    {
    case StandardMode: {
        if (direction & JoyControlStick::StickUp)
        {
            JoyControlStickButton *button = this->buttons.value(JoyControlStick::StickUp);
            temphash.insert(JoyControlStick::StickUp, button);
        }

        if (direction & JoyControlStick::StickRight)
        {
            JoyControlStickButton *button = this->buttons.value(JoyControlStick::StickRight);
            temphash.insert(JoyControlStick::StickRight, button);
        }

        if (direction & JoyControlStick::StickDown)
        {
            JoyControlStickButton *button = this->buttons.value(JoyControlStick::StickDown);
            temphash.insert(JoyControlStick::StickDown, button);
        }

        if (direction & JoyControlStick::StickLeft)
        {
            JoyControlStickButton *button = this->buttons.value(JoyControlStick::StickLeft);
            temphash.insert(JoyControlStick::StickLeft, button);
        }

        break;
    }
    case EightWayMode: {
        if ((direction == JoyControlStick::StickUp) || (direction == JoyControlStick::StickDown) ||
            (direction == JoyControlStick::StickLeft) || (direction == JoyControlStick::StickRight) ||
            (direction == JoyControlStick::StickRightUp) || (direction == JoyControlStick::StickRightDown) ||
            (direction == JoyControlStick::StickLeftDown) || (direction == JoyControlStick::StickLeftUp))
        {
            temphash.insert(direction, buttons.value(direction));
        }

        break;
    }
    case FourWayCardinal: {
        if ((direction == JoyControlStick::StickUp) || (direction == JoyControlStick::StickDown) ||
            (direction == JoyControlStick::StickLeft) || (direction == JoyControlStick::StickRight))
        {
            temphash.insert(direction, buttons.value(direction));
        }

        break;
    }
    case FourWayDiagonal: {
        if ((direction == JoyControlStick::StickRightUp) || (direction == JoyControlStick::StickRightDown) ||
            (direction == JoyControlStick::StickLeftDown) || (direction == JoyControlStick::StickLeftUp))
        {
            temphash.insert(direction, buttons.value(direction));
        }

        break;
    }
    }

    return temphash;
}
