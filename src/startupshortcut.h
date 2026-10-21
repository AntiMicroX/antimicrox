// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STARTUPSHORTCUT_H
#define STARTUPSHORTCUT_H

#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace StartupShortcut {

/** Update a startup shortcut without recreating an existing link or touching its target. */
inline bool setEnabled(const QString &applicationPath, const QString &shortcutPath, bool enabled)
{
    const QFileInfo shortcut(shortcutPath);
    // Qt 5 may hide malformed .lnk files from exists() and report absent paths as
    // links. Directory entries and an open file give us the actual placeholder.
    const QStringList entries = QDir(shortcut.absolutePath()).entryList(QDir::AllEntries | QDir::Hidden | QDir::System);
#ifdef Q_OS_WIN
    const bool present = entries.contains(shortcut.fileName(), Qt::CaseInsensitive);
#else
    const bool present = entries.contains(shortcut.fileName());
#endif
    const bool link = !shortcut.symLinkTarget().isEmpty();

    if (!enabled)
        return !present || QFile::remove(shortcutPath);

    // Preserve existing shortcuts, including any arguments configured by the user.
    if (present && link)
        return true;

    if (!QFileInfo(applicationPath).isFile())
        return false;

    if (present)
    {
        // Earlier versions created an empty .lnk before calling QFile::link().
        // Only repair that placeholder; preserve other files and directories.
        QFile placeholder(shortcutPath);
        if (shortcut.isDir() || !placeholder.open(QIODevice::ReadOnly) || placeholder.size() != 0)
            return false;
        placeholder.close();
        if (!QFile::remove(shortcutPath))
            return false;
    }

    // QFile::link() creates the shortcut itself and requires an absent destination.
    return QFile::link(applicationPath, shortcutPath);
}

} // namespace StartupShortcut

#endif // STARTUPSHORTCUT_H
