/* antimicroX Gamepad to KB+M event mapper
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

#include "autoprofilewatcher.h"

#include "messagehandler.h"
#include "autoprofileinfo.h"
#include "antimicrosettings.h"

#include <QDebug>
#include <QListIterator>
#include <QStringListIterator>
#include <QSetIterator>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QApplication>

#if defined(WITH_X11)
    #include "x11extras.h"
#endif


AutoProfileWatcher* AutoProfileWatcher::_instance = nullptr;
QTimer AutoProfileWatcher::checkWindowTimer;


AutoProfileWatcher::AutoProfileWatcher(AntiMicroSettings *settings, QObject *parent) :
    QObject(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->settings = settings;
    allDefaultInfo = nullptr;
    currentApplication = "";
    _instance = this;

    syncProfileAssignment();

    connect(&(checkWindowTimer), &QTimer::timeout, _instance, &AutoProfileWatcher::runAppCheck);
}

AutoProfileWatcher::~AutoProfileWatcher()
{
    if (checkWindowTimer.isActive()) {

        checkWindowTimer.stop();
        disconnect(&(checkWindowTimer), &QTimer::timeout, _instance, nullptr);
    }

    _instance = nullptr;
}

AutoProfileWatcher* AutoProfileWatcher::getAutoProfileWatcherInstance()
{
    return _instance;
}

void AutoProfileWatcher::disconnectWindowTimer()
{
    checkWindowTimer.stop();
    disconnect(&(checkWindowTimer), &QTimer::timeout, _instance, nullptr);
}

void AutoProfileWatcher::startTimer()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    checkWindowTimer.start(CHECKTIME);
}

void AutoProfileWatcher::stopTimer()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    checkWindowTimer.stop();
}


void AutoProfileWatcher::runAppCheck()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);


        qDebug() << qApp->applicationFilePath();


    QString appLocation = QString();
    QString baseAppFileName = QString();
    getUniqeIDSetLocal().clear();

    // Check whether program path needs to be parsed. Removes processing time
    // and need to run Linux specific code searching /proc.
    if (!getAppProfileAssignments().isEmpty())
    {
        appLocation = findAppLocation();
        qDebug() << "appLocation is " << appLocation << endl;
    }

    // More portable check for whether antimicroX is the current application
    // with focus.
    QWidget *focusedWidget = qApp->activeWindow();
    if (focusedWidget != nullptr) qDebug() << "get active window of app" << endl;
    QString nowWindow = QString();
    QString nowWindowClass = QString();
    QString nowWindowName = QString();

    long currentWindow = X11Extras::getInstance()->getWindowInFocus();
    qDebug() << "getWindowInFocus: " << currentWindow << endl;

    if (currentWindow > 0)
    {
        long tempWindow = X11Extras::getInstance()->findParentClient(currentWindow);
        qDebug() << "findParentClient: " << tempWindow << endl;

        if (tempWindow > 0) currentWindow = tempWindow;

        nowWindow = QString::number(currentWindow);
        qDebug() << "number of window now: " << nowWindow << endl;

        nowWindowClass = X11Extras::getInstance()->getWindowClass(static_cast<Window>(currentWindow));
        qDebug() << "class of window now: " << nowWindowClass << endl;

        nowWindowName = X11Extras::getInstance()->getWindowTitle(static_cast<Window>(currentWindow));
        qDebug() << "title of window now: " << nowWindowName << endl;
    }

    qDebug() << "WINDOW CLASS: " << nowWindowClass;
    qDebug() << "WINDOW NAME: " << nowWindowName;
    qDebug() << "WINDOW IN FOCUS: " << nowWindow;

    bool checkForTitleChange = getWindowNameProfileAssignments().size() > 0;

    qDebug() << "window profile assignments size: " << getWindowNameProfileAssignments().size() << endl;

    qDebug() << "checkForTitleChange: " << checkForTitleChange;

    if (!focusedWidget && ((!nowWindow.isEmpty() && (nowWindow != currentApplication)) ||
        (checkForTitleChange && (nowWindowName != currentAppWindowTitle))))
    {

        currentApplication = nowWindow;
        currentAppWindowTitle = nowWindowName;

        Logger::LogDebug(QObject::tr("Active window changed to: Title = \"%1\", "
                     "Class = \"%2\", Program = \"%3\" or \"%4\".").
             arg(nowWindowName, nowWindowClass, appLocation, baseAppFileName));

        QSet<AutoProfileInfo*> fullSet;

        if (!appLocation.isEmpty() && getAppProfileAssignments().contains(appLocation))
        {
            QSet<AutoProfileInfo*> tempSet;
            tempSet = getAppProfileAssignments().value(appLocation).toSet();
            fullSet.unite(tempSet);
        }
        else if (!baseAppFileName.isEmpty() && getAppProfileAssignments().contains(baseAppFileName))
        {
            QSet<AutoProfileInfo*> tempSet;
            tempSet = getAppProfileAssignments().value(baseAppFileName).toSet();
            fullSet.unite(tempSet);
        }

        if (!nowWindowClass.isEmpty() && getWindowClassProfileAssignments().contains(nowWindowClass))
        {
            QSet<AutoProfileInfo*> tempSet;
            tempSet = getWindowClassProfileAssignments().value(nowWindowClass).toSet();
            fullSet.unite(tempSet);
        }

        // part window title
        if (!nowWindowName.isEmpty())
        {
            QHashIterator< QString, QList< AutoProfileInfo *> > iter(getWindowNameProfileAssignments());
            while (iter.hasNext())
            {
                iter.next();

                bool hasOnePartName = false;

                QListIterator< AutoProfileInfo* > iterList(iter.value());
                while (iterList.hasNext()) {

                    AutoProfileInfo* autoInfo = iterList.next();

                    if (autoInfo->isPartialState()) {
                        hasOnePartName = true;
                        break;
                    }
                }

                if (hasOnePartName) {

                    qDebug() << "IT HAS A PARTIAL TITLE NAME";

                    if (nowWindowName.contains(iter.key())) {

                        qDebug() << "WINDOW: \"" << nowWindowName << "\" includes \"" << iter.key() << "\"";

                        QSet< AutoProfileInfo* > tempSet;
                        QList< AutoProfileInfo *> list = iter.value();
                        tempSet = list.toSet();
                        fullSet = fullSet.unite(tempSet);

                    } else {

                        qDebug() << "WINDOW: \"" << nowWindowName << "\" doesn't include \"" << iter.key() << "\"";
                    }

                } else {

                    qDebug() << "IT HAS A FULL TITLE NAME";

                    if (iter.key() == nowWindowName) {

                        qDebug() << "WINDOW: \"" << nowWindowName << "\" is equal to hash key: \"" << iter.key() << "\"";

                        QSet<AutoProfileInfo*> tempSet;
                        tempSet = getWindowNameProfileAssignments().value(nowWindowName).toSet();
                        fullSet = fullSet.unite(tempSet);

                    } else {

                        qDebug() << "WINDOW: \"" << nowWindowName << "\" is not equal to hash key: \"" << iter.key() << "\"";
                    }
                }
            }
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


                if (info->isPartialState()) {

                    numMatched += (!info->getWindowName().isEmpty() &&
                                   nowWindowName.contains(info->getWindowName())) ? 1 : 0;
                } else {

                    numMatched += (!info->getWindowName().isEmpty() &&
                                   info->getWindowName() == nowWindowName) ? 1 : 0;
                }


                if (numProps == numMatched && highestMatchCount.contains(info->getUniqueID()))
                {
                    int currentHigh = highestMatchCount.value(info->getUniqueID());
                    if (numMatched > currentHigh)
                    {
                        highestMatchCount.insert(info->getUniqueID(), numMatched);
                        highestMatches.insert(info->getUniqueID(), info);
                    }
                }
                else if (numProps == numMatched && !highestMatchCount.contains(info->getUniqueID()))
                {
                    highestMatchCount.insert(info->getUniqueID(), numMatched);
                    highestMatches.insert(info->getUniqueID(), info);
                }
            }
        }

        QHashIterator<QString, AutoProfileInfo*> highIter(highestMatches);

        while (highIter.hasNext())
        {
            AutoProfileInfo *info = highIter.next().value();
            getUniqeIDSetLocal().insert(info->getUniqueID());
            emit foundApplicableProfile(info);
        }

        if ((!getDefaultProfileAssignments().isEmpty() || allDefaultInfo) && !focusedWidget)
        {
            if (allDefaultInfo != nullptr && allDefaultInfo->isActive() && !getUniqeIDSetLocal().contains("all"))
            {
               emit foundApplicableProfile(allDefaultInfo);
            }

            QHashIterator<QString, AutoProfileInfo*> iter(getDefaultProfileAssignments());

            while (iter.hasNext())
            {
                iter.next();
                AutoProfileInfo *info = iter.value();

                if (info->isActive() && !getUniqeIDSetLocal().contains(info->getUniqueID()))
                {
                    emit foundApplicableProfile(info);
                }
            }
        }
    }
}

void AutoProfileWatcher::syncProfileAssignment()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    clearProfileAssignments();

    currentApplication = "";

    settings->getLock()->lock();
    settings->beginGroup("DefaultAutoProfiles");
    QString exe = QString();
    QString uniqueID = QString();
    QString profile = QString();
    QString active = QString();
    QString windowClass = QString();
    QString windowName = QString();

    QStringList registeredUniques = settings->value("Uniques", QStringList()).toStringList();

    settings->endGroup();

    QString allProfile = settings->value(QString("DefaultAutoProfileAll/Profile"), "all").toString();
    QString allActive = settings->value(QString("DefaultAutoProfileAll/Active"), "0").toString();

    // Handle overall Default profile assignment
    bool defaultActive = allActive == "1" ? true : false;

    if (defaultActive)
    {
        allDefaultInfo = new AutoProfileInfo("all", allProfile, defaultActive, 0, this);
        allDefaultInfo->setDefaultState(true);
    }

    // Handle device specific Default profile assignments
    QStringListIterator iter(registeredUniques);

    while (iter.hasNext())
    {
        QString tempkey = iter.next();
        QString uniqueID = QString(tempkey).replace("UniqueID", "");
        QString profile = settings->value(QString("DefaultAutoProfile-%1/Profile").arg(uniqueID), "").toString();
        QString active = settings->value(QString("DefaultAutoProfile-%1/Active").arg(uniqueID), "").toString();
        QString exe = settings->value(QString("DefaultAutoProfile-%1/Exe").arg(uniqueID), "").toString();
        QString partialTitle = settings->value(QString("DefaultAutoProfile-%1/PartialTitle").arg(uniqueID), "").toString();
        QString windowClass = settings->value(QString("DefaultAutoProfile-%1/WindowClass").arg(uniqueID), "").toString();
        QString windowName = settings->value(QString("DefaultAutoProfile-%1/WindowName").arg(uniqueID), "").toString();

        // need to change when it's needed to add windowClass, title and partial name
        if (!uniqueID.isEmpty() && !profile.isEmpty())
        {
            bool profileActive = active == "1" ? true : false;

            if (profileActive && uniqueID != "all")
            {
                AutoProfileInfo *info = new AutoProfileInfo(uniqueID, profile, profileActive, 0, this);
                info->setExe(exe);
                info->setWindowName(windowName);
                info->setWindowClass(windowClass);
                info->setPartialState(partialTitle == "1" ? true : false);
                info->setDefaultState(true);
                defaultProfileAssignments.insert(uniqueID, info);
            }
        }
    }

    settings->beginGroup("AutoProfiles");
    bool quitSearch = false;

    for (int i = 1; !quitSearch; i++)
    {
        exe = settings->value(QString("AutoProfile%1Exe").arg(i), "").toString();
        exe = QDir::toNativeSeparators(exe);

        convToUniqueIDAutoProfGroupSett(settings, QString("AutoProfile%1GUID").arg(i), QString("AutoProfile%1UniqueID").arg(i));

        uniqueID = settings->value(QString("AutoProfile%1UniqueID").arg(i), "").toString();
        profile = settings->value(QString("AutoProfile%1Profile").arg(i), "").toString();
        active = settings->value(QString("AutoProfile%1Active").arg(i), 0).toString();
        windowName = settings->value(QString("AutoProfile%1WindowName").arg(i), "").toString();
        QString partialTitle = settings->value(QString("AutoProfile%1PartialTitle").arg(i), 0).toString();
        bool partialTitleBool = partialTitle == "1" ? true : false;


#ifdef Q_OS_UNIX
        windowClass = settings->value(QString("AutoProfile%1WindowClass").arg(i), "").toString();
#else
        windowClass.clear();
#endif

        // Check if all required elements exist. If not, assume that the end of the
        // list has been reached.
        if ((!exe.isEmpty() || !windowClass.isEmpty() || !windowName.isEmpty()) &&
           !uniqueID.isEmpty())
        {
            bool profileActive = active == "1" ? true : false;

            if (profileActive)
            {
                AutoProfileInfo *info = new AutoProfileInfo(uniqueID, profile, profileActive, partialTitleBool, this);

                if (!windowClass.isEmpty())
                {
                    info->setWindowClass(windowClass);

                    QList<AutoProfileInfo*> templist;

                    if (getWindowClassProfileAssignments().contains(windowClass))
                        templist = getWindowClassProfileAssignments().value(windowClass);

                    templist.append(info);
                    windowClassProfileAssignments.insert(windowClass, templist);
                }

                // partly watching is needed for window title
                if (!windowName.isEmpty())
                {
                    info->setWindowName(windowName);
                    qDebug() << "WINDOW NAME IN AUTOPROFILEWATCHER: " << windowName;

                    QList<AutoProfileInfo*> templist;

                    QHashIterator<QString, QList<AutoProfileInfo*> > windows(getWindowNameProfileAssignments());

                    qDebug() << "getWindowNameProfileAssignments contains such elements like: ";

                    while(windows.hasNext()) {

                        windows.next();
                        qDebug() << windows.key();
                    }

                    if (getWindowNameProfileAssignments().contains(windowName))
                    {
                        qDebug() << "getWindowNameProfileAssignments contains " << windowName;
                        templist = getWindowNameProfileAssignments().value(windowName);

                    } else {

                        qDebug() << "getWindowNameProfileAssignments doesn't contain " << windowName;
                    }

                    templist.append(info);
                    windowNameProfileAssignments.insert(windowName, templist);
                }

                if (!exe.isEmpty())
                {
                    info->setExe(exe);

                    QList<AutoProfileInfo*> templist;

                    if (getAppProfileAssignments().contains(exe))
                        templist = getAppProfileAssignments().value(exe);

                    templist.append(info);
                    appProfileAssignments.insert(exe, templist);
                    QString baseExe = QFileInfo(exe).fileName();

                    if (!baseExe.isEmpty() && baseExe != exe)
                    {
                        QList<AutoProfileInfo*> templist;

                        if (getAppProfileAssignments().contains(baseExe))
                            templist = getAppProfileAssignments().value(baseExe);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QSet<AutoProfileInfo*> terminateProfiles;
    QListIterator<QList<AutoProfileInfo*> > iterDelete(getAppProfileAssignments().values());

    while (iterDelete.hasNext())
    {
        QList<AutoProfileInfo*> templist = iterDelete.next();
        terminateProfiles.unite(templist.toSet());
    }

    appProfileAssignments.clear();

    QListIterator<QList<AutoProfileInfo*> > iterClassDelete(getWindowClassProfileAssignments().values());

    while (iterClassDelete.hasNext())
    {
        QList<AutoProfileInfo*> templist = iterClassDelete.next();
        terminateProfiles.unite(templist.toSet());
    }

    windowClassProfileAssignments.clear();

    QListIterator<QList<AutoProfileInfo*> > iterNameDelete(getWindowNameProfileAssignments().values());

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
        if (info != nullptr)
        {
            delete info;
            info = nullptr;
        }
    }

    QListIterator<AutoProfileInfo*> iterDefaultsDelete(getDefaultProfileAssignments().values());

    while (iterDefaultsDelete.hasNext())
    {
        AutoProfileInfo *info = iterDefaultsDelete.next();
        if (info != nullptr)
        {
            delete info;
            info = nullptr;
        }
    }

    defaultProfileAssignments.clear();
    allDefaultInfo = nullptr;
    getUniqeIDSetLocal().clear();
}

QString AutoProfileWatcher::findAppLocation()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString exepath = QString();

    #ifdef WITH_X11
    Window currentWindow = 0;
    int pid = 0;

    currentWindow = X11Extras::getInstance()->getWindowInFocus();
    if (currentWindow) pid = X11Extras::getInstance()->getApplicationPid(currentWindow);
    if (pid > 0) exepath = X11Extras::getInstance()->getApplicationLocation(pid);
    #endif

    return exepath;
}

QList<AutoProfileInfo*>* AutoProfileWatcher::getCustomDefaults()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QList<AutoProfileInfo*> *temp = new QList<AutoProfileInfo*>();
    QHashIterator<QString, AutoProfileInfo*> iter(getDefaultProfileAssignments());

    while (iter.hasNext())
    {
        iter.next();
        temp->append(iter.value());
    }

    return temp;
}

AutoProfileInfo* AutoProfileWatcher::getDefaultAllProfile()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return allDefaultInfo;
}


bool AutoProfileWatcher::isUniqueIDLocked(QString uniqueID)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getUniqeIDSetLocal().contains(uniqueID);
}

QHash<QString, QList<AutoProfileInfo*> > const& AutoProfileWatcher::getAppProfileAssignments() {

    return appProfileAssignments;
}

QHash<QString, QList<AutoProfileInfo*> > const& AutoProfileWatcher::getWindowClassProfileAssignments() {

    return windowClassProfileAssignments;
}

QHash<QString, QList<AutoProfileInfo*> > const& AutoProfileWatcher::getWindowNameProfileAssignments() {

    return windowNameProfileAssignments;
}

QHash<QString, AutoProfileInfo*> const& AutoProfileWatcher::getDefaultProfileAssignments() {

    return defaultProfileAssignments;
}


QSet<QString>& AutoProfileWatcher::getUniqeIDSetLocal() {

    return uniqueIDSet;
}


void AutoProfileWatcher::convToUniqueIDAutoProfGroupSett(QSettings* sett, QString guidAutoProfSett, QString uniqueAutoProfSett)
{
    if (sett->contains(guidAutoProfSett))
    {
        sett->setValue(uniqueAutoProfSett, sett->value(guidAutoProfSett));
        sett->remove(guidAutoProfSett);
    }
}
