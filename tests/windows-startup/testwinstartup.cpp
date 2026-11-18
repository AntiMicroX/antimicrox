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
#include <string>
#include <windows.h>

#include <sddl.h>

#include <taskschd.h>

class WindowsStartupTest : public QObject
{
    Q_OBJECT

  private:
    QTemporaryDir m_directory;
    QString m_task_name;
    QString m_shortcut;
    QString m_executable;

    QString accountSid(const QString &account)
    {
        if (account.startsWith("S-1-"))
            return account;
        DWORD sid_size = 0;
        DWORD domain_size = 0;
        SID_NAME_USE type;
        const wchar_t *name = reinterpret_cast<const wchar_t *>(account.utf16());
        LookupAccountNameW(nullptr, name, nullptr, &sid_size, nullptr, &domain_size, &type);
        QByteArray sid(static_cast<int>(sid_size), 0);
        std::wstring domain(domain_size, L'\0');
        if (!LookupAccountNameW(nullptr, name, sid.data(), &sid_size, &domain[0], &domain_size, &type))
            return QString();
        wchar_t *text = nullptr;
        if (!ConvertSidToStringSidW(sid.data(), &text))
            return QString();
        const QString result = QString::fromWCharArray(text);
        LocalFree(text);
        return result;
    }

    QString queryTask()
    {
        QString xml;
        HRESULT initialized = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (FAILED(initialized) && initialized != RPC_E_CHANGED_MODE)
            return xml;

        ITaskService *service = nullptr;
        ITaskFolder *folder = nullptr;
        IRegisteredTask *task = nullptr;
        BSTR definition = nullptr;
        BSTR root = SysAllocString(L"\\");
        BSTR name = SysAllocString(reinterpret_cast<const wchar_t *>(m_task_name.utf16()));
        VARIANT empty;
        VariantInit(&empty);
        if (SUCCEEDED(CoCreateInstance(CLSID_TaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskService,
                                       reinterpret_cast<void **>(&service))) &&
            SUCCEEDED(service->Connect(empty, empty, empty, empty)) && SUCCEEDED(service->GetFolder(root, &folder)) &&
            SUCCEEDED(folder->GetTask(name, &task)) && SUCCEEDED(task->get_Xml(&definition)))
            xml = QString::fromWCharArray(definition, SysStringLen(definition));

        SysFreeString(definition);
        SysFreeString(name);
        SysFreeString(root);
        if (task)
            task->Release();
        if (folder)
            folder->Release();
        if (service)
            service->Release();
        if (SUCCEEDED(initialized))
            CoUninitialize();
        return xml;
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

        const QString xml = queryTask();
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
        QCOMPARE(accountSid(fields["UserId"].first()), accountSid(fields["UserId"].last()));
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
        WinStartup startup(QString(), m_shortcut, m_task_name);
        QString error;
        WinStartup::Mode mode;
        QVERIFY2(startup.read(mode, error), qPrintable(error));
        QCOMPARE(mode, WinStartup::Mode::Normal);
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
