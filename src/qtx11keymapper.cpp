/* antimicroX Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define XK_MISCELLANY
#define XK_LATIN1
#define XK_KOREAN
#define XK_XKB_KEYS

#include "qtx11keymapper.h"

#include <QDebug>
#include <QApplication>
#include <QHashIterator>
#include <QHash>
#include <QChar>

#include <X11/keysymdef.h>
#include <X11/XF86keysym.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>

#include "x11extras.h"
#include "messagehandler.h"


QtX11KeyMapper::QtX11KeyMapper(QObject *parent) :
    QtKeyMapperBase(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    identifier = "xtest";
    populateMappingHashes();
    populateCharKeyInformation();
}

/*
 * The following mappings are mainly taken from qkeymapper_x11.cpp.
 * There are portions of the mapping that are customized to work around
 * some of the ambiguity introduced with some Qt keys
 * (XK_Alt_L and XK_Alt_R become Qt::Key_Alt in Qt).
 */
void QtX11KeyMapper::populateMappingHashes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (qtKeyToVirtKeyHash.isEmpty())
    {
        mapMiscKeysQtX11(qtKeyToVirtKeyHash);
        mapCursorMovesKeysQtX11(qtKeyToVirtKeyHash);
        mapModifiersKeysQtX11(qtKeyToVirtKeyHash);
        mapAdditionalKeysQtX11(qtKeyToVirtKeyHash);
        mapKeypadKeysQtX11(qtKeyToVirtKeyHash);
        mapInternationKeysQtX11(qtKeyToVirtKeyHash);

        // Misc Functions
        qtKeyToVirtKeyHash[Qt::Key_Mode_switch] = XK_Mode_switch;
        //qtKeyToX11KeySym[Qt::Key_Mode_switch] = XK_script_switch;

        mapJapanKeysQtX11(qtKeyToVirtKeyHash);

#ifdef XK_KOREAN
        mapKoreanKeysQtX11(qtKeyToVirtKeyHash);
#endif // XK_KOREAN

        mapDeadKeysQtX11(qtKeyToVirtKeyHash);
        mapBrowseKeysQtX11(qtKeyToVirtKeyHash);
        mapMediaKeysQtX11(qtKeyToVirtKeyHash);
        mapLaunchKeysQtX11(qtKeyToVirtKeyHash);

        // Map initial ASCII keys
        for (int i = 0; i <= (XK_at - XK_space); i++)
            qtKeyToVirtKeyHash[Qt::Key_Space + i] = XK_space + i;

        // Map lowercase alpha keys
        for (int i = 0; i <= (XK_z - XK_a); i++)
            qtKeyToVirtKeyHash[Qt::Key_A + i] = XK_a + i;

        // Map [ to ` ASCII keys
        for (int i = 0; i <= (XK_grave - XK_bracketleft); i++)
            qtKeyToVirtKeyHash[Qt::Key_BracketLeft + i] = XK_bracketleft + i;

        // Map { to ~ ASCII keys
        for (int i = 0; i <= (XK_asciitilde - XK_braceleft); i++)
            qtKeyToVirtKeyHash[Qt::Key_BraceLeft + i] = XK_braceleft + i;

        // Map function keys
        for (int i = 0; i <= (XK_F35 - XK_F1); i++)
            qtKeyToVirtKeyHash[Qt::Key_F1 + i] = XK_F1 + i;

        // Misc
        //qtKeyToVirtKeyHash[Qt::KeyBri]

        // Map custom defined keys
        qtKeyToVirtKeyHash[AntKey_Shift_R] = XK_Shift_R;
        qtKeyToVirtKeyHash[AntKey_Control_R] = XK_Control_R;
        //qtKeyToX11KeySym[AntKey_Shift_Lock] = XK_Shift_Lock;
        //qtKeyToVirtKeyHash[AntKey_Meta_R] = XK_Meta_R;
        qtKeyToVirtKeyHash[AntKey_Alt_R] = XK_Alt_R;
        qtKeyToVirtKeyHash[AntKey_KP_Multiply] = XK_KP_Multiply;

        // Map 0 to 9
        for (int i = 0; i <= (XK_KP_9 - XK_KP_0); i++)
            qtKeyToVirtKeyHash[AntKey_KP_0 + i] = XK_KP_0 + i;

        // Map lower-case latin characters to their capital equivalents
        for( int i = 0; i <= (XK_odiaeresis - XK_agrave); i++)
            qtKeyToVirtKeyHash[ Qt::Key_Agrave + i ] = XK_agrave + i;

        for( int i = 0; i <= (XK_thorn - XK_oslash); i++)
            qtKeyToVirtKeyHash[ Qt::Key_Ooblique + i ] = XK_oslash + i;

        QHashIterator<int, int> iter(qtKeyToVirtKeyHash);

        while (iter.hasNext())
        {
            iter.next();
            virtKeyToQtKeyHash[iter.value()] = iter.key();
        }
    }
}

