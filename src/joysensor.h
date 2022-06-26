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

#include <QHash>
#include <QObject>
#include <QTimer>

#include "joysensordirection.h"
#include "joysensortype.h"
#include "pt1filter.h"

class SetJoystick;
class JoySensorButton;
class QXmlStreamReader;
class QXmlStreamWriter;

/**
 * @brief Represents one sensor in a SetJoystick and its connections to
 *  other parts of the application.
 *  Receives hardware input events from InputDaemon, processes them and
 *  generates GUI as well as Mouse+Keyboard events.
 */
class JoySensor : public QObject
{
    Q_OBJECT

  public:
    explicit JoySensor(JoySensorType type, int originset, SetJoystick *parent_set, QObject *parent);
    virtual ~JoySensor();

    void joyEvent(float *values, bool ignoresets = false);
    void queuePendingEvent(float *values, bool ignoresets = false);
    void activatePendingEvent();
    bool hasPendingEvent() const;
    void clearPendingEvent();

    void copyAssignments(JoySensor *dest_sensor);
    bool hasSlotsAssigned() const;

    QString getPartialName(bool forceFullFormat = false, bool displayNames = false) const;
    QString getSensorName() const;

    JoySensorType getType() const;
    JoySensorDirection getCurrentDirection() const;
    double getDeadZone() const;
    double getDiagonalRange() const;
    double getMaxZone() const;
    unsigned int getSensorDelay() const;
    virtual float getXCoordinate() const = 0;
    virtual float getYCoordinate() const = 0;
    virtual float getZCoordinate() const = 0;
    virtual QString sensorTypeName() const = 0;

    bool inDeadZone(float *values) const;
    double getDistanceFromDeadZone() const;
    double getDistanceFromDeadZone(double x, double y, double z) const;
    double calculateXDistanceFromDeadZone() const;
    double calculateXDistanceFromDeadZone(double x, double y, double z) const;
    double calculateYDistanceFromDeadZone() const;
    double calculateYDistanceFromDeadZone(double x, double y, double z) const;
    double calculateZDistanceFromDeadZone() const;
    double calculateZDistanceFromDeadZone(double x, double y, double z) const;
    double calculateDistance() const;
    double calculateDistance(double x, double y, double z) const;
    double calculatePitch() const;
    double calculatePitch(double x, double y, double z) const;
    double calculateRoll() const;
    double calculateRoll(double x, double y, double z) const;
    double calculateDirectionalDistance(JoySensorDirection direction) const;

    static double radToDeg(double value);
    static double degToRad(double value);

    bool isCalibrated() const;
    void resetCalibration();
    virtual void getCalibration(double *offsetX, double *offsetY, double *offsetZ) const = 0;
    virtual void setCalibration(double offsetX, double offsetY, double offsetZ) = 0;

    QHash<JoySensorDirection, JoySensorButton *> *getButtons();
    JoySensorButton *getDirectionButton(JoySensorDirection direction);

    bool isDefault() const;
    void readConfig(QXmlStreamReader *xml);
    void writeConfig(QXmlStreamWriter *xml) const;

    SetJoystick *getParentSet() const;

  signals:
    void moved(float xaxis, float yaxis, float zaxis);
    void active(float xaxis, float yaxis, float zaxis);
    void released(float xaxis, float yaxis, float zaxis);
    void deadZoneChanged(double value);
    void diagonalRangeChanged(double value);
    void maxZoneChanged(double value);
    void sensorDelayChanged(int value);
    void sensorNameChanged();
    void propertyUpdated();

  public slots:
    virtual void reset();
    void setDeadZone(double value);
    void setMaxZone(double value);
    void setDiagonalRange(double value);
    void setSensorDelay(unsigned int value);
    void setSensorName(QString tempName);
    void establishPropertyUpdatedConnection();

  private slots:
    void delayTimerExpired();

  protected:
    void resetButtons();
    virtual void populateButtons() = 0;
    virtual JoySensorDirection calculateSensorDirection() = 0;
    virtual void applyCalibration() = 0;
    void determineSensorEvent(JoySensorButton **eventbutton) const;
    void createDeskEvent(JoySensorDirection direction, bool ignoresets = false);

    JoySensorType m_type;
    double m_dead_zone;
    double m_diagonal_range;
    double m_max_zone;
    unsigned int m_sensor_delay;

    bool m_active;
    static const size_t ACTIVE_BUTTON_COUNT = 3;
    JoySensorButton *m_active_button[ACTIVE_BUTTON_COUNT];

    float m_current_value[3];
    float m_pending_value[3];
    bool m_calibrated;
    double m_calibration_value[3];
    bool m_pending_event;
    bool m_pending_ignore_sets;

    int m_originset;
    QString m_sensor_name;
    QTimer m_delay_timer;

    JoySensorDirection m_current_direction;
    SetJoystick *m_parent_set;
    QHash<JoySensorDirection, JoySensorButton *> m_buttons;
};
