/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
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

#ifndef MOUSESETTINGSDIALOG_H
#define MOUSESETTINGSDIALOG_H

#include <QDialog>
#include <QTime>

#include "joybutton.h"

namespace Ui {
class MouseSettingsDialog;
}

class MouseSettingsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit MouseSettingsDialog(QWidget *parent = 0);
    ~MouseSettingsDialog();
    
protected:
    void updateAccelerationCurvePresetComboBox(JoyButton::JoyMouseCurve mouseCurve);
    void updateExtraAccelerationCurvePresetComboBox(JoyButton::JoyExtraAccelerationCurve curve);
    JoyButton::JoyMouseCurve getMouseCurveForIndex(int index);
    JoyButton::JoyExtraAccelerationCurve getExtraAccelCurveForIndex(int index);

    Ui::MouseSettingsDialog *ui;
    QTime lastMouseStatUpdate;

public slots:
    void changeSettingsWidgetStatus(int index);
    void changeSpringSectionStatus(int index);
    void changeMouseSpeedBoxStatus(int index);
    void changeWheelSpeedBoxStatus(int index);
    void updateHorizontalSpeedConvertLabel(int value);
    void updateVerticalSpeedConvertLabel(int value);
    void moveSpeedsTogether(int value);
    //void changeSmoothingStatus(int index);
    void updateWheelVerticalSpeedLabel(int value);
    void updateWheelHorizontalSpeedLabel(int value);
    void changeSensitivityStatusForMouseMode(int index);

    virtual void changeMouseMode(int index) = 0;
    virtual void changeMouseCurve(int index) = 0;

private slots:
    void updateMouseCursorStatusLabels(int mouseX, int mouseY, int elapsed);
    void updateMouseSpringStatusLabels(int coordX, int coordY);
    void refreshMouseCursorSpeedValues(int index);
    void disableReleaseSpringBox(bool enable);
    void resetReleaseRadius(bool enabled);
};

#endif // MOUSESETTINGSDIALOG_H
