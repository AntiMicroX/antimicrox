#include "antimicrosettings.h"

AntiMicroSettings::AntiMicroSettings(const QString &fileName, Format format, QObject *parent) :
    QSettings(fileName, format, parent)
{
}

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

void AntiMicroSettings::importFromCommandLine(CommandLineUtility &cmdutility)
{
    cmdSettings.clear();

    if (cmdutility.isLaunchInTrayEnabled())
    {
        cmdSettings.setValue("LaunchInTray", 1);
    }
}
