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

#include "autoprofileinfo.h"

#include <QFileInfo>
#include <QDebug>


AutoProfileInfo::AutoProfileInfo(QString guid, QString profileLocation,
                                 QString exe, bool active, QObject *parent) :
    QObject(parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    setGUID(guid);
    setProfileLocation(profileLocation);
    setActive(active);
    setDefaultState(false);
}

AutoProfileInfo::AutoProfileInfo(QObject *parent) :
    QObject(parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    setActive(true);
    setDefaultState(false);
}

AutoProfileInfo::~AutoProfileInfo()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
}

void AutoProfileInfo::setGUID(QString guid)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->guid = guid;
}

QString AutoProfileInfo::getGUID()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return guid;
}

void AutoProfileInfo::setProfileLocation(QString profileLocation)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QFileInfo info(profileLocation);
    if ((profileLocation != this->profileLocation) &&
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return profileLocation;
}

void AutoProfileInfo::setExe(QString exe)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (!exe.isEmpty())
    {
        QFileInfo info(exe);
        if ((exe != this->exe) && info.exists() && info.isExecutable())
        {
            this->exe = exe;
        }
#ifdef Q_OS_WIN
        else if ((exe != this->exe) && (info.suffix() == "exe"))
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return exe;
}

void AutoProfileInfo::setWindowClass(QString windowClass)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->windowClass = windowClass;
}

QString AutoProfileInfo::getWindowClass()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return windowClass;
}

void AutoProfileInfo::setWindowName(QString winName)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->windowName = winName;
}

QString AutoProfileInfo::getWindowName()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return windowName;
}

void AutoProfileInfo::setActive(bool active)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->active = active;
}

bool AutoProfileInfo::isActive()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return active;
}

void AutoProfileInfo::setDefaultState(bool value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->defaultState = value;
}

bool AutoProfileInfo::isCurrentDefault()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return defaultState;
}

void AutoProfileInfo::setDeviceName(QString name)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->deviceName = name;
}

QString AutoProfileInfo::getDeviceName()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return deviceName;
}
