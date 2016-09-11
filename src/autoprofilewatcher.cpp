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

//#include <QDebug>
#include <QListIterator>
#include <QStringListIterator>
#include <QSetIterator>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QApplication>

#include "autoprofilewatcher.h"

#if defined(Q_OS_UNIX) && defined(WITH_X11)
#include "x11extras.h"

#elif defined(Q_OS_WIN)
#include "winextras.h"

#endif

AutoProfileWatcher::AutoProfileWatcher(AntiMicroSettings *settings, QObject *parent) :
    QObject(parent)
{
    this->settings = settings;
    allDefaultInfo = 0;
    currentApplication = "";

    syncProfileAssignment();

    connect(&appTimer, SIGNAL(timeout()), this, SLOT(runAppCheck()));
}

void AutoProfileWatcher::startTimer()
{
    appTimer.start(CHECKTIME);
}

void AutoProfileWatcher::stopTimer()
{
    appTimer.stop();
}

void AutoProfileWatcher::runAppCheck()
{
    //qDebug() << qApp->applicationFilePath();
    QString appLocation;
    QString baseAppFileName;
    guidSet.clear();

    // Check whether program path needs to be parsed. Removes processing time
    // and need to run Linux specific code searching /proc.
#ifdef Q_OS_LINUX
    if (!appProfileAssignments.isEmpty())
    {
        appLocation = findAppLocation();
    }
#else
    // In Windows, get program location no matter what.
    appLocation = findAppLocation();
    if (!appLocation.isEmpty())
    {
        baseAppFileName = QFileInfo(appLocation).fileName();
    }
#endif

    // More portable check for whether antimicro is the current application
    // with focus.
    QWidget *focusedWidget = qApp->activeWindow();

    QString nowWindow;
    QString nowWindowClass;
    QString nowWindowName;

#ifdef Q_OS_WIN
    nowWindowName = WinExtras::getCurrentWindowText();
#else
    unsigned long currentWindow = X11Extras::getInstance()->getWindowInFocus();
    if (currentWindow > 0)
    {
        unsigned long tempWindow = X11Extras::getInstance()->findParentClient(currentWindow);
        if (tempWindow > 0)
        {
            currentWindow = tempWindow;
        }
        nowWindow = QString::number(currentWindow);
        nowWindowClass = X11Extras::getInstance()->getWindowClass(currentWindow);
        nowWindowName = X11Extras::getInstance()->getWindowTitle(currentWindow);
        //qDebug() << nowWindowClass;
        //qDebug() << nowWindowName;
    }
#endif

    bool checkForTitleChange = windowNameProfileAssignments.size() > 0;

#ifdef Q_OS_WIN
    if (!focusedWidget && ((!appLocation.isEmpty() && appLocation != currentApplication) ||
        (checkForTitleChange && nowWindowName != currentAppWindowTitle)))

#else
    if (!focusedWidget && ((!nowWindow.isEmpty() && nowWindow != currentApplication) ||
        (checkForTitleChange && nowWindowName != currentAppWindowTitle)))

#endif
    {

#ifdef Q_OS_WIN
        currentApplication = appLocation;
#else
        currentApplication = nowWindow;
#endif

        currentAppWindowTitle = nowWindowName;
        //currentApplication = appLocation;

	Logger::LogDebug(QObject::tr("Active window changed to: Title = \"%1\", "
				     "Class = \"%2\", Program = \"%3\" or \"%4\".").
			 arg(nowWindowName, nowWindowClass, appLocation, baseAppFileName));

        QSet<AutoProfileInfo*> fullSet;

        if (!appLocation.isEmpty() && appProfileAssignments.contains(appLocation))
        {
            QSet<AutoProfileInfo*> tempSet;
            tempSet = appProfileAssignments.value(appLocation).toSet();
            fullSet.unite(tempSet);
        }
        else if (!baseAppFileName.isEmpty() && appProfileAssignments.contains(baseAppFileName))
        {
            QSet<AutoProfileInfo*> tempSet;
            tempSet = appProfileAssignments.value(baseAppFileName).toSet();
            fullSet.unite(tempSet);
        }

        if (!nowWindowClass.isEmpty() && windowClassProfileAssignments.contains(nowWindowClass))
        {
            QSet<AutoProfileInfo*> tempSet;
            tempSet = windowClassProfileAssignments.value(nowWindowClass).toSet();
            fullSet.unite(tempSet);
        }

        if (!nowWindowName.isEmpty() && windowNameProfileAssignments.contains(nowWindowName))
        {
            QSet<AutoProfileInfo*> tempSet;
            tempSet = windowNameProfileAssignments.value(nowWindowName).toSet();
            fullSet = fullSet.unite(tempSet);
        }

        QHash<QString, int> highestMatchCount;
        QHash<QString, AutoProfileInfo*> highestMatches;

        QSetIterator<AutoProfileInfo*> fullSetIter(fullSet);
        while (fullSetIter.hasNext())
        {
            AutoProfileInfo *info = fullSetIter.next();
            if (info->isActive())
            {
                int numProps = 0;
                numProps += !info->getExe().isEmpty() ? 1 : 0;
                numProps += !info->getWindowClass().isEmpty() ? 1 : 0;
                numProps += !info->getWindowName().isEmpty() ? 1 : 0;

                int numMatched = 0;
                numMatched += (!info->getExe().isEmpty() &&
                               (info->getExe() == appLocation ||
                                info->getExe() == baseAppFileName)) ? 1 : 0;
                numMatched += (!info->getWindowClass().isEmpty() &&
                               info->getWindowClass() == nowWindowClass) ? 1 : 0;
                numMatched += (!info->getWindowName().isEmpty() &&
                               info->getWindowName() == nowWindowName) ? 1 : 0;

                if (numProps == numMatched)
                {
                    if (highestMatchCount.contains(info->getGUID()))
                    {
                        int currentHigh = highestMatchCount.value(info->getGUID());
                        if (numMatched > currentHigh)
                        {
                            highestMatchCount.insert(info->getGUID(), numMatched);
                            highestMatches.insert(info->getGUID(), info);
                        }
                    }
                    else
                    {
                        highestMatchCount.insert(info->getGUID(), numMatched);
                        highestMatches.insert(info->getGUID(), info);
                    }
                }
            }
        }

        QHashIterator<QString, AutoProfileInfo*> highIter(highestMatches);
        while (highIter.hasNext())
        {
            AutoProfileInfo *info = highIter.next().value();
            guidSet.insert(info->getGUID());
            emit foundApplicableProfile(info);
        }

        if ((!defaultProfileAssignments.isEmpty() || allDefaultInfo) && !focusedWidget)
             //antiProgramLocation != appLocation)
        {
            if (allDefaultInfo)
            {
                if (allDefaultInfo->isActive() && !guidSet.contains("all"))
                {
                    emit foundApplicableProfile(allDefaultInfo);
                }
            }

            QHashIterator<QString, AutoProfileInfo*> iter(defaultProfileAssignments);
            while (iter.hasNext())
            {
                iter.next();
                AutoProfileInfo *info = iter.value();
                if (info->isActive() && !guidSet.contains(info->getGUID()))
                {
                    emit foundApplicableProfile(info);
                }
            }
        }
    }
}

