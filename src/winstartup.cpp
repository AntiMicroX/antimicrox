#include "winstartup.h"

#include <windows.h>

#include <sddl.h>
#include <taskschd.h>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUuid>
#include <QXmlStreamWriter>
#include <memory>

namespace {
struct ComRelease
{
    void operator()(IUnknown *object) const { object->Release(); }
};

struct BstrFree
{
    void operator()(wchar_t *string) const { SysFreeString(string); }
};

using Bstr = std::unique_ptr<wchar_t, BstrFree>;

Bstr toBstr(const QString &string) { return Bstr(SysAllocString(reinterpret_cast<const wchar_t *>(string.utf16()))); }

QString currentUserSid()
{
    HANDLE token = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
        return QString();

    DWORD size = 0;
    GetTokenInformation(token, TokenUser, nullptr, 0, &size);
    QByteArray buffer(static_cast<int>(size), 0);
    bool success = GetTokenInformation(token, TokenUser, buffer.data(), size, &size);
    CloseHandle(token);
    if (!success)
        return QString();

    wchar_t *sid = nullptr;
    if (!ConvertSidToStringSidW(reinterpret_cast<TOKEN_USER *>(buffer.data())->User.Sid, &sid))
        return QString();

    QString result = QString::fromWCharArray(sid);
    LocalFree(sid);
    return result;
}

bool shortcutExists(const QString &path)
{
    return GetFileAttributesW(reinterpret_cast<const wchar_t *>(path.utf16())) != INVALID_FILE_ATTRIBUTES;
}

bool taskError(HRESULT result, QString &error)
{
    if (SUCCEEDED(result))
        return false;

    if (result == E_ACCESSDENIED)
    {
        error = QCoreApplication::translate("WinStartup",
                                            "Access denied. Use the shield button in the main window to "
                                            "restart AntiMicroX as administrator, then change this setting again.");
        return true;
    }

    wchar_t *message = nullptr;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
                   result, 0, reinterpret_cast<wchar_t *>(&message), 0, nullptr);
    error = QStringLiteral("0x%1").arg(static_cast<quint32>(result), 8, 16, QLatin1Char('0'));
    if (message)
    {
        error += QStringLiteral(": ") + QString::fromWCharArray(message).trimmed();
        LocalFree(message);
    }
    return true;
}

struct TaskFolder
{
    HRESULT result;
    bool initialized;
    std::unique_ptr<ITaskFolder, ComRelease> folder;

    TaskFolder()
    {
        result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        initialized = SUCCEEDED(result);
        if (FAILED(result) && result != RPC_E_CHANGED_MODE)
            return;

        ITaskService *service = nullptr;
        result = CoCreateInstance(CLSID_TaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskService,
                                  reinterpret_cast<void **>(&service));
        if (FAILED(result))
            return;
        std::unique_ptr<ITaskService, ComRelease> service_guard(service);
        VARIANT empty;
        VariantInit(&empty);
        result = service->Connect(empty, empty, empty, empty);
        if (FAILED(result))
            return;

        ITaskFolder *root = nullptr;
        result = service->GetFolder(toBstr(QStringLiteral("\\")).get(), &root);
        folder.reset(root);
    }

    ~TaskFolder()
    {
        folder.reset();
        if (initialized)
            CoUninitialize();
    }
};

bool missingTask(HRESULT result)
{
    return result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) || result == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
}

QString taskXml(const QString &user_sid, const QString &executable)
{
    QString xml;
    QXmlStreamWriter writer(&xml);
    writer.writeStartDocument();
    writer.writeStartElement(QStringLiteral("Task"));
    writer.writeDefaultNamespace(QStringLiteral("http://schemas.microsoft.com/windows/2004/02/mit/task"));
    writer.writeAttribute(QStringLiteral("version"), QStringLiteral("1.2"));
    writer.writeStartElement(QStringLiteral("Triggers"));
    writer.writeStartElement(QStringLiteral("LogonTrigger"));
    writer.writeTextElement(QStringLiteral("UserId"), user_sid);
    writer.writeEndElement();
    writer.writeEndElement();
    writer.writeStartElement(QStringLiteral("Principals"));
    writer.writeStartElement(QStringLiteral("Principal"));
    writer.writeAttribute(QStringLiteral("id"), QStringLiteral("User"));
    writer.writeTextElement(QStringLiteral("UserId"), user_sid);
    writer.writeTextElement(QStringLiteral("LogonType"), QStringLiteral("InteractiveToken"));
    writer.writeTextElement(QStringLiteral("RunLevel"), QStringLiteral("HighestAvailable"));
    writer.writeEndElement();
    writer.writeEndElement();
    writer.writeStartElement(QStringLiteral("Settings"));
    writer.writeTextElement(QStringLiteral("MultipleInstancesPolicy"), QStringLiteral("IgnoreNew"));
    writer.writeTextElement(QStringLiteral("DisallowStartIfOnBatteries"), QStringLiteral("false"));
    writer.writeTextElement(QStringLiteral("StopIfGoingOnBatteries"), QStringLiteral("false"));
    writer.writeTextElement(QStringLiteral("ExecutionTimeLimit"), QStringLiteral("PT0S"));
    // Task Scheduler defaults to priority 7 (below normal); 5 matches a normally launched process.
    writer.writeTextElement(QStringLiteral("Priority"), QStringLiteral("5"));
    writer.writeEndElement();
    writer.writeStartElement(QStringLiteral("Actions"));
    writer.writeAttribute(QStringLiteral("Context"), QStringLiteral("User"));
    writer.writeStartElement(QStringLiteral("Exec"));
    writer.writeTextElement(QStringLiteral("Command"), QDir::toNativeSeparators(executable));
    writer.writeTextElement(QStringLiteral("WorkingDirectory"),
                            QDir::toNativeSeparators(QFileInfo(executable).absolutePath()));
    writer.writeEndDocument();
    return xml;
}
} // namespace

