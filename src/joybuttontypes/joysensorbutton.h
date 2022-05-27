/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2022 Max Maisel <max.maisel@posteo.de>
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
#pragma once

#include "joybuttontypes/joygradientbutton.h"
#include "joysensordirection.h"
#include "joysensortype.h"
#include "logger.h"

class SetJoystick;
class JoySensor;

/**
 * @brief Represents a sensor mapping in a SetJoystick
 */
class JoySensorButton : public JoyGradientButton
{
    Q_OBJECT

  public:
    explicit JoySensorButton(JoySensor *sensor, int index, int originset, SetJoystick *parentSet, QObject *parent);

    virtual int getRealJoyNumber() const override;
    virtual QString getPartialName(bool forceFullFormat = false, bool displayNames = false) const override;
    virtual QString getXmlName() override;

    virtual double getDistanceFromDeadZone() override;
    virtual double getMouseDistanceFromDeadZone() override;

    virtual void setChangeSetCondition(SetChangeCondition condition, bool passive = false,
                                       bool updateActiveString = true) override;
    virtual bool isPartRealAxis() override;
    virtual JoyMouseCurve getDefaultMouseCurve() const override;

    JoySensor *getSensor() const;
    virtual QString getDirectionName() const = 0;
    JoySensorDirection getDirection() const;

  signals:
    void setAssignmentChanged(JoySensorDirection direction, JoySensorType type, int associated_set, int mode);

  private:
    JoySensor *m_sensor;
};
