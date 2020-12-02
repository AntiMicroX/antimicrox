/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
 * Copyright (C) 2020 Paweł Kotiuk <kotiuk@zohomail.eu>
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

#include <QtTest/QtTest>

/*
resetBtn should be enabled when !stick->wasCalibrated()
resetBtn should be disabled when stick->wasCalibrated()
Calibration->findChildren<QProgressBar*>() count is equal to 2
startButton cannot be disabled
cancelBtn cannot be disabled
saveBtn is enabled when ui->steps->text() == tr("\n---Calibration done!---\n") && stick->setCalibrationSummary != ui->Informations->text() || this->text != stick->setCalibrationSummary
saveBtn is disabled when ui->steps->text() != tr("\n---Calibration done!---\n") && stick->setCalibrationSummary == ui->Informations->text() || this->text == stick->setCalibrationSummary
controllersBox->count() == this->joysticks->count

int currContrBoxIndex = ui->controllersBox->currentIndex();
QList<JoyControlStick*> sticksList = joysticks->value(currContrBoxIndex)->getSetJoystick(currContrBoxIndex)->getSticks().values();
QList<JoyControlStick*> currStick(sticksList);
ui->axesBox->count == currStick->count

// test for other controllers and sticks too at the same time

if each controlstick.data()->thread()->priority() != InheritPriority  // if each thread is running, otherwise show priority

if window title == tr("Calibration")

if ui->stickStatusBoxWidget->getStick == ui->axesBox at 0


// ACTIONS


if there are ui->axesBox->count > 1 choose the next one and check if ui->stickStatusBoxWidget->getStick == ui->axesBox at 0 &&

joyAxisX = controlstick->getAxisX();
joyAxisY = controlstick->getAxisY();

if progressBarsLayout->->itemAt(0)->widget() == tr("Axis %1").arg(joyAxisX->getRealJoyIndex())
and
progressBarsLayout->->itemAt(2)->widget() == tr("Axis %1").arg(joyAxisY->getRealJoyIndex())


check results of
connect(joyAxisX, &JoyAxis::moved, this, &Calibration::checkX);
connect(joyAxisY, &JoyAxis::moved, this, &Calibration::checkY);

test methods
1) no values
2) various expected values
3) various unexpected values
4) really big values
5) values below 0

int Calibration::chooseMinMax(QString min_max_sign, QList<int> ax_values)
Calibration::checkX(int value)
Calibration::checkY(int value)
Calibration::setController(QString controllerName)
Calibration::setProgressBars(int inputDevNr, int setJoyNr, int stickNr)
Calibration::setProgressBars(JoyControlStick* controlstick)
bool Calibration::enoughProb(int x_count, int y_count, QString character)
int Calibration::calibratedDeadZone(int center, int deadzone)
void Calibration::summarizeValues(int &numbFromList, const int &mappednumb)
void Calibration::getMinVal(int &numbFromList, const int &mappednumb)
void Calibration::getMaxVal(int &numbFromList, const int &mappednumb)
void Calibration::setQuadraticZoneCalibrated(int &max_axis_val_x, int &min_axis_val_x, int &max_axis_val_y, int &min_axis_val_y)
void Calibration::restoreCalValues() -> if all local variables are default again


*/

class TestCalibration: public QObject
{
    Q_OBJECT

public:
    TestCalibration(QObject* parent = 0);

private slots:

private:
    TestCalibration calibrationDialog;
};

TestCalibration::TestCalibration(QObject* parent) :
    QObject(parent),
    Calibration()
{
    QTestEventLoop::instance().enterLoop(1);
}

// QTEST_MAIN(TestAdvanceButtonDialog)
#include "testcalibration.moc"