void QtX11KeyMapper::populateCharKeyInformation()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    virtkeyToCharKeyInfo.clear();
    Display* display = X11Extras::getInstance()->display();
    int total = 0;

    for (int i = 8; i <= 255; i++)
    {
        for (int j = 0; j <= 3; j++)
        {
            Qt::KeyboardModifiers dicis;

            if (j >= 2)
                dicis |= Qt::MetaModifier;

            if ((j == 1) || (j == 3))
                dicis |= Qt::ShiftModifier;

            int testsym = XkbKeycodeToKeysym(display, i,
                                            (dicis & Qt::MetaModifier) ? 1 : 0,
                                            (dicis & Qt::ShiftModifier) ? 1 : 0);

            if (testsym != NoSymbol)
            {
                XKeyPressedEvent tempevent;
                tempevent.keycode = i;
                tempevent.type = KeyPress;
                tempevent.display = display;
                tempevent.state = 0;

                if (dicis & Qt::ShiftModifier)
                    tempevent.state |= ShiftMask;

                if (dicis & Qt::MetaModifier)
                    tempevent.state |= Mod1Mask;

                char returnstring[256];
                memset(returnstring, 0, sizeof(returnstring));
                int bitestoreturn = sizeof(returnstring) - 1;
                int numchars = XLookupString(&tempevent, returnstring, bitestoreturn, nullptr, nullptr);

                if (numchars > 0)
                {
                    returnstring[numchars] = '\0';
                    QString tempstring = QString::fromUtf8(returnstring);

                    if (tempstring.length() == 1)
                    {
                        QChar tempchar(tempstring.at(0));
                        charKeyInformation testKeyInformation;
                        testKeyInformation.modifiers = dicis;
                        testKeyInformation.virtualkey = testsym;

                        if (!virtkeyToCharKeyInfo.contains(tempchar.unicode()))
                        {
                            virtkeyToCharKeyInfo.insert(tempchar.unicode(), testKeyInformation);
                            total++;
                        }
                    }
                }
            }
        }
    }

    qDebug() << "TOTAL: " << total;
}

void QtX11KeyMapper::mapMiscKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash)
{
    qtKeyToVirtKeyHash[Qt::Key_Escape] = XK_Escape;
    qtKeyToVirtKeyHash[Qt::Key_Tab] = XK_Tab;
    qtKeyToVirtKeyHash[Qt::Key_Backtab] = XK_ISO_Left_Tab;
    qtKeyToVirtKeyHash[Qt::Key_Backspace] = XK_BackSpace;
    qtKeyToVirtKeyHash[Qt::Key_Return] = XK_Return;
    qtKeyToVirtKeyHash[Qt::Key_Insert] = XK_Insert;
    qtKeyToVirtKeyHash[Qt::Key_Delete] = XK_Delete;
    //qtKeyToX11KeySym[Qt::Key_Delete] = XK_Clear;
    qtKeyToVirtKeyHash[Qt::Key_Pause] = XK_Pause;
    qtKeyToVirtKeyHash[Qt::Key_Print] = XK_Print;
}