WinStartup::WinStartup(const QString &executable, const QString &shortcut_path, const QString &task_name)
    : m_executable(executable)
    , m_shortcut_path(shortcut_path.isEmpty()
                          ? QDir(QString::fromUtf8(qgetenv("APPDATA")))
                                .filePath(QStringLiteral("Microsoft/Windows/Start Menu/Programs/Startup/antimicrox.lnk"))
                          : shortcut_path)
    , m_task_name(task_name)
    , m_user_sid(currentUserSid())
{
    if (m_task_name.isEmpty())
        m_task_name = QStringLiteral("AntiMicroX-Startup-") + m_user_sid;
}

/** @brief Read the actual Windows configuration rather than a cached preference. */
bool WinStartup::read(Mode &mode, QString &error) const
{
    error.clear();
    mode = Mode::Disabled;
    if (m_user_sid.isEmpty())
    {
        error = QCoreApplication::translate("WinStartup", "Could not identify the current Windows user.");
        return false;
    }
    TaskFolder scheduler;
    if (taskError(scheduler.result, error))
        return false;

    IRegisteredTask *task = nullptr;
    HRESULT result = scheduler.folder->GetTask(toBstr(m_task_name).get(), &task);
    std::unique_ptr<IRegisteredTask, ComRelease> task_guard(task);
    if (!missingTask(result))
    {
        if (taskError(result, error))
            return false;
        VARIANT_BOOL enabled = VARIANT_FALSE;
        if (taskError(task->get_Enabled(&enabled), error))
            return false;
        if (enabled == VARIANT_TRUE)
        {
            mode = Mode::Administrator;
            return true;
        }
    }
    if (shortcutExists(m_shortcut_path))
        mode = Mode::Normal;
    return true;
}

/** @brief Switch startup modes, restoring the previous shortcut if the task update fails. */
bool WinStartup::write(Mode mode, QString &error) const
{
    Mode current;
    if (!read(current, error))
        return false;
    if (current == mode && (mode != Mode::Normal || !QFileInfo(m_shortcut_path).symLinkTarget().isEmpty()))
        return true;

    TaskFolder scheduler;
    if (taskError(scheduler.result, error))
        return false;

    QString backup = m_shortcut_path + QStringLiteral(".") + QUuid::createUuid().toString(QUuid::WithoutBraces);
    bool had_shortcut = shortcutExists(m_shortcut_path);
    if (had_shortcut && QFileInfo(m_shortcut_path).isDir())
    {
        error = QCoreApplication::translate("WinStartup", "A directory is blocking the Windows startup shortcut.");
        return false;
    }
    if (had_shortcut && !MoveFileW(reinterpret_cast<const wchar_t *>(m_shortcut_path.utf16()),
                                   reinterpret_cast<const wchar_t *>(backup.utf16())))
    {
        taskError(HRESULT_FROM_WIN32(GetLastError()), error);
        return false;
    }

    bool created_shortcut = false;
    if (mode == Mode::Normal)
    {
        created_shortcut = QFile::link(m_executable, m_shortcut_path);
        if (!created_shortcut)
            error = QCoreApplication::translate("WinStartup", "Could not create the Windows startup shortcut.");
    }

    // Only touch the scheduler when a task must be created or removed, so normal startup never needs it.
    if (error.isEmpty() && (mode == Mode::Administrator || current == Mode::Administrator))
    {
        HRESULT result;
        if (mode == Mode::Administrator)
        {
            VARIANT empty;
            VariantInit(&empty);
            IRegisteredTask *task = nullptr;
            result = scheduler.folder->RegisterTask(toBstr(m_task_name).get(),
                                                    toBstr(taskXml(m_user_sid, m_executable)).get(), TASK_CREATE_OR_UPDATE,
                                                    empty, empty, TASK_LOGON_INTERACTIVE_TOKEN, empty, &task);
            std::unique_ptr<IRegisteredTask, ComRelease> task_guard(task);
        } else
        {
            result = scheduler.folder->DeleteTask(toBstr(m_task_name).get(), 0);
            if (missingTask(result))
                result = S_OK;
        }
        taskError(result, error);
    }

    if (!error.isEmpty())
    {
        if (created_shortcut && !DeleteFileW(reinterpret_cast<const wchar_t *>(m_shortcut_path.utf16())))
            error += QCoreApplication::translate("WinStartup", " Could not remove the new startup shortcut.");
        if (had_shortcut && !MoveFileW(reinterpret_cast<const wchar_t *>(backup.utf16()),
                                       reinterpret_cast<const wchar_t *>(m_shortcut_path.utf16())))
            error += QCoreApplication::translate("WinStartup", " Could not restore the previous startup shortcut from %1.")
                         .arg(backup);
        return false;
    }
    if (had_shortcut && !DeleteFileW(reinterpret_cast<const wchar_t *>(backup.utf16())))
    {
        error = QCoreApplication::translate("WinStartup",
                                            "Startup was updated, but the old shortcut could not be removed from %1.")
                    .arg(backup);
        return false;
    }
    return true;
}
