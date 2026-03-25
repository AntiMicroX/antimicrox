/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2022 Max Maisel <max.maisel@posteo.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "joygyroscopesensor.h"
#include "joysensordirection.h"

#include <QtTest/QtTest>
#include <cmath>

/**
 * Tests for gyro deadzone calculations.
 *
 * Covers two bugs related to gyro drift:
 * 1. calculateX/Y/ZDistanceFromDeadZone() could return negative values when
 *    one axis is small but within the deadzone sphere, causing spurious mouse
 *    movement in the opposite direction (asymmetric drift).
 * 2. calculateSensorDirection() used strict less-than (distance < deadzone)
 *    so a sensor reading exactly at the deadzone boundary was not treated as
 *    centered. Fixed to use less-than-or-equal (distance <= deadzone).
 */
class TestJoySensorDead : public QObject
{
    Q_OBJECT

  public:
    TestJoySensorDead(QObject *parent = nullptr);
    ~TestJoySensorDead();

  private slots:
    // Per-axis distance functions must never return a negative value
    void xDistanceNeverNegative();
    void yDistanceNeverNegative();
    void zDistanceNeverNegative();

    // Distance functions return 0 when the sensor is inside the dead zone
    void xDistanceZeroInsideDeadZone();
    void yDistanceZeroInsideDeadZone();
    void zDistanceZeroInsideDeadZone();

    // calculateSensorDirection returns SENSOR_CENTERED at exactly the deadzone boundary
    void directionCenteredAtDeadZoneBoundary();

    // calculateSensorDirection returns SENSOR_CENTERED strictly inside the deadzone
    void directionCenteredInsideDeadZone();

  private:
    // Sensor with default deadzone (~20°)
    JoyGyroscopeSensor *m_sensor;
};

TestJoySensorDead::TestJoySensorDead(QObject *parent)
    : QObject(parent)
    , m_sensor(new JoyGyroscopeSensor(0, nullptr, this))
{
}

TestJoySensorDead::~TestJoySensorDead()
{
    delete m_sensor;
}

/**
 * Regression test for: calculateXDistanceFromDeadZone() returned a negative
 * value when abs(x) < sqrt(dead_zone^2 - y^2 - z^2), causing the sensor to
 * produce spurious mouse movement when it should have been suppressed.
 *
 * Scenario: x is small (0.1 rad/s), y is non-zero (0.3 rad/s), z = 0.
 * With the default 20° (~0.349 rad) deadzone the discriminant is positive
 * and sqrt(discriminant) > abs(x), so the old code returned a negative value.
 */
void TestJoySensorDead::xDistanceNeverNegative()
{
    // x small, y large enough to make discriminant positive
    double result = m_sensor->calculateXDistanceFromDeadZone(0.1, 0.3, 0.0);
    QVERIFY2(result >= 0.0, "calculateXDistanceFromDeadZone returned a negative value");
}

/**
 * Regression test for the same bug in the Y axis variant.
 */
void TestJoySensorDead::yDistanceNeverNegative()
{
    double result = m_sensor->calculateYDistanceFromDeadZone(0.3, 0.1, 0.0);
    QVERIFY2(result >= 0.0, "calculateYDistanceFromDeadZone returned a negative value");
}

/**
 * Regression test for the same bug in the Z axis variant.
 */
void TestJoySensorDead::zDistanceNeverNegative()
{
    double result = m_sensor->calculateZDistanceFromDeadZone(0.3, 0.0, 0.1);
    QVERIFY2(result >= 0.0, "calculateZDistanceFromDeadZone returned a negative value");
}

/**
 * When the sensor vector is fully inside the dead zone sphere, all per-axis
 * distance functions must return 0.
 */
void TestJoySensorDead::xDistanceZeroInsideDeadZone()
{
    // All values very small — well inside the 20° deadzone
    double result = m_sensor->calculateXDistanceFromDeadZone(0.01, 0.01, 0.01);
    QCOMPARE(result, 0.0);
}

void TestJoySensorDead::yDistanceZeroInsideDeadZone()
{
    double result = m_sensor->calculateYDistanceFromDeadZone(0.01, 0.01, 0.01);
    QCOMPARE(result, 0.0);
}

void TestJoySensorDead::zDistanceZeroInsideDeadZone()
{
    double result = m_sensor->calculateZDistanceFromDeadZone(0.01, 0.01, 0.01);
    QCOMPARE(result, 0.0);
}

/**
 * Regression test for the boundary condition bug:
 * The old code used (distance < m_dead_zone), so a reading whose 3-D magnitude
 * is exactly equal to the deadzone was NOT treated as centered — it fell through
 * to the direction-detection logic and could trigger movement.
 * The fix changes it to (distance <= m_dead_zone).
 *
 * We set a sensor value whose magnitude equals the default deadzone exactly and
 * verify that calculateSensorDirection() returns SENSOR_CENTERED.
 */
void TestJoySensorDead::directionCenteredAtDeadZoneBoundary()
{
    double deadZoneDeg = m_sensor->getDeadZone(); // degrees
    double deadZoneRad = JoySensor::degToRad(deadZoneDeg);

    // Place all energy on the X axis so distance == deadZoneRad exactly
    float values[3] = {static_cast<float>(deadZoneRad), 0.0f, 0.0f};

    // joyEvent writes m_current_value and calls calculateSensorDirection internally.
    // We call it with ignoresets=true so no desk events are fired.
    m_sensor->joyEvent(values, true);

    QCOMPARE(m_sensor->getCurrentDirection(), JoySensorDirection::SENSOR_CENTERED);
}

/**
 * Basic sanity check: a sensor vector strictly inside the dead zone must
 * always produce SENSOR_CENTERED.
 */
void TestJoySensorDead::directionCenteredInsideDeadZone()
{
    // Very small reading — far inside deadzone
    float values[3] = {0.001f, 0.001f, 0.001f};
    m_sensor->joyEvent(values, true);
    QCOMPARE(m_sensor->getCurrentDirection(), JoySensorDirection::SENSOR_CENTERED);
}

QTEST_MAIN(TestJoySensorDead)
#include "testjoysensordead.moc"
