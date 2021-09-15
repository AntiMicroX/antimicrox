#define _WIN32_WINNT 0x0600

#include <qt_windows.h>
#include <psapi.h>
//#include <QDebug>
#include <QHashIterator>
#include <QSettings>
#include <QCoreApplication>
#include <QDir>

#include "winextras.h"
#include <shlobj.h>

typedef DWORD(WINAPI *MYPROC)(HANDLE, DWORD, LPTSTR, PDWORD);
// Check if QueryFullProcessImageNameW function exists in kernel32.dll.
// Function does not exist in Windows XP.
static MYPROC pQueryFullProcessImageNameW = (MYPROC) GetProcAddress(
            GetModuleHandle(TEXT("kernel32.dll")), "QueryFullProcessImageNameW");

/*static bool isWindowsVistaOrHigher()
{
    OSVERSIONINFO osvi;
    memset(&osvi, 0, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);
    return (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion >= 6);
}
*/

const unsigned int WinExtras::EXTENDED_FLAG = 0x100;
int WinExtras::originalMouseAccel = 0;

static const QString ROOTASSOCIATIONKEY("HKEY_CURRENT_USER\\Software\\Classes");
static const QString FILEASSOCIATIONKEY(QString("%1\\%2").arg(ROOTASSOCIATIONKEY).arg(".amgp"));
static const QString PROGRAMASSOCIATIONKEY(QString("%1\\%2").arg(ROOTASSOCIATIONKEY).arg("AntiMicro.amgp"));

WinExtras WinExtras::_instance;

WinExtras::WinExtras(QObject *parent) :
    QObject(parent)
{
    populateKnownAliases();
}

QString WinExtras::getDisplayString(unsigned int virtualkey)
{
    QString temp;
    if (virtualkey <= 0)
    {
        temp = tr("[NO KEY]");
    }
    else if (_instance.knownAliasesVKStrings.contains(virtualkey))
    {
        temp = _instance.knownAliasesVKStrings.value(virtualkey);
    }

    return temp;
}

unsigned int WinExtras::getVirtualKey(QString codestring)
{
    int temp = 0;
    if (_instance.knownAliasesX11SymVK.contains(codestring))
    {
        temp = _instance.knownAliasesX11SymVK.value(codestring);
    }

    return temp;
}

