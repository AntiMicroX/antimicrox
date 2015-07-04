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

#ifndef AUTOPROFILEINFO_H
#define AUTOPROFILEINFO_H

#include <QObject>
#include <QMetaType>

class AutoProfileInfo : public QObject
{
    Q_OBJECT
public:
    explicit AutoProfileInfo(QString guid, QString profileLocation,
                             bool active, QObject *parent = 0);
    explicit AutoProfileInfo(QString guid, QString profileLocation,
                             QString exe, bool active, QObject *parent = 0);
    explicit AutoProfileInfo(QObject *parent=0);
    ~AutoProfileInfo();

    void setGUID(QString guid);
    QString getGUID();

    void setProfileLocation(QString profileLocation);
    QString getProfileLocation();

    void setExe(QString exe);
    QString getExe();

    void setWindowClass(QString windowClass);
    QString getWindowClass();

    void setWindowName(QString winName);
    QString getWindowName();

    void setActive(bool active);
    bool isActive();

    void setDeviceName(QString name);
    QString getDeviceName();

    void setDefaultState(bool value);
    bool isCurrentDefault();

protected:
    QString guid;
    QString profileLocation;
    QString exe;
    QString deviceName;
    QString windowClass;
    QString windowName;
    bool active;
    bool defaultState;

signals:

public slots:

};

Q_DECLARE_METATYPE(AutoProfileInfo*)

#endif // AUTOPROFILEINFO_H
