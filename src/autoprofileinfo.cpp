/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail.com>
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

#include <QDebug>
#include <QFileInfo>
#include <utility>

AutoProfileInfo::AutoProfileInfo(QString uniqueID, QString profileLocation, QString exe, bool active, bool partialTitle,
                                 QObject *parent)
    : QObject(parent)
{
    setUniqueID(std::move(uniqueID));
    setProfileLocation(std::move(profileLocation));
    setExe(std::move(exe));
    setActive(active);
    setDefaultState(false);
    setPartialState(partialTitle);
}

AutoProfileInfo::AutoProfileInfo(QString uniqueID, QString profileLocation, bool active, bool partialTitle, QObject *parent)
    : QObject(parent)
{
    setUniqueID(std::move(uniqueID));
    setProfileLocation(std::move(profileLocation));
    setActive(active);
    setDefaultState(false);
    setPartialState(partialTitle);
}

AutoProfileInfo::AutoProfileInfo(QObject *parent)
    : QObject(parent)
{
    setActive(true);
    setDefaultState(false);
    setPartialState(false);
}

AutoProfileInfo::~AutoProfileInfo() {}

void AutoProfileInfo::setUniqueID(QString uniqueID) { this->uniqueID = std::move(uniqueID); }

QString AutoProfileInfo::getUniqueID() const { return uniqueID; }

void AutoProfileInfo::setProfileLocation(const QString &profileLocation)
{
    QFileInfo info(profileLocation);

    if ((profileLocation != this->profileLocation) && info.exists() && info.isReadable())
    {
        this->profileLocation = profileLocation;
    } else if (profileLocation.isEmpty())
    {
        this->profileLocation = "";
    }
}

QString AutoProfileInfo::getProfileLocation() const { return profileLocation; }

void AutoProfileInfo::setExe(const QString &exe)
{
    if (!exe.isEmpty())
    {
        QFileInfo info(exe);

        if ((exe != this->exe) && info.exists() && info.isExecutable())
        {
            this->exe = exe;
        }
#ifdef Q_OS_WIN
        else if (exe != this->exe && info.suffix() == "exe")
        {
            this->exe = exe;
        }
#endif
    } else
    {
        this->exe = exe;
    }
}

QString AutoProfileInfo::getExe() const { return exe; }

void AutoProfileInfo::setWindowClass(QString windowClass) { this->windowClass = std::move(windowClass); }

QString AutoProfileInfo::getWindowClass() const { return windowClass; }

void AutoProfileInfo::setWindowName(QString winName) { this->windowName = std::move(winName); }

QString AutoProfileInfo::getWindowName() const { return windowName; }

void AutoProfileInfo::setActive(bool active) { this->active = active; }

bool AutoProfileInfo::isActive() { return active; }

void AutoProfileInfo::setDefaultState(bool value) { this->defaultState = value; }

/**
 * @brief is this autoprofile loaded by default?
 * There is one default profile for all of controllers and there can be also some default profiles for selected controllers
 */
bool AutoProfileInfo::isCurrentDefault() { return defaultState; }

void AutoProfileInfo::setDeviceName(QString name) { this->deviceName = std::move(name); }

QString AutoProfileInfo::getDeviceName() const
{
    if (deviceName.isNull())
        return "";
    else
        return deviceName;
}

void AutoProfileInfo::setPartialState(bool value) { this->partialState = value; }

bool AutoProfileInfo::isPartialState() { return partialState; }

QString AutoProfileInfo::toString() const
{
    return QString("ID of assigned controller:%1, Profile Location:%2, Exe:%3,WindowClass:%4, WindowName:%5, isActive:%6, "
                   "DeviceName:%7")
        .arg(uniqueID, profileLocation, exe, windowClass, windowName, active ? "true" : "false", deviceName);
}
