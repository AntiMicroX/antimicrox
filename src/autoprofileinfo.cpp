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
    else if (profileLocation.isEmpty())
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
    if (!exe.isEmpty())
    {
        QFileInfo info(exe);
        if (exe != this->exe && info.exists() && info.isExecutable())
        {
            this->exe = exe;
        }
#ifdef Q_OS_WIN
        else if (exe != this->exe && info.suffix() == "exe")
        {
            this->exe = exe;
        }
#endif
    }
    else
    {
        this->exe = exe;
    }
}

QString AutoProfileInfo::getExe()
{
    return exe;
}

void AutoProfileInfo::setWindowClass(QString windowClass)
{
    this->windowClass = windowClass;
}

QString AutoProfileInfo::getWindowClass()
{
    return windowClass;
}

void AutoProfileInfo::setWindowName(QString winName)
{
    this->windowName = winName;
}

QString AutoProfileInfo::getWindowName()
{
    return windowName;
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
