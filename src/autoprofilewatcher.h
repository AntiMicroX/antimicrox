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

#ifndef AUTOPROFILEWATCHER_H
#define AUTOPROFILEWATCHER_H

#include <QObject>
#include <QTimer>
#include <QHash>
#include <QList>
#include <QSet>

#include "autoprofileinfo.h"
#include "antimicrosettings.h"


class AutoProfileWatcher : public QObject
{
    Q_OBJECT
public:
    explicit AutoProfileWatcher(AntiMicroSettings *settings, QObject *parent = 0);
    void startTimer();
    void stopTimer();
    QList<AutoProfileInfo*>* getCustomDefaults();
    AutoProfileInfo* getDefaultAllProfile();
    bool isGUIDLocked(QString guid);

    static const int CHECKTIME = 1000; // time in ms

protected:
    QString findAppLocation();
    void clearProfileAssignments();

    QTimer appTimer;
    AntiMicroSettings *settings;
    // Path, QList<AutoProfileInfo*>
    QHash<QString, QList<AutoProfileInfo*> > appProfileAssignments;
    // WM_CLASS, QList<AutoProfileInfo*>
    QHash<QString, QList<AutoProfileInfo*> > windowClassProfileAssignments;
    // WM_NAME, QList<AutoProfileInfo*>
    QHash<QString, QList<AutoProfileInfo*> > windowNameProfileAssignments;
    // GUID, AutoProfileInfo*
    QHash<QString, AutoProfileInfo*> defaultProfileAssignments;
    //QList<AutoProfileInfo*> *customDefaults;
    AutoProfileInfo *allDefaultInfo;
    QString currentApplication;
    QString currentAppWindowTitle;
    QSet<QString> guidSet;

signals:
    void foundApplicableProfile(AutoProfileInfo *info);

public slots:
    void syncProfileAssignment();

private slots:
    void runAppCheck();
};

#endif // AUTOPROFILEWATCHER_H
