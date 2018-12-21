/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
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

//#include <QDebug>
#include <QApplication>
#include <QHashIterator>

#include <X11/keysymdef.h>
#include <X11/XF86keysym.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>

#include "qtx11keymapper.h"
#include "x11extras.h"

QtX11KeyMapper::QtX11KeyMapper(QObject *parent) :
    QtKeyMapperBase(parent)
{
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
    if (qtKeyToVirtualKey.isEmpty())
    {
        // misc keys
        qtKeyToVirtualKey[Qt::Key_Escape] = XK_Escape;
        qtKeyToVirtualKey[Qt::Key_Tab] = XK_Tab;
        qtKeyToVirtualKey[Qt::Key_Backtab] = XK_ISO_Left_Tab;
        qtKeyToVirtualKey[Qt::Key_Backspace] = XK_BackSpace;
        qtKeyToVirtualKey[Qt::Key_Return] = XK_Return;
        qtKeyToVirtualKey[Qt::Key_Insert] = XK_Insert;
        qtKeyToVirtualKey[Qt::Key_Delete] = XK_Delete;
        qtKeyToVirtualKey[Qt::Key_Delete] = XK_Delete;
        //qtKeyToX11KeySym[Qt::Key_Delete] = XK_Clear;
        qtKeyToVirtualKey[Qt::Key_Pause] = XK_Pause;
        qtKeyToVirtualKey[Qt::Key_Print] = XK_Print;

        // cursor movement
        qtKeyToVirtualKey[Qt::Key_Home] = XK_Home;
        qtKeyToVirtualKey[Qt::Key_End] = XK_End;
        qtKeyToVirtualKey[Qt::Key_Left] = XK_Left;
        qtKeyToVirtualKey[Qt::Key_Up] = XK_Up;
        qtKeyToVirtualKey[Qt::Key_Right] = XK_Right;
        qtKeyToVirtualKey[Qt::Key_Down] = XK_Down;
        qtKeyToVirtualKey[Qt::Key_PageUp] = XK_Prior;
        qtKeyToVirtualKey[Qt::Key_PageDown] = XK_Next;

        // modifiers
        qtKeyToVirtualKey[Qt::Key_Shift] = XK_Shift_L;
        //qtKeyToX11KeySym[Qt::Key_Shift] = XK_Shift_R;
        //qtKeyToX11KeySym[Qt::Key_Shift] = XK_Shift_Lock;
        qtKeyToVirtualKey[Qt::Key_Control] = XK_Control_L;
        //qtKeyToX11KeySym[Qt::Key_Control] = XK_Control_R;
        //qtKeyToVirtualKey[Qt::Key_Meta] = XK_Meta_L;
        //qtKeyToX11KeySym[Qt::Key_Meta] = XK_Meta_R;
        qtKeyToVirtualKey[Qt::Key_Alt] = XK_Alt_L;
        //qtKeyToX11KeySym[Qt::Key_Alt] = XK_Alt_R;

        qtKeyToVirtualKey[Qt::Key_CapsLock] = XK_Caps_Lock;
        qtKeyToVirtualKey[Qt::Key_NumLock] = XK_Num_Lock;
        qtKeyToVirtualKey[Qt::Key_ScrollLock] = XK_Scroll_Lock;
        qtKeyToVirtualKey[Qt::Key_Meta] = XK_Super_L;
        qtKeyToVirtualKey[AntKey_Meta_R] = XK_Super_R;
        //qtKeyToVirtualKey[Qt::Key_Super_L] = XK_Super_L;
        //qtKeyToVirtualKey[Qt::Key_Super_R] = XK_Super_R;
        qtKeyToVirtualKey[Qt::Key_Menu] = XK_Menu;
        qtKeyToVirtualKey[Qt::Key_Hyper_L] = XK_Hyper_L;
        qtKeyToVirtualKey[Qt::Key_Hyper_R] = XK_Hyper_R;
        qtKeyToVirtualKey[Qt::Key_Help] = XK_Help;

        // numeric and function keypad keys
        //qtKeyToVirtualKey[Qt::Key_Space] = XK_KP_Space;
        //qtKeyToX11KeySym[Qt::Key_Tab] = XK_KP_Tab;
        qtKeyToVirtualKey[Qt::Key_Enter] = XK_KP_Enter;
        qtKeyToVirtualKey[AntKey_KP_Home] = XK_KP_Home;
        //qtKeyToX11KeySym[Qt::Key_Home] = XK_KP_Home;
        qtKeyToVirtualKey[AntKey_KP_Left] = XK_KP_Left;
        qtKeyToVirtualKey[AntKey_KP_Up] = XK_KP_Up;
        qtKeyToVirtualKey[AntKey_KP_Right] = XK_KP_Right;
        qtKeyToVirtualKey[AntKey_KP_Down] = XK_KP_Down;
        qtKeyToVirtualKey[AntKey_KP_Prior] = XK_KP_Prior;
        qtKeyToVirtualKey[AntKey_KP_Next] = XK_KP_Next;
        qtKeyToVirtualKey[AntKey_KP_End] = XK_KP_End;
        qtKeyToVirtualKey[AntKey_KP_Begin] = XK_KP_Begin;
        qtKeyToVirtualKey[AntKey_KP_Insert] = XK_KP_Insert;
        qtKeyToVirtualKey[AntKey_KP_Delete] = XK_KP_Delete;
        //qtKeyToX11KeySym[AntKey_KP_Equal] = XK_KP_Equal;
        qtKeyToVirtualKey[AntKey_KP_Add] = XK_KP_Add;
        //qtKeyToX11KeySym[AntKey_KP_Separator] = XK_KP_Separator;
        qtKeyToVirtualKey[AntKey_KP_Subtract] = XK_KP_Subtract;

        qtKeyToVirtualKey[AntKey_KP_Decimal] = XK_KP_Decimal;
        qtKeyToVirtualKey[AntKey_KP_Divide] = XK_KP_Divide;

        // International input method support keys

        // International & multi-key character composition
        qtKeyToVirtualKey[Qt::Key_AltGr] = XK_ISO_Level3_Shift;
        qtKeyToVirtualKey[Qt::Key_Multi_key] = XK_Multi_key;
        qtKeyToVirtualKey[Qt::Key_Codeinput] = XK_Codeinput;
        qtKeyToVirtualKey[Qt::Key_SingleCandidate] = XK_SingleCandidate;
        qtKeyToVirtualKey[Qt::Key_MultipleCandidate] = XK_MultipleCandidate;
        qtKeyToVirtualKey[Qt::Key_PreviousCandidate] = XK_PreviousCandidate;

        // Misc Functions
        qtKeyToVirtualKey[Qt::Key_Mode_switch] = XK_Mode_switch;
        //qtKeyToX11KeySym[Qt::Key_Mode_switch] = XK_script_switch;

        // Japanese keyboard support
        qtKeyToVirtualKey[Qt::Key_Kanji] = XK_Kanji;
        qtKeyToVirtualKey[Qt::Key_Muhenkan] = XK_Muhenkan;
        qtKeyToVirtualKey[Qt::Key_Henkan] = XK_Henkan_Mode;
        //qtKeyToX11KeySym[Qt::Key_Henkan] = XK_Henkan;
        qtKeyToVirtualKey[Qt::Key_Romaji] = XK_Romaji;
        qtKeyToVirtualKey[Qt::Key_Hiragana] = XK_Hiragana;
        qtKeyToVirtualKey[Qt::Key_Katakana] = XK_Katakana;
        qtKeyToVirtualKey[Qt::Key_Hiragana_Katakana] = XK_Hiragana_Katakana;
        qtKeyToVirtualKey[Qt::Key_Zenkaku] = XK_Zenkaku;
        qtKeyToVirtualKey[Qt::Key_Hankaku] = XK_Hankaku;
        qtKeyToVirtualKey[Qt::Key_Zenkaku_Hankaku] = XK_Zenkaku_Hankaku;
        qtKeyToVirtualKey[Qt::Key_Touroku] = XK_Touroku;
        qtKeyToVirtualKey[Qt::Key_Massyo] = XK_Massyo;
        qtKeyToVirtualKey[Qt::Key_Kana_Lock] = XK_Kana_Lock;
        qtKeyToVirtualKey[Qt::Key_Kana_Shift] = XK_Kana_Shift;
        qtKeyToVirtualKey[Qt::Key_Eisu_Shift] = XK_Eisu_Shift;
        qtKeyToVirtualKey[Qt::Key_Eisu_toggle] = XK_Eisu_toggle;
        qtKeyToVirtualKey[Qt::Key_Codeinput] = XK_Kanji_Bangou;
        //qtKeyToX11KeySym[Qt::Key_MultipleCandidate] = XK_Zen_Koho;
        //qtKeyToX11KeySym[Qt::Key_PreviousCandidate] = XK_Mae_Koho;

#ifdef XK_KOREAN
        qtKeyToVirtualKey[Qt::Key_Hangul] = XK_Hangul;
        qtKeyToVirtualKey[Qt::Key_Hangul_Start] = XK_Hangul_Start;
        qtKeyToVirtualKey[Qt::Key_Hangul_End] = XK_Hangul_End;
        qtKeyToVirtualKey[Qt::Key_Hangul_Hanja] = XK_Hangul_Hanja;
        qtKeyToVirtualKey[Qt::Key_Hangul_Jamo] = XK_Hangul_Jamo;
        qtKeyToVirtualKey[Qt::Key_Hangul_Romaja] = XK_Hangul_Romaja;
        //qtKeyToX11KeySym[Qt::Key_Codeinput] = XK_Hangul_Codeinput;
        qtKeyToVirtualKey[Qt::Key_Hangul_Jeonja] = XK_Hangul_Jeonja;
        qtKeyToVirtualKey[Qt::Key_Hangul_Banja] = XK_Hangul_Banja;
        qtKeyToVirtualKey[Qt::Key_Hangul_PreHanja] = XK_Hangul_PreHanja;
        qtKeyToVirtualKey[Qt::Key_Hangul_PostHanja] = XK_Hangul_PostHanja;
        //qtKeyToX11KeySym[Qt::Key_SingleCandidate] = XK_Hangul_SingleCandidate;
        //qtKeyToX11KeySym[Qt::Key_MultipleCandidate] = XK_Hangul_MultipleCandidate;
        //qtKeyToX11KeySym[Qt::Key_PreviousCandidate] = XK_Hangul_PreviousCandidate;
        qtKeyToVirtualKey[Qt::Key_Hangul_Special] = XK_Hangul_Special;
        //qtKeyToX11KeySym[Qt::Key_Mode_switch] = XK_Hangul_switch;

#endif // XK_KOREAN

        // dead keys
        qtKeyToVirtualKey[Qt::Key_Dead_Grave] = XK_dead_grave;
        qtKeyToVirtualKey[Qt::Key_Dead_Acute] = XK_dead_acute;
        qtKeyToVirtualKey[Qt::Key_Dead_Circumflex] = XK_dead_circumflex;
        qtKeyToVirtualKey[Qt::Key_Dead_Tilde] = XK_dead_tilde;
        qtKeyToVirtualKey[Qt::Key_Dead_Macron] = XK_dead_macron;
        qtKeyToVirtualKey[Qt::Key_Dead_Breve] = XK_dead_breve;
        qtKeyToVirtualKey[Qt::Key_Dead_Abovedot] = XK_dead_abovedot;
        qtKeyToVirtualKey[Qt::Key_Dead_Diaeresis] = XK_dead_diaeresis;
        qtKeyToVirtualKey[Qt::Key_Dead_Abovering] = XK_dead_abovering;
        qtKeyToVirtualKey[Qt::Key_Dead_Doubleacute] = XK_dead_doubleacute;
        qtKeyToVirtualKey[Qt::Key_Dead_Caron] = XK_dead_caron;
        qtKeyToVirtualKey[Qt::Key_Dead_Cedilla] = XK_dead_cedilla;
        qtKeyToVirtualKey[Qt::Key_Dead_Ogonek] = XK_dead_ogonek;
        qtKeyToVirtualKey[Qt::Key_Dead_Iota] = XK_dead_iota;
        qtKeyToVirtualKey[Qt::Key_Dead_Voiced_Sound] = XK_dead_voiced_sound;
        qtKeyToVirtualKey[Qt::Key_Dead_Semivoiced_Sound] = XK_dead_semivoiced_sound;
        qtKeyToVirtualKey[Qt::Key_Dead_Belowdot] = XK_dead_belowdot;
        qtKeyToVirtualKey[Qt::Key_Dead_Hook] = XK_dead_hook;
        qtKeyToVirtualKey[Qt::Key_Dead_Horn] = XK_dead_horn;

        // Special multimedia keys
        // currently only tested with MS internet keyboard

        // browsing keys
        qtKeyToVirtualKey[Qt::Key_Back] = XF86XK_Back;
        qtKeyToVirtualKey[Qt::Key_Forward] = XF86XK_Forward;
        qtKeyToVirtualKey[Qt::Key_Stop] = XF86XK_Stop;
        qtKeyToVirtualKey[Qt::Key_Refresh] = XF86XK_Refresh;
        qtKeyToVirtualKey[Qt::Key_Favorites] = XF86XK_Favorites;
        qtKeyToVirtualKey[Qt::Key_LaunchMedia] = XF86XK_AudioMedia;
        qtKeyToVirtualKey[Qt::Key_OpenUrl] = XF86XK_OpenURL;
        qtKeyToVirtualKey[Qt::Key_HomePage] = XF86XK_HomePage;
        qtKeyToVirtualKey[Qt::Key_Search] = XF86XK_Search;

        // media keys
        qtKeyToVirtualKey[Qt::Key_VolumeDown] = XF86XK_AudioLowerVolume;
        qtKeyToVirtualKey[Qt::Key_VolumeMute] = XF86XK_AudioMute;
        qtKeyToVirtualKey[Qt::Key_VolumeUp] = XF86XK_AudioRaiseVolume;
        qtKeyToVirtualKey[Qt::Key_MediaPlay] = XF86XK_AudioPlay;
        qtKeyToVirtualKey[Qt::Key_MediaStop] = XF86XK_AudioStop;
        qtKeyToVirtualKey[Qt::Key_MediaPrevious] = XF86XK_AudioPrev;
        qtKeyToVirtualKey[Qt::Key_MediaNext] = XF86XK_AudioNext;
        qtKeyToVirtualKey[Qt::Key_MediaRecord] = XF86XK_AudioRecord;

        // launch keys
        qtKeyToVirtualKey[Qt::Key_LaunchMail] = XF86XK_Mail;
        qtKeyToVirtualKey[Qt::Key_Launch0] = XF86XK_MyComputer;
        qtKeyToVirtualKey[Qt::Key_Launch1] = XF86XK_Calculator;
        qtKeyToVirtualKey[Qt::Key_Standby] = XF86XK_Standby;

        qtKeyToVirtualKey[Qt::Key_Launch2] = XF86XK_Launch0;
        qtKeyToVirtualKey[Qt::Key_Launch3] = XF86XK_Launch1;
        qtKeyToVirtualKey[Qt::Key_Launch4] = XF86XK_Launch2;
        qtKeyToVirtualKey[Qt::Key_Launch5] = XF86XK_Launch3;
        qtKeyToVirtualKey[Qt::Key_Launch6] = XF86XK_Launch4;
        qtKeyToVirtualKey[Qt::Key_Launch7] = XF86XK_Launch5;
        qtKeyToVirtualKey[Qt::Key_Launch8] = XF86XK_Launch6;
        qtKeyToVirtualKey[Qt::Key_Launch9] = XF86XK_Launch7;
        qtKeyToVirtualKey[Qt::Key_LaunchA] = XF86XK_Launch8;
        qtKeyToVirtualKey[Qt::Key_LaunchB] = XF86XK_Launch9;
        qtKeyToVirtualKey[Qt::Key_LaunchC] = XF86XK_LaunchA;
        qtKeyToVirtualKey[Qt::Key_LaunchD] = XF86XK_LaunchB;
        qtKeyToVirtualKey[Qt::Key_LaunchE] = XF86XK_LaunchC;
        qtKeyToVirtualKey[Qt::Key_LaunchF] = XF86XK_LaunchD;

        // Map initial ASCII keys
        for (int i=0; i <= (XK_at - XK_space); i++)
        {
            qtKeyToVirtualKey[Qt::Key_Space + i] = XK_space + i;
        }

        // Map lowercase alpha keys
        for (int i=0; i <= (XK_z - XK_a); i++)
        {
            qtKeyToVirtualKey[Qt::Key_A + i] = XK_a + i;
        }

        // Map [ to ` ASCII keys
        for (int i=0; i <= (XK_grave - XK_bracketleft); i++)
        {
            qtKeyToVirtualKey[Qt::Key_BracketLeft + i] = XK_bracketleft + i;
        }

        // Map { to ~ ASCII keys
        for (int i=0; i <= (XK_asciitilde - XK_braceleft); i++)
        {
            qtKeyToVirtualKey[Qt::Key_BraceLeft + i] = XK_braceleft + i;
        }

        // Map function keys
        for (int i=0; i <= (XK_F35 - XK_F1); i++)
        {
            qtKeyToVirtualKey[Qt::Key_F1 + i] = XK_F1 + i;
        }

        // Misc
        //qtKeyToVirtualKey[Qt::KeyBri]

        // Map custom defined keys
        qtKeyToVirtualKey[AntKey_Shift_R] = XK_Shift_R;
        qtKeyToVirtualKey[AntKey_Control_R] = XK_Control_R;
        //qtKeyToX11KeySym[AntKey_Shift_Lock] = XK_Shift_Lock;
        //qtKeyToVirtualKey[AntKey_Meta_R] = XK_Meta_R;
        qtKeyToVirtualKey[AntKey_Alt_R] = XK_Alt_R;
        qtKeyToVirtualKey[AntKey_KP_Multiply] = XK_KP_Multiply;

	// Map 0 to 9
        for (int i=0; i <= (XK_KP_9 - XK_KP_0); i++)
        {
            qtKeyToVirtualKey[AntKey_KP_0 + i] = XK_KP_0 + i;
        }

	// Map lower-case latin characters to their capital equivalents
	for( int i=0; i <= (XK_odiaeresis - XK_agrave); i++) {
	  qtKeyToVirtualKey[ Qt::Key_Agrave + i ] = XK_agrave + i;
	}
	for( int i=0; i <= (XK_thorn - XK_oslash); i++) {
	  qtKeyToVirtualKey[ Qt::Key_Ooblique + i ] = XK_oslash + i;
	}

        QHashIterator<unsigned int, unsigned int> iter(qtKeyToVirtualKey);
        while (iter.hasNext())
        {
            iter.next();
            virtualKeyToQtKey[iter.value()] = iter.key();
        }
    }
}

