/* antimicroX Gamepad to KB+M event mapper
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

#include "antimicrosettings.h"

#include "messagehandler.h"

#include <QDebug>


AntiMicroSettings::AntiMicroSettings(const QString &fileName, Format format, QObject *parent) :
    QSettings(fileName, format, parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);
}

/**
 * @brief Get the currently used value such as an setting overridden
 *     with a command line argument.
 * @param Setting key
 * @param Default value to use if key does not exist
 * @return Stored value or the default value passed
 */
QVariant AntiMicroSettings::runtimeValue(const QString &key, const QVariant &defaultValue) const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QVariant settingValue;
    QString inGroup = group();
    QString fullKey = QString(inGroup).append("/").append(key);

    if (cmdSettings.contains(fullKey)) settingValue = cmdSettings.value(fullKey, defaultValue);
    else settingValue = value(key, defaultValue);

    return settingValue;
}

/**
 * @brief Import relevant options given on the command line into a QSettings
 *     instance. Used to override any options that might be present in the
 *     main settings file. Keys will have to be changed to the appropriate
 *     config key.
 * @param Interpreted options set on the command line.
 */
void AntiMicroSettings::importFromCommandLine(CommandLineUtility &cmdutility)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    getCmdSettings().clear();

    if (cmdutility.isLaunchInTrayEnabled())
    {
        getCmdSettings().setValue("LaunchInTray", 1);
    }
    if (cmdutility.shouldMapController())
    {
        getCmdSettings().setValue("DisplaySDLMapping", 1);
    }
}

QMutex* AntiMicroSettings::getLock()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return &lock;
}

QSettings& AntiMicroSettings::getCmdSettings() {

    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return cmdSettings;
}
