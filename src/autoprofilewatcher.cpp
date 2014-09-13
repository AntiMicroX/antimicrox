#include <QDebug>
#include <QStringListIterator>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QListIterator>
#include <QApplication>

#include "autoprofilewatcher.h"

#if defined(Q_OS_UNIX) && defined(WITH_X11)
#include "x11info.h"

#elif defined(Q_OS_WIN)
#include "wininfo.h"

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
    QString appLocation = findAppLocation();
    QString antiProgramLocation = QDir::toNativeSeparators(qApp->applicationFilePath());
    if (!appLocation.isEmpty() && appLocation != currentApplication)
    {
        currentApplication = appLocation;

        if (appProfileAssignments.contains(appLocation))
        {
            QList<AutoProfileInfo*> autoentries = appProfileAssignments.value(appLocation);
            QListIterator<AutoProfileInfo*> iter(autoentries);
            while (iter.hasNext())
            {
                AutoProfileInfo *info = iter.next();
                if (info->isActive())
                {
                    emit foundApplicableProfile(info);
                }
            }
        }
        else if ((!defaultProfileAssignments.isEmpty() || allDefaultInfo) &&
                 antiProgramLocation != appLocation)
        {
            if (allDefaultInfo)
            {
                if (allDefaultInfo->isActive())
                {
                    emit foundApplicableProfile(allDefaultInfo);
                }
            }

            QHashIterator<QString, AutoProfileInfo*> iter(defaultProfileAssignments);
            while (iter.hasNext())
            {
                iter.next();
                AutoProfileInfo *info = iter.value();
                if (info->isActive())
                {
                    emit foundApplicableProfile(info);
                }
            }

            //currentApplication = "";
        }
    }
}

void AutoProfileWatcher::syncProfileAssignment()
{
    QListIterator<QList<AutoProfileInfo*> > iterDelete(appProfileAssignments.values());
    while (iterDelete.hasNext())
    {
        QList<AutoProfileInfo*> templist = iterDelete.next();
        QListIterator<AutoProfileInfo*> iterAuto(templist);
        while (iterAuto.hasNext())
        {
            AutoProfileInfo *info = iterAuto.next();
            if (info)
            {
                delete info;
                info = 0;
            }
        }
    }
    appProfileAssignments.clear();

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
    currentApplication = "";

    //QStringList assignments = settings->allKeys();
    //QStringListIterator iter(assignments);

    settings->beginGroup("DefaultAutoProfiles");
    QString exe;
    QString guid;
    QString profile;
    QString active;

    QStringList registeredGUIDs = settings->value("GUIDs", QStringList()).toStringList();
    //QStringList defaultkeys = settings->allKeys();
    settings->endGroup();

    QString allProfile = settings->value(QString("DefaultAutoProfileAll/Profile"), "").toString();
    QString allActive = settings->value(QString("DefaultAutoProfileAll/Active"), "0").toString();

    bool defaultActive = allActive == "1" ? true : false;
    if (defaultActive)
    {
        allDefaultInfo = new AutoProfileInfo("all", allProfile, defaultActive, this);
        allDefaultInfo->setDefaultState(true);
    }

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

    QHash<QString, QList<QString> > tempAssociation;
    for (int i = 1; !quitSearch; i++)
    {
        exe = settings->value(QString("AutoProfile%1Exe").arg(i), "").toString();
        exe = QDir::toNativeSeparators(exe);
        guid = settings->value(QString("AutoProfile%1GUID").arg(i), "").toString();
        profile = settings->value(QString("AutoProfile%1Profile").arg(i), "").toString();
        active = settings->value(QString("AutoProfile%1Active").arg(i), 0).toString();

        // Check if all required elements exist. If not, assume that the end of the
        // list has been reached.
        if (!exe.isEmpty() && !guid.isEmpty())
        {
            bool profileActive = active == "1" ? true : false;
            if (profileActive)
            {
                QList<AutoProfileInfo*> templist;
                if (appProfileAssignments.contains(exe))
                {
                    templist = appProfileAssignments.value(exe);
                }

                QList<QString> tempguids;
                if (tempAssociation.contains(exe))
                {
                    tempguids = tempAssociation.value(exe);
                }

                if (!tempguids.contains(guid))
                {
                    AutoProfileInfo *info = new AutoProfileInfo(guid, profile, profileActive, this);
                    tempguids.append(guid);
                    tempAssociation.insert(exe, tempguids);
                    templist.append(info);
                    appProfileAssignments.insert(exe, templist);
                }
            }
        }
        else
        {
            quitSearch = true;
        }
    }

    settings->endGroup();
}

QString AutoProfileWatcher::findAppLocation()
{
    QString exepath;

#if defined(Q_OS_UNIX)
    #ifdef WITH_X11
    Window currentWindow = 0;
    int focusState = 0;
    int pid = 0;

    Display *display = X11Info::getInstance()->display();
    XGetInputFocus(display, &currentWindow, &focusState);
    if (currentWindow)
    {
        pid = X11Info::getInstance()->getApplicationPid(currentWindow);
    }

    if (pid > 0)
    {
        exepath = X11Info::getInstance()->getApplicationLocation(pid);
    }
    #endif

#elif defined(Q_OS_WIN)
    exepath = WinInfo::getForegroundWindowExePath();
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