void QtX11KeyMapper::mapCursorMovesKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash)
{
    qtKeyToVirtKeyHash[Qt::Key_Home] = XK_Home;
    qtKeyToVirtKeyHash[Qt::Key_End] = XK_End;
    qtKeyToVirtKeyHash[Qt::Key_Left] = XK_Left;
    qtKeyToVirtKeyHash[Qt::Key_Up] = XK_Up;
    qtKeyToVirtKeyHash[Qt::Key_Right] = XK_Right;
    qtKeyToVirtKeyHash[Qt::Key_Down] = XK_Down;
    qtKeyToVirtKeyHash[Qt::Key_PageUp] = XK_Prior;
    qtKeyToVirtKeyHash[Qt::Key_PageDown] = XK_Next;
}

void QtX11KeyMapper::mapModifiersKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash)
{
    qtKeyToVirtKeyHash[Qt::Key_Shift] = XK_Shift_L;
    //qtKeyToX11KeySym[Qt::Key_Shift] = XK_Shift_R;
    //qtKeyToX11KeySym[Qt::Key_Shift] = XK_Shift_Lock;
    qtKeyToVirtKeyHash[Qt::Key_Control] = XK_Control_L;
    //qtKeyToX11KeySym[Qt::Key_Control] = XK_Control_R;
    //qtKeyToVirtKeyHash[Qt::Key_Meta] = XK_Meta_L;
    //qtKeyToX11KeySym[Qt::Key_Meta] = XK_Meta_R;
    qtKeyToVirtKeyHash[Qt::Key_Alt] = XK_Alt_L;
    //qtKeyToX11KeySym[Qt::Key_Alt] = XK_Alt_R;
}

void QtX11KeyMapper::mapAdditionalKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash)
{
    qtKeyToVirtKeyHash[Qt::Key_CapsLock] = XK_Caps_Lock;
    qtKeyToVirtKeyHash[Qt::Key_NumLock] = XK_Num_Lock;
    qtKeyToVirtKeyHash[Qt::Key_ScrollLock] = XK_Scroll_Lock;
    qtKeyToVirtKeyHash[Qt::Key_Meta] = XK_Super_L;
    qtKeyToVirtKeyHash[AntKey_Meta_R] = XK_Super_R;
    //qtKeyToVirtKeyHash[Qt::Key_Super_L] = XK_Super_L;
    //qtKeyToVirtKeyHash[Qt::Key_Super_R] = XK_Super_R;
    qtKeyToVirtKeyHash[Qt::Key_Menu] = XK_Menu;
    qtKeyToVirtKeyHash[Qt::Key_Hyper_L] = XK_Hyper_L;
    qtKeyToVirtKeyHash[Qt::Key_Hyper_R] = XK_Hyper_R;
    qtKeyToVirtKeyHash[Qt::Key_Help] = XK_Help;
}

void QtX11KeyMapper::mapKeypadKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash)
{
    //qtKeyToVirtKeyHash[Qt::Key_Space] = XK_KP_Space;
    //qtKeyToX11KeySym[Qt::Key_Tab] = XK_KP_Tab;
    qtKeyToVirtKeyHash[AntKey_KP_Enter] = XK_KP_Enter;
    qtKeyToVirtKeyHash[AntKey_KP_Home] = XK_KP_Home;
    //qtKeyToX11KeySym[Qt::Key_Home] = XK_KP_Home;
    qtKeyToVirtKeyHash[AntKey_KP_Left] = XK_KP_Left;
    qtKeyToVirtKeyHash[AntKey_KP_Up] = XK_KP_Up;
    qtKeyToVirtKeyHash[AntKey_KP_Right] = XK_KP_Right;
    qtKeyToVirtKeyHash[AntKey_KP_Down] = XK_KP_Down;
    qtKeyToVirtKeyHash[AntKey_KP_Prior] = XK_KP_Prior;
    qtKeyToVirtKeyHash[AntKey_KP_Next] = XK_KP_Next;
    qtKeyToVirtKeyHash[AntKey_KP_End] = XK_KP_End;
    qtKeyToVirtKeyHash[AntKey_KP_Begin] = XK_KP_Begin;
    qtKeyToVirtKeyHash[AntKey_KP_Insert] = XK_KP_Insert;
    qtKeyToVirtKeyHash[AntKey_KP_Delete] = XK_KP_Delete;
    //qtKeyToX11KeySym[AntKey_KP_Equal] = XK_KP_Equal;
    qtKeyToVirtKeyHash[AntKey_KP_Add] = XK_KP_Add;
    //qtKeyToX11KeySym[AntKey_KP_Separator] = XK_KP_Separator;
    qtKeyToVirtKeyHash[AntKey_KP_Subtract] = XK_KP_Subtract;
    qtKeyToVirtKeyHash[AntKey_KP_Decimal] = XK_KP_Decimal;
    qtKeyToVirtKeyHash[AntKey_KP_Divide] = XK_KP_Divide;
    qtKeyToVirtKeyHash[AntKey_KP_1] = XK_KP_1;
    qtKeyToVirtKeyHash[AntKey_KP_2] = XK_KP_2;
    qtKeyToVirtKeyHash[AntKey_KP_3] = XK_KP_3;
    qtKeyToVirtKeyHash[AntKey_KP_4] = XK_KP_4;
    qtKeyToVirtKeyHash[AntKey_KP_5] = XK_KP_5;
    qtKeyToVirtKeyHash[AntKey_KP_6] = XK_KP_6;
    qtKeyToVirtKeyHash[AntKey_KP_7] = XK_KP_7;
    qtKeyToVirtKeyHash[AntKey_KP_8] = XK_KP_8;
    qtKeyToVirtKeyHash[AntKey_KP_9] = XK_KP_9;
    qtKeyToVirtKeyHash[AntKey_KP_0] = XK_KP_0;
}

