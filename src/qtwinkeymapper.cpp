#include <qt_windows.h>
#include <QHashIterator>

#include "qtwinkeymapper.h"

QtWinKeyMapper QtWinKeyMapper::_instance;

QtWinKeyMapper::QtWinKeyMapper(QObject *parent) :
    QObject(parent)
{
    populateMappingHashes();
}

void QtWinKeyMapper::populateMappingHashes()
{
    if (qtKeyToWinVirtualKey.isEmpty())
    {
        qtKeyToWinVirtualKey[Qt::Key_Cancel] = VK_CANCEL;
        qtKeyToWinVirtualKey[Qt::Key_Backspace] = VK_BACK;
        qtKeyToWinVirtualKey[Qt::Key_Tab] = VK_TAB;
        qtKeyToWinVirtualKey[Qt::Key_Clear] = VK_CLEAR;
        qtKeyToWinVirtualKey[Qt::Key_Return] = VK_RETURN;
        //qtKeyToWinVirtualKey[Qt::Key_Shift] = VK_SHIFT;
        //qtKeyToWinVirtualKey[Qt::Key_Control] = VK_CONTROL;
        //qtKeyToWinVirtualKey[Qt::Key_Alt] = VK_MENU;
        qtKeyToWinVirtualKey[Qt::Key_Pause] = VK_PAUSE;
        qtKeyToWinVirtualKey[Qt::Key_CapsLock] = VK_CAPITAL;
        qtKeyToWinVirtualKey[Qt::Key_Escape] = VK_ESCAPE;
        qtKeyToWinVirtualKey[Qt::Key_Mode_switch] = VK_MODECHANGE;
        qtKeyToWinVirtualKey[Qt::Key_Space] = VK_SPACE;
        qtKeyToWinVirtualKey[Qt::Key_PageUp] = VK_PRIOR;
        qtKeyToWinVirtualKey[Qt::Key_PageDown] = VK_NEXT;
        qtKeyToWinVirtualKey[Qt::Key_End] = VK_END;
        qtKeyToWinVirtualKey[Qt::Key_Home] = VK_HOME;
        qtKeyToWinVirtualKey[Qt::Key_Left] = VK_LEFT;
        qtKeyToWinVirtualKey[Qt::Key_Up] = VK_UP;
        qtKeyToWinVirtualKey[Qt::Key_Right] = VK_RIGHT;
        qtKeyToWinVirtualKey[Qt::Key_Down] = VK_DOWN;
        qtKeyToWinVirtualKey[Qt::Key_Select] = VK_SELECT;
        qtKeyToWinVirtualKey[Qt::Key_Printer] = VK_PRINT;
        qtKeyToWinVirtualKey[Qt::Key_Execute] = VK_EXECUTE;
        qtKeyToWinVirtualKey[Qt::Key_Print] = VK_SNAPSHOT;
        qtKeyToWinVirtualKey[Qt::Key_Insert] = VK_INSERT;
        qtKeyToWinVirtualKey[Qt::Key_Delete] = VK_DELETE;
        qtKeyToWinVirtualKey[Qt::Key_Help] = VK_HELP;
        qtKeyToWinVirtualKey[Qt::Key_Meta] = VK_LWIN;
        //qtKeyToWinVirtualKey[Qt::Key_Meta] = VK_RWIN;
        qtKeyToWinVirtualKey[Qt::Key_Menu] = VK_APPS;
        qtKeyToWinVirtualKey[Qt::Key_Sleep] = VK_SLEEP;

        qtKeyToWinVirtualKey[Qt::Key_Asterisk] = VK_MULTIPLY;
        qtKeyToWinVirtualKey[Qt::Key_Plus] = VK_ADD;
        qtKeyToWinVirtualKey[Qt::Key_Comma] = VK_SEPARATOR;
        qtKeyToWinVirtualKey[Qt::Key_Minus] = VK_SUBTRACT;
        qtKeyToWinVirtualKey[Qt::Key_Period] = VK_DECIMAL;
        qtKeyToWinVirtualKey[Qt::Key_Slash] = VK_DIVIDE;

        qtKeyToWinVirtualKey[Qt::Key_NumLock] = VK_NUMLOCK;
        qtKeyToWinVirtualKey[Qt::Key_ScrollLock] = VK_SCROLL;
        qtKeyToWinVirtualKey[Qt::Key_Massyo] = VK_OEM_FJ_MASSHOU;
        qtKeyToWinVirtualKey[Qt::Key_Touroku] = VK_OEM_FJ_TOUROKU;

        qtKeyToWinVirtualKey[Qt::Key_Shift] = VK_LSHIFT;
        //qtKeyToWinVirtualKey[Qt::Key_Shift] = VK_RSHIFT;
        qtKeyToWinVirtualKey[Qt::Key_Control] = VK_LCONTROL;
        //qtKeyToWinVirtualKey[Qt::Key_Control] = VK_RCONTROL;
        qtKeyToWinVirtualKey[Qt::Key_Alt] = VK_LMENU;
        //qtKeyToWinVirtualKey[Qt::Key_Alt] = VK_RMENU;
        qtKeyToWinVirtualKey[Qt::Key_Back] = VK_BROWSER_BACK;
        qtKeyToWinVirtualKey[Qt::Key_Forward] = VK_BROWSER_FORWARD;
        qtKeyToWinVirtualKey[Qt::Key_Refresh] = VK_BROWSER_REFRESH;
        qtKeyToWinVirtualKey[Qt::Key_Stop] = VK_BROWSER_STOP;
        qtKeyToWinVirtualKey[Qt::Key_Search] = VK_BROWSER_SEARCH;
        qtKeyToWinVirtualKey[Qt::Key_Favorites] = VK_BROWSER_FAVORITES;
        qtKeyToWinVirtualKey[Qt::Key_HomePage] = VK_BROWSER_HOME;
        qtKeyToWinVirtualKey[Qt::Key_VolumeMute] = VK_VOLUME_MUTE;
        qtKeyToWinVirtualKey[Qt::Key_VolumeDown] = VK_VOLUME_DOWN;
        qtKeyToWinVirtualKey[Qt::Key_VolumeUp] = VK_VOLUME_UP;
        qtKeyToWinVirtualKey[Qt::Key_MediaNext] = VK_MEDIA_NEXT_TRACK;
        qtKeyToWinVirtualKey[Qt::Key_MediaPrevious] = VK_MEDIA_PREV_TRACK;
        qtKeyToWinVirtualKey[Qt::Key_MediaStop] = VK_MEDIA_STOP;
        qtKeyToWinVirtualKey[Qt::Key_MediaPlay] = VK_MEDIA_PLAY_PAUSE;
        qtKeyToWinVirtualKey[Qt::Key_LaunchMail] = VK_LAUNCH_MAIL;
        qtKeyToWinVirtualKey[Qt::Key_LaunchMedia] = VK_LAUNCH_MEDIA_SELECT;
        qtKeyToWinVirtualKey[Qt::Key_Launch0] = VK_LAUNCH_APP1;
        qtKeyToWinVirtualKey[Qt::Key_Launch1] = VK_LAUNCH_APP2;

        qtKeyToWinVirtualKey[Qt::Key_Play] = VK_PLAY;
        qtKeyToWinVirtualKey[Qt::Key_Zoom] = VK_ZOOM;
        //qtKeyToWinVirtualKey[Qt::Key_Clear] = VK_OEM_CLEAR;

        qtKeyToWinVirtualKey[Qt::Key_QuoteLeft] = VK_OEM_3;
        qtKeyToWinVirtualKey[Qt::Key_BracketLeft] = VK_OEM_4;
        qtKeyToWinVirtualKey[Qt::Key_Backslash] = VK_OEM_5;
        qtKeyToWinVirtualKey[Qt::Key_BracketRight] = VK_OEM_6;

        // Map 0-9 ASCII codes
        for (int i=0; i <= (VK_9 - VK_0); i++)
        {
            qtKeyToWinVirtualKey[Qt::Key_0 + i] = VK_0 + i;
        }

        // Map [-` ASCII codes
        for (int i=0; i <= (VK_9 - VK_0); i++)
        {
            qtKeyToWinVirtualKey[Qt::Key_0 + i] = VK_0 + i;
        }

        // Map A-Z ASCII codes
        for (int i=0; i <= (VK_Z - VK_A); i++)
        {
            qtKeyToWinVirtualKey[Qt::Key_A + i] = VK_A + i;
        }

        // Map function keys
        for (int i=0; i <= (VK_F24 - VK_F1); i++)
        {
            qtKeyToWinVirtualKey[Qt::Key_F1 + i] = VK_F1 + i;
        }

        // Map numpad keys
        for (int i=0; i <= (VK_NUMPAD9 - VK_NUMPAD0); i++)
        {
            qtKeyToWinVirtualKey[AntKey_KP_0 + i] = VK_NUMPAD0 + i;
        }

        // Map custom keys
        qtKeyToWinVirtualKey[AntKey_Alt_R] = VK_RMENU;
        qtKeyToWinVirtualKey[AntKey_Meta_R] = VK_RWIN;
        qtKeyToWinVirtualKey[AntKey_Shift_R] = VK_RSHIFT;
        qtKeyToWinVirtualKey[AntKey_Control_R] = VK_RCONTROL;

        QHashIterator<unsigned int, unsigned int> iter(qtKeyToWinVirtualKey);
        while (iter.hasNext())
        {
            iter.next();
            winVirtualKeyToQtKey[iter.value()] = iter.key();
        }
    }
}

unsigned int QtWinKeyMapper::returnVirtualKey(unsigned int qkey)
{
    return _instance.qtKeyToWinVirtualKey.value(qkey);
}

unsigned int QtWinKeyMapper::returnQtKey(unsigned int key)
{
    return _instance.winVirtualKeyToQtKey.value(key);
}