void AutoProfileWatcher::syncProfileAssignment()
{
    clearProfileAssignments();

    currentApplication = "";

    //QStringList assignments = settings->allKeys();
    //QStringListIterator iter(assignments);

    settings->getLock()->lock();
    settings->beginGroup("DefaultAutoProfiles");
    QString exe;
    QString guid;
    QString profile;
    QString active;
    QString windowClass;
    QString windowName;

    QStringList registeredGUIDs = settings->value("GUIDs", QStringList()).toStringList();
    //QStringList defaultkeys = settings->allKeys();
    settings->endGroup();

    QString allProfile = settings->value(QString("DefaultAutoProfileAll/Profile"), "").toString();
    QString allActive = settings->value(QString("DefaultAutoProfileAll/Active"), "0").toString();

    // Handle overall Default profile assignment
    bool defaultActive = allActive == "1" ? true : false;
    if (defaultActive)
    {
        allDefaultInfo = new AutoProfileInfo("all", allProfile, defaultActive, this);
        allDefaultInfo->setDefaultState(true);
    }

    // Handle device specific Default profile assignments
    QStringListIterator iter(registeredGUIDs);
    while (iter.hasNext())
    {
        QString tempkey = iter.next();
        QString guid = QString(tempkey).replace("GUID", "");

        QString profile = settings->value(QString("DefaultAutoProfile-%1/Profile").arg(guid), "").toString();
        QString active = settings->value(QString("DefaultAutoProfile-%1/Active").arg(guid), "").toString();

        if (!guid.isEmpty() && !profile.isEmpty())
        {
            bool profileActive = active == "1" ? true : false;
            if (profileActive && guid != "all")
            {
                AutoProfileInfo *info = new AutoProfileInfo(guid, profile, profileActive, this);
                info->setDefaultState(true);
                defaultProfileAssignments.insert(guid, info);
            }
        }
    }

    settings->beginGroup("AutoProfiles");
    bool quitSearch = false;

    //QHash<QString, QList<QString> > tempAssociation;
    for (int i = 1; !quitSearch; i++)
    {
        exe = settings->value(QString("AutoProfile%1Exe").arg(i), "").toString();
        exe = QDir::toNativeSeparators(exe);
        guid = settings->value(QString("AutoProfile%1GUID").arg(i), "").toString();
        profile = settings->value(QString("AutoProfile%1Profile").arg(i), "").toString();
        active = settings->value(QString("AutoProfile%1Active").arg(i), 0).toString();
        windowName = settings->value(QString("AutoProfile%1WindowName").arg(i), "").toString();
#ifdef Q_OS_UNIX
        windowClass = settings->value(QString("AutoProfile%1WindowClass").arg(i), "").toString();
#else
        windowClass.clear();
#endif

        // Check if all required elements exist. If not, assume that the end of the
        // list has been reached.
        if ((!exe.isEmpty() || !windowClass.isEmpty() || !windowName.isEmpty()) &&
            !guid.isEmpty())
        {
            bool profileActive = active == "1" ? true : false;
            if (profileActive)
            {
                AutoProfileInfo *info = new AutoProfileInfo(guid, profile, profileActive, this);

                if (!windowClass.isEmpty())
                {
                    info->setWindowClass(windowClass);

                    QList<AutoProfileInfo*> templist;
                    if (windowClassProfileAssignments.contains(windowClass))
                    {
                        templist = windowClassProfileAssignments.value(windowClass);
                    }
                    templist.append(info);
                    windowClassProfileAssignments.insert(windowClass, templist);
                }

                if (!windowName.isEmpty())
                {
                    info->setWindowName(windowName);

                    QList<AutoProfileInfo*> templist;
                    if (windowNameProfileAssignments.contains(windowName))
                    {
                        templist = windowNameProfileAssignments.value(windowName);
                    }
                    templist.append(info);
                    windowNameProfileAssignments.insert(windowName, templist);
                }

                if (!exe.isEmpty())
                {
                    info->setExe(exe);

                    QList<AutoProfileInfo*> templist;
                    if (appProfileAssignments.contains(exe))
                    {
                        templist = appProfileAssignments.value(exe);
                    }
                    templist.append(info);
                    appProfileAssignments.insert(exe, templist);

                    QString baseExe = QFileInfo(exe).fileName();
                    if (!baseExe.isEmpty() && baseExe != exe)
                    {
                        QList<AutoProfileInfo*> templist;
                        if (appProfileAssignments.contains(baseExe))
                        {
                            templist = appProfileAssignments.value(baseExe);
                        }
                        templist.append(info);
                        appProfileAssignments.insert(baseExe, templist);
                    }
                }
            }
        }
        else
        {
            quitSearch = true;
        }
    }

    settings->endGroup();
    settings->getLock()->unlock();
}

