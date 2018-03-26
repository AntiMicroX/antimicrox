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

#include "mousebuttonsettingsdialoghelper.h"
#include "joybutton.h"
#include "joybuttonslot.h"

#include <QDebug>

MouseButtonSettingsDialogHelper::MouseButtonSettingsDialogHelper(JoyButton *button, QObject *parent) :
    QObject(parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    Q_ASSERT(button);

    this->button = button;
}

void MouseButtonSettingsDialogHelper::updateExtraAccelerationStatus(bool checked)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    button->setExtraAccelerationStatus(checked);
}

void MouseButtonSettingsDialogHelper::updateExtraAccelerationMultiplier(double value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    button->setExtraAccelerationMultiplier(value);
}

void MouseButtonSettingsDialogHelper::updateStartMultiPercentage(double value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    button->setStartAccelMultiplier(value);
}

void MouseButtonSettingsDialogHelper::updateMinAccelThreshold(double value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    button->setMinAccelThreshold(value);
}

void MouseButtonSettingsDialogHelper::updateMaxAccelThreshold(double value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    button->setMaxAccelThreshold(value);
}

void MouseButtonSettingsDialogHelper::updateAccelExtraDuration(double value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    button->setAccelExtraDuration(value);
}

void MouseButtonSettingsDialogHelper::updateReleaseSpringRadius(int value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    button->setSpringDeadCircleMultiplier(value);
}

void MouseButtonSettingsDialogHelper::updateSpringRelativeStatus(bool value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    button->setSpringRelativeStatus(value);
}
