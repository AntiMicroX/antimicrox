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

#include "messagehandler.h"

#include <QFileInfo>
#include <QDebug>


AutoProfileInfo::AutoProfileInfo(QString guid, QString profileLocation,
                                 QString exe, bool active, QObject *parent) :
    QObject(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    setGUID(guid);
    setProfileLocation(profileLocation);
    setActive(active);
    setDefaultState(false);
}

AutoProfileInfo::AutoProfileInfo(QObject *parent) :
    QObject(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    setActive(true);
    setDefaultState(false);
}

AutoProfileInfo::~AutoProfileInfo()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);
}

void AutoProfileInfo::setGUID(QString guid)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->guid = guid;
}

QString AutoProfileInfo::getGUID() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return guid;
}

void AutoProfileInfo::setProfileLocation(QString profileLocation)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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

QString AutoProfileInfo::getProfileLocation() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return profileLocation;
}

void AutoProfileInfo::setExe(QString exe)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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

QString AutoProfileInfo::getExe() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return exe;
}

void AutoProfileInfo::setWindowClass(QString windowClass)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->windowClass = windowClass;
}

QString AutoProfileInfo::getWindowClass() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return windowClass;
}

void AutoProfileInfo::setWindowName(QString winName)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->windowName = winName;
}

QString AutoProfileInfo::getWindowName() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return windowName;
}

void AutoProfileInfo::setActive(bool active)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->active = active;
}

bool AutoProfileInfo::isActive()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return active;
}

void AutoProfileInfo::setDefaultState(bool value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->defaultState = value;
}

bool AutoProfileInfo::isCurrentDefault()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return defaultState;
}

void AutoProfileInfo::setDeviceName(QString name)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->deviceName = name;
}

QString AutoProfileInfo::getDeviceName() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return deviceName;
}
