/* antimicroX Gamepad to KB+M event mapper
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


#ifndef MOUSEDPADSETTINGSDIALOG_H
#define MOUSEDPADSETTINGSDIALOG_H

#include "ui_mousesettingsdialog.h"

#include "mousesettingsdialog.h"
#include "uihelpers/mousedpadsettingsdialoghelper.h"


class JoyDPad;
class SpringModeRegionPreview;

class MouseDPadSettingsDialog : public MouseSettingsDialog
{
    Q_OBJECT

public:
    explicit MouseDPadSettingsDialog(JoyDPad *dpad, QWidget *parent = nullptr);

    JoyDPad *getDPad() const;
    SpringModeRegionPreview *getSpringPreviewWidget() const;
    MouseDpadSettingsDialogHelper const& getHelper();

protected:
    void selectCurrentMouseModePreset();
    void calculateSpringPreset();
    void calculateMouseSpeedPreset();
    void calculateWheelSpeedPreset();
    void updateWindowTitleDPadName();
    void calculateReleaseSpringRadius();
    void calculateExtraAccelerationCurve();

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
    void updateReleaseSpringRadius(int value);
    void updateExtraAccelerationCurve(int index);

private:
    MouseDpadSettingsDialogHelper& getHelperLocal();

    JoyDPad *dpad;
    SpringModeRegionPreview *springPreviewWidget;
    MouseDpadSettingsDialogHelper helper;

};

#endif // MOUSEDPADSETTINGSDIALOG_H