void WinExtras::populateKnownAliases()
{
    // These aliases are needed for xstrings that would
    // return empty space characters from XLookupString
    if (knownAliasesX11SymVK.isEmpty())
    {
        knownAliasesX11SymVK.insert("Escape", VK_ESCAPE);
        knownAliasesX11SymVK.insert("Tab", VK_TAB);
        knownAliasesX11SymVK.insert("space", VK_SPACE);
        knownAliasesX11SymVK.insert("Delete", VK_DELETE);
        knownAliasesX11SymVK.insert("Return", VK_RETURN);
        knownAliasesX11SymVK.insert("KP_Enter", VK_RETURN);
        knownAliasesX11SymVK.insert("BackSpace", VK_BACK);
        knownAliasesX11SymVK.insert("F1", VK_F1);
        knownAliasesX11SymVK.insert("F2", VK_F2);
        knownAliasesX11SymVK.insert("F3", VK_F3);
        knownAliasesX11SymVK.insert("F4", VK_F4);
        knownAliasesX11SymVK.insert("F5", VK_F5);
        knownAliasesX11SymVK.insert("F6", VK_F6);
        knownAliasesX11SymVK.insert("F7", VK_F7);
        knownAliasesX11SymVK.insert("F8", VK_F8);
        knownAliasesX11SymVK.insert("F9", VK_F9);
        knownAliasesX11SymVK.insert("F10", VK_F10);
        knownAliasesX11SymVK.insert("F11", VK_F11);
        knownAliasesX11SymVK.insert("F12", VK_F12);
        knownAliasesX11SymVK.insert("Shift_L", VK_LSHIFT);
        knownAliasesX11SymVK.insert("Shift_R", VK_RSHIFT);
        knownAliasesX11SymVK.insert("Insert", VK_INSERT);
        knownAliasesX11SymVK.insert("Pause", VK_PAUSE);
        knownAliasesX11SymVK.insert("grave", VK_OEM_3);
        knownAliasesX11SymVK.insert("minus", VK_OEM_MINUS);
        knownAliasesX11SymVK.insert("equal", VK_OEM_PLUS);
        knownAliasesX11SymVK.insert("Caps_Lock", VK_CAPITAL);
        knownAliasesX11SymVK.insert("Control_L", VK_CONTROL);
        knownAliasesX11SymVK.insert("Control_R", VK_RCONTROL);
        knownAliasesX11SymVK.insert("Alt_L", VK_MENU);
        knownAliasesX11SymVK.insert("Alt_R", VK_RMENU);
        knownAliasesX11SymVK.insert("Super_L", VK_LWIN);
        knownAliasesX11SymVK.insert("Menu", VK_APPS);
        knownAliasesX11SymVK.insert("Prior", VK_PRIOR);
        knownAliasesX11SymVK.insert("Next", VK_NEXT);
        knownAliasesX11SymVK.insert("Home", VK_HOME);
        knownAliasesX11SymVK.insert("End", VK_END);
        knownAliasesX11SymVK.insert("Up", VK_UP);
        knownAliasesX11SymVK.insert("Down", VK_DOWN);
        knownAliasesX11SymVK.insert("Left", VK_LEFT);
        knownAliasesX11SymVK.insert("Right", VK_RIGHT);
        knownAliasesX11SymVK.insert("bracketleft", VK_OEM_4);
        knownAliasesX11SymVK.insert("bracketright", VK_OEM_6);
        knownAliasesX11SymVK.insert("backslash", VK_OEM_5);
        knownAliasesX11SymVK.insert("slash", VK_OEM_2);
        knownAliasesX11SymVK.insert("semicolon", VK_OEM_1);
        knownAliasesX11SymVK.insert("apostrophe", VK_OEM_7);
        knownAliasesX11SymVK.insert("comma", VK_OEM_COMMA);
        knownAliasesX11SymVK.insert("period", VK_OEM_PERIOD);
        knownAliasesX11SymVK.insert("KP_0", VK_NUMPAD0);
        knownAliasesX11SymVK.insert("KP_1", VK_NUMPAD1);
        knownAliasesX11SymVK.insert("KP_2", VK_NUMPAD2);
        knownAliasesX11SymVK.insert("KP_3", VK_NUMPAD3);
        knownAliasesX11SymVK.insert("KP_4", VK_NUMPAD4);
        knownAliasesX11SymVK.insert("KP_5", VK_NUMPAD5);
        knownAliasesX11SymVK.insert("KP_6", VK_NUMPAD6);
        knownAliasesX11SymVK.insert("KP_7", VK_NUMPAD7);
        knownAliasesX11SymVK.insert("KP_8", VK_NUMPAD8);
        knownAliasesX11SymVK.insert("KP_9", VK_NUMPAD9);
        knownAliasesX11SymVK.insert("Num_Lock", VK_NUMLOCK);
        knownAliasesX11SymVK.insert("KP_Divide", VK_DIVIDE);
        knownAliasesX11SymVK.insert("KP_Multiply", VK_MULTIPLY);
        knownAliasesX11SymVK.insert("KP_Subtract", VK_SUBTRACT);
        knownAliasesX11SymVK.insert("KP_Add", VK_ADD);
        knownAliasesX11SymVK.insert("KP_Decimal", VK_DECIMAL);
        knownAliasesX11SymVK.insert("Scroll_Lock", VK_SCROLL);
        knownAliasesX11SymVK.insert("Print", VK_SNAPSHOT);
        knownAliasesX11SymVK.insert("Multi_key", VK_RMENU);
    }

    if (knownAliasesVKStrings.isEmpty())
    {
        knownAliasesVKStrings.insert(VK_LWIN, QObject::tr("Super"));
        knownAliasesVKStrings.insert(VK_APPS, QObject::tr("Menu"));
        knownAliasesVKStrings.insert(VK_VOLUME_MUTE, QObject::tr("Mute"));
        knownAliasesVKStrings.insert(VK_VOLUME_UP, QObject::tr("Vol+"));
        knownAliasesVKStrings.insert(VK_VOLUME_DOWN, QObject::tr("Vol-"));
        knownAliasesVKStrings.insert(VK_MEDIA_PLAY_PAUSE, QObject::tr("Play/Pause"));
        knownAliasesVKStrings.insert(VK_PLAY, QObject::tr("Play"));
        knownAliasesVKStrings.insert(VK_PAUSE, QObject::tr("Pause"));
        knownAliasesVKStrings.insert(VK_MEDIA_PREV_TRACK, QObject::tr("Prev"));
        knownAliasesVKStrings.insert(VK_MEDIA_NEXT_TRACK, QObject::tr("Next"));
        knownAliasesVKStrings.insert(VK_LAUNCH_MAIL, QObject::tr("Mail"));
        knownAliasesVKStrings.insert(VK_HOME, QObject::tr("Home"));
        knownAliasesVKStrings.insert(VK_LAUNCH_MEDIA_SELECT, QObject::tr("Media"));
        knownAliasesVKStrings.insert(VK_BROWSER_SEARCH, QObject::tr("Search"));
    }
}

