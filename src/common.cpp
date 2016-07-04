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

#include <QApplication>
#include <QLibraryInfo>
#ifdef Q_OS_WIN
#include <QStandardPaths>
#endif

#include "common.h"

namespace PadderCommon
{
    QString preferredProfileDir(AntiMicroSettings *settings)
    {
        QString lastProfileDir = settings->value("LastProfileDir", "").toString();
        QString defaultProfileDir = settings->value("DefaultProfileDir", "").toString();
        QString lookupDir;

        if (!defaultProfileDir.isEmpty())
        {
            QFileInfo dirinfo(defaultProfileDir);
            if (dirinfo.isDir() && dirinfo.isReadable())
            {
                lookupDir = defaultProfileDir;
            }
        }

        if (lookupDir.isEmpty() && !lastProfileDir.isEmpty())
        {
            QFileInfo dirinfo(lastProfileDir);
            if (dirinfo.isDir() && dirinfo.isReadable())
            {
                lookupDir = lastProfileDir;
            }
        }

        if (lookupDir.isEmpty())
        {
#ifdef Q_OS_WIN
    #ifdef WIN_PORTABLE_PACKAGE
            QString portableProDir = QDir::currentPath().append("/profiles");
            QFileInfo portableProDirInfo(portableProDir);
            if (portableProDirInfo.isDir() && portableProDirInfo.isReadable())
            {
                lookupDir = portableProDir;
            }
            else
            {
                lookupDir =  QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
            }
    #else
            lookupDir =  QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    #endif
#else
            lookupDir = QDir::homePath();
#endif
        }

        return lookupDir;
    }

    QStringList arguments(int &argc, char **argv)
    {
        QStringList list;

        for (int a = 0; a < argc; ++a) {
            list << QString::fromLocal8Bit(argv[a]);
        }

        return list;
    }

    QStringList parseArgumentsString(QString tempString)
    {
        bool inside = (!tempString.isEmpty() && tempString.at(0) == QChar('"'));
        QStringList tempList = tempString.split(QRegExp("\""), QString::SkipEmptyParts);
        QStringList finalList;
        QStringListIterator iter(tempList);
        while (iter.hasNext())
        {
            QString temp = iter.next();
            if (inside)
            {
                finalList.append(temp);
            }
            else
            {
                finalList.append(temp.split(QRegExp("\\s+"), QString::SkipEmptyParts));
            }

            inside = !inside;
        }

        return finalList;
    }

    /**
     * @brief Reload main application and base Qt translation files.
     * @param Based Qt translator
     * @param Application translator
     * @param Language code
     */
    void reloadTranslations(QTranslator *translator,
                           QTranslator *appTranslator,
                           QString language)
    {
        // Remove application specific translation strings
        qApp->removeTranslator(translator);

        // Remove old Qt translation strings
        qApp->removeTranslator(appTranslator);

        // Load new Qt translation strings
    #if defined(Q_OS_UNIX)
        translator->load(QString("qt_").append(language), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    #elif defined(Q_OS_WIN)
      #ifdef QT_DEBUG
        translator->load(QString("qt_").append(language), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
      #else
        translator->load(QString("qt_").append(language),
                          QApplication::applicationDirPath().append("\\share\\qt\\translations"));
      #endif
    #endif

        qApp->installTranslator(appTranslator);

        // Load application specific translation strings
    #if defined(Q_OS_UNIX)
        translator->load("antimicro_" + language, QApplication::applicationDirPath().append("/../share/antimicro/translations"));
    #elif defined(Q_OS_WIN)
        translator->load("antimicro_" + language, QApplication::applicationDirPath().append("\\share\\antimicro\\translations"));
    #endif
        qApp->installTranslator(translator);
    }

    void lockInputDevices()
    {
        sdlWaitMutex.lock();
        /*editingLock.lockForWrite();
        editingBindings = true;
        editingLock.unlock();

        waitMutex.lock();
        //editingBindings = true;
        waitThisOut.wait(&waitMutex);
        */
    }

    void unlockInputDevices()
    {
        sdlWaitMutex.unlock();
        /*editingLock.lockForWrite();
        editingBindings = false;
        editingLock.unlock();

        waitMutex.unlock();
        */
    }

    QWaitCondition waitThisOut;
    QMutex sdlWaitMutex;
    QMutex inputDaemonMutex;
    QReadWriteLock editingLock;
    bool editingBindings = false;
    MouseHelper mouseHelperObj;
}
