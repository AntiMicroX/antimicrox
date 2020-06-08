/* antimicroX Gamepad to KB+M event mapper
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


#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "joycontrolstickeditdialog.h"

#include <SDL2/SDL_joystick.h>

#include <QProgressBar>


class JoyControlStick;
class InputDevice;

namespace Ui {
class Calibration;
}

class Calibration : public QWidget
{
    Q_OBJECT

public:
    explicit Calibration(InputDevice* joystick, QWidget *parent = 0);
    ~Calibration();

    int chooseMinMax(QString min_max_sign, QList<int> ax_values);
    void setQuadraticZoneCalibrated(int &max_axis_val_x, int &min_axis_val_x, int &max_axis_val_y, int &min_axis_val_y);

protected:
    void setProgressBars(int setJoyNr, int stickNr);
    void setProgressBars(JoyControlStick* controlstick);
    void updateAxesBox();
    void restoreCalValues();
    bool enoughProb(int x_count, int y_count, QString character);
    int calibratedDeadZone(int center, int deadzone);


private:
    static int fakeMapFunc(const int &x);
    static void summarizeValues(int &numbFromList, const int &mappednumb);
    static void getMinVal(int &numbFromList, const int &mappednumb);
    static void getMaxVal(int &numbFromList, const int &mappednumb);

    Ui::Calibration *ui;
    JoyControlStick *stick;
    InputDevice* currentJoystick;
    JoyControlStickEditDialogHelper helper;
    JoyAxis* joyAxisX;
    JoyAxis* joyAxisY;
    QProgressBar *axisBarX;
    QProgressBar *axisBarY;
    QMultiHash<QString,int> x_es_val;
    QMultiHash<QString,int> y_es_val;
    int center_calibrated_x;
    int center_calibrated_y;
    int max_axis_val_x;
    int min_axis_val_x;
    int max_axis_val_y;
    int min_axis_val_y;
    int deadzone_calibrated_x;
    int deadzone_calibrated_y;
    int sumX;
    int sumY;
    bool calibrated;
    QString text;

public slots:
    void saveSettings();
    void checkX(int value);
    void checkY(int value);
    void createAxesConnection();
    void startCalibration();
    void startSecondStep();
    void startLastStep();

protected slots:
    void resetSettings(bool silentReset, bool clicked = false);

signals:
    void deadZoneChanged(int value);
    void propertyUpdated();

};

#endif // CALIBRATION_H