/**
 * @brief Obtain a more specific virtual key (unsigned int) for a key grab event.
 * @param Scan code obtained from a key grab event
 * @param Virtual key obtained from a key grab event
 * @return Corrected virtual key as an unsigned int
 */
unsigned int WinExtras::correctVirtualKey(unsigned int scancode, unsigned int virtualkey)
{
    int mapvirtual = MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX);
    int extended = (scancode & EXTENDED_FLAG) != 0;

    int finalvirtual = 0;
    switch (virtualkey)
    {
    case VK_CONTROL:
        finalvirtual = extended ? VK_RCONTROL : VK_LCONTROL;
        break;
    case VK_SHIFT:
        finalvirtual = mapvirtual;
        break;
    case VK_MENU:
        finalvirtual = extended ? VK_RMENU : VK_LMENU;
        break;
    case 0x5E:
        // Ignore System Reserved VK
        finalvirtual = 0;
        break;
    default:
        finalvirtual = virtualkey;
    }

    return finalvirtual;
}

/**
 * @brief Convert a virtual key into the corresponding keyboard scan code.
 * @param Windows virtual key
 * @param Qt key alias
 * @return Keyboard scan code as an unsigned int
 */
unsigned int WinExtras::scancodeFromVirtualKey(unsigned int virtualkey, unsigned int alias)
{
    int scancode = 0;
    if (virtualkey == VK_PAUSE)
    {
        // MapVirtualKey does not work with VK_PAUSE
        scancode = 0x45;
    }
    else
    {
        scancode = MapVirtualKey(virtualkey, MAPVK_VK_TO_VSC);
    }

    switch (virtualkey)
    {
         case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN: // arrow keys
         case VK_PRIOR: case VK_NEXT: // page up and page down
         case VK_END: case VK_HOME:
         case VK_INSERT: case VK_DELETE:
         case VK_DIVIDE: // numpad slash
         case VK_NUMLOCK:
         case VK_RCONTROL:
         case VK_RMENU:
         {
             scancode |= EXTENDED_FLAG; // set extended bit
             break;
         }
         case VK_RETURN:
         {
             // Remove ambiguity between Enter and Numpad Enter.
             // In Windows, VK_RETURN is used for both.
             if (alias == Qt::Key_Enter)
             {
                 scancode |= EXTENDED_FLAG; // set extended bit
                 break;
             }
         }
    }

    return scancode;
}

/**
 * @brief Check foreground window (window in focus) and obtain the
 *     corresponding exe file path.
 * @return File path of executable
 */
