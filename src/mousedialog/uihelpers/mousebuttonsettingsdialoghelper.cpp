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

#include "messagehandler.h"
#include "joybutton.h"
#include "joybuttonslot.h"

#include <QDebug>

MouseButtonSettingsDialogHelper::MouseButtonSettingsDialogHelper(JoyButton *button, QObject *parent) :
    QObject(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    Q_ASSERT(button);

    this->button = button;
}

void MouseButtonSettingsDialogHelper::updateExtraAccelerationStatus(bool checked)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    button->setExtraAccelerationStatus(checked);
}

void MouseButtonSettingsDialogHelper::updateExtraAccelerationMultiplier(double value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    button->setExtraAccelerationMultiplier(value);
}

void MouseButtonSettingsDialogHelper::updateStartMultiPercentage(double value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    button->setStartAccelMultiplier(value);
}

void MouseButtonSettingsDialogHelper::updateMinAccelThreshold(double value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    button->setMinAccelThreshold(value);
}

void MouseButtonSettingsDialogHelper::updateMaxAccelThreshold(double value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    button->setMaxAccelThreshold(value);
}

void MouseButtonSettingsDialogHelper::updateAccelExtraDuration(double value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    button->setAccelExtraDuration(value);
}

void MouseButtonSettingsDialogHelper::updateReleaseSpringRadius(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    button->setSpringDeadCircleMultiplier(value);
}

void MouseButtonSettingsDialogHelper::updateSpringRelativeStatus(bool value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    button->setSpringRelativeStatus(value);
}

JoyButton *MouseButtonSettingsDialogHelper::getButton() const {

    return button;
}
