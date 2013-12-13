#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

#include <QHashIterator>

#include "qtwinkeymapper.h"

QtWinKeyMapper QtWinKeyMapper::_instance;

QtWinKeyMapper::QtWinKeyMapper(QObject *parent) :
    QtKeyMapperBase(parent)
{
    populateMappingHashes();
}

static void QtWinKeyMapper::populateMappingHashes()
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
        qtKeyToVirtualKey[Qt::Key_Plus] = VK_ADD;
        qtKeyToVirtualKey[Qt::Key_Comma] = VK_SEPARATOR;
        qtKeyToVirtualKey[Qt::Key_Minus] = VK_SUBTRACT;
        qtKeyToVirtualKey[Qt::Key_Period] = VK_DECIMAL;
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

        qtKeyToVirtualKey[Qt::Key_Play] = VK_PLAY;
        qtKeyToVirtualKey[Qt::Key_Zoom] = VK_ZOOM;
        //qtKeyToWinVirtualKey[Qt::Key_Clear] = VK_OEM_CLEAR;

        qtKeyToVirtualKey[Qt::Key_QuoteLeft] = VK_OEM_3;
        qtKeyToVirtualKey[Qt::Key_BracketLeft] = VK_OEM_4;
        qtKeyToVirtualKey[Qt::Key_Backslash] = VK_OEM_5;
        qtKeyToVirtualKey[Qt::Key_BracketRight] = VK_OEM_6;

        // Map 0-9 ASCII codes
        for (int i=0; i <= (VK_9 - VK_0); i++)
        {
            qtKeyToVirtualKey[Qt::Key_0 + i] = VK_0 + i;
        }

        // Map [-` ASCII codes
        for (int i=0; i <= (VK_9 - VK_0); i++)
        {
            qtKeyToVirtualKey[Qt::Key_0 + i] = VK_0 + i;
        }

        // Map A-Z ASCII codes
        for (int i=0; i <= (VK_Z - VK_A); i++)
        {
            qtKeyToVirtualKey[Qt::Key_A + i] = VK_A + i;
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

        QHashIterator<unsigned int, unsigned int> iter(qtKeyToVirtualKey);
        while (iter.hasNext())
        {
            iter.next();
            virtualKeyToQtKey[iter.value()] = iter.key();
        }
    }
}

unsigned int QtWinKeyMapper::returnInstanceQtKey(unsigned int key)
{
    return virtualKeyToQtKey.value(key);
}

unsigned int QtWinKeyMapper::returnInstanceVirtualKey(unsigned int qkey)
{
    return qtKeyToVirtualKey.value(qkey);
}

unsigned int QtWinKeyMapper::returnQtKey(unsigned int key)
{
    return _instance.returnInstanceQtKey(key);
}

unsigned int QtWinKeyMapper::returnVirtualKey(unsigned int qkey)
{
    return _instance.returnVirtualKey(qkey);
}
