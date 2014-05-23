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
#ifdef Q_OS_WIN
            QString portableProDir = QDir::currentPath().append("/profiles");
            QFileInfo portableProDirInfo(portableProDir)
            if (portableProDirInfo.isDir() && portableProDirInfo.isReadable())
            {
                lookupDir = portableProDir;
            }
            else
            {
                lookupDir =  QDir::currentPath();
            }
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
}