void QtX11KeyMapper::populateCharKeyInformation()
{
    virtualkeyToCharKeyInformation.clear();
    Display* display = X11Extras::getInstance()->display();

    unsigned int total = 0;
    for (int i=8; i <= 255; i++)
    {
        for (int j=0; j <= 3; j++)
        {
            Qt::KeyboardModifiers dicis;
            if (j >= 2)
            {
                dicis |= Qt::MetaModifier;
            }

            if (j == 1 || j == 3)
            {
                dicis |= Qt::ShiftModifier;
            }

            unsigned int testsym = XkbKeycodeToKeysym(display, i,
                                                      dicis & Qt::MetaModifier ? 1 : 0,
                                                      dicis & Qt::ShiftModifier ? 1 : 0);
            if (testsym != NoSymbol)
            {
                XKeyPressedEvent tempevent;
                tempevent.keycode = i;
                tempevent.type = KeyPress;
                tempevent.display = display;
                tempevent.state = 0;
                if (dicis & Qt::ShiftModifier)
                {
                    tempevent.state |= ShiftMask;
                }

                if (dicis & Qt::MetaModifier)
                {
                    tempevent.state |= Mod1Mask;
                }

                char returnstring[256];
                memset(returnstring, 0, sizeof(returnstring));
                int bitestoreturn = sizeof(returnstring) - 1;
                int numchars = XLookupString(&tempevent, returnstring, bitestoreturn, NULL, NULL);
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
                        if (!virtualkeyToCharKeyInformation.contains(tempchar.unicode()))
                        {
                            virtualkeyToCharKeyInformation.insert(tempchar.unicode(), testKeyInformation);
                            //qDebug() << "I FOUND SOMETHING: " << tempchar;
                            total++;
                        }
                    }
                    else
                    {
                        //qDebug() << "YOU FAIL: " << tempchar;
                    }
                }

            }
        }
    }

    //qDebug() << "TOTAL: " << total;
    //qDebug() << "";

    QChar tempa('*');
    if (virtualkeyToCharKeyInformation.contains(tempa.unicode()))
    {
        charKeyInformation projects = virtualkeyToCharKeyInformation.value(tempa.unicode());
    }
}