QString WinExtras::getForegroundWindowExePath()
{
    QString exePath;
    HWND foreground = GetForegroundWindow();
    HANDLE windowProcess = NULL;
    if (foreground)
    {
        DWORD processId;
        GetWindowThreadProcessId(foreground, &processId);
        windowProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, true, processId);
    }

    if (windowProcess != NULL)
    {
        TCHAR filename[MAX_PATH];
        memset(filename, 0, sizeof(filename));
        //qDebug() << QString::number(sizeof(filename)/sizeof(TCHAR));
        if (pQueryFullProcessImageNameW)
        {
            // Windows Vista and later
            DWORD pathLength = MAX_PATH * sizeof(TCHAR);
            pQueryFullProcessImageNameW(windowProcess, 0, filename, &pathLength);
            //qDebug() << pathLength;
        }
        else
        {
            // Windows XP
            GetModuleFileNameEx(windowProcess, NULL, filename, MAX_PATH * sizeof(TCHAR));
            //qDebug() << pathLength;
        }

        exePath = QString::fromWCharArray(filename);
        //qDebug() << QString::fromWCharArray(filename);
        CloseHandle(windowProcess);
    }

    return exePath;
}

bool WinExtras::containsFileAssociationinRegistry()
{
    bool result = false;

    QSettings associationReg(FILEASSOCIATIONKEY, QSettings::NativeFormat);
    QString temp = associationReg.value("Default", "").toString();
    if (!temp.isEmpty())
    {
        result = true;
    }

    return result;
}

void WinExtras::writeFileAssocationToRegistry()
{
    QSettings fileAssociationReg(FILEASSOCIATIONKEY, QSettings::NativeFormat);
    fileAssociationReg.setValue("Default", "AntiMicro.amgp");
    fileAssociationReg.sync();

    QSettings programAssociationReg(PROGRAMASSOCIATIONKEY, QSettings::NativeFormat);
    programAssociationReg.setValue("Default", tr("AntiMicro Profile"));
    programAssociationReg.setValue("shell/open/command/Default", QString("\"%1\" \"%2\"").arg(QDir::toNativeSeparators(qApp->applicationFilePath())).arg("%1"));
    programAssociationReg.setValue("DefaultIcon/Default", QString("%1,%2").arg(QDir::toNativeSeparators(qApp->applicationFilePath())).arg("0"));
    programAssociationReg.sync();

    // Required to refresh settings used in Windows Explorer
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);
}

void WinExtras::removeFileAssociationFromRegistry()
{
    QSettings fileAssociationReg(FILEASSOCIATIONKEY, QSettings::NativeFormat);
    QString currentValue = fileAssociationReg.value("Default", "").toString();
    if (currentValue == "AntiMicro.amgp")
    {
        fileAssociationReg.remove("Default");
        fileAssociationReg.sync();
    }

    QSettings programAssociationReg(PROGRAMASSOCIATIONKEY, QSettings::NativeFormat);
    programAssociationReg.remove("");
    programAssociationReg.sync();

    // Required to refresh settings used in Windows Explorer
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);
}

/**
 * @brief Attempt to elevate process using runas
 * @return Execution status
 */
bool WinExtras::elevateAntiMicro()
{
    QString antiProgramLocation = QDir::toNativeSeparators(qApp->applicationFilePath());
    QByteArray temp = antiProgramLocation.toUtf8();
    SHELLEXECUTEINFO sei = { sizeof(sei) };
    wchar_t tempverb[6];
    wchar_t tempfile[antiProgramLocation.length() + 1];
    QString("runas").toWCharArray(tempverb);
    antiProgramLocation.toWCharArray(tempfile);
    tempverb[5] = '\0';
    tempfile[antiProgramLocation.length()] = '\0';
    sei.lpVerb = tempverb;
    sei.lpFile = tempfile;
    sei.hwnd = NULL;
    sei.nShow = SW_NORMAL;
    BOOL result = ShellExecuteEx(&sei);
    return result;
}

