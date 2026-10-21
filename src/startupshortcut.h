// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STARTUPSHORTCUT_H
#define STARTUPSHORTCUT_H

#include <QFile>
#include <QFileInfo>

namespace StartupShortcut {

/** Update a startup shortcut without recreating an existing link or touching its target. */
inline bool setEnabled(const QString &applicationPath, const QString &shortcutPath, bool enabled)
{
    const QFileInfo shortcut(shortcutPath);
    const bool present = shortcut.exists() || shortcut.isSymLink();

    if (!enabled)
        return !present || QFile::remove(shortcutPath);

    // Preserve existing shortcuts, including any arguments configured by the user.
    if (shortcut.isSymLink())
        return true;

    if (!QFileInfo(applicationPath).isFile())
        return false;

    if (present)
    {
        // Earlier versions created an empty .lnk before calling QFile::link().
        // Only repair that placeholder; preserve other files and directories.
        if (!shortcut.isFile() || shortcut.size() != 0 || !QFile::remove(shortcutPath))
            return false;
    }

    // QFile::link() creates the shortcut itself and requires an absent destination.
    return QFile::link(applicationPath, shortcutPath);
}

} // namespace StartupShortcut

#endif // STARTUPSHORTCUT_H
