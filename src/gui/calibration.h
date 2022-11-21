/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail.com>
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

#include "pt1filter.h"
#include "statisticsestimator.h"

#include <QDateTime>
#include <QDialog>
#include <QElapsedTimer>

class JoyControlStick;
class JoySensor;
class InputDevice;

namespace Ui {
class Calibration;
}

/**
 * @brief Opens calibration window used for calibration
 *
 * @details Class used for calibration of joysticks, gyroscopes etc.
 */
class Calibration : public QDialog
{
    Q_OBJECT

  public:
    enum CalibrationType
    {
        CAL_NONE,
        CAL_STICK,
        CAL_ACCELEROMETER,
        CAL_GYROSCOPE,

        CAL_TYPE_MASK = 0x0000FFFF,
        CAL_INDEX_MASK = 0xFFFF0000,
        CAL_INDEX_POS = 16
    };

    explicit Calibration(InputDevice *joystick, QDialog *parent = 0);
    ~Calibration();

  protected:
    void resetCalibrationValues();
    bool askConfirmation(QString message, bool confirmed);
    void showSensorCalibrationValues(bool offsetXvalid, double offsetX, bool offsetYvalid, double offsetY, bool offsetZvalid,
                                     double offsetZ);
    void showStickCalibrationValues(bool offsetXvalid, double offsetX, bool gainXvalid, double gainX, bool offsetYvalid,
                                    double offsetY, bool gainYvalid, double gainY);
    void hideCalibrationData();
    void selectTypeIndex(unsigned int type_index);
    static void stickRegression(double *offset, double *gain, double xoffset, double xmin, double xmax);

  private:
    Ui::Calibration *m_ui;
    CalibrationType m_type;
    unsigned int m_index;
    bool m_calibrated;
    bool m_changed;
    JoyControlStick *m_stick;
    JoySensor *m_sensor;
    InputDevice *m_joystick;

    StatisticsEstimator m_offset[3];
    StatisticsEstimator m_min[2];
    StatisticsEstimator m_max[2];
    PT1Filter m_filter[2];
    double m_last_slope[2];
    QDateTime m_end_time;
    QElapsedTimer m_rate_timer;
    int m_sample_count;
    int m_phase;

    static const int CAL_MIN_SAMPLES;
    static const double CAL_ACCURACY_SQ;
    static const double STICK_CAL_TAU;
    static const int STICK_RATE_SAMPLES;
    static const int CAL_TIMEOUT;

  public slots:
    void saveSettings();
    void startAccelerometerCalibration();
    void startAccelerometerAngleCalibration();
    void startGyroscopeCalibration();
    void startGyroscopeOffsetCalibration();
    void startStickOffsetCalibration();
    void startStickGainCalibration();

  protected slots:
    void closeEvent(QCloseEvent *event) override;
    void resetSettings();
    void deviceSelectionChanged(int index);
    void onSensorOffsetData(float x, float y, float z);
    void onStickOffsetData(int x, int y);
    void onStickGainData(int x, int y);

  signals:
    void propertyUpdated();
};
