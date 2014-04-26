#include "common.h"

namespace PadderCommon
{
    QString preferredProfileDir(QSettings *settings)
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
            lookupDir = QDir::homePath();
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
}
