#include "antimicrosettings.h"

AntiMicroSettings::AntiMicroSettings(const QString &fileName, Format format, QObject *parent) :
    QSettings(fileName, format, parent)
{
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
    QVariant settingValue;
    QString inGroup = group();
    QString fullKey = QString(inGroup).append("/").append(key);
    if (cmdSettings.contains(fullKey))
    {
        settingValue = cmdSettings.value(fullKey, defaultValue);
    }
    else
    {
        settingValue = value(key, defaultValue);
    }

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
    cmdSettings.clear();

    if (cmdutility.isLaunchInTrayEnabled())
    {
        cmdSettings.setValue("LaunchInTray", 1);
    }
    if (cmdutility.shouldMapController())
    {
        cmdSettings.setValue("DisplaySDLMapping", 1);
    }
}
