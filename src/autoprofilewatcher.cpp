#include <QStringListIterator>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QListIterator>
#include <QApplication>
#include <unistd.h>

#include "autoprofilewatcher.h"

#if defined(Q_OS_UNIX)
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "x11info.h"

#elif defined(Q_OS_WIN)
#include "wininfo.h"

#endif

AutoProfileWatcher::AutoProfileWatcher(QSettings *settings, QObject *parent) :
    QObject(parent)
{
    this->settings = settings;
    allDefaultInfo = 0;

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
                    QString guid = info->getGUID();
                    QString profileLocation = info->getProfileLocation();
                    emit foundApplicableProfile(guid, profileLocation);
                }
            }
        }
        else if ((!defaultProfileAssignments.isEmpty() || allDefaultInfo) &&
                 qApp->applicationFilePath() != appLocation)
        {
            if (allDefaultInfo)
            {
                QString guid = allDefaultInfo->getGUID();
                QString profileLocation = allDefaultInfo->getProfileLocation();
                emit foundApplicableProfile(guid, profileLocation);
            }

            QHashIterator<QString, AutoProfileInfo*> iter(defaultProfileAssignments);
            while (iter.hasNext())
            {
                iter.next();
                AutoProfileInfo *info = iter.value();
                if (info->isActive())
                {
                    QString guid = info->getGUID();
                    QString profileLocation = info->getProfileLocation();
                    emit foundApplicableProfile(guid, profileLocation);
                }
            }

            currentApplication = "";
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

    QStringList defaultkeys = settings->allKeys();
    settings->endGroup();

    QString allProfile = settings->value(QString("DefaultAutoProfileAll/Profile"), "").toString();
    QString allActive = settings->value(QString("DefaultAutoProfileAll/Active"), "0").toString();

    if (!allProfile.isEmpty())
    {
        bool defaultActive = allActive == "1" ? true : false;
        allDefaultInfo = new AutoProfileInfo("all", profile, defaultActive, this);
        allDefaultInfo->setDefaultState(true);
    }

    QStringListIterator iter(defaultkeys);
    while (iter.hasNext())
    {
        QString tempkey = iter.next();
        QString guid = QString(tempkey).replace("GUID", "");

        QString profile = settings->value(QString("DefaultAutoProfile-%1/Profile").arg(guid), "").toString();
        QString active = settings->value(QString("DefaultAutoProfile-%1/Active").arg(guid), "").toString();

        if (!guid.isEmpty() && !profile.isEmpty())
        {
            bool profileActive = active == "1" ? true : false;
            if (guid != "all")
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
        guid = settings->value(QString("AutoProfile%1GUID").arg(i), "").toString();
        profile = settings->value(QString("AutoProfile%1Profile").arg(i), "").toString();
        active = settings->value(QString("AutoProfile%1Active").arg(i), 0).toString();

        // Check if all required elements exist. If not, assume that the end of the
        // list has been reached.
        if (!exe.isEmpty() && !guid.isEmpty() && !profile.isEmpty())
        {
            bool profileActive = active == "1" ? true : false;
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
    Window currentWindow = 0;
    int focusState = 0;

    Display *display = X11Info::display();
    XGetInputFocus(display, &currentWindow, &focusState);
    int pid = X11Info::getApplicationPid(currentWindow);
    if (pid > 0)
    {
        exepath = X11Info::getApplicationLocation(pid);
    }

#elif defined(Q_OS_WIN)
    exepath = WinInfo::getForegroundWindowExePath();
    //qDebug() << exepath;
#endif

    /*atom = XInternAtom(display, "_NET_WM_PID", True);
    status = XGetWindowProperty(display, currentWindow, atom, 0, 1024, false, AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &prop);
    if (status == 0 && prop)
    {
        pid = prop[1] << 8;
        pid += prop[0];
        qDebug() << status;
        qDebug() << pid;
        XFree(prop);
    }
    else if (status == 0)
    {
        XFree(prop);

        Window parent = 1;
        Window root = 0;
        Window * children;
        unsigned int num_children;
        bool quitTraversal = false;

        while (!quitTraversal)
        {
            XQueryTree(display, currentWindow, &root, &parent, &children, &num_children);
            if (children) { //must test for null
                XFree(children);
            }
            status = XGetWindowProperty(display, parent, atom, 0, 1024, false, AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &prop);
            if (status == 0 && prop)
            {
                pid = prop[1] * 256;
                pid += prop[0];
                qDebug() << status;
                qDebug() << pid;

                quitTraversal = true;
            }
            else if (parent == root)
            {
                quitTraversal = true;
            }
            else
            {
                currentWindow = parent;
            }

            XFree(prop);
        }
    }

    qDebug() << "PID: " << pid;
    if (pid > 0)
    {
        QString procString = QString("/proc/%1/exe").arg(pid);
        char buf[1024];
        QByteArray tempByteArray = procString.toLocal8Bit();
        ssize_t len = readlink(tempByteArray.constData(), buf, sizeof(buf)-1);
        if (len != -1)
        {
            buf[len] = '\0';
        }
        exepath = QString::fromLocal8Bit(buf);
        qDebug() << exepath;
        qDebug() << "REBEL REBEL";
    }*/

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
