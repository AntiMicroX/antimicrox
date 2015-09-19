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

#ifndef MOUSECONTROLSTICKSETTINGSDIALOG_H
#define MOUSECONTROLSTICKSETTINGSDIALOG_H

#include "mousesettingsdialog.h"
#include "springmoderegionpreview.h"

#include <joycontrolstick.h>
#include "uihelpers/mousecontrolsticksettingsdialoghelper.h"

class MouseControlStickSettingsDialog : public MouseSettingsDialog
{
    Q_OBJECT
public:
    explicit MouseControlStickSettingsDialog(JoyControlStick *stick, QWidget *parent=0);
    
protected:
    void selectCurrentMouseModePreset();
    void calculateSpringPreset();
    void calculateMouseSpeedPreset();
    void calculateWheelSpeedPreset();
    void updateWindowTitleStickName();

    void calculateExtraAccelrationStatus();
    void calculateExtraAccelerationMultiplier();

    void calculateStartAccelerationMultiplier();
    void calculateMinAccelerationThreshold();
    void calculateMaxAccelerationThreshold();
    void calculateAccelExtraDuration();

    void calculateReleaseSpringRadius();
    void calculateExtraAccelerationCurve();

    JoyControlStick *stick;
    SpringModeRegionPreview *springPreviewWidget;
    MouseControlStickSettingsDialogHelper helper;

signals:
    
public slots:
    void changeMouseMode(int index);
    void changeMouseCurve(int index);
    void updateConfigHorizontalSpeed(int value);
    void updateConfigVerticalSpeed(int value);
    void updateSpringWidth(int value);
    void updateSpringHeight(int value);
    void updateSensitivity(double value);
    void updateAccelerationCurvePresetComboBox();
    void updateWheelSpeedHorizontalSpeed(int value);
    void updateWheelSpeedVerticalSpeed(int value);
    void updateSpringRelativeStatus(bool value);

private slots:
    void updateExtraAccelerationCurve(int index);
};

#endif // MOUSECONTROLSTICKSETTINGSDIALOG_H