void QtX11KeyMapper::mapInternationKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash)
{
    qtKeyToVirtKeyHash[Qt::Key_AltGr] = XK_ISO_Level3_Shift;
    qtKeyToVirtKeyHash[Qt::Key_Multi_key] = XK_Multi_key;
    qtKeyToVirtKeyHash[Qt::Key_Codeinput] = XK_Codeinput;
    qtKeyToVirtKeyHash[Qt::Key_SingleCandidate] = XK_SingleCandidate;
    qtKeyToVirtKeyHash[Qt::Key_MultipleCandidate] = XK_MultipleCandidate;
    qtKeyToVirtKeyHash[Qt::Key_PreviousCandidate] = XK_PreviousCandidate;
}

void QtX11KeyMapper::mapJapanKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash)
{
    qtKeyToVirtKeyHash[Qt::Key_Kanji] = XK_Kanji;
    qtKeyToVirtKeyHash[Qt::Key_Muhenkan] = XK_Muhenkan;
    qtKeyToVirtKeyHash[Qt::Key_Henkan] = XK_Henkan_Mode;
    //qtKeyToX11KeySym[Qt::Key_Henkan] = XK_Henkan;
    qtKeyToVirtKeyHash[Qt::Key_Romaji] = XK_Romaji;
    qtKeyToVirtKeyHash[Qt::Key_Hiragana] = XK_Hiragana;
    qtKeyToVirtKeyHash[Qt::Key_Katakana] = XK_Katakana;
    qtKeyToVirtKeyHash[Qt::Key_Hiragana_Katakana] = XK_Hiragana_Katakana;
    qtKeyToVirtKeyHash[Qt::Key_Zenkaku] = XK_Zenkaku;
    qtKeyToVirtKeyHash[Qt::Key_Hankaku] = XK_Hankaku;
    qtKeyToVirtKeyHash[Qt::Key_Zenkaku_Hankaku] = XK_Zenkaku_Hankaku;
    qtKeyToVirtKeyHash[Qt::Key_Touroku] = XK_Touroku;
    qtKeyToVirtKeyHash[Qt::Key_Massyo] = XK_Massyo;
    qtKeyToVirtKeyHash[Qt::Key_Kana_Lock] = XK_Kana_Lock;
    qtKeyToVirtKeyHash[Qt::Key_Kana_Shift] = XK_Kana_Shift;
    qtKeyToVirtKeyHash[Qt::Key_Eisu_Shift] = XK_Eisu_Shift;
    qtKeyToVirtKeyHash[Qt::Key_Eisu_toggle] = XK_Eisu_toggle;
    qtKeyToVirtKeyHash[Qt::Key_Codeinput] = XK_Kanji_Bangou;
    //qtKeyToX11KeySym[Qt::Key_MultipleCandidate] = XK_Zen_Koho;
    //qtKeyToX11KeySym[Qt::Key_PreviousCandidate] = XK_Mae_Koho;
}

