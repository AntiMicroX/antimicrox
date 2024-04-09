/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail.com>
 * Copyright (C) 2020 Paweł Kotiuk <kotiuk@zohomail.eu>
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

#include "common.h"

#include <QApplication>
#include <QDebug>
#include <QDirIterator>
#include <QLibraryInfo>
#include <QReadWriteLock>
#include <QRegularExpression>

#ifdef Q_OS_WIN
    #include <QStandardPaths>
#endif

namespace PadderCommon {
QString preferredProfileDir(AntiMicroSettings *settings)
{
    QString lastProfileDir = settings->value("LastProfileDir", "").toString();
    QString defaultProfileDir = settings->value("DefaultProfileDir", "").toString();
    QString lookupDir = QString();

    if (!defaultProfileDir.isEmpty())
    {
        QFileInfo dirinfo(defaultProfileDir);

        if (dirinfo.isDir() && dirinfo.isReadable())
            lookupDir = defaultProfileDir;
    }

    if (lookupDir.isEmpty() && !lastProfileDir.isEmpty())
    {
        QFileInfo dirinfo(lastProfileDir);

        if (dirinfo.isDir() && dirinfo.isReadable())
            lookupDir = lastProfileDir;
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
        } else
        {
            lookupDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        }
    #else
        lookupDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    #endif
#else
        lookupDir = QDir::homePath();
#endif
    }

    return lookupDir;
}

QStringList arguments(const int &argc, char **argv)
{
    QStringList list = QStringList();

    for (int a = 0; a < argc; ++a)
        list << QString::fromLocal8Bit(argv[a]);

    return list;
}

QStringList parseArgumentsString(QString tempString)
{
    bool inside = (!tempString.isEmpty() && tempString.at(0) == QChar('"'));
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    QStringList tempList = tempString.split(QRegularExpression("\""), Qt::SkipEmptyParts);
#else
    QStringList tempList = tempString.split(QRegularExpression("\""), QString::SkipEmptyParts);
#endif
    QStringList finalList = QStringList();
    QStringListIterator iter(tempList);

    while (iter.hasNext())
    {
        QString temp = iter.next();

        if (inside)
            finalList.append(temp);
        else
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            finalList.append(temp.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts));
#else
            finalList.append(temp.split(QRegularExpression("\\s+"), QString::SkipEmptyParts));
#endif
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
void reloadTranslations(QTranslator *translator, QTranslator *appTranslator, QString language)
{ // Remove application specific translation strings
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
    translator->load("antimicrox_" + language,
                     QApplication::applicationDirPath().append("/../share/antimicrox/translations"));
#elif defined(Q_OS_WIN)
    translator->load("antimicrox_" + language,
                     QApplication::applicationDirPath().append("\\share\\antimicrox\\translations"));
#endif

    qApp->installTranslator(translator);
}

void lockInputDevices() { sdlWaitMutex.lock(); }

void unlockInputDevices() { sdlWaitMutex.unlock(); }

QIcon loadIcon(const QString &name, const QString &fallback_location)
{
    qDebug() << " Application theme has icon named: " << name << " " << QIcon::hasThemeIcon(name);
    QFileInfo f(fallback_location);
    if (!f.exists())
    {
        qWarning() << "file " << fallback_location << " does not exist!";
    }
    return QIcon::fromTheme(name, QIcon(fallback_location));
}

QIcon loadIcon(QString name)
{
    bool has_icon = QIcon::hasThemeIcon(name);
    qDebug() << " Application theme has icon named: " << name << " " << has_icon;
    if (has_icon)
        return QIcon::fromTheme(name);

    QDirIterator it(":images/", QDirIterator::Subdirectories);
    QString fallback_location = "";
    // search also for variants with underscore like document_save.png for document-save
    QRegularExpression regex = QRegularExpression(".*" + name.replace(QChar('-'), "[_-]") + "\\.(svg|png)");
    while (it.hasNext())
    {
        QString value = it.next();
        if (value.contains(regex))
        {
            fallback_location = value;
            qDebug() << "Found fallback icon: " << value << " for name: " << name << "and regex: " << regex;
            break;
        }
    }

    QFileInfo f(fallback_location);
    if (!f.exists())
    {
        qWarning() << "file: " << fallback_location << " does not exist!";
    }
    return QIcon::fromTheme(name, QIcon(fallback_location));
}

void log_system_config()
{
    VERBOSE() << "AntiMicroX version: " << PadderCommon::programVersion
#ifdef ANTIMICROX_PKG_VERSION
              << " Package: " << ANTIMICROX_PKG_VERSION
#endif
#ifdef QT_DEBUG
              << " Type: Debug"
#else
              << " Type: Release"
#endif
        ;
    VERBOSE() << "SDL version: " << PadderCommon::sdlVersionUsed << " (Compiled with: " << PadderCommon::sdlVersionCompiled
              << ")";
    VERBOSE() << QString("Host OS: %1 Version: %2 Architecture: %3")
                     .arg(QSysInfo::productType(), QSysInfo::productVersion(), QSysInfo::currentCpuArchitecture());
}

QWaitCondition waitThisOut;
QMutex sdlWaitMutex;
QMutex inputDaemonMutex;
bool editingBindings = false;
MouseHelper mouseHelperObj;
} // namespace PadderCommon