void AutoProfileWatcher::clearProfileAssignments()
{
    QSet<AutoProfileInfo*> terminateProfiles;

    QListIterator<QList<AutoProfileInfo*> > iterDelete(appProfileAssignments.values());
    while (iterDelete.hasNext())
    {
        QList<AutoProfileInfo*> templist = iterDelete.next();
        terminateProfiles.unite(templist.toSet());
    }
    appProfileAssignments.clear();

    QListIterator<QList<AutoProfileInfo*> > iterClassDelete(windowClassProfileAssignments.values());
    while (iterClassDelete.hasNext())
    {
        QList<AutoProfileInfo*> templist = iterClassDelete.next();
        terminateProfiles.unite(templist.toSet());
    }
    windowClassProfileAssignments.clear();

    QListIterator<QList<AutoProfileInfo*> > iterNameDelete(windowNameProfileAssignments.values());
    while (iterNameDelete.hasNext())
    {
        QList<AutoProfileInfo*> templist = iterNameDelete.next();
        terminateProfiles.unite(templist.toSet());
    }
    windowNameProfileAssignments.clear();

    QSetIterator<AutoProfileInfo*> iterTerminate(terminateProfiles);
    while (iterTerminate.hasNext())
    {
        AutoProfileInfo *info = iterTerminate.next();
        if (info)
        {
            delete info;
            info = 0;
        }
    }

    QListIterator<AutoProfileInfo*> iterDefaultsDelete(defaultProfileAssignments.values());
    while (iterDefaultsDelete.hasNext())
    {
        AutoProfileInfo *info = iterDefaultsDelete.next();
        if (info)
        {
            delete info;
            info = 0;
        }
    }
    defaultProfileAssignments.clear();

    allDefaultInfo = 0;
    guidSet.clear();
}

QString AutoProfileWatcher::findAppLocation()
{
    QString exepath;

#if defined(Q_OS_LINUX)
    #ifdef WITH_X11
    Window currentWindow = 0;
    int pid = 0;

    currentWindow = X11Extras::getInstance()->getWindowInFocus();
    if (currentWindow)
    {
        pid = X11Extras::getInstance()->getApplicationPid(currentWindow);
    }

    if (pid > 0)
    {
        exepath = X11Extras::getInstance()->getApplicationLocation(pid);
    }
    #endif

#elif defined(Q_OS_WIN)
    exepath = WinExtras::getForegroundWindowExePath();
    //qDebug() << exepath;
#endif

    return exepath;
}

QList<AutoProfileInfo*>* AutoProfileWatcher::getCustomDefaults()
{
    QList<AutoProfileInfo*> *temp = new QList<AutoProfileInfo*>();
    QHashIterator<QString, AutoProfileInfo*> iter(defaultProfileAssignments);
    while (iter.hasNext())
    {
        iter.next();
        temp->append(iter.value());
    }
    return temp;
}

AutoProfileInfo* AutoProfileWatcher::getDefaultAllProfile()
{
    return allDefaultInfo;
}

bool AutoProfileWatcher::isGUIDLocked(QString guid)
{
    return guidSet.contains(guid);
}