void QtX11KeyMapper::mapKoreanKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash)
{
    qtKeyToVirtKeyHash[Qt::Key_Hangul] = XK_Hangul;
    qtKeyToVirtKeyHash[Qt::Key_Hangul_Start] = XK_Hangul_Start;
    qtKeyToVirtKeyHash[Qt::Key_Hangul_End] = XK_Hangul_End;
    qtKeyToVirtKeyHash[Qt::Key_Hangul_Hanja] = XK_Hangul_Hanja;
    qtKeyToVirtKeyHash[Qt::Key_Hangul_Jamo] = XK_Hangul_Jamo;
    qtKeyToVirtKeyHash[Qt::Key_Hangul_Romaja] = XK_Hangul_Romaja;
    //qtKeyToX11KeySym[Qt::Key_Codeinput] = XK_Hangul_Codeinput;
    qtKeyToVirtKeyHash[Qt::Key_Hangul_Jeonja] = XK_Hangul_Jeonja;
    qtKeyToVirtKeyHash[Qt::Key_Hangul_Banja] = XK_Hangul_Banja;
    qtKeyToVirtKeyHash[Qt::Key_Hangul_PreHanja] = XK_Hangul_PreHanja;
    qtKeyToVirtKeyHash[Qt::Key_Hangul_PostHanja] = XK_Hangul_PostHanja;
    //qtKeyToX11KeySym[Qt::Key_SingleCandidate] = XK_Hangul_SingleCandidate;
    //qtKeyToX11KeySym[Qt::Key_MultipleCandidate] = XK_Hangul_MultipleCandidate;
    //qtKeyToX11KeySym[Qt::Key_PreviousCandidate] = XK_Hangul_PreviousCandidate;
    qtKeyToVirtKeyHash[Qt::Key_Hangul_Special] = XK_Hangul_Special;
    //qtKeyToX11KeySym[Qt::Key_Mode_switch] = XK_Hangul_switch;
}

void QtX11KeyMapper::mapDeadKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash)
{
    qtKeyToVirtKeyHash[Qt::Key_Dead_Grave] = XK_dead_grave;
    qtKeyToVirtKeyHash[Qt::Key_Dead_Acute] = XK_dead_acute;
    qtKeyToVirtKeyHash[Qt::Key_Dead_Circumflex] = XK_dead_circumflex;
    qtKeyToVirtKeyHash[Qt::Key_Dead_Tilde] = XK_dead_tilde;
    qtKeyToVirtKeyHash[Qt::Key_Dead_Macron] = XK_dead_macron;
    qtKeyToVirtKeyHash[Qt::Key_Dead_Breve] = XK_dead_breve;
    qtKeyToVirtKeyHash[Qt::Key_Dead_Abovedot] = XK_dead_abovedot;
    qtKeyToVirtKeyHash[Qt::Key_Dead_Diaeresis] = XK_dead_diaeresis;
    qtKeyToVirtKeyHash[Qt::Key_Dead_Abovering] = XK_dead_abovering;
    qtKeyToVirtKeyHash[Qt::Key_Dead_Doubleacute] = XK_dead_doubleacute;
    qtKeyToVirtKeyHash[Qt::Key_Dead_Caron] = XK_dead_caron;
    qtKeyToVirtKeyHash[Qt::Key_Dead_Cedilla] = XK_dead_cedilla;
    qtKeyToVirtKeyHash[Qt::Key_Dead_Ogonek] = XK_dead_ogonek;
    qtKeyToVirtKeyHash[Qt::Key_Dead_Iota] = XK_dead_iota;
    qtKeyToVirtKeyHash[Qt::Key_Dead_Voiced_Sound] = XK_dead_voiced_sound;
    qtKeyToVirtKeyHash[Qt::Key_Dead_Semivoiced_Sound] = XK_dead_semivoiced_sound;
    qtKeyToVirtKeyHash[Qt::Key_Dead_Belowdot] = XK_dead_belowdot;
    qtKeyToVirtKeyHash[Qt::Key_Dead_Hook] = XK_dead_hook;
    qtKeyToVirtKeyHash[Qt::Key_Dead_Horn] = XK_dead_horn;
}

