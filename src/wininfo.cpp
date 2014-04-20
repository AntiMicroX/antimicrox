#define _WIN32_WINNT 0x0600

#include <qt_windows.h>
#include <psapi.h>
#include <QHashIterator>

#include "wininfo.h"

typedef DWORD(WINAPI *MYPROC)(HANDLE, DWORD,LPTSTR,PDWORD);
// Check if QueryFullProcessImageNameW function exists in kernel32.dll.
// Function does not exist in Windows XP.
static MYPROC pQueryFullProcessImageNameW = (MYPROC) GetProcAddress(
            GetModuleHandle(TEXT("kernel32.dll")), "QueryFullProcessImageNameW");

static bool isWindowsVistaOrHigher()
{
    OSVERSIONINFO osvi;
    memset(&osvi, 0, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);
    return (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion >= 6);
}

const unsigned int WinInfo::EXTENDED_FLAG = 0x100;
WinInfo WinInfo::_instance;

WinInfo::WinInfo(QObject *parent) :
    QObject(parent)
{
    populateKnownAliases();
}

QString WinInfo::getDisplayString(unsigned int virtualkey)
{
    QString temp;
    if (_instance.knownAliasesVKStrings.contains(virtualkey))
    {
        temp = _instance.knownAliasesVKStrings.value(virtualkey);
    }

    return temp;
}

unsigned int WinInfo::getVirtualKey(QString codestring)
{
    int temp = 0;
    if (_instance.knownAliasesX11SymVK.contains(codestring))
    {
        temp = _instance.knownAliasesX11SymVK.value(codestring);
    }

    return temp;
}

void WinInfo::populateKnownAliases()
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
        knownAliasesX11SymVK.insert("Shift_L", VK_SHIFT);
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
    }

    if (knownAliasesVKStrings.isEmpty())
    {
        knownAliasesVKStrings.insert(VK_LWIN, QObject::tr("Super"));
        knownAliasesVKStrings.insert(VK_APPS, QObject::tr("Menu"));
    }
}

unsigned int WinInfo::correctVirtualKey(unsigned int scancode, unsigned int virtualkey)
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
    default:
        finalvirtual = virtualkey;
    }

    return finalvirtual;
}

unsigned int WinInfo::scancodeFromVirtualKey(unsigned int virtualkey, unsigned int alias)
{
    int scancode = 0;
    if (virtualkey == VK_PAUSE)
    {
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
             if (alias == Qt::Key_Enter)
             {
                 scancode |= EXTENDED_FLAG; // set extended bit
                 break;
             }
         }
    }

    return scancode;
}

QString WinInfo::getForegroundWindowExePath()
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
