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

#include "messagehandler.h"
#include "joyaxis.h"

#include <QDebug>

MouseAxisSettingsDialogHelper::MouseAxisSettingsDialogHelper(JoyAxis *axis, QObject *parent) :
    QObject(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_ASSERT(axis);

    this->axis = axis;
}

void MouseAxisSettingsDialogHelper::updateExtraAccelerationStatus(bool checked)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    axis->getPAxisButton()->setExtraAccelerationStatus(checked);
    axis->getNAxisButton()->setExtraAccelerationStatus(checked);
}

void MouseAxisSettingsDialogHelper::updateExtraAccelerationMultiplier(double value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    axis->getPAxisButton()->setExtraAccelerationMultiplier(value);
    axis->getNAxisButton()->setExtraAccelerationMultiplier(value);
}

void MouseAxisSettingsDialogHelper::updateStartMultiPercentage(double value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    axis->getPAxisButton()->setStartAccelMultiplier(value);
    axis->getNAxisButton()->setStartAccelMultiplier(value);
}

void MouseAxisSettingsDialogHelper::updateMinAccelThreshold(double value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    axis->getPAxisButton()->setMinAccelThreshold(value);
    axis->getNAxisButton()->setMinAccelThreshold(value);
}

void MouseAxisSettingsDialogHelper::updateMaxAccelThreshold(double value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    axis->getPAxisButton()->setMaxAccelThreshold(value);
    axis->getNAxisButton()->setMaxAccelThreshold(value);
}

void MouseAxisSettingsDialogHelper::updateAccelExtraDuration(double value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    axis->getPAxisButton()->setAccelExtraDuration(value);
    axis->getNAxisButton()->setAccelExtraDuration(value);
}

void MouseAxisSettingsDialogHelper::updateReleaseSpringRadius(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    axis->getPAxisButton()->setSpringDeadCircleMultiplier(value);
    axis->getNAxisButton()->setSpringDeadCircleMultiplier(value);
}

JoyAxis* MouseAxisSettingsDialogHelper::getAxis() const {

    return axis;
}