void QtX11KeyMapper::mapBrowseKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash)
{
    qtKeyToVirtKeyHash[Qt::Key_Back] = XF86XK_Back;
    qtKeyToVirtKeyHash[Qt::Key_Forward] = XF86XK_Forward;
    qtKeyToVirtKeyHash[Qt::Key_Stop] = XF86XK_Stop;
    qtKeyToVirtKeyHash[Qt::Key_Refresh] = XF86XK_Refresh;
    qtKeyToVirtKeyHash[Qt::Key_Favorites] = XF86XK_Favorites;
    qtKeyToVirtKeyHash[Qt::Key_LaunchMedia] = XF86XK_AudioMedia;
    qtKeyToVirtKeyHash[Qt::Key_OpenUrl] = XF86XK_OpenURL;
    qtKeyToVirtKeyHash[Qt::Key_HomePage] = XF86XK_HomePage;
    qtKeyToVirtKeyHash[Qt::Key_Search] = XF86XK_Search;
}

void QtX11KeyMapper::mapMediaKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash)
{
    qtKeyToVirtKeyHash[Qt::Key_VolumeDown] = XF86XK_AudioLowerVolume;
    qtKeyToVirtKeyHash[Qt::Key_VolumeMute] = XF86XK_AudioMute;
    qtKeyToVirtKeyHash[Qt::Key_VolumeUp] = XF86XK_AudioRaiseVolume;
    qtKeyToVirtKeyHash[Qt::Key_MediaPlay] = XF86XK_AudioPlay;
    qtKeyToVirtKeyHash[Qt::Key_MediaStop] = XF86XK_AudioStop;
    qtKeyToVirtKeyHash[Qt::Key_MediaPrevious] = XF86XK_AudioPrev;
    qtKeyToVirtKeyHash[Qt::Key_MediaNext] = XF86XK_AudioNext;
    qtKeyToVirtKeyHash[Qt::Key_MediaRecord] = XF86XK_AudioRecord;
}

void QtX11KeyMapper::mapLaunchKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash)
{
    qtKeyToVirtKeyHash[Qt::Key_LaunchMail] = XF86XK_Mail;
    qtKeyToVirtKeyHash[Qt::Key_Launch0] = XF86XK_MyComputer;
    qtKeyToVirtKeyHash[Qt::Key_Launch1] = XF86XK_Calculator;
    qtKeyToVirtKeyHash[Qt::Key_Standby] = XF86XK_Standby;
    qtKeyToVirtKeyHash[Qt::Key_Launch2] = XF86XK_Launch0;
    qtKeyToVirtKeyHash[Qt::Key_Launch3] = XF86XK_Launch1;
    qtKeyToVirtKeyHash[Qt::Key_Launch4] = XF86XK_Launch2;
    qtKeyToVirtKeyHash[Qt::Key_Launch5] = XF86XK_Launch3;
    qtKeyToVirtKeyHash[Qt::Key_Launch6] = XF86XK_Launch4;
    qtKeyToVirtKeyHash[Qt::Key_Launch7] = XF86XK_Launch5;
    qtKeyToVirtKeyHash[Qt::Key_Launch8] = XF86XK_Launch6;
    qtKeyToVirtKeyHash[Qt::Key_Launch9] = XF86XK_Launch7;
    qtKeyToVirtKeyHash[Qt::Key_LaunchA] = XF86XK_Launch8;
    qtKeyToVirtKeyHash[Qt::Key_LaunchB] = XF86XK_Launch9;
    qtKeyToVirtKeyHash[Qt::Key_LaunchC] = XF86XK_LaunchA;
    qtKeyToVirtKeyHash[Qt::Key_LaunchD] = XF86XK_LaunchB;
    qtKeyToVirtKeyHash[Qt::Key_LaunchE] = XF86XK_LaunchC;
    qtKeyToVirtKeyHash[Qt::Key_LaunchF] = XF86XK_LaunchD;
}
