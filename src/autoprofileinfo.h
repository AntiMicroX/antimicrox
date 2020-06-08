/* antimicroX Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
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

class AutoProfileInfo : public QObject
{

    Q_OBJECT

public:
    explicit AutoProfileInfo(QString uniqueID, QString profileLocation,
                             bool active, bool partialTitle, QObject *parent = nullptr);
    explicit AutoProfileInfo(QString uniqueID, QString profileLocation,
                             QString exe, bool active, bool partialTitle, QObject *parent = nullptr);
    explicit AutoProfileInfo(QObject *parent=0);
    ~AutoProfileInfo();

   // void setGUID(QString guid);
   // QString getGUID() const;

    void setUniqueID(QString guid);
    QString getUniqueID() const;

    void setProfileLocation(QString profileLocation);
    QString getProfileLocation() const;

    void setExe(QString exe);
    QString getExe() const;

    void setWindowClass(QString windowClass);
    QString getWindowClass() const;

    void setWindowName(QString winName);
    QString getWindowName() const;

    void setActive(bool active);
    bool isActive();

    void setDeviceName(QString name);
    QString getDeviceName() const;

    void setDefaultState(bool value);
    bool isCurrentDefault();

    void setPartialState(bool value);
    bool isPartialState();

private:
    QString guid;
    QString uniqueID;
    QString profileLocation;
    QString exe;
    QString deviceName;
    QString windowClass;
    QString windowName;
    bool active;
    bool defaultState;
    bool partialState;

};

Q_DECLARE_METATYPE(AutoProfileInfo*)

#endif // AUTOPROFILEINFO_H