/**
 * @brief Check if the application is running with administrative privileges.
 * @return Status indicating administrative privileges
 */
bool WinExtras::IsRunningAsAdmin()
{
    BOOL isAdmin = FALSE;
    PSID administratorsGroup;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    isAdmin = AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                             DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                             &administratorsGroup);
    if (isAdmin)
    {
        if (!CheckTokenMembership(NULL, administratorsGroup, &isAdmin))
        {
            isAdmin = FALSE;
        }
        FreeSid(administratorsGroup);
    }

    return isAdmin;
}

/**
 * @brief Temporarily disable "Enhanced Pointer Precision".
 */
void WinExtras::disablePointerPrecision()
{
    int mouseInfo[3];
    SystemParametersInfo(SPI_GETMOUSE, 0, &mouseInfo, 0);
    if (mouseInfo[2] == 1 && mouseInfo[2] == originalMouseAccel)
    {
        mouseInfo[2] = 0;
        SystemParametersInfo(SPI_SETMOUSE, 0, &mouseInfo, 0);
    }
}

/**
 * @brief If "Enhanced Pointer Precision" is currently disabled and
 *     the setting has not been changed explicitly by the user while
 *     the program has been running, re-enable "Enhanced Pointer Precision".
 *     Return the mouse behavior to normal.
 */
void WinExtras::enablePointerPrecision()
{
    int mouseInfo[3];
    SystemParametersInfo(SPI_GETMOUSE, 0, &mouseInfo, 0);
    if (mouseInfo[2] == 0 && mouseInfo[2] != originalMouseAccel)
    {
        mouseInfo[2] = originalMouseAccel;
        SystemParametersInfo(SPI_SETMOUSE, 0, &mouseInfo, 0);
    }
}

/**
 * @brief Used to check if the "Enhance Pointer Precision" Windows
 *     option is currently enabled.
 * @return Status of "Enhanced Pointer Precision"
 */
bool WinExtras::isUsingEnhancedPointerPrecision()
{
    bool result = false;

    int mouseInfo[3];
    SystemParametersInfo(SPI_GETMOUSE, 0, &mouseInfo, 0);

    if (mouseInfo[2] > 0)
    {
        result = true;
    }

    return result;
}

/**
 * @brief Get the value of "Enhanced Pointer Precision" when the program
 *     first starts. Needed to not override setting if the option has
 *     been disabled in Windows by the user.
 */
void WinExtras::grabCurrentPointerPrecision()
{
    int mouseInfo[3];
    SystemParametersInfo(SPI_GETMOUSE, 0, &mouseInfo, 0);
    originalMouseAccel = mouseInfo[2];
}

/**
 * @brief Get the window text of the window currently in focus.
 * @return Window title of application in focus.
 */
QString WinExtras::getCurrentWindowText()
{
    QString windowText;

    HWND foreground = GetForegroundWindow();

    if (foreground != NULL)
    {
        TCHAR foundWindowTitle[256];
        memset(foundWindowTitle, 0, sizeof(foundWindowTitle));
        GetWindowTextW(foreground, foundWindowTitle, 255);
        QString temp = QString::fromWCharArray(foundWindowTitle);
        if (temp.isEmpty())
        {
            memset(foundWindowTitle, 0, sizeof(foundWindowTitle));
            SendMessageA(foreground, WM_GETTEXT, 255, (LPARAM)foundWindowTitle);
            temp = QString::fromWCharArray(foundWindowTitle);
        }

        if (!temp.isEmpty())
        {
            windowText = temp;
        }
    }

    return windowText;
}

bool WinExtras::raiseProcessPriority()
{
    bool result = false;
    result = SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    /*if (!result)
    {
        qDebug() << "COULD NOT RAISE PROCESS PRIORITY";
    }
    */

    return result;
}

QPoint WinExtras::getCursorPos()
{
    POINT cursorPoint;
    GetCursorPos(&cursorPoint);

    QPoint temp(cursorPoint.x, cursorPoint.y);
    return temp;
}
