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

#include "calibration.h"
#include "ui_calibration.h"

#include "globalvariables.h"
#include "inputdevice.h"
#include "joycontrolstick.h"
#include "joysensor.h"

#include <QCloseEvent>
#include <QDebug>
#include <QMessageBox>

const int Calibration::CAL_MIN_SAMPLES = 10;
// Use squared accuracy to avoid root calculation. 1e-4 corresponds to an accuracy of 1%.
const double Calibration::CAL_ACCURACY_SQ = 1e-4;
const double Calibration::STICK_CAL_TAU = 0.045;
const int Calibration::STICK_RATE_SAMPLES = 100;
const int Calibration::CAL_TIMEOUT = 30;

Calibration::Calibration(InputDevice *joystick, QDialog *parent)
    : QDialog(parent)
    , m_ui(new Ui::Calibration)
    , m_type(CAL_NONE)
    , m_calibrated(false)
    , m_changed(false)
    , m_joystick(joystick)
{
    m_ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowTitle(tr("Calibration"));
    hideCalibrationData();

    int device_count = 0;

    QMap<QString, int> dropdown_sticks;
    QHash<int, JoyControlStick *> sticks = m_joystick->getActiveSetJoystick()->getSticks();
    for (auto iter = sticks.cbegin(); iter != sticks.cend(); ++iter)
    {
        dropdown_sticks.insert(iter.value()->getPartialName(), CAL_STICK | (iter.key() << CAL_INDEX_POS));
    }

    for (auto iter = dropdown_sticks.cbegin(); iter != dropdown_sticks.cend(); ++iter)
    {
        m_ui->deviceComboBox->addItem(iter.key(), QVariant(int(iter.value())));
        ++device_count;
    }

    if (m_joystick->getActiveSetJoystick()->hasSensor(GYROSCOPE))
    {
        m_ui->deviceComboBox->addItem(tr("Gyroscope"), QVariant(int(CAL_GYROSCOPE)));
        ++device_count;
    }

    if (m_joystick->getActiveSetJoystick()->hasSensor(ACCELEROMETER))
    {
        m_ui->deviceComboBox->addItem(tr("Accelerometer"), QVariant(int(CAL_ACCELEROMETER)));
        ++device_count;
    }

    connect(m_joystick, &InputDevice::destroyed, this, &Calibration::close);
    connect(m_ui->resetBtn, &QPushButton::clicked, this, &Calibration::resetSettings);
    connect(m_ui->saveBtn, &QPushButton::clicked, this, &Calibration::saveSettings);
    connect(m_ui->cancelBtn, &QPushButton::clicked, this, &Calibration::close);
    connect(m_ui->deviceComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &Calibration::deviceSelectionChanged);

    if (device_count == 0)
    {
        m_ui->steps->setText(tr("Selected device doesn't have any inputs to calibrate."));
    } else
    {
        int index = m_ui->deviceComboBox->currentIndex();
        unsigned int data = m_ui->deviceComboBox->itemData(index).toInt();
        selectTypeIndex(data);
    }

    update();
}

Calibration::~Calibration() { delete m_ui; }

/**
 * @brief Ask for confirmation when the dialog is closed with unsafed changed.
 */
void Calibration::closeEvent(QCloseEvent *event)
{
    event->ignore();
    if (m_changed)
    {
        if (askConfirmation(tr("Calibration was not saved for the preset. Do you really want to continue?"), false))
        {
            event->accept();
        }
    } else
    {
        event->accept();
    }
}

/**
 * @brief Asks for confirmation and resets calibration values of the selected device
 *  afterwards.
 */
