#define XK_MISCELLANY
#define XK_LATIN1
#define XK_KOREAN
#define XK_XKB_KEYS
#include <X11/keysymdef.h>
#include <X11/XF86keysym.h>

#include <QHashIterator>

#include "qtx11keymapper.h"

QtX11KeyMapper QtX11KeyMapper::_instance;

QtX11KeyMapper::QtX11KeyMapper(QObject *parent) :
    QObject(parent)
{
    populateMappingHashes();
}

/*
 * The following mappings are mainly taken from qkeymapper_x11.cpp.
 * There are portions of the mapping are customized to work around
 * some of the ambiguity introduced with some Qt keys
 * (XK_Alt_L and XK_Alt_R become Qt::Key_Alt
 */
void QtX11KeyMapper::populateMappingHashes()
{
    if (qtKeyToX11KeySym.isEmpty())
    {
        // misc keys

        qtKeyToX11KeySym[Qt::Key_Escape] = XK_Escape;
        qtKeyToX11KeySym[Qt::Key_Tab] = XK_Tab;
        qtKeyToX11KeySym[Qt::Key_Backtab] = XK_ISO_Left_Tab;
        qtKeyToX11KeySym[Qt::Key_Backspace] = XK_BackSpace;
        qtKeyToX11KeySym[Qt::Key_Return] = XK_Return;
        qtKeyToX11KeySym[Qt::Key_Insert] = XK_Insert;
        qtKeyToX11KeySym[Qt::Key_Delete] = XK_Delete;
        qtKeyToX11KeySym[Qt::Key_Delete] = XK_Delete;
        //qtKeyToX11KeySym[Qt::Key_Delete] = XK_Clear;
        qtKeyToX11KeySym[Qt::Key_Pause] = XK_Pause;
        qtKeyToX11KeySym[Qt::Key_Print] = XK_Print;

        // cursor movement

        qtKeyToX11KeySym[Qt::Key_Home] = XK_Home;
        qtKeyToX11KeySym[Qt::Key_End] = XK_End;
        qtKeyToX11KeySym[Qt::Key_Left] = XK_Left;
        qtKeyToX11KeySym[Qt::Key_Up] = XK_Up;
        qtKeyToX11KeySym[Qt::Key_Right] = XK_Right;
        qtKeyToX11KeySym[Qt::Key_Down] = XK_Down;
        qtKeyToX11KeySym[Qt::Key_PageUp] = XK_Prior;
        qtKeyToX11KeySym[Qt::Key_PageDown] = XK_Next;

        // modifiers

        qtKeyToX11KeySym[Qt::Key_Shift] = XK_Shift_L;
        //qtKeyToX11KeySym[Qt::Key_Shift] = XK_Shift_R;
        //qtKeyToX11KeySym[Qt::Key_Shift] = XK_Shift_Lock;
        qtKeyToX11KeySym[Qt::Key_Control] = XK_Control_L;
        //qtKeyToX11KeySym[Qt::Key_Control] = XK_Control_R;
        qtKeyToX11KeySym[Qt::Key_Meta] = XK_Meta_L;
        //qtKeyToX11KeySym[Qt::Key_Meta] = XK_Meta_R;
        qtKeyToX11KeySym[Qt::Key_Alt] = XK_Alt_L;
        //qtKeyToX11KeySym[Qt::Key_Alt] = XK_Alt_R;

        qtKeyToX11KeySym[Qt::Key_CapsLock] = XK_Caps_Lock;
        qtKeyToX11KeySym[Qt::Key_NumLock] = XK_Num_Lock;
        qtKeyToX11KeySym[Qt::Key_ScrollLock] = XK_Scroll_Lock;
        qtKeyToX11KeySym[Qt::Key_Super_L] = XK_Super_L;
        qtKeyToX11KeySym[Qt::Key_Super_R] = XK_Super_R;
        qtKeyToX11KeySym[Qt::Key_Menu] = XK_Menu;
        qtKeyToX11KeySym[Qt::Key_Hyper_L] = XK_Hyper_L;
        qtKeyToX11KeySym[Qt::Key_Hyper_R] = XK_Hyper_R;
        qtKeyToX11KeySym[Qt::Key_Help] = XK_Help;

        // numeric and function keypad keys

        qtKeyToX11KeySym[Qt::Key_Space] = XK_KP_Space;
        //qtKeyToX11KeySym[Qt::Key_Tab] = XK_KP_Tab;
        qtKeyToX11KeySym[Qt::Key_Enter] = XK_KP_Enter;
        qtKeyToX11KeySym[AntKey_KP_Home] = XK_KP_Home;
        //qtKeyToX11KeySym[Qt::Key_Home] = XK_KP_Home;
        qtKeyToX11KeySym[AntKey_KP_Left] = XK_KP_Left;
        qtKeyToX11KeySym[AntKey_KP_Up] = XK_KP_Up;
        qtKeyToX11KeySym[AntKey_KP_Right] = XK_KP_Right;
        qtKeyToX11KeySym[AntKey_KP_Down] = XK_KP_Down;
        qtKeyToX11KeySym[AntKey_KP_Prior] = XK_KP_Prior;
        qtKeyToX11KeySym[AntKey_KP_Next] = XK_KP_Next;
        qtKeyToX11KeySym[AntKey_KP_End] = XK_KP_End;
        qtKeyToX11KeySym[AntKey_KP_Begin] = XK_KP_Begin;
        qtKeyToX11KeySym[AntKey_KP_Insert] = XK_KP_Insert;
        qtKeyToX11KeySym[AntKey_KP_Delete] = XK_KP_Delete;
        qtKeyToX11KeySym[AntKey_KP_Delete] = XK_KP_Delete;
        //qtKeyToX11KeySym[AntKey_KP_Equal] = XK_KP_Equal;
        qtKeyToX11KeySym[AntKey_KP_Multiply] = XK_KP_Multiply;
        qtKeyToX11KeySym[AntKey_KP_Add] = XK_KP_Add;
        //qtKeyToX11KeySym[AntKey_KP_Separator] = XK_KP_Separator;
        qtKeyToX11KeySym[AntKey_KP_Subtract] = XK_KP_Subtract;

        qtKeyToX11KeySym[AntKey_KP_Decimal] = XK_KP_Decimal;
        qtKeyToX11KeySym[AntKey_KP_Divide] = XK_KP_Divide;

        // International input method support keys

        // International & multi-key character composition
        qtKeyToX11KeySym[Qt::Key_AltGr] = XK_ISO_Level3_Shift;
        qtKeyToX11KeySym[Qt::Key_Multi_key] = XK_Multi_key;
        qtKeyToX11KeySym[Qt::Key_Codeinput] = XK_Codeinput;
        qtKeyToX11KeySym[Qt::Key_SingleCandidate] = XK_SingleCandidate;
        qtKeyToX11KeySym[Qt::Key_MultipleCandidate] = XK_MultipleCandidate;
        qtKeyToX11KeySym[Qt::Key_PreviousCandidate] = XK_PreviousCandidate;

        // Misc Functions
        qtKeyToX11KeySym[Qt::Key_Mode_switch] = XK_Mode_switch;
        //qtKeyToX11KeySym[Qt::Key_Mode_switch] = XK_script_switch;

        // Japanese keyboard support
        qtKeyToX11KeySym[Qt::Key_Kanji] = XK_Kanji;
        qtKeyToX11KeySym[Qt::Key_Muhenkan] = XK_Muhenkan;
        qtKeyToX11KeySym[Qt::Key_Henkan] = XK_Henkan_Mode;
        //qtKeyToX11KeySym[Qt::Key_Henkan] = XK_Henkan;
        qtKeyToX11KeySym[Qt::Key_Romaji] = XK_Romaji;
        qtKeyToX11KeySym[Qt::Key_Hiragana] = XK_Hiragana;
        qtKeyToX11KeySym[Qt::Key_Katakana] = XK_Katakana;
        qtKeyToX11KeySym[Qt::Key_Hiragana_Katakana] = XK_Hiragana_Katakana;
        qtKeyToX11KeySym[Qt::Key_Zenkaku] = XK_Zenkaku;
        qtKeyToX11KeySym[Qt::Key_Hankaku] = XK_Hankaku;
        qtKeyToX11KeySym[Qt::Key_Zenkaku_Hankaku] = XK_Zenkaku_Hankaku;
        qtKeyToX11KeySym[Qt::Key_Touroku] = XK_Touroku;
        qtKeyToX11KeySym[Qt::Key_Massyo] = XK_Massyo;
        qtKeyToX11KeySym[Qt::Key_Kana_Lock] = XK_Kana_Lock;
        qtKeyToX11KeySym[Qt::Key_Kana_Shift] = XK_Kana_Shift;
        qtKeyToX11KeySym[Qt::Key_Eisu_Shift] = XK_Eisu_Shift;
        qtKeyToX11KeySym[Qt::Key_Eisu_toggle] = XK_Eisu_toggle;
        qtKeyToX11KeySym[Qt::Key_Codeinput] = XK_Kanji_Bangou;
        //qtKeyToX11KeySym[Qt::Key_MultipleCandidate] = XK_Zen_Koho;
        //qtKeyToX11KeySym[Qt::Key_PreviousCandidate] = XK_Mae_Koho;

#ifdef XK_KOREAN
        qtKeyToX11KeySym[Qt::Key_Hangul] = XK_Hangul;
        qtKeyToX11KeySym[Qt::Key_Hangul_Start] = XK_Hangul_Start;
        qtKeyToX11KeySym[Qt::Key_Hangul_End] = XK_Hangul_End;
        qtKeyToX11KeySym[Qt::Key_Hangul_Hanja] = XK_Hangul_Hanja;
        qtKeyToX11KeySym[Qt::Key_Hangul_Jamo] = XK_Hangul_Jamo;
        qtKeyToX11KeySym[Qt::Key_Hangul_Romaja] = XK_Hangul_Romaja;
        //qtKeyToX11KeySym[Qt::Key_Codeinput] = XK_Hangul_Codeinput;
        qtKeyToX11KeySym[Qt::Key_Hangul_Jeonja] = XK_Hangul_Jeonja;
        qtKeyToX11KeySym[Qt::Key_Hangul_Banja] = XK_Hangul_Banja;
        qtKeyToX11KeySym[Qt::Key_Hangul_PreHanja] = XK_Hangul_PreHanja;
        qtKeyToX11KeySym[Qt::Key_Hangul_PostHanja] = XK_Hangul_PostHanja;
        //qtKeyToX11KeySym[Qt::Key_SingleCandidate] = XK_Hangul_SingleCandidate;
        //qtKeyToX11KeySym[Qt::Key_MultipleCandidate] = XK_Hangul_MultipleCandidate;
        //qtKeyToX11KeySym[Qt::Key_PreviousCandidate] = XK_Hangul_PreviousCandidate;
        qtKeyToX11KeySym[Qt::Key_Hangul_Special] = XK_Hangul_Special;
        //qtKeyToX11KeySym[Qt::Key_Mode_switch] = XK_Hangul_switch;

#endif // XK_KOREAN

        // dead keys
        qtKeyToX11KeySym[Qt::Key_Dead_Grave] = XK_dead_grave;
        qtKeyToX11KeySym[Qt::Key_Dead_Acute] = XK_dead_acute;
        qtKeyToX11KeySym[Qt::Key_Dead_Circumflex] = XK_dead_circumflex;
        qtKeyToX11KeySym[Qt::Key_Dead_Tilde] = XK_dead_tilde;
        qtKeyToX11KeySym[Qt::Key_Dead_Macron] = XK_dead_macron;
        qtKeyToX11KeySym[Qt::Key_Dead_Breve] = XK_dead_breve;
        qtKeyToX11KeySym[Qt::Key_Dead_Abovedot] = XK_dead_abovedot;
        qtKeyToX11KeySym[Qt::Key_Dead_Diaeresis] = XK_dead_diaeresis;
        qtKeyToX11KeySym[Qt::Key_Dead_Abovering] = XK_dead_abovering;
        qtKeyToX11KeySym[Qt::Key_Dead_Doubleacute] = XK_dead_doubleacute;
        qtKeyToX11KeySym[Qt::Key_Dead_Caron] = XK_dead_caron;
        qtKeyToX11KeySym[Qt::Key_Dead_Cedilla] = XK_dead_cedilla;
        qtKeyToX11KeySym[Qt::Key_Dead_Ogonek] = XK_dead_ogonek;
        qtKeyToX11KeySym[Qt::Key_Dead_Iota] = XK_dead_iota;
        qtKeyToX11KeySym[Qt::Key_Dead_Voiced_Sound] = XK_dead_voiced_sound;
        qtKeyToX11KeySym[Qt::Key_Dead_Semivoiced_Sound] = XK_dead_semivoiced_sound;
        qtKeyToX11KeySym[Qt::Key_Dead_Belowdot] = XK_dead_belowdot;
        qtKeyToX11KeySym[Qt::Key_Dead_Hook] = XK_dead_hook;
        qtKeyToX11KeySym[Qt::Key_Dead_Horn] = XK_dead_horn;

        // Special multimedia keys
        // currently only tested with MS internet keyboard

        // browsing keys
        qtKeyToX11KeySym[Qt::Key_Back] = XF86XK_Back;
        qtKeyToX11KeySym[Qt::Key_Forward] = XF86XK_Forward;
        qtKeyToX11KeySym[Qt::Key_Stop] = XF86XK_Stop;
        qtKeyToX11KeySym[Qt::Key_Refresh] = XF86XK_Refresh;
        qtKeyToX11KeySym[Qt::Key_Favorites] = XF86XK_Favorites;
        qtKeyToX11KeySym[Qt::Key_LaunchMedia] = XF86XK_AudioMedia;
        qtKeyToX11KeySym[Qt::Key_OpenUrl] = XF86XK_OpenURL;
        qtKeyToX11KeySym[Qt::Key_HomePage] = XF86XK_HomePage;
        qtKeyToX11KeySym[Qt::Key_Search] = XF86XK_Search;

        // media keys
        qtKeyToX11KeySym[Qt::Key_VolumeDown] = XF86XK_AudioLowerVolume;
        qtKeyToX11KeySym[Qt::Key_VolumeMute] = XF86XK_AudioMute;
        qtKeyToX11KeySym[Qt::Key_VolumeUp] = XF86XK_AudioRaiseVolume;
        qtKeyToX11KeySym[Qt::Key_MediaPlay] = XF86XK_AudioPlay;
        qtKeyToX11KeySym[Qt::Key_MediaStop] = XF86XK_AudioStop;
        qtKeyToX11KeySym[Qt::Key_MediaPrevious] = XF86XK_AudioPrev;
        qtKeyToX11KeySym[Qt::Key_MediaNext] = XF86XK_AudioNext;
        qtKeyToX11KeySym[Qt::Key_MediaRecord] = XF86XK_AudioRecord;

        // launch keys
        qtKeyToX11KeySym[Qt::Key_LaunchMail] = XF86XK_Mail;
        qtKeyToX11KeySym[Qt::Key_Launch0] = XF86XK_MyComputer;
        qtKeyToX11KeySym[Qt::Key_Launch1] = XF86XK_Calculator;
        qtKeyToX11KeySym[Qt::Key_Standby] = XF86XK_Standby;

        qtKeyToX11KeySym[Qt::Key_Launch2] = XF86XK_Launch0;
        qtKeyToX11KeySym[Qt::Key_Launch3] = XF86XK_Launch1;
        qtKeyToX11KeySym[Qt::Key_Launch4] = XF86XK_Launch2;
        qtKeyToX11KeySym[Qt::Key_Launch5] = XF86XK_Launch3;
        qtKeyToX11KeySym[Qt::Key_Launch6] = XF86XK_Launch4;
        qtKeyToX11KeySym[Qt::Key_Launch7] = XF86XK_Launch5;
        qtKeyToX11KeySym[Qt::Key_Launch8] = XF86XK_Launch6;
        qtKeyToX11KeySym[Qt::Key_Launch9] = XF86XK_Launch7;
        qtKeyToX11KeySym[Qt::Key_LaunchA] = XF86XK_Launch8;
        qtKeyToX11KeySym[Qt::Key_LaunchB] = XF86XK_Launch9;
        qtKeyToX11KeySym[Qt::Key_LaunchC] = XF86XK_LaunchA;
        qtKeyToX11KeySym[Qt::Key_LaunchD] = XF86XK_LaunchB;
        qtKeyToX11KeySym[Qt::Key_LaunchE] = XF86XK_LaunchC;
        qtKeyToX11KeySym[Qt::Key_LaunchF] = XF86XK_LaunchD;

        // Map initial ASCII keys
        for (int i=0; i <= (XK_at - XK_space); i++)
        {
            qtKeyToX11KeySym[Qt::Key_Space + i] = XK_space + i;
        }

        // Map lowercase alpha keys
        for (int i=0; i <= (XK_z - XK_a); i++)
        {
            qtKeyToX11KeySym[Qt::Key_A + i] = XK_a + i;
        }

        // Map [ to ` ASCII keys
        for (int i=0; i <= (XK_quoteleft - XK_bracketleft); i++)
        {
            qtKeyToX11KeySym[Qt::Key_BracketLeft + i] = XK_bracketleft + i;
        }

        // Map { to ~ ASCII keys
        for (int i=0; i <= (XK_asciitilde - XK_braceleft); i++)
        {
            qtKeyToX11KeySym[Qt::Key_BraceLeft + i] = XK_braceleft + i;
        }

        // Map function keys
        for (int i=0; i <= (XK_F35 - XK_F1); i++)
        {
            qtKeyToX11KeySym[Qt::Key_F1 + i] = XK_F1 + i;
        }

        // Map custom defined keys
        qtKeyToX11KeySym[AntKey_Shift_R] = XK_Shift_R;
        qtKeyToX11KeySym[AntKey_Control_R] = XK_Control_R;
        //qtKeyToX11KeySym[AntKey_Shift_Lock] = XK_Shift_Lock;
        qtKeyToX11KeySym[AntKey_Meta_R] = XK_Meta_R;
        qtKeyToX11KeySym[AntKey_Alt_R] = XK_Alt_R;

        for (int i=0; i <= (XK_KP_9 - XK_KP_0); i++)
        {
            qtKeyToX11KeySym[AntKey_KP_0 + i] = XK_KP_0 + i;
        }

        QHashIterator<unsigned int, KeySym> iter(qtKeyToX11KeySym);
        while (iter.hasNext())
        {
            iter.next();
            X11KeySymToQtKey[iter.value()] = iter.key();
        }
    }
}

KeySym QtX11KeyMapper::returnVirtualKey(unsigned int qkey)
{
    return _instance.qtKeyToX11KeySym.value(qkey);
}

unsigned int QtX11KeyMapper::returnQtKey(KeySym key)
{
    return _instance.X11KeySymToQtKey.value(key);
}
