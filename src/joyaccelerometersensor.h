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

#include "joysensor.h"

class SetJoystick;

/**
 * @brief Represents an accelerometer sensor.
 */
class JoyAccelerometerSensor : public JoySensor
{
  public:
    explicit JoyAccelerometerSensor(double rate, int originset, SetJoystick *parent_set, QObject *parent);
    virtual ~JoyAccelerometerSensor();

    virtual float getXCoordinate() const override;
    virtual float getYCoordinate() const override;
    virtual float getZCoordinate() const override;
    virtual QString sensorTypeName() const override;

    virtual void getCalibration(double *offsetX, double *offsetY, double *offsetZ) const override;
    virtual void setCalibration(double offsetX, double offsetY, double offsetZ) override;

  public slots:
    virtual void reset() override;

  protected:
    static const double SHOCK_DETECT_THRESHOLD;
    static const double SHOCK_SUPPRESS_FACTOR;
    static const double SHOCK_TAU;

    virtual void populateButtons() override;
    virtual JoySensorDirection calculateSensorDirection() override;
    virtual void applyCalibration() override;

    double m_rate;
    PT1Filter m_shock_filter;
    size_t m_shock_suppress_count;
    double m_calibration_matrix[3][3];
};