void Calibration::resetSettings()
{
    QMessageBox msgBox;
    msgBox.setText(tr("Do you really want to reset calibration of current device?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

    switch (msgBox.exec())
    {
    case QMessageBox::Yes:
        resetCalibrationValues();
        m_ui->steps->clear();
        break;

    case QMessageBox::No:
        break;

    default:
        break;
    }
}

/**
 * @brief Shows the sensor offset calibration values to the user.
 */
void Calibration::showSensorCalibrationValues(bool offsetXvalid, double offsetX, bool offsetYvalid, double offsetY,
                                              bool offsetZvalid, double offsetZ)
{
    QPalette paletteBlack = m_ui->offsetXValue->palette();
    paletteBlack.setColor(m_ui->offsetXValue->foregroundRole(), Qt::black);
    QPalette paletteRed = m_ui->offsetXValue->palette();
    paletteRed.setColor(m_ui->offsetXValue->foregroundRole(), Qt::red);

    m_ui->offsetXValue->setPalette(offsetXvalid ? paletteBlack : paletteRed);
    m_ui->offsetYValue->setPalette(offsetYvalid ? paletteBlack : paletteRed);
    m_ui->offsetZValue->setPalette(offsetZvalid ? paletteBlack : paletteRed);

    if (m_type == CAL_ACCELEROMETER)
    {
        m_ui->offsetXValue->setText(QString::number(offsetX));
        m_ui->offsetYValue->setText(QString::number(offsetY));
        m_ui->offsetZValue->setText(QString::number(offsetZ));
    } else if (m_type == CAL_GYROSCOPE)
    {
        m_ui->offsetXValue->setText(QString::number(JoySensor::radToDeg(offsetX)));
        m_ui->offsetYValue->setText(QString::number(JoySensor::radToDeg(offsetY)));
        m_ui->offsetZValue->setText(QString::number(JoySensor::radToDeg(offsetZ)));
    }
}

/**
 * @brief Shows the stick offset and gain calibration values to the user.
 */
void Calibration::showStickCalibrationValues(bool offsetXvalid, double offsetX, bool gainXvalid, double gainX,
                                             bool offsetYvalid, double offsetY, bool gainYvalid, double gainY)
{
    QPalette paletteBlack = m_ui->offsetXValue->palette();
    paletteBlack.setColor(m_ui->offsetXValue->foregroundRole(), Qt::black);
    QPalette paletteRed = m_ui->offsetXValue->palette();
    paletteRed.setColor(m_ui->offsetXValue->foregroundRole(), Qt::red);

    m_ui->offsetXValue->setPalette(offsetXvalid ? paletteBlack : paletteRed);
    m_ui->gainXValue->setPalette(gainXvalid ? paletteBlack : paletteRed);
    m_ui->offsetYValue->setPalette(offsetYvalid ? paletteBlack : paletteRed);
    m_ui->gainYValue->setPalette(gainYvalid ? paletteBlack : paletteRed);

    m_ui->offsetXValue->setText(QString::number(offsetX));
    m_ui->gainXValue->setText(QString::number(gainX));
    m_ui->offsetYValue->setText(QString::number(offsetY));
    m_ui->gainYValue->setText(QString::number(gainY));
}

/**
 * @brief hides all calibration values and their labels.
 */
void Calibration::hideCalibrationData()
{
    m_ui->xAxisLabel->setVisible(false);
    m_ui->yAxisLabel->setVisible(false);
    m_ui->zAxisLabel->setVisible(false);
    m_ui->offsetXLabel->setVisible(false);
    m_ui->offsetYLabel->setVisible(false);
    m_ui->offsetZLabel->setVisible(false);
    m_ui->offsetXValue->setVisible(false);
    m_ui->offsetYValue->setVisible(false);
    m_ui->offsetZValue->setVisible(false);
    m_ui->gainXLabel->setVisible(false);
    m_ui->gainYLabel->setVisible(false);
    m_ui->gainZLabel->setVisible(false);
    m_ui->gainXValue->setVisible(false);
    m_ui->gainYValue->setVisible(false);
    m_ui->gainZValue->setVisible(false);
    m_ui->steps->clear();
}

/**
 * @brief Prepares calibration for the selected device type.
 *  Show all used values and labels and connect buttons to the corresponding
 *  event handlers.
 */
void Calibration::selectTypeIndex(unsigned int type_index)
{
    CalibrationType type = static_cast<CalibrationType>(type_index & CAL_TYPE_MASK);
    unsigned int index = (type_index & CAL_INDEX_MASK) >> CAL_INDEX_POS;

    if (m_type == type && m_index == index)
        return;

    disconnect(m_ui->startBtn, &QPushButton::clicked, this, nullptr);
    m_type = type;
    m_index = index;
    m_changed = false;
    hideCalibrationData();

    if (m_type == CAL_STICK)
    {
        m_ui->statusStack->setCurrentIndex(1);
        m_stick = m_joystick->getActiveSetJoystick()->getSticks().value(m_index);
        m_calibrated = m_stick->isCalibrated();

        if (m_calibrated)
        {
            double offsetX, gainX, offsetY, gainY;
            m_stick->getCalibration(&offsetX, &gainX, &offsetY, &gainY);
            showStickCalibrationValues(true, offsetX, true, gainX, true, offsetY, true, gainY);
        } else
        {
            showStickCalibrationValues(false, 0.0, false, 1.0, false, 0.0, false, 1.0);
        }

        m_ui->xAxisLabel->setVisible(true);
        m_ui->yAxisLabel->setVisible(true);
        m_ui->offsetXLabel->setVisible(true);
        m_ui->offsetYLabel->setVisible(true);
        m_ui->offsetXValue->setVisible(true);
        m_ui->offsetYValue->setVisible(true);
        m_ui->gainXLabel->setVisible(true);
        m_ui->gainYLabel->setVisible(true);
        m_ui->gainXValue->setVisible(true);
        m_ui->gainYValue->setVisible(true);

        m_ui->resetBtn->setEnabled(m_calibrated);
        m_ui->saveBtn->setEnabled(false);

        m_ui->stickStatusBoxWidget->setFocus();
        m_ui->stickStatusBoxWidget->setStick(m_stick);
        m_ui->stickStatusBoxWidget->update();

        connect(m_ui->startBtn, &QPushButton::clicked, this, &Calibration::startStickOffsetCalibration);
        m_ui->startBtn->setEnabled(true);
        m_ui->resetBtn->setEnabled(true);
    } else if (m_type == CAL_ACCELEROMETER || m_type == CAL_GYROSCOPE)
    {
        if (m_type == CAL_ACCELEROMETER)
        {
            m_sensor = m_joystick->getActiveSetJoystick()->getSensor(ACCELEROMETER);
            m_ui->steps->setText(tr("Accelerometer calibration sets the neutral controller orientation."));
            connect(m_ui->startBtn, &QPushButton::clicked, this, &Calibration::startAccelerometerCalibration);
        } else if (m_type == CAL_GYROSCOPE)
        {
            m_sensor = m_joystick->getActiveSetJoystick()->getSensor(GYROSCOPE);
            m_ui->steps->setText(tr("Gyroscope calibration corrects the sensor offset. "
                                    "This prevents cursor movement while the controller is at rest."));
            connect(m_ui->startBtn, &QPushButton::clicked, this, &Calibration::startGyroscopeCalibration);
        }

        m_ui->statusStack->setCurrentIndex(0);
        m_calibrated = m_sensor->isCalibrated();

        if (m_calibrated)
        {
            double offsetX, offsetY, offsetZ;
            m_sensor->getCalibration(&offsetX, &offsetY, &offsetZ);
            showSensorCalibrationValues(true, offsetX, true, offsetY, true, offsetZ);
        } else
        {
            showSensorCalibrationValues(false, 0.0, false, 0.0, false, 0.0);
        }

        m_ui->xAxisLabel->setVisible(true);
        m_ui->yAxisLabel->setVisible(true);
        m_ui->zAxisLabel->setVisible(true);
        m_ui->offsetXLabel->setVisible(true);
        m_ui->offsetYLabel->setVisible(true);
        m_ui->offsetZLabel->setVisible(true);
        m_ui->offsetXValue->setVisible(true);
        m_ui->offsetYValue->setVisible(true);
        m_ui->offsetZValue->setVisible(true);

        m_ui->resetBtn->setEnabled(m_calibrated);
        m_ui->saveBtn->setEnabled(false);

        m_ui->sensorStatusBoxWidget->setFocus();
        m_ui->sensorStatusBoxWidget->setSensor(m_sensor);
        m_ui->sensorStatusBoxWidget->update();

        m_ui->startBtn->setEnabled(true);
        m_ui->resetBtn->setEnabled(true);
    }
}

/**
 * @brief Performs linear regression on the measurement values of one axis to
 *  determine offset and gain.
 * @param[out] offset The calculated offset.
 * @param[out] gain The calculated gain.
 * @param[in] xoffset The measured X value at the point (x, 0)
 * @param[in] xmin The measured X value at the point (x, AXISMIN)
 * @param[in] xmax The measured X value at the point (x, AXISMAX)
 *
 * Since the sum (AXISMIN + 0 + AXISMAX) is 0, the calculation below could
 *  be simplified.
 */
void Calibration::stickRegression(double *offset, double *gain, double xoffset, double xmin, double xmax)
{
    double ymin = GlobalVariables::JoyAxis::AXISMIN;
    double ymax = GlobalVariables::JoyAxis::AXISMAX;

    double sum_X = xoffset + xmin + xmax;
    double sum_X2 = xoffset * xoffset + xmin * xmin + xmax * xmax;
    double sum_XY = xmin * ymin + xmax * ymax;

    *offset = (-sum_X * sum_XY) / (3 * sum_X2 - sum_X * sum_X);
    *gain = 3 * sum_XY / (3 * sum_X2 - sum_X * sum_X);
}

/**
 * @brief Resets calibration values of the currently selected device and
 *  updates UI.
 */
void Calibration::resetCalibrationValues()
{
    if (m_type == CAL_STICK && m_stick != nullptr)
    {
        m_stick->resetCalibration();
        showStickCalibrationValues(false, 0, false, 0, false, 0, false, 0);
    } else if ((m_type == CAL_ACCELEROMETER || m_type == CAL_GYROSCOPE) && m_sensor != nullptr)
    {
        m_sensor->resetCalibration();
        showSensorCalibrationValues(false, 0, false, 0, false, 0);
    }

    m_calibrated = false;
    m_ui->saveBtn->setEnabled(false);
    m_ui->resetBtn->setEnabled(false);
    m_ui->stickStatusBoxWidget->update();
    update();
}

/**
 * @brief Asks the user for confirmation with a given message if the given
 *  condition is false.
 * @param[in] message The message to show.
 * @param[in] confirmed True, if the action is already confirmed or no confirmation is necessary.
 * @returns True, if the action was confirmed. False otherwise.
 */
bool Calibration::askConfirmation(QString message, bool confirmed)
{
    if (!confirmed)
    {
        QMessageBox msgBox;
        msgBox.setText(message);
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

        switch (msgBox.exec())
        {
        case QMessageBox::Ok:
            return true;

        case QMessageBox::Cancel:
            return false;

        default:
            return true;
        }
    }
    return true;
}

/**
 * @brief Device change event handler. Asks for confirmation if there are unsaved changes.
 */
void Calibration::deviceSelectionChanged(int index)
{
    if (askConfirmation(tr("Calibration was not saved for the preset. Do you really want to continue?"), !m_changed))
    {
        int data = m_ui->deviceComboBox->itemData(index).toInt();
        selectTypeIndex(data);
    } else
    {
        int index = m_ui->deviceComboBox->findData(QVariant(m_type | (m_index << CAL_INDEX_POS)));
        m_ui->deviceComboBox->blockSignals(true);
        m_ui->deviceComboBox->setCurrentIndex(index);
        m_ui->deviceComboBox->blockSignals(false);
    }
}

/**
 * @brief Sensor data event handler.
 *  Performs gyroscope offset and accelerometer neutral orientation estimation
 *  and stops itself if the value was found or the process timed out.
 *
 * The gyroscope is only offset calibrated since gain calibration would require
 * an accurate turntable to apply a known rotation rate on all axes.
 * The offset is determined by calculating the mean output value at rest.
 *
 * The accelerometer calibration only rotates the coordinate system to the desired neutral
 * orientation. Calibrating offset and gain would require an accurate fixture
 * to align all axes parallel to gravity.
 * The neutral angles are determined by calculating the mean output value at the desired position.
 */
void Calibration::onSensorOffsetData(float x, float y, float z)
{
    m_offset[0].process(x);
    m_offset[1].process(y);
    m_offset[2].process(z);

    bool xvalid = m_offset[0].calculateRelativeErrorSq() < CAL_ACCURACY_SQ && m_offset[0].getCount() > CAL_MIN_SAMPLES;
    bool yvalid = m_offset[1].calculateRelativeErrorSq() < CAL_ACCURACY_SQ && m_offset[1].getCount() > CAL_MIN_SAMPLES;
    bool zvalid = m_offset[2].calculateRelativeErrorSq() < CAL_ACCURACY_SQ && m_offset[2].getCount() > CAL_MIN_SAMPLES;

    showSensorCalibrationValues(xvalid, m_offset[0].getMean(), yvalid, m_offset[1].getMean(), zvalid, m_offset[2].getMean());

    // Abort when end time is reached to avoid infinite loop
    // in case of noisy sensors.

    if ((xvalid && yvalid && zvalid) || (QDateTime::currentDateTime() > m_end_time))
    {
        m_changed = true;
        disconnect(m_sensor, &JoySensor::moved, this, &Calibration::onSensorOffsetData);
        disconnect(m_ui->startBtn, &QPushButton::clicked, this, nullptr);
        if (m_type == CAL_ACCELEROMETER)
            connect(m_ui->startBtn, &QPushButton::clicked, this, &Calibration::startAccelerometerCalibration);
        else if (m_type == CAL_GYROSCOPE)
            connect(m_ui->startBtn, &QPushButton::clicked, this, &Calibration::startGyroscopeCalibration);
        m_ui->steps->setText(tr("Calibration completed."));
        m_ui->startBtn->setText(tr("Start calibration"));
        m_ui->startBtn->setEnabled(true);
        m_ui->saveBtn->setEnabled(true);
        m_ui->deviceComboBox->setEnabled(true);
        update();
    }
}

/**
 * @brief Stick data event handler. Performs stick offset estimation
 *  and stops itself if the value was found or the process timed out.
 *
 * At the beginning, it waits for the first stick moved event and calculates
 *  the event rate for the denoise lag filter.
 * Then it looks for local minima and maxima within the sticks dead zone
 *  which are used to estimate the sticks center position.
 */
void Calibration::onStickOffsetData(int x, int y)
{
    if (m_phase == 0)
    {
        m_rate_timer.start();
        m_sample_count = 0;
        m_phase = 1;
    } else if (m_phase == 1)
    {
        if (++m_sample_count == STICK_RATE_SAMPLES)
        {
            int delta_t = m_rate_timer.elapsed();
            m_rate_timer.invalidate();
            m_filter[0] = PT1Filter(STICK_CAL_TAU, STICK_RATE_SAMPLES * 1000.0 / delta_t);
            m_filter[1] = PT1Filter(STICK_CAL_TAU, STICK_RATE_SAMPLES * 1000.0 / delta_t);
            m_sample_count = 0;
            m_end_time = QDateTime::currentDateTime().addSecs(CAL_TIMEOUT);
            m_phase = 2;
        }
    } else if (m_phase == 2)
    {
        double slopex = m_filter[0].getValue() - m_filter[0].process(x);
        double slopey = m_filter[1].getValue() - m_filter[1].process(y);

        if (((m_last_slope[0] < 0 && slopex > 0) || (m_last_slope[0] > 0 && slopex < 0)) && abs(x) < m_stick->getDeadZone())
        {
            m_offset[0].process(x);
        }
        if (((m_last_slope[1] < 0 && slopey > 0) || (m_last_slope[1] > 0 && slopey < 0)) && abs(y) < m_stick->getDeadZone())
        {
            m_offset[1].process(y);
        }

        // There are two move events generated for every hardware event,
        // one updates the X the other the Y value. This causes every second
        // derivate value to be zero. Ignore those values to get the real derivative.
        if (slopex != 0)
            m_last_slope[0] = slopex;
        if (slopey != 0)
            m_last_slope[1] = slopey;

        bool xvalid = m_offset[0].calculateRelativeErrorSq() < CAL_ACCURACY_SQ && m_offset[0].getCount() > CAL_MIN_SAMPLES;
        bool yvalid = m_offset[1].calculateRelativeErrorSq() < CAL_ACCURACY_SQ && m_offset[1].getCount() > CAL_MIN_SAMPLES;

        showStickCalibrationValues(xvalid, m_offset[0].getMean(), false, 1, yvalid, m_offset[1].getMean(), false, 1);

        if ((xvalid && yvalid) || QDateTime::currentDateTime() > m_end_time)
            m_phase = 3;
    } else if (m_phase == 3)
    {
        disconnect(m_stick, &JoyControlStick::moved, this, &Calibration::onStickOffsetData);
        m_ui->steps->setText(
            tr("Offset calibration completed. Click \"Continue calibration\" to continue with gain calibration."));
        m_ui->startBtn->setEnabled(true);
        update();
    }
}

/**
 * @brief Stick data event handler. Performs stick gain estimation
 *  and stops itself if the value was found or the process timed out.
 *
 * It looks for local minima and maxima outside the sticks dead zone
 *  which are used to estimate the sticks minimum and maximum position.
 */
void Calibration::onStickGainData(int x, int y)
{
    double slopex = m_filter[0].getValue() - m_filter[0].process(x);
    double slopey = m_filter[1].getValue() - m_filter[1].process(y);

    if (m_last_slope[0] > 0 && slopex < 0 && m_filter[0].getValue() < -m_stick->getDeadZone())
    {
        m_min[0].process(m_filter[0].getValue());
    } else if (m_last_slope[0] < 0 && slopex > 0 && m_filter[0].getValue() > m_stick->getDeadZone())
    {
        m_max[0].process(m_filter[0].getValue());
    }

    if (m_last_slope[1] > 0 && slopey < 0 && m_filter[1].getValue() < -m_stick->getDeadZone())
    {
        m_min[1].process(m_filter[1].getValue());
    } else if (m_last_slope[1] < 0 && slopey > 0 && m_filter[1].getValue() > m_stick->getDeadZone())
    {
        m_max[1].process(m_filter[1].getValue());
    }

    // There are two move events generated for every hardware event,
    // one updates the X the other the Y value. This causes every second
    // derivate value to be zero. Ignore those values to get the real derivative.
    if (slopex != 0)
        m_last_slope[0] = slopex;
    if (slopey != 0)
        m_last_slope[1] = slopey;

    bool xvalid = m_min[0].calculateRelativeErrorSq() < CAL_ACCURACY_SQ && m_min[0].getCount() > CAL_MIN_SAMPLES &&
                  m_max[0].calculateRelativeErrorSq() < CAL_ACCURACY_SQ && m_max[0].getCount() > CAL_MIN_SAMPLES;
    bool yvalid = m_min[1].calculateRelativeErrorSq() < CAL_ACCURACY_SQ && m_min[1].getCount() > CAL_MIN_SAMPLES &&
                  m_max[1].calculateRelativeErrorSq() < CAL_ACCURACY_SQ && m_max[1].getCount() > CAL_MIN_SAMPLES;

    double offsetX, gainX, offsetY, gainY;
    stickRegression(&offsetX, &gainX, m_offset[0].getMean(), m_min[0].getMean(), m_max[0].getMean());
    stickRegression(&offsetY, &gainY, m_offset[1].getMean(), m_min[1].getMean(), m_max[1].getMean());
    showStickCalibrationValues(true, offsetX, xvalid, gainX, true, offsetY, yvalid, gainY);

    if ((xvalid && yvalid) || QDateTime::currentDateTime() > m_end_time)
    {
        m_changed = true;
        disconnect(m_stick, &JoyControlStick::moved, this, &Calibration::onStickGainData);
        disconnect(m_ui->startBtn, &QPushButton::clicked, this, nullptr);
        connect(m_ui->startBtn, &QPushButton::clicked, this, &Calibration::startStickOffsetCalibration);
        m_ui->steps->setText("Calibration completed.");
        m_ui->startBtn->setText(tr("Start calibration"));
        m_ui->startBtn->setEnabled(true);
        m_ui->saveBtn->setEnabled(true);
        m_ui->deviceComboBox->setEnabled(true);
        update();
    }
}

/**
 * @brief Save calibration values into the device object.
 */
void Calibration::saveSettings()
{
    if (m_type == CAL_STICK)
    {
        double offsetX, gainX, offsetY, gainY;
        stickRegression(&offsetX, &gainX, m_offset[0].getMean(), m_min[0].getMean(), m_max[0].getMean());
        stickRegression(&offsetY, &gainY, m_offset[1].getMean(), m_min[1].getMean(), m_max[1].getMean());

        m_joystick->updateStickCalibration(m_index, offsetX, gainX, offsetY, gainY);
        showStickCalibrationValues(true, offsetX, true, gainX, true, offsetY, true, gainY);
    } else if (m_type == CAL_ACCELEROMETER)
    {
        m_joystick->updateAccelerometerCalibration(m_offset[0].getMean(), m_offset[1].getMean(), m_offset[2].getMean());
    } else if (m_type == CAL_GYROSCOPE)
    {
        m_joystick->updateGyroscopeCalibration(m_offset[0].getMean(), m_offset[1].getMean(), m_offset[2].getMean());
    }
    m_changed = false;
    m_calibrated = true;
    m_ui->saveBtn->setEnabled(false);
    m_ui->resetBtn->setEnabled(true);
}

/**
 * @brief Shows user instructions for accelerometer calibration and initializes estimators.
 */
void Calibration::startAccelerometerCalibration()
{
    if (m_sensor == nullptr)
        return;

    if (askConfirmation(tr("Calibration was saved for the preset. Do you really want to reset settings?"), !m_calibrated))
    {
        m_offset[0].reset();
        m_offset[1].reset();
        m_offset[2].reset();

        m_sensor->resetCalibration();
        m_calibrated = false;

        m_ui->steps->setText(tr("Hold the controller in the desired neutral position and press continue."));
        setWindowTitle(tr("Calibrating accelerometer"));
        m_ui->startBtn->setText(tr("Continue calibration"));
        update();

        m_ui->resetBtn->setEnabled(false);
        m_ui->deviceComboBox->setEnabled(false);
        disconnect(m_ui->startBtn, &QPushButton::clicked, this, nullptr);
        connect(m_ui->startBtn, &QPushButton::clicked, this, &Calibration::startAccelerometerAngleCalibration);
    }
}
/**
 * @brief Show calibration message to the user and enable accelerometer data event handler.
 */
void Calibration::startAccelerometerAngleCalibration()
{
    if (m_sensor != nullptr)
    {
        m_end_time = QDateTime::currentDateTime().addSecs(CAL_TIMEOUT);
        m_ui->steps->setText(
            tr("Collecting accelerometer data...\nPlease hold the controller still.\nThis can take up to %1 seconds.")
                .arg(CAL_TIMEOUT));
        connect(m_sensor, &JoySensor::moved, this, &Calibration::onSensorOffsetData);
        update();

        m_ui->startBtn->setEnabled(false);
        disconnect(m_ui->startBtn, &QPushButton::clicked, this, nullptr);
    }
}

/**
 * @brief Shows user instructions for gyroscope calibration and initializes estimators.
 */
void Calibration::startGyroscopeCalibration()
{
    if (m_sensor == nullptr)
        return;

    if (askConfirmation(tr("Calibration was saved for the preset. Do you really want to reset settings?"), !m_calibrated))
    {
        m_offset[0].reset();
        m_offset[1].reset();
        m_offset[2].reset();

        m_sensor->resetCalibration();
        m_calibrated = false;

        m_ui->steps->setText(tr("Place the controller at rest, e.g. put it on the desk, "
                                "and press continue."));
        setWindowTitle(tr("Calibrating gyroscope"));
        m_ui->startBtn->setText(tr("Continue calibration"));
        update();

        m_ui->resetBtn->setEnabled(false);
        m_ui->deviceComboBox->setEnabled(false);
        disconnect(m_ui->startBtn, &QPushButton::clicked, this, nullptr);
        connect(m_ui->startBtn, &QPushButton::clicked, this, &Calibration::startGyroscopeOffsetCalibration);
    }
}

/**
 * @brief Show calibration message to the user and enable gyroscope data event handler.
 */
void Calibration::startGyroscopeOffsetCalibration()
{
    if (m_sensor != nullptr)
    {
        m_end_time = QDateTime::currentDateTime().addSecs(CAL_TIMEOUT);
        m_ui->steps->setText(tr("Collecting gyroscope data...\nThis can take up to %1 seconds.").arg(CAL_TIMEOUT));
        connect(m_sensor, &JoySensor::moved, this, &Calibration::onSensorOffsetData);
        update();

        m_ui->startBtn->setEnabled(false);
        disconnect(m_ui->startBtn, &QPushButton::clicked, this, nullptr);
    }
}

/**
 * @brief Shows user instructions for stick offset calibration, initializes estimators
 *  and connects stick data event handlers.
 */
void Calibration::startStickOffsetCalibration()
{
    if (m_stick == nullptr)
        return;

    if (askConfirmation(tr("Calibration was saved for the preset. Do you really want to reset settings?"), !m_calibrated))
    {
        m_offset[0].reset();
        m_offset[1].reset();
        m_last_slope[0] = 0;
        m_last_slope[1] = 0;

        m_stick->resetCalibration();
        m_calibrated = false;
        m_phase = 0;

        m_ui->steps->setText(
            tr("Now move the stick several times to the maximum in different direction and back to center.\n"
               "This can take up to %1 seconds.")
                .arg(CAL_TIMEOUT));
        setWindowTitle(tr("Calibrating stick"));
        m_ui->startBtn->setText(tr("Continue calibration"));
        m_ui->startBtn->setEnabled(false);

        m_ui->resetBtn->setEnabled(false);
        m_ui->deviceComboBox->setEnabled(false);
        disconnect(m_ui->startBtn, &QPushButton::clicked, this, nullptr);
        connect(m_ui->startBtn, &QPushButton::clicked, this, &Calibration::startStickGainCalibration);
        connect(m_stick, &JoyControlStick::moved, this, &Calibration::onStickOffsetData);
        update();
    }
}

/**
 * @brief Shows user instructions for stick gain calibration, initializes estimators
 *  and connects stick data event handlers.
 */
void Calibration::startStickGainCalibration()
{
    if (m_stick == nullptr)
        return;

    m_min[0].reset();
    m_min[1].reset();
    m_max[0].reset();
    m_max[1].reset();
    m_filter[0].reset();
    m_filter[1].reset();

    m_ui->steps->setText(tr("Now move the stick in full circles for several times.\n"
                            "This can take up to %1 seconds.")
                             .arg(CAL_TIMEOUT));
    m_ui->startBtn->setEnabled(false);
    update();

    disconnect(m_ui->startBtn, &QPushButton::clicked, this, nullptr);
    connect(m_ui->startBtn, &QPushButton::clicked, this, &Calibration::startStickOffsetCalibration);
    connect(m_stick, &JoyControlStick::moved, this, &Calibration::onStickGainData);
    m_end_time = QDateTime::currentDateTime().addSecs(CAL_TIMEOUT);
}
