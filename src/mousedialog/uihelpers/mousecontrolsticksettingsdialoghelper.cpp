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

#include "mousecontrolsticksettingsdialoghelper.h"

MouseControlStickSettingsDialogHelper::MouseControlStickSettingsDialogHelper(JoyControlStick *stick,
                                                                             QObject *parent) :
    QObject(parent)
{
    Q_ASSERT(stick);

    this->stick = stick;
}

void MouseControlStickSettingsDialogHelper::updateExtraAccelerationStatus(bool checked)
{
    stick->setButtonsExtraAccelerationStatus(checked);
}

void MouseControlStickSettingsDialogHelper::updateExtraAccelerationMultiplier(double value)
{
    stick->setButtonsExtraAccelerationMultiplier(value);
}

void MouseControlStickSettingsDialogHelper::updateStartMultiPercentage(double value)
{
    stick->setButtonsStartAccelerationMultiplier(value);
}

void MouseControlStickSettingsDialogHelper::updateMinAccelThreshold(double value)
{
    stick->setButtonsMinAccelerationThreshold(value);
}

void MouseControlStickSettingsDialogHelper::updateMaxAccelThreshold(double value)
{
    stick->setButtonsMaxAccelerationThreshold(value);
}

void MouseControlStickSettingsDialogHelper::updateAccelExtraDuration(double value)
{
    stick->setButtonsAccelerationExtraDuration(value);
}

void MouseControlStickSettingsDialogHelper::updateReleaseSpringRadius(int value)
{
    stick->setButtonsSpringDeadCircleMultiplier(value);
}
