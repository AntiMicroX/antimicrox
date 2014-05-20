#include <QFileInfo>

#include "autoprofileinfo.h"

AutoProfileInfo::AutoProfileInfo(QString guid, QString profileLocation,
                                 QString exe, bool active, QObject *parent) :
    QObject(parent)
{
    setGUID(guid);
    setProfileLocation(profileLocation);
    setExe(exe);
    setActive(active);
    setDefaultState(false);
}

AutoProfileInfo::AutoProfileInfo(QString guid, QString profileLocation,
                                 bool active, QObject *parent) :
    QObject(parent)
{
    setGUID(guid);
    setProfileLocation(profileLocation);
    setActive(active);
    setDefaultState(false);
}

AutoProfileInfo::AutoProfileInfo(QObject *parent) :
    QObject(parent)
{
    setActive(true);
    setDefaultState(false);
}

AutoProfileInfo::~AutoProfileInfo()
{
}

void AutoProfileInfo::setGUID(QString guid)
{
    this->guid = guid;
}

QString AutoProfileInfo::getGUID()
{
    return guid;
}

void AutoProfileInfo::setProfileLocation(QString profileLocation)
{
    QFileInfo info(profileLocation);
    if (profileLocation != this->profileLocation &&
        info.exists() && info.isReadable())
    {
        this->profileLocation = profileLocation;
    }
    else if (guid == "all" && profileLocation.isEmpty())
    {
        this->profileLocation = "";
    }
}

QString AutoProfileInfo::getProfileLocation()
{
    return profileLocation;
}

void AutoProfileInfo::setExe(QString exe)
{
    QFileInfo info(exe);
    if (exe != this->exe && info.exists() && info.isExecutable())
    {
        this->exe = exe;
    }
}

QString AutoProfileInfo::getExe()
{
    return exe;
}

void AutoProfileInfo::setActive(bool active)
{
    this->active = active;
}

bool AutoProfileInfo::isActive()
{
    return active;
}

void AutoProfileInfo::setDefaultState(bool value)
{
    this->defaultState = value;
}

bool AutoProfileInfo::isCurrentDefault()
{
    return defaultState;
}

void AutoProfileInfo::setDeviceName(QString name)
{
    this->deviceName = name;
}

QString AutoProfileInfo::getDeviceName()
{
    return deviceName;
}
