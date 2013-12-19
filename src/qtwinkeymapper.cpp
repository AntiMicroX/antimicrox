#include <qt_windows.h>
#include <QHashIterator>

#include "qtwinkeymapper.h"

QtWinKeyMapper::QtWinKeyMapper(QObject *parent) :
    QtKeyMapperBase(parent)
{
    populateMappingHashes();
}

void QtWinKeyMapper::populateMappingHashes()
{
    if (qtKeyToVirtualKey.isEmpty())
    {
        qtKeyToVirtualKey[Qt::Key_Cancel] = VK_CANCEL;
        qtKeyToVirtualKey[Qt::Key_Backspace] = VK_BACK;
        qtKeyToVirtualKey[Qt::Key_Tab] = VK_TAB;
        qtKeyToVirtualKey[Qt::Key_Clear] = VK_CLEAR;
        qtKeyToVirtualKey[Qt::Key_Return] = VK_RETURN;
        //qtKeyToWinVirtualKey[Qt::Key_Shift] = VK_SHIFT;
        //qtKeyToWinVirtualKey[Qt::Key_Control] = VK_CONTROL;
        //qtKeyToWinVirtualKey[Qt::Key_Alt] = VK_MENU;
        qtKeyToVirtualKey[Qt::Key_Pause] = VK_PAUSE;
        qtKeyToVirtualKey[Qt::Key_CapsLock] = VK_CAPITAL;
        qtKeyToVirtualKey[Qt::Key_Escape] = VK_ESCAPE;
        qtKeyToVirtualKey[Qt::Key_Mode_switch] = VK_MODECHANGE;
        qtKeyToVirtualKey[Qt::Key_Space] = VK_SPACE;
        qtKeyToVirtualKey[Qt::Key_PageUp] = VK_PRIOR;
        qtKeyToVirtualKey[Qt::Key_PageDown] = VK_NEXT;
        qtKeyToVirtualKey[Qt::Key_End] = VK_END;
        qtKeyToVirtualKey[Qt::Key_Home] = VK_HOME;
        qtKeyToVirtualKey[Qt::Key_Left] = VK_LEFT;
        qtKeyToVirtualKey[Qt::Key_Up] = VK_UP;
        qtKeyToVirtualKey[Qt::Key_Right] = VK_RIGHT;
        qtKeyToVirtualKey[Qt::Key_Down] = VK_DOWN;
        qtKeyToVirtualKey[Qt::Key_Select] = VK_SELECT;
        qtKeyToVirtualKey[Qt::Key_Printer] = VK_PRINT;
        qtKeyToVirtualKey[Qt::Key_Execute] = VK_EXECUTE;
        qtKeyToVirtualKey[Qt::Key_Print] = VK_SNAPSHOT;
        qtKeyToVirtualKey[Qt::Key_Insert] = VK_INSERT;
        qtKeyToVirtualKey[Qt::Key_Delete] = VK_DELETE;
        qtKeyToVirtualKey[Qt::Key_Help] = VK_HELP;
        qtKeyToVirtualKey[Qt::Key_Meta] = VK_LWIN;
        //qtKeyToWinVirtualKey[Qt::Key_Meta] = VK_RWIN;
        qtKeyToVirtualKey[Qt::Key_Menu] = VK_APPS;
        qtKeyToVirtualKey[Qt::Key_Sleep] = VK_SLEEP;

        qtKeyToVirtualKey[Qt::Key_Asterisk] = VK_MULTIPLY;
        qtKeyToVirtualKey[AntKey_KP_Add] = VK_ADD;
        qtKeyToVirtualKey[Qt::Key_Comma] = VK_SEPARATOR;
        qtKeyToVirtualKey[AntKey_KP_Subtract] = VK_SUBTRACT;
        qtKeyToVirtualKey[AntKey_KP_Decimal] = VK_DECIMAL;
        qtKeyToVirtualKey[Qt::Key_Slash] = VK_DIVIDE;

        qtKeyToVirtualKey[Qt::Key_NumLock] = VK_NUMLOCK;
        qtKeyToVirtualKey[Qt::Key_ScrollLock] = VK_SCROLL;
        qtKeyToVirtualKey[Qt::Key_Massyo] = VK_OEM_FJ_MASSHOU;
        qtKeyToVirtualKey[Qt::Key_Touroku] = VK_OEM_FJ_TOUROKU;

        qtKeyToVirtualKey[Qt::Key_Shift] = VK_LSHIFT;
        //qtKeyToWinVirtualKey[Qt::Key_Shift] = VK_RSHIFT;
        qtKeyToVirtualKey[Qt::Key_Control] = VK_LCONTROL;
        //qtKeyToWinVirtualKey[Qt::Key_Control] = VK_RCONTROL;
        qtKeyToVirtualKey[Qt::Key_Alt] = VK_LMENU;
        //qtKeyToWinVirtualKey[Qt::Key_Alt] = VK_RMENU;
        qtKeyToVirtualKey[Qt::Key_Back] = VK_BROWSER_BACK;
        qtKeyToVirtualKey[Qt::Key_Forward] = VK_BROWSER_FORWARD;
        qtKeyToVirtualKey[Qt::Key_Refresh] = VK_BROWSER_REFRESH;
        qtKeyToVirtualKey[Qt::Key_Stop] = VK_BROWSER_STOP;
        qtKeyToVirtualKey[Qt::Key_Search] = VK_BROWSER_SEARCH;
        qtKeyToVirtualKey[Qt::Key_Favorites] = VK_BROWSER_FAVORITES;
        qtKeyToVirtualKey[Qt::Key_HomePage] = VK_BROWSER_HOME;
        qtKeyToVirtualKey[Qt::Key_VolumeMute] = VK_VOLUME_MUTE;
        qtKeyToVirtualKey[Qt::Key_VolumeDown] = VK_VOLUME_DOWN;
        qtKeyToVirtualKey[Qt::Key_VolumeUp] = VK_VOLUME_UP;
        qtKeyToVirtualKey[Qt::Key_MediaNext] = VK_MEDIA_NEXT_TRACK;
        qtKeyToVirtualKey[Qt::Key_MediaPrevious] = VK_MEDIA_PREV_TRACK;
        qtKeyToVirtualKey[Qt::Key_MediaStop] = VK_MEDIA_STOP;
        qtKeyToVirtualKey[Qt::Key_MediaPlay] = VK_MEDIA_PLAY_PAUSE;
        qtKeyToVirtualKey[Qt::Key_LaunchMail] = VK_LAUNCH_MAIL;
        qtKeyToVirtualKey[Qt::Key_LaunchMedia] = VK_LAUNCH_MEDIA_SELECT;
        qtKeyToVirtualKey[Qt::Key_Launch0] = VK_LAUNCH_APP1;
        qtKeyToVirtualKey[Qt::Key_Launch1] = VK_LAUNCH_APP2;

        qtKeyToVirtualKey[Qt::Key_Semicolon] = VK_OEM_1;
        qtKeyToVirtualKey[Qt::Key_Plus] = VK_OEM_2;
        qtKeyToVirtualKey[Qt::Key_Equal] = VK_OEM_PLUS;
        qtKeyToVirtualKey[Qt::Key_Minus] = VK_OEM_MINUS;
        qtKeyToVirtualKey[Qt::Key_Period]  = VK_OEM_PERIOD;
        qtKeyToVirtualKey[Qt::Key_QuoteLeft] = VK_OEM_3;
        qtKeyToVirtualKey[Qt::Key_BracketLeft] = VK_OEM_4;
        qtKeyToVirtualKey[Qt::Key_Backslash] = VK_OEM_5;
        qtKeyToVirtualKey[Qt::Key_BracketRight] = VK_OEM_6;
        qtKeyToVirtualKey[Qt::Key_Apostrophe] = VK_OEM_7;

        qtKeyToVirtualKey[Qt::Key_Play] = VK_PLAY;
        qtKeyToVirtualKey[Qt::Key_Zoom] = VK_ZOOM;
        //qtKeyToWinVirtualKey[Qt::Key_Clear] = VK_OEM_CLEAR;

        // Map 0-9 ASCII codes
        for (int i=0; i <= (0x39 - 0x30); i++)
        {
            qtKeyToVirtualKey[Qt::Key_0 + i] = 0x30 + i;
        }

        // Map A-Z ASCII codes
        for (int i=0; i <= (0x5a - 0x41); i++)
        {
            qtKeyToVirtualKey[Qt::Key_A + i] = 0x41 + i;
        }

        // Map function keys
        for (int i=0; i <= (VK_F24 - VK_F1); i++)
        {
            qtKeyToVirtualKey[Qt::Key_F1 + i] = VK_F1 + i;
        }

        // Map numpad keys
        for (int i=0; i <= (VK_NUMPAD9 - VK_NUMPAD0); i++)
        {
            qtKeyToVirtualKey[AntKey_KP_0 + i] = VK_NUMPAD0 + i;
        }

        // Map custom keys
        qtKeyToVirtualKey[AntKey_Alt_R] = VK_RMENU;
        qtKeyToVirtualKey[AntKey_Meta_R] = VK_RWIN;
        qtKeyToVirtualKey[AntKey_Shift_R] = VK_RSHIFT;
        qtKeyToVirtualKey[AntKey_Control_R] = VK_RCONTROL;

        // Populate other hash. Flip key and value so mapping
        // goes VK -> Qt Key
        QHashIterator<unsigned int, unsigned int> iter(qtKeyToVirtualKey);
        while (iter.hasNext())
        {
            iter.next();
            virtualKeyToQtKey[iter.value()] = iter.key();
        }
    }
}
