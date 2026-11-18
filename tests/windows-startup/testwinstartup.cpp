#include "winstartup.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QTemporaryDir>
#include <QTest>
#include <QUuid>
#include <QXmlStreamReader>
#include <windows.h>

class WindowsStartupTest : public QObject
{
    Q_OBJECT

  private:
    QTemporaryDir m_directory;
    QString m_task_name;
    QString m_shortcut;
    QString m_executable;

    QByteArray queryTask()
    {
        QProcess process;
        process.start(QStringLiteral("schtasks.exe"), {"/Query", "/TN", m_task_name, "/XML"});
        if (!process.waitForFinished() || process.exitCode() != 0)
            return QByteArray();
        return process.readAllStandardOutput();
    }

  private slots:
    void init()
    {
        QVERIFY(m_directory.isValid());
        m_task_name = QStringLiteral("AntiMicroX-Test-") + QUuid::createUuid().toString(QUuid::WithoutBraces);
        m_shortcut = m_directory.filePath(QStringLiteral("startup.lnk"));
        // Exercise XML escaping and non-ASCII paths without launching the test executable at logon.
        m_executable = m_directory.filePath(QString::fromUtf8("手柄 & startup.exe"));
        QVERIFY(QFile::copy(QCoreApplication::applicationFilePath(), m_executable));
    }

    void cleanup()
    {
        QProcess::execute(QStringLiteral("schtasks.exe"), {"/Delete", "/TN", m_task_name, "/F"});
        QDir directory(m_directory.path());
        for (const QString &entry : directory.entryList(QDir::Files))
            QVERIFY(QFile::remove(directory.filePath(entry)));
    }

    void startupLifecycle()
    {
        WinStartup startup(m_executable, m_shortcut, m_task_name);
        WinStartup::Mode mode;
        QString error;
        QVERIFY2(startup.read(mode, error), qPrintable(error));
        QCOMPARE(mode, WinStartup::Mode::Disabled);

        QVERIFY2(startup.write(WinStartup::Mode::Normal, error), qPrintable(error));
        QCOMPARE(QFileInfo(m_shortcut).symLinkTarget(), m_executable);
        QVERIFY2(startup.write(WinStartup::Mode::Normal, error), qPrintable(error));
        QVERIFY(QFileInfo::exists(m_shortcut));
        QVERIFY2(startup.read(mode, error), qPrintable(error));
        QCOMPARE(mode, WinStartup::Mode::Normal);

        QVERIFY2(startup.write(WinStartup::Mode::Administrator, error), qPrintable(error));
        QVERIFY(!QFileInfo::exists(m_shortcut));
        QVERIFY2(startup.read(mode, error), qPrintable(error));
        QCOMPARE(mode, WinStartup::Mode::Administrator);
        QVERIFY2(startup.write(WinStartup::Mode::Administrator, error), qPrintable(error));

        const QByteArray xml = queryTask();
        QVERIFY(!xml.isEmpty());
        QXmlStreamReader reader(xml);
        QMap<QString, QStringList> fields;
        while (!reader.atEnd())
        {
            reader.readNext();
            if (reader.isStartElement())
            {
                const QString name = reader.name().toString();
                if (QStringList({"UserId", "RunLevel", "LogonType", "Command", "WorkingDirectory", "ExecutionTimeLimit",
                                 "DisallowStartIfOnBatteries", "StopIfGoingOnBatteries"})
                        .contains(name))
                    fields[name].append(reader.readElementText());
            }
        }
        QVERIFY2(!reader.hasError(), qPrintable(reader.errorString()));
        QCOMPARE(fields["RunLevel"], QStringList({"HighestAvailable"}));
        QCOMPARE(fields["LogonType"], QStringList({"InteractiveToken"}));
        QCOMPARE(fields["Command"], QStringList({QDir::toNativeSeparators(m_executable)}));
        QCOMPARE(fields["WorkingDirectory"], QStringList({QDir::toNativeSeparators(m_directory.path())}));
        QCOMPARE(fields["UserId"].size(), 2);
        QCOMPARE(fields["UserId"].first(), fields["UserId"].last());
        QVERIFY(fields["UserId"].first().startsWith("S-1-"));
        QCOMPARE(fields["ExecutionTimeLimit"], QStringList({"PT0S"}));
        QCOMPARE(fields["DisallowStartIfOnBatteries"], QStringList({"false"}));
        QCOMPARE(fields["StopIfGoingOnBatteries"], QStringList({"false"}));

        QVERIFY2(startup.write(WinStartup::Mode::Normal, error), qPrintable(error));
        QVERIFY(QFileInfo::exists(m_shortcut));
        QVERIFY(queryTask().isEmpty());
        QVERIFY2(startup.write(WinStartup::Mode::Administrator, error), qPrintable(error));
        QVERIFY2(startup.write(WinStartup::Mode::Disabled, error), qPrintable(error));
        QVERIFY2(startup.read(mode, error), qPrintable(error));
        QCOMPARE(mode, WinStartup::Mode::Disabled);
        QVERIFY(!QFileInfo::exists(m_shortcut));
        QVERIFY(queryTask().isEmpty());
    }

    void failedRegistrationRestoresShortcut()
    {
        QVERIFY(QFile::link(m_executable, m_shortcut));
        WinStartup startup(m_executable, m_shortcut, m_task_name + QStringLiteral(":invalid"));
        QString error;
        QVERIFY(!startup.write(WinStartup::Mode::Administrator, error));
        QVERIFY(!error.isEmpty());
        QCOMPARE(QFileInfo(m_shortcut).symLinkTarget(), m_executable);
        QCOMPARE(QDir(m_directory.path()).entryList(QDir::Files).size(), 2);
    }

    void repairsEmptyShortcut()
    {
        QFile placeholder(m_shortcut);
        QVERIFY(placeholder.open(QIODevice::WriteOnly));
        placeholder.close();
        WinStartup startup(m_executable, m_shortcut, m_task_name);
        QString error;
        QVERIFY2(startup.write(WinStartup::Mode::Normal, error), qPrintable(error));
        QCOMPARE(QFileInfo(m_shortcut).symLinkTarget(), m_executable);
        QVERIFY2(startup.write(WinStartup::Mode::Disabled, error), qPrintable(error));
        QVERIFY(!QFileInfo::exists(m_shortcut));
    }
};

QTEST_GUILESS_MAIN(WindowsStartupTest)
#include "testwinstartup.moc"
