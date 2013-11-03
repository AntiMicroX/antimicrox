#include "wininfo.h"

#include <windows.h>
#include <QHashIterator>

WinInfo WinInfo::_instance;

WinInfo::WinInfo(QObject *parent) :
    QObject(parent)
{
    populateKnownAliases();
}

WinInfo::~WinInfo()
{
    QHashIterator<QString, KeyInfo*> iter(_instance.knownAliases);
    while(iter.hasNext())
    {
        KeyInfo *info = iter.next().value();
        if (info)
        {
            delete info;
            info = 0;
        }
    }

    _instance.knownAliases.clear();
}

QString WinInfo::getDisplayString(QString codestring)
{
    QString temp;
    if (_instance.knownAliases.contains(codestring))
    {
        KeyInfo *currentKey = _instance.knownAliases.value(codestring);
        temp = currentKey->name;
    }

    return temp;
}

unsigned int WinInfo::getVirtualKey(QString codestring)
{
    int temp = 0;
    if (_instance.knownAliases.contains(codestring))
    {
        KeyInfo *currentKey = _instance.knownAliases.value(codestring);
        temp = currentKey->vk;
    }

    return temp;
}

void WinInfo::populateKnownAliases()
{
    // These aliases are needed for xstrings that would
     // return empty space characters from XLookupString
     if (knownAliases.isEmpty())
     {
         knownAliases.insert("Escape", new KeyInfo(VK_ESCAPE, tr("ESC")));
         knownAliases.insert("Tab", new KeyInfo(VK_TAB, tr("Tab")));
         knownAliases.insert("space", new KeyInfo(VK_SPACE, tr("Space")));
         knownAliases.insert("Delete", new KeyInfo(VK_DELETE, tr("DEL")));
         knownAliases.insert("Return", new KeyInfo(VK_RETURN, tr("Return")));
         knownAliases.insert("KP_Enter", new KeyInfo(VK_RETURN, tr("KP_Enter")));
         knownAliases.insert("BackSpace", new KeyInfo(VK_BACK, tr("Backspace")));
         knownAliases.insert("F1", new KeyInfo(VK_F1, tr("F1")));
         knownAliases.insert("F2", new KeyInfo(VK_F2, tr("F2")));
         knownAliases.insert("F3", new KeyInfo(VK_F3, tr("F3")));
         knownAliases.insert("F4", new KeyInfo(VK_F4, tr("F4")));
         knownAliases.insert("F5", new KeyInfo(VK_F5, tr("F5")));
         knownAliases.insert("F6", new KeyInfo(VK_F6, tr("F6")));
         knownAliases.insert("F7", new KeyInfo(VK_F7, tr("F7")));
         knownAliases.insert("F8", new KeyInfo(VK_F8, tr("F8")));
         knownAliases.insert("F9", new KeyInfo(VK_F9, tr("F9")));
         knownAliases.insert("F10", new KeyInfo(VK_F10, tr("F10")));
         knownAliases.insert("F11", new KeyInfo(VK_F11, tr("F11")));
         knownAliases.insert("F12", new KeyInfo(VK_F12, tr("F12")));
         knownAliases.insert("Shift_L", new KeyInfo(VK_SHIFT, tr("Shift (L)")));
         knownAliases.insert("Shift_R", new KeyInfo(VK_RSHIFT, tr("Shift (R)")));
         knownAliases.insert("Insert", new KeyInfo(VK_INSERT, tr("Insert")));
         knownAliases.insert("Pause", new KeyInfo(VK_PAUSE, tr("Pause")));
         knownAliases.insert("grave", new KeyInfo(VK_OEM_3, tr("grave")));
         knownAliases.insert("minus", new KeyInfo(VK_OEM_MINUS, tr("-")));
         knownAliases.insert("equal", new KeyInfo(VK_OEM_PLUS, tr("=")));
         knownAliases.insert("Caps_Lock", new KeyInfo(VK_CAPITAL, tr("Caps")));
         knownAliases.insert("Control_L", new KeyInfo(VK_CONTROL, tr("Ctrl (L)")));
         knownAliases.insert("Control_R", new KeyInfo(VK_RCONTROL, tr("Ctrl (R)")));
         knownAliases.insert("Alt_L", new KeyInfo(VK_MENU, tr("Alt (L)")));
         knownAliases.insert("Alt_R", new KeyInfo(VK_RMENU, tr("Alt (R)")));
         knownAliases.insert("Super_L", new KeyInfo(VK_LWIN, tr("Super")));
         knownAliases.insert("Menu", new KeyInfo(VK_RBUTTON, tr("Menu")));
         knownAliases.insert("Prior", new KeyInfo(VK_PRIOR, tr("PgUp")));
         knownAliases.insert("Next", new KeyInfo(VK_NEXT, tr("PgDn")));
         knownAliases.insert("Home", new KeyInfo(VK_HOME, tr("Home")));
         knownAliases.insert("End", new KeyInfo(VK_END, tr("End")));
         knownAliases.insert("Up", new KeyInfo(VK_UP, tr("Up")));
         knownAliases.insert("Down", new KeyInfo(VK_DOWN, tr("Down")));
         knownAliases.insert("Left", new KeyInfo(VK_LEFT, tr("Left")));
         knownAliases.insert("Right", new KeyInfo(VK_RIGHT, tr("Right")));
         knownAliases.insert("bracketleft", new KeyInfo(VK_OEM_4, tr("[")));
         knownAliases.insert("bracketright", new KeyInfo(VK_OEM_6, tr("]")));
         knownAliases.insert("backslash", new KeyInfo(VK_SEPARATOR, tr("\\")));
         knownAliases.insert("slash", new KeyInfo(VK_OEM_2, tr("/")));
         knownAliases.insert("semicolon", new KeyInfo(VK_OEM_1, tr(";")));
         knownAliases.insert("apostrophe", new KeyInfo(VK_OEM_7, tr("'")));
         knownAliases.insert("comma", new KeyInfo(VK_OEM_COMMA, tr(";")));
         knownAliases.insert("period", new KeyInfo(VK_OEM_PERIOD, tr(".")));
         knownAliases.insert("KP_0", new KeyInfo(VK_NUMPAD0, tr("0")));
         knownAliases.insert("KP_1", new KeyInfo(VK_NUMPAD1, tr("1")));
         knownAliases.insert("KP_2", new KeyInfo(VK_NUMPAD2, tr("2")));
         knownAliases.insert("KP_3", new KeyInfo(VK_NUMPAD3, tr("3")));
         knownAliases.insert("KP_4", new KeyInfo(VK_NUMPAD4, tr("4")));
         knownAliases.insert("KP_5", new KeyInfo(VK_NUMPAD5, tr("5")));
         knownAliases.insert("KP_6", new KeyInfo(VK_NUMPAD6, tr("6")));
         knownAliases.insert("KP_7", new KeyInfo(VK_NUMPAD7, tr("7")));
         knownAliases.insert("KP_8", new KeyInfo(VK_NUMPAD8, tr("8")));
         knownAliases.insert("KP_9", new KeyInfo(VK_NUMPAD9, tr("9")));
         knownAliases.insert("Num_Lock", new KeyInfo(VK_NUMLOCK, tr("NUM\nLK")));
         knownAliases.insert("KP_Divide", new KeyInfo(VK_DIVIDE, tr("/")));
         knownAliases.insert("KP_Multiply", new KeyInfo(VK_MULTIPLY, tr("*")));
         knownAliases.insert("KP_Subtract", new KeyInfo(VK_SUBTRACT, tr("-")));
         knownAliases.insert("KP_Add", new KeyInfo(VK_ADD, tr("+")));
         knownAliases.insert("KP_Decimal", new KeyInfo(VK_DECIMAL, tr(".")));
         knownAliases.insert("Scroll_Lock", new KeyInfo(VK_SCROLL, tr("SCLK")));
         knownAliases.insert("Print", new KeyInfo(VK_SNAPSHOT, tr("PRTSC")));
     }
}
