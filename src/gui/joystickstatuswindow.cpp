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

#include "joystickstatuswindow.h"
#include "ui_joystickstatuswindow.h"

#include "common.h"
#include "globalvariables.h"
#include "inputdevice.h"
#include "joybuttonstatusbox.h"
#include "joybuttontypes/joydpadbutton.h"
#include "joydpad.h"
#include "joysensor.h"

#include <QDebug>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QWidget>

JoystickStatusWindow::JoystickStatusWindow(InputDevice *joystick, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::JoystickStatusWindow)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    this->joystick = joystick;

    PadderCommon::inputDaemonMutex.lock();

    setWindowTitle(tr("%1 (#%2) Properties").arg(joystick->getSDLName()).arg(joystick->getRealJoyNumber()));

    SDL_JoystickPowerLevel powerLevel = SDL_JoystickCurrentPowerLevel(joystick->getJoyHandle());

    switch (powerLevel)
    {
    case SDL_JOYSTICK_POWER_EMPTY:

        ui->batteryValueLabel->setText(tr("Empty"));
        break;

    case SDL_JOYSTICK_POWER_LOW:

        ui->batteryValueLabel->setText(tr("Low"));
        break;

    case SDL_JOYSTICK_POWER_MEDIUM:

        ui->batteryValueLabel->setText(tr("Medium"));
        break;

    case SDL_JOYSTICK_POWER_FULL:
    case SDL_JOYSTICK_POWER_MAX:

        ui->batteryValueLabel->setText(tr("Full"));
        break;

    case SDL_JOYSTICK_POWER_UNKNOWN:

        ui->batteryValueLabel->setText(tr("Unknown"));
        break;

    case SDL_JOYSTICK_POWER_WIRED:

        ui->batteryValueLabel->setText(tr("Wired"));
        break;

    default:

        ui->batteryValueLabel->setText(tr("Different: %1").arg(powerLevel));
        WARN() << "Unknown battery level:" << powerLevel;
        break;
    }

    ui->joystickNameLabel->setText(joystick->getSDLName());
    ui->joystickNumberLabel->setText(QString::number(joystick->getRealJoyNumber()));
    ui->joystickAxesLabel->setText(QString::number(joystick->getNumberRawAxes()));
    ui->joystickButtonsLabel->setText(QString::number(joystick->getNumberRawButtons()));
    ui->joystickHatsLabel->setText(QString::number(joystick->getNumberRawHats()));

    if (joystick->hasRawSensor(ACCELEROMETER) && joystick->hasRawSensor(GYROSCOPE))
        ui->joystickSensorsLabel->setText(tr("Accelerometer + Gyroscope"));
    else if (joystick->hasRawSensor(ACCELEROMETER))
        ui->joystickSensorsLabel->setText(tr("Accelerometer"));
    else if (joystick->hasRawSensor(GYROSCOPE))
        ui->joystickSensorsLabel->setText(tr("Gyroscope"));
    else
        ui->joystickSensorsLabel->setText(tr("None"));

    joystick->getActiveSetJoystick()->setIgnoreEventState(true);
    joystick->getActiveSetJoystick()->release();
    joystick->resetButtonDownCount();

    QVBoxLayout *axesBox = new QVBoxLayout();
    axesBox->setSpacing(4);
    for (int i = 0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getActiveSetJoystick()->getJoyAxis(i);

        if (axis != nullptr)
        {
            QHBoxLayout *hbox = new QHBoxLayout();

            QLabel *axisLabel = new QLabel();
            axisLabel->setText(tr("Axis %1").arg(axis->getRealJoyIndex()));
            QProgressBar *axisBar = new QProgressBar();
            axisBar->setMinimum(GlobalVariables::JoyAxis::AXISMIN);
            axisBar->setMaximum(GlobalVariables::JoyAxis::AXISMAX);
            axisBar->setFormat("%v");
            axisBar->setValue(axis->getCurrentRawValue());
            hbox->addWidget(axisLabel);
            hbox->addWidget(axisBar);
            hbox->addSpacing(10);
            axesBox->addLayout(hbox);

            connect(axis, &JoyAxis::moved, axisBar, &QProgressBar::setValue);
        }
    }

    ui->axesScrollArea->setLayout(axesBox);

    QGridLayout *buttonsGrid = new QGridLayout();
    buttonsGrid->setHorizontalSpacing(10);
    buttonsGrid->setVerticalSpacing(10);

    int currentRow = 0;
    int currentColumn = 0;
    for (int i = 0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getActiveSetJoystick()->getJoyButton(i);
        if (button != nullptr)
        {
            JoyButtonStatusBox *statusbox = new JoyButtonStatusBox(button);
            statusbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            buttonsGrid->addWidget(statusbox, currentRow, currentColumn);
            currentColumn++;
            if (currentColumn >= 6)
            {
                currentRow++;
                currentColumn = 0;
            }
        }
    }

    ui->buttonsScrollArea->setLayout(buttonsGrid);

    QVBoxLayout *hatsBox = new QVBoxLayout();
    hatsBox->setSpacing(4);
    for (int i = 0; i < joystick->getNumberHats(); i++)
    {
        JoyDPad *dpad = joystick->getActiveSetJoystick()->getJoyDPad(i);
        if (dpad != nullptr)
        {
            QHBoxLayout *hbox = new QHBoxLayout();

            QLabel *dpadLabel = new QLabel();
            dpadLabel->setText(tr("Hat %1").arg(dpad->getRealJoyNumber()));
            QProgressBar *dpadBar = new QProgressBar();
            dpadBar->setMinimum(JoyDPadButton::DpadCentered);
            dpadBar->setMaximum(JoyDPadButton::DpadLeftDown);
            dpadBar->setFormat("%v");
            dpadBar->setValue(dpad->getCurrentDirection());
            hbox->addWidget(dpadLabel);
            hbox->addWidget(dpadBar);
            hbox->addSpacing(10);
            hatsBox->addLayout(hbox);

            connect(dpad, &JoyDPad::active, dpadBar, &QProgressBar::setValue);
            connect(dpad, &JoyDPad::released, dpadBar, &QProgressBar::setValue);
        }
    }

    hatsBox->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Preferred, QSizePolicy::Fixed));

    if (ui->hatsGroupBox->layout())
    {
        delete ui->hatsGroupBox->layout();
    }

    ui->hatsGroupBox->setLayout(hatsBox);

    QVBoxLayout *sensorsBox = new QVBoxLayout();
    sensorsBox->setSpacing(4);

    for (int i = 0; i < SENSOR_COUNT; ++i)
    {
        JoySensorType type = static_cast<JoySensorType>(i);
        JoySensor *sensor = joystick->getActiveSetJoystick()->getSensor(type);
        QProgressBar **axes;
        if (type == ACCELEROMETER)
            axes = m_accel_axes;
        else
            axes = m_gyro_axes;

        if (sensor != nullptr)
        {
            for (int i = 0; i < 3; ++i)
            {
                QHBoxLayout *hbox = new QHBoxLayout();

                QLabel *axisLabel = new QLabel();
                axes[i] = new QProgressBar();
                if (type == ACCELEROMETER)
                {
                    axes[i]->setMinimum(GlobalVariables::JoySensor::ACCEL_MIN * 1000);
                    axes[i]->setMaximum(GlobalVariables::JoySensor::ACCEL_MAX * 1000);
                } else
                {
                    axes[i]->setMinimum(GlobalVariables::JoySensor::GYRO_MIN * 1000);
                    axes[i]->setMaximum(GlobalVariables::JoySensor::GYRO_MAX * 1000);
                }
                axes[i]->setFormat("%v");
                if (i == 0)
                {
                    axisLabel->setText(QString("%1 X").arg(sensor->sensorTypeName()));
                    axes[i]->setValue(sensor->getXCoordinate());
                } else if (i == 1)
                {
                    axisLabel->setText(QString("%1 Y").arg(sensor->sensorTypeName()));
                    axes[i]->setValue(sensor->getYCoordinate());
                } else
                {
                    axisLabel->setText(QString("%1 Z").arg(sensor->sensorTypeName()));
                    axes[i]->setValue(sensor->getZCoordinate());
                }
                hbox->addWidget(axisLabel);
                hbox->addWidget(axes[i]);
                hbox->addSpacing(10);
                sensorsBox->addLayout(hbox);
            }

            if (type == ACCELEROMETER)
            {
                connect(sensor, &JoySensor::moved, this, &JoystickStatusWindow::updateAccelerometerValues);
            } else
            {
                connect(sensor, &JoySensor::moved, this, &JoystickStatusWindow::updateGyroscopeValues);
            }
        }
    }

    sensorsBox->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Preferred, QSizePolicy::Fixed));
    ui->sensorsGroupBox->setLayout(sensorsBox);

    //    QString guidString = joystick->getGUIDString();
    //    if (!guidString.isEmpty())
    //    {
    //        ui->guidHeaderLabel->show();
    //        ui->guidLabel->setText(guidString);
    //        ui->guidLabel->show();
    //    }
    //    else
    //    {
    //        ui->guidHeaderLabel->hide();
    //        ui->guidLabel->hide();
    //    }

    QString uniqueString = joystick->getUniqueIDString();

    if (!uniqueString.isEmpty())
    {
        ui->guidHeaderLabel->show();
        ui->guidLabel->setText(uniqueString);
        ui->guidLabel->show();
    } else
    {
        ui->guidHeaderLabel->hide();
        ui->guidLabel->hide();
    }

    QString usingGameController = tr("No");
    if (joystick->isGameController())
    {
        usingGameController = tr("Yes");
    }

    ui->sdlGameControllerLabel->setText(usingGameController);

    PadderCommon::inputDaemonMutex.unlock();

    connect(joystick, &InputDevice::destroyed, this, &JoystickStatusWindow::obliterate);
    connect(this, &JoystickStatusWindow::finished, this, &JoystickStatusWindow::restoreButtonStates);
}

