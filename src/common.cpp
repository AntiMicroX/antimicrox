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

#include "messagehandler.h"

#include <QApplication>
#include <QDebug>
#include <QLibraryInfo>
#include <QReadWriteLock>

namespace PadderCommon {
QString preferredProfileDir(AntiMicroSettings *settings)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
        lookupDir = QDir::homePath();
    }

    return lookupDir;
}

QStringList arguments(const int &argc, char **argv)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QStringList list = QStringList();

    for (int a = 0; a < argc; ++a)
        list << QString::fromLocal8Bit(argv[a]);

    return list;
}

QStringList parseArgumentsString(QString tempString)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool inside = (!tempString.isEmpty() && tempString.at(0) == QChar('"'));
    QStringList tempList = tempString.split(QRegExp("\""), QString::SkipEmptyParts);
    QStringList finalList = QStringList();
    QStringListIterator iter(tempList);

    while (iter.hasNext())
    {
        QString temp = iter.next();

        if (inside)
            finalList.append(temp);
        else
            finalList.append(temp.split(QRegExp("\\s+"), QString::SkipEmptyParts));

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
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    // Remove application specific translation strings
    qApp->removeTranslator(translator);

    // Remove old Qt translation strings
    qApp->removeTranslator(appTranslator);

    // Load new Qt translation strings
    translator->load(QString("qt_").append(language), QLibraryInfo::location(QLibraryInfo::TranslationsPath));

    qApp->installTranslator(appTranslator);

    // Load application specific translation strings
    translator->load("antimicrox_" + language,
                     QApplication::applicationDirPath().append("/../share/antimicrox/translations"));

    qApp->installTranslator(translator);
}

void lockInputDevices()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    sdlWaitMutex.lock();
}

void unlockInputDevices()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    sdlWaitMutex.unlock();
}
/**
 * @brief Universal method for loading icons
 *
 * @param name - name of used icon
 * @param fallback_location - location of icon loaded when icon described by name not found
 * @return QIcon
 */
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

QWaitCondition waitThisOut;
QMutex sdlWaitMutex;
QMutex inputDaemonMutex;
QReadWriteLock editingLock;
bool editingBindings = false;
MouseHelper mouseHelperObj;
} // namespace PadderCommon
