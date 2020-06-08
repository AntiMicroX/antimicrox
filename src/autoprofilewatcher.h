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


#ifndef AUTOPROFILEWATCHER_H
#define AUTOPROFILEWATCHER_H

#include <QTimer>
#include <QHash>
#include <QSet>

class AntiMicroSettings;
class AutoProfileInfo;
class QSettings;

class AutoProfileWatcher : public QObject
{
    Q_OBJECT

public:
    explicit AutoProfileWatcher(AntiMicroSettings *settings, QObject *parent = nullptr);
    ~AutoProfileWatcher();

    void startTimer();
    void stopTimer();
    static void disconnectWindowTimer();
    static AutoProfileWatcher* getAutoProfileWatcherInstance();
    QList<AutoProfileInfo*>* getCustomDefaults();
    AutoProfileInfo* getDefaultAllProfile();
    //bool isGUIDLocked(QString guid);
    bool isUniqueIDLocked(QString uniqueID);
    QHash<QString, QList<AutoProfileInfo*> > const& getAppProfileAssignments();
    QHash<QString, QList<AutoProfileInfo*> > const& getWindowClassProfileAssignments();
    QHash<QString, QList<AutoProfileInfo*> > const& getWindowNameProfileAssignments();
    QHash<QString, AutoProfileInfo*> const& getDefaultProfileAssignments();

    static const int CHECKTIME = 500; // time in ms


protected:
    QString findAppLocation();
    void clearProfileAssignments();
    void convToUniqueIDAutoProfGroupSett(QSettings* sett, QString guidAutoProfSett, QString uniqueAutoProfSett);

signals:
    void foundApplicableProfile(AutoProfileInfo *info);

public slots:
    void syncProfileAssignment();

private slots:
    void runAppCheck();

private:
    //QSet<QString>& getGuidSetLocal();
    QSet<QString>& getUniqeIDSetLocal();

    static AutoProfileWatcher* _instance;
    static QTimer checkWindowTimer;
    AntiMicroSettings *settings;
    QHash<QString, QList<AutoProfileInfo*> > appProfileAssignments;
    QHash<QString, QList<AutoProfileInfo*> > windowClassProfileAssignments;
    QHash<QString, QList<AutoProfileInfo*> > windowNameProfileAssignments;
    QHash<QString, AutoProfileInfo*> defaultProfileAssignments;
    AutoProfileInfo *allDefaultInfo;
    QString currentApplication;
    QString currentAppWindowTitle;
    //QSet<QString> guidSet;
    QSet<QString> uniqueIDSet;
};

#endif // AUTOPROFILEWATCHER_H