JoystickStatusWindow::~JoystickStatusWindow() { delete ui; }

void JoystickStatusWindow::restoreButtonStates(int code)
{
    if (code == QDialogButtonBox::AcceptRole)
    {
        PadderCommon::inputDaemonMutex.lock();

        joystick->getActiveSetJoystick()->setIgnoreEventState(false);
        joystick->getActiveSetJoystick()->release();

        PadderCommon::inputDaemonMutex.unlock();
    }
}

void JoystickStatusWindow::obliterate() { this->done(QDialogButtonBox::DestructiveRole); }

/**
 * @brief Accelerometer "moved" event handler
 *  Updates raw accelerometer values on the screen.
 */
void JoystickStatusWindow::updateAccelerometerValues(float valueX, float valueY, float valueZ)
{
    m_accel_axes[0]->setValue(valueX * 1000);
    m_accel_axes[1]->setValue(valueY * 1000);
    m_accel_axes[2]->setValue(valueZ * 1000);
}

/**
 * @brief Gyroscope "moved" event handler
 *  Updates raw gyroscope values on the screen.
 */
void JoystickStatusWindow::updateGyroscopeValues(float valueX, float valueY, float valueZ)
{
    m_gyro_axes[0]->setValue(JoySensor::radToDeg(valueX) * 1000);
    m_gyro_axes[1]->setValue(JoySensor::radToDeg(valueY) * 1000);
    m_gyro_axes[2]->setValue(JoySensor::radToDeg(valueZ) * 1000);
}

InputDevice *JoystickStatusWindow::getJoystick() const { return joystick; }
