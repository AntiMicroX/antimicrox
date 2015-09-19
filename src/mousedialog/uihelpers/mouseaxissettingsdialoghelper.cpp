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

#include "mouseaxissettingsdialoghelper.h"

MouseAxisSettingsDialogHelper::MouseAxisSettingsDialogHelper(JoyAxis *axis, QObject *parent) :
    QObject(parent)
{
    Q_ASSERT(axis);

    this->axis = axis;
}

void MouseAxisSettingsDialogHelper::updateExtraAccelerationStatus(bool checked)
{
    axis->getPAxisButton()->setExtraAccelerationStatus(checked);
    axis->getNAxisButton()->setExtraAccelerationStatus(checked);
}

void MouseAxisSettingsDialogHelper::updateExtraAccelerationMultiplier(double value)
{
    axis->getPAxisButton()->setExtraAccelerationMultiplier(value);
    axis->getNAxisButton()->setExtraAccelerationMultiplier(value);
}

void MouseAxisSettingsDialogHelper::updateStartMultiPercentage(double value)
{
    axis->getPAxisButton()->setStartAccelMultiplier(value);
    axis->getNAxisButton()->setStartAccelMultiplier(value);
}

void MouseAxisSettingsDialogHelper::updateMinAccelThreshold(double value)
{
    axis->getPAxisButton()->setMinAccelThreshold(value);
    axis->getNAxisButton()->setMinAccelThreshold(value);
}

void MouseAxisSettingsDialogHelper::updateMaxAccelThreshold(double value)
{
    axis->getPAxisButton()->setMaxAccelThreshold(value);
    axis->getNAxisButton()->setMaxAccelThreshold(value);
}

void MouseAxisSettingsDialogHelper::updateAccelExtraDuration(double value)
{
    axis->getPAxisButton()->setAccelExtraDuration(value);
    axis->getNAxisButton()->setAccelExtraDuration(value);
}

void MouseAxisSettingsDialogHelper::updateReleaseSpringRadius(int value)
{
    axis->getPAxisButton()->setSpringDeadCircleMultiplier(value);
    axis->getNAxisButton()->setSpringDeadCircleMultiplier(value);
}

