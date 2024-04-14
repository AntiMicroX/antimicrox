/* antimicrox Gamepad to KB+M event mapper
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

#include <linux/input.h>
#include <linux/uinput.h>

//#include <QDebug>

#include <QChar>
#include <QList>
#include <QListIterator>

#include "qtuinputkeymapper.h"

QtUInputKeyMapper::QtUInputKeyMapper(QObject *parent)
    : QtKeyMapperBase(parent)
{
    identifier = "uinput";
    populateMappingHashes();
    populateCharKeyInformation();
}

void QtUInputKeyMapper::populateAlphaHashes()
{
    // Map lowercase alpha keys
    qtKeyToVirtKeyHash[Qt::Key_A] = KEY_A;
    qtKeyToVirtKeyHash[Qt::Key_B] = KEY_B;
    qtKeyToVirtKeyHash[Qt::Key_C] = KEY_C;
    qtKeyToVirtKeyHash[Qt::Key_D] = KEY_D;
    qtKeyToVirtKeyHash[Qt::Key_E] = KEY_E;
    qtKeyToVirtKeyHash[Qt::Key_F] = KEY_F;
    qtKeyToVirtKeyHash[Qt::Key_G] = KEY_G;
    qtKeyToVirtKeyHash[Qt::Key_H] = KEY_H;
    qtKeyToVirtKeyHash[Qt::Key_I] = KEY_I;
    qtKeyToVirtKeyHash[Qt::Key_J] = KEY_J;
    qtKeyToVirtKeyHash[Qt::Key_K] = KEY_K;
    qtKeyToVirtKeyHash[Qt::Key_L] = KEY_L;
    qtKeyToVirtKeyHash[Qt::Key_M] = KEY_M;
    qtKeyToVirtKeyHash[Qt::Key_N] = KEY_N;
    qtKeyToVirtKeyHash[Qt::Key_O] = KEY_O;
    qtKeyToVirtKeyHash[Qt::Key_P] = KEY_P;
    qtKeyToVirtKeyHash[Qt::Key_Q] = KEY_Q;
    qtKeyToVirtKeyHash[Qt::Key_R] = KEY_R;
    qtKeyToVirtKeyHash[Qt::Key_S] = KEY_S;
    qtKeyToVirtKeyHash[Qt::Key_T] = KEY_T;
    qtKeyToVirtKeyHash[Qt::Key_U] = KEY_U;
    qtKeyToVirtKeyHash[Qt::Key_V] = KEY_V;
    qtKeyToVirtKeyHash[Qt::Key_W] = KEY_W;
    qtKeyToVirtKeyHash[Qt::Key_X] = KEY_X;
    qtKeyToVirtKeyHash[Qt::Key_Y] = KEY_Y;
    qtKeyToVirtKeyHash[Qt::Key_Z] = KEY_Z;
}

void QtUInputKeyMapper::populateFKeyHashes()
{
    // Map F1 - F10
    for (int i = 0; i <= (KEY_F10 - KEY_F1); i++)
        qtKeyToVirtKeyHash[Qt::Key_F1 + i] = KEY_F1 + i;

    // Map F11 and F12
    for (int i = 0; i <= (KEY_F12 - KEY_F11); i++)
        qtKeyToVirtKeyHash[Qt::Key_F11 + i] = KEY_F11 + i;

    // Map F13 - F24
    for (int i = 0; i <= (KEY_F24 - KEY_F13); i++)
        qtKeyToVirtKeyHash[Qt::Key_F13 + i] = KEY_F13 + i;
}

void QtUInputKeyMapper::populateNumPadHashes()
{
    // Map Numpad 0
    qtKeyToVirtKeyHash[AntKey_KP_0] = KEY_KP0;

    // Map Numpad 1 - 3
    for (int i = 0; i <= (KEY_KP3 - KEY_KP1); i++)
        qtKeyToVirtKeyHash[AntKey_KP_1 + i] = KEY_KP1 + i;

    // Map Numpad 4 - 6
    for (int i = 0; i <= (KEY_KP6 - KEY_KP4); i++)
        qtKeyToVirtKeyHash[AntKey_KP_4 + i] = KEY_KP4 + i;

    // Map Numpad 7 - 9
    for (int i = 0; i <= (KEY_KP9 - KEY_KP7); i++)
        qtKeyToVirtKeyHash[AntKey_KP_7 + i] = KEY_KP7 + i;
}

void QtUInputKeyMapper::populateSpecialCharHashes()
{
    qtKeyToVirtKeyHash[Qt::Key_QuoteLeft] = KEY_GRAVE;
    qtKeyToVirtKeyHash[Qt::Key_Minus] = KEY_MINUS;
    qtKeyToVirtKeyHash[Qt::Key_Equal] = KEY_EQUAL;
    qtKeyToVirtKeyHash[Qt::Key_BracketLeft] = KEY_LEFTBRACE;
    qtKeyToVirtKeyHash[Qt::Key_BracketRight] = KEY_RIGHTBRACE;
    qtKeyToVirtKeyHash[Qt::Key_Semicolon] = KEY_SEMICOLON;
    qtKeyToVirtKeyHash[Qt::Key_Apostrophe] = KEY_APOSTROPHE;
    qtKeyToVirtKeyHash[Qt::Key_Comma] = KEY_COMMA;
    qtKeyToVirtKeyHash[Qt::Key_Period] = KEY_DOT;
    qtKeyToVirtKeyHash[Qt::Key_Slash] = KEY_SLASH;
    qtKeyToVirtKeyHash[Qt::Key_Backslash] = KEY_BACKSLASH;
}

void QtUInputKeyMapper::populateMappingHashes()
{
    if (qtKeyToVirtKeyHash.isEmpty())
    {
        mapMiscKeysQtUinput(qtKeyToVirtKeyHash);
        mapCursorMovesKeysQtUinput(qtKeyToVirtKeyHash);
        mapModifiersKeysQtUinput(qtKeyToVirtKeyHash);
        mapMediaKeysQtUinput(qtKeyToVirtKeyHash);

        // Map 0-9 keys
        for (int i = 0; i <= (KEY_9 - KEY_1); i++)
            qtKeyToVirtKeyHash[Qt::Key_1 + i] = KEY_1 + i;

        qtKeyToVirtKeyHash[Qt::Key_0] = KEY_0;

        populateSpecialCharHashes();
        populateAlphaHashes();
        populateFKeyHashes();
        populateNumPadHashes();

        // Map custom defined keys
        qtKeyToVirtKeyHash[AntKey_Shift_R] = KEY_RIGHTSHIFT;
        qtKeyToVirtKeyHash[AntKey_Control_R] = KEY_RIGHTCTRL;
        qtKeyToVirtKeyHash[AntKey_Alt_R] = KEY_RIGHTALT;
        qtKeyToVirtKeyHash[AntKey_KP_Multiply] = KEY_KPASTERISK;

        mapKeypadKeysQtUinput(qtKeyToVirtKeyHash);

        // International input method support keys

        // Misc Functions
        qtKeyToVirtKeyHash[Qt::Key_Mode_switch] = KEY_SWITCHVIDEOMODE;

        mapJapanKeysQtUinput(qtKeyToVirtKeyHash);

#ifdef XK_KOREAN
        mapKoreanKeysQtUinput(qtKeyToVirtKeyHash);
#endif // XK_KOREAN

        // Populate other hash. Flip key and value so mapping
        // goes VK -> Qt Key.
        QHashIterator<int, int> iter(qtKeyToVirtKeyHash);
        while (iter.hasNext())
        {
            iter.next();
            virtKeyToQtKeyHash[iter.value()] = iter.key();
        }

        // Override some entries.
        virtKeyToQtKeyHash[KEY_KP0] = AntKey_KP_0;
        virtKeyToQtKeyHash[KEY_KP1] = AntKey_KP_1;
        virtKeyToQtKeyHash[KEY_KP2] = AntKey_KP_2;
        virtKeyToQtKeyHash[KEY_KP3] = AntKey_KP_3;
        virtKeyToQtKeyHash[KEY_KP4] = AntKey_KP_4;
        virtKeyToQtKeyHash[KEY_KP5] = AntKey_KP_5;
        virtKeyToQtKeyHash[KEY_KP6] = AntKey_KP_6;
        virtKeyToQtKeyHash[KEY_KP7] = AntKey_KP_7;
        virtKeyToQtKeyHash[KEY_KP8] = AntKey_KP_8;
        virtKeyToQtKeyHash[KEY_KP9] = AntKey_KP_9;
        virtKeyToQtKeyHash[KEY_CALC] = Qt::Key_Launch1;
    }
}

void QtUInputKeyMapper::populateCharKeyInformation()
{
    virtkeyToCharKeyInfo.clear();

    char16_t unicodeTempValue = 0;
    int listIndex = 0;
    charKeyInformation charKeyInfo;
    charKeyInfo.modifiers = Qt::NoModifier;
    charKeyInfo.virtualkey = 0;

    // Map 0-9 keys
    for (char16_t i = QChar('1').unicode(); i <= QChar('9').unicode(); i++)
        addVirtualKeyToHash(KEY_1 + i, i, charKeyInfo);

    addVirtualKeyToHash(KEY_0, QChar('0'), charKeyInfo);
    addVirtualKeyToHash(KEY_MINUS, QChar('-'), charKeyInfo);
    addVirtualKeyToHash(KEY_EQUAL, QChar('='), charKeyInfo);

    QList<int> tempKeys;
    tempKeys.append(KEY_A);
    tempKeys.append(KEY_B);
    tempKeys.append(KEY_C);
    tempKeys.append(KEY_D);
    tempKeys.append(KEY_E);
    tempKeys.append(KEY_F);
    tempKeys.append(KEY_G);
    tempKeys.append(KEY_H);
    tempKeys.append(KEY_I);
    tempKeys.append(KEY_J);
    tempKeys.append(KEY_K);
    tempKeys.append(KEY_L);
    tempKeys.append(KEY_M);
    tempKeys.append(KEY_N);
    tempKeys.append(KEY_O);
    tempKeys.append(KEY_P);
    tempKeys.append(KEY_Q);
    tempKeys.append(KEY_R);
    tempKeys.append(KEY_S);
    tempKeys.append(KEY_T);
    tempKeys.append(KEY_U);
    tempKeys.append(KEY_V);
    tempKeys.append(KEY_W);
    tempKeys.append(KEY_X);
    tempKeys.append(KEY_Y);
    tempKeys.append(KEY_Z);

    unicodeTempValue = QChar('a').unicode();
    QListIterator<int> tempIter(tempKeys);

    while (tempIter.hasNext())
    {
        addVirtualKeyToHash(tempIter.next(), unicodeTempValue, charKeyInfo);
        unicodeTempValue++;
    }

    tempIter.toFront();

    charKeyInfo.modifiers = Qt::ShiftModifier;
    unicodeTempValue = QChar('A').unicode();

    while (tempIter.hasNext())
    {
        addVirtualKeyToHash(tempIter.next(), unicodeTempValue, charKeyInfo);
        unicodeTempValue++;
    }

    tempKeys.clear();

    charKeyInfo.modifiers = Qt::ShiftModifier;
    tempKeys.append(QChar('!').unicode());
    tempKeys.append(QChar('@').unicode());
    tempKeys.append(QChar('#').unicode());
    tempKeys.append(QChar('$').unicode());
    tempKeys.append(QChar('%').unicode());
    tempKeys.append(QChar('^').unicode());
    tempKeys.append(QChar('&').unicode());
    tempKeys.append(QChar('*').unicode());
    tempKeys.append(QChar('(').unicode());
    tempKeys.append(QChar(')').unicode());
    tempKeys.append(QChar('_').unicode());
    tempKeys.append(QChar('+').unicode());

    tempIter = QListIterator<int>(tempKeys);
    listIndex = 0;

    while (tempIter.hasNext())
    {
        addVirtualKeyToHash(KEY_1 + listIndex, QChar(tempIter.next()), charKeyInfo);
        listIndex++;
    }

    tempKeys.clear();

    charKeyInfo.modifiers = Qt::NoModifier;

    addVirtualKeyToHash(KEY_SPACE, QChar(' '), charKeyInfo);
    addVirtualKeyToHash(KEY_LEFTBRACE, QChar('['), charKeyInfo);
    addVirtualKeyToHash(KEY_RIGHTBRACE, QChar(']'), charKeyInfo);
    addVirtualKeyToHash(KEY_BACKSLASH, QChar('\\'), charKeyInfo);
    addVirtualKeyToHash(KEY_SEMICOLON, QChar(';'), charKeyInfo);
    addVirtualKeyToHash(KEY_APOSTROPHE, QChar('\''), charKeyInfo);
    addVirtualKeyToHash(KEY_COMMA, QChar(','), charKeyInfo);
    addVirtualKeyToHash(KEY_DOT, QChar('.'), charKeyInfo);
    addVirtualKeyToHash(KEY_SLASH, QChar('/'), charKeyInfo);
    addVirtualKeyToHash(KEY_LEFTBRACE, QChar('{'), charKeyInfo);
    addVirtualKeyToHash(KEY_RIGHTBRACE, QChar('}'), charKeyInfo);
    addVirtualKeyToHash(KEY_BACKSLASH, QChar('|'), charKeyInfo);
    addVirtualKeyToHash(KEY_SEMICOLON, QChar(':'), charKeyInfo);
    addVirtualKeyToHash(KEY_APOSTROPHE, QChar('"'), charKeyInfo);
    addVirtualKeyToHash(KEY_COMMA, QChar('<'), charKeyInfo);
    addVirtualKeyToHash(KEY_DOT, QChar('>'), charKeyInfo);
    addVirtualKeyToHash(KEY_SLASH, QChar('?'), charKeyInfo);
}

void QtUInputKeyMapper::addVirtualKeyToHash(int key, QChar character, charKeyInformation &charKeyInfo)
{
    charKeyInfo.virtualkey = key;
    virtkeyToCharKeyInfo.insert(character.unicode(), charKeyInfo);
}

void QtUInputKeyMapper::mapMiscKeysQtUinput(QHash<int, int> &qtKeyToVirtKeyHash)
{
    qtKeyToVirtKeyHash[Qt::Key_Escape] = KEY_ESC;
    qtKeyToVirtKeyHash[Qt::Key_Tab] = KEY_TAB;
    qtKeyToVirtKeyHash[Qt::Key_Backspace] = KEY_BACKSPACE;
    qtKeyToVirtKeyHash[Qt::Key_Return] = KEY_ENTER;
    qtKeyToVirtKeyHash[Qt::Key_Insert] = KEY_INSERT;
    qtKeyToVirtKeyHash[Qt::Key_Delete] = KEY_DELETE;
    qtKeyToVirtKeyHash[Qt::Key_Pause] = KEY_PAUSE;
    qtKeyToVirtKeyHash[Qt::Key_Print] = KEY_PRINT;
    qtKeyToVirtKeyHash[Qt::Key_Space] = KEY_SPACE;
    qtKeyToVirtKeyHash[Qt::Key_SysReq] = KEY_SYSRQ;
    qtKeyToVirtKeyHash[Qt::Key_PowerOff] = KEY_POWER;
    qtKeyToVirtKeyHash[Qt::Key_Stop] = KEY_STOP;
    qtKeyToVirtKeyHash[Qt::Key_Refresh] = KEY_REFRESH;
    qtKeyToVirtKeyHash[Qt::Key_Copy] = KEY_COPY;
    qtKeyToVirtKeyHash[Qt::Key_Paste] = KEY_PASTE;
    // qtKeyToVirtKeyHash[Qt::Key_Search] = KEY_FIND;
    qtKeyToVirtKeyHash[Qt::Key_Cut] = KEY_CUT;
    qtKeyToVirtKeyHash[Qt::Key_Sleep] = KEY_SLEEP;
    qtKeyToVirtKeyHash[Qt::Key_Calculator] = KEY_CALC;
    qtKeyToVirtKeyHash[Qt::Key_Launch0] = KEY_COMPUTER;
    qtKeyToVirtKeyHash[Qt::Key_Launch1] = KEY_CALC;
    qtKeyToVirtKeyHash[Qt::Key_Launch2] = KEY_PROG1;
    qtKeyToVirtKeyHash[Qt::Key_Launch3] = KEY_PROG2;
    qtKeyToVirtKeyHash[Qt::Key_Launch4] = KEY_PROG3;
    qtKeyToVirtKeyHash[Qt::Key_Launch5] = KEY_PROG4;
    qtKeyToVirtKeyHash[Qt::Key_HomePage] = KEY_HOMEPAGE;
    qtKeyToVirtKeyHash[Qt::Key_LaunchMail] = KEY_MAIL;
    qtKeyToVirtKeyHash[Qt::Key_Back] = KEY_BACK;
    qtKeyToVirtKeyHash[Qt::Key_Favorites] = KEY_FAVORITES;
    qtKeyToVirtKeyHash[Qt::Key_Forward] = KEY_FORWARD;
    qtKeyToVirtKeyHash[Qt::Key_Suspend] = KEY_SUSPEND;
    qtKeyToVirtKeyHash[Qt::Key_Close] = KEY_CLOSE;
    // qtKeyToVirtKeyHash[Qt::Key_Search] = KEY_SEARCH;
    qtKeyToVirtKeyHash[Qt::Key_Camera] = KEY_CAMERA;
    qtKeyToVirtKeyHash[Qt::Key_MonBrightnessUp] = KEY_BRIGHTNESSUP;
    qtKeyToVirtKeyHash[Qt::Key_MonBrightnessDown] = KEY_BRIGHTNESSDOWN;
    qtKeyToVirtKeyHash[Qt::Key_Send] = KEY_SEND;
    qtKeyToVirtKeyHash[Qt::Key_Reply] = KEY_REPLY;
    qtKeyToVirtKeyHash[Qt::Key_Forward] = KEY_FORWARDMAIL;
    qtKeyToVirtKeyHash[Qt::Key_Save] = KEY_SAVE;
    qtKeyToVirtKeyHash[Qt::Key_Documents] = KEY_DOCUMENTS;
    qtKeyToVirtKeyHash[Qt::Key_Battery] = KEY_BATTERY;
    qtKeyToVirtKeyHash[Qt::Key_Bluetooth] = KEY_BLUETOOTH;
    qtKeyToVirtKeyHash[Qt::Key_WLAN] = KEY_WLAN;
    qtKeyToVirtKeyHash[Qt::Key_Cancel] = KEY_CANCEL;
    qtKeyToVirtKeyHash[Qt::Key_Shop] = KEY_SHOP;
    qtKeyToVirtKeyHash[Qt::Key_Finance] = KEY_FINANCE;
    qtKeyToVirtKeyHash[Qt::Key_Question] = KEY_QUESTION;
    qtKeyToVirtKeyHash[Qt::Key_BassBoost] = KEY_BASSBOOST;
}

void QtUInputKeyMapper::mapCursorMovesKeysQtUinput(QHash<int, int> &qtKeyToVirtKeyHash)
{
    qtKeyToVirtKeyHash[Qt::Key_Home] = KEY_HOME;
    qtKeyToVirtKeyHash[Qt::Key_End] = KEY_END;
    qtKeyToVirtKeyHash[Qt::Key_Left] = KEY_LEFT;
    qtKeyToVirtKeyHash[Qt::Key_Up] = KEY_UP;
    qtKeyToVirtKeyHash[Qt::Key_Right] = KEY_RIGHT;
    qtKeyToVirtKeyHash[Qt::Key_Down] = KEY_DOWN;
    qtKeyToVirtKeyHash[Qt::Key_PageUp] = KEY_PAGEUP;
    qtKeyToVirtKeyHash[Qt::Key_PageDown] = KEY_PAGEDOWN;
}

void QtUInputKeyMapper::mapModifiersKeysQtUinput(QHash<int, int> &qtKeyToVirtKeyHash)
{
    qtKeyToVirtKeyHash[Qt::Key_Shift] = KEY_LEFTSHIFT;
    qtKeyToVirtKeyHash[Qt::Key_Control] = KEY_LEFTCTRL;
    qtKeyToVirtKeyHash[Qt::Key_Alt] = KEY_LEFTALT;
    qtKeyToVirtKeyHash[Qt::Key_CapsLock] = KEY_CAPSLOCK;
    qtKeyToVirtKeyHash[Qt::Key_NumLock] = KEY_NUMLOCK;
    qtKeyToVirtKeyHash[Qt::Key_ScrollLock] = KEY_SCROLLLOCK;
    qtKeyToVirtKeyHash[Qt::Key_Meta] = KEY_LEFTMETA;
    qtKeyToVirtKeyHash[AntKey_Meta_R] = KEY_RIGHTMETA;
    qtKeyToVirtKeyHash[Qt::Key_Menu] = KEY_COMPOSE;
    qtKeyToVirtKeyHash[Qt::Key_Help] = KEY_HELP;
}

void QtUInputKeyMapper::mapKeypadKeysQtUinput(QHash<int, int> &qtKeyToVirtKeyHash)
{
    qtKeyToVirtKeyHash[AntKey_KP_Enter] = KEY_KPENTER;
    qtKeyToVirtKeyHash[AntKey_KP_Home] = KEY_HOME;
    qtKeyToVirtKeyHash[AntKey_KP_Left] = KEY_LEFT;
    qtKeyToVirtKeyHash[AntKey_KP_Up] = KEY_UP;
    qtKeyToVirtKeyHash[AntKey_KP_Right] = KEY_RIGHT;
    qtKeyToVirtKeyHash[AntKey_KP_Down] = KEY_DOWN;
    qtKeyToVirtKeyHash[AntKey_KP_Prior] = KEY_PAGEUP;
    qtKeyToVirtKeyHash[AntKey_KP_Next] = KEY_PAGEDOWN;
    qtKeyToVirtKeyHash[AntKey_KP_End] = KEY_END;
    qtKeyToVirtKeyHash[AntKey_KP_Begin] = KEY_LEFTMETA;
    qtKeyToVirtKeyHash[AntKey_KP_Insert] = KEY_INSERT;
    qtKeyToVirtKeyHash[AntKey_KP_Add] = KEY_KPPLUS;
    qtKeyToVirtKeyHash[AntKey_KP_Subtract] = KEY_KPMINUS;
    qtKeyToVirtKeyHash[AntKey_KP_Delete] = KEY_KPDOT;
    qtKeyToVirtKeyHash[AntKey_KP_Decimal] = KEY_KPDOT;
    qtKeyToVirtKeyHash[AntKey_KP_Divide] = KEY_KPSLASH;
    qtKeyToVirtKeyHash[AntKey_KP_1] = KEY_KP1;
    qtKeyToVirtKeyHash[AntKey_KP_2] = KEY_KP2;
    qtKeyToVirtKeyHash[AntKey_KP_3] = KEY_KP3;
    qtKeyToVirtKeyHash[AntKey_KP_4] = KEY_KP4;
    qtKeyToVirtKeyHash[AntKey_KP_5] = KEY_KP5;
    qtKeyToVirtKeyHash[AntKey_KP_6] = KEY_KP6;
    qtKeyToVirtKeyHash[AntKey_KP_7] = KEY_KP7;
    qtKeyToVirtKeyHash[AntKey_KP_8] = KEY_KP8;
    qtKeyToVirtKeyHash[AntKey_KP_9] = KEY_KP9;
    qtKeyToVirtKeyHash[AntKey_KP_0] = KEY_KP0;
}

void QtUInputKeyMapper::mapJapanKeysQtUinput(QHash<int, int> &qtKeyToVirtKeyHash)
{
    // qtKeyToVirtKeyHash[Qt::Key_Kanji] = XK_Kanji;
    qtKeyToVirtKeyHash[Qt::Key_Muhenkan] = KEY_MUHENKAN;
    qtKeyToVirtKeyHash[Qt::Key_Henkan] = KEY_HENKAN;
    qtKeyToVirtKeyHash[Qt::Key_Romaji] = KEY_RO;
    qtKeyToVirtKeyHash[Qt::Key_Hiragana] = KEY_HIRAGANA;
    qtKeyToVirtKeyHash[Qt::Key_Katakana] = KEY_KATAKANA;
    qtKeyToVirtKeyHash[Qt::Key_Hiragana_Katakana] = KEY_KATAKANAHIRAGANA;
    // qtKeyToVirtKeyHash[Qt::Key_Zenkaku] = XK_Zenkaku;
    // qtKeyToVirtKeyHash[Qt::Key_Hankaku] = XK_Hankaku;
    qtKeyToVirtKeyHash[Qt::Key_Zenkaku_Hankaku] = KEY_ZENKAKUHANKAKU;
    /* qtKeyToVirtKeyHash[Qt::Key_Touroku] = XK_Touroku;
     qtKeyToVirtKeyHash[Qt::Key_Massyo] = XK_Massyo;
     qtKeyToVirtKeyHash[Qt::Key_Kana_Lock] = XK_Kana_Lock;
     qtKeyToVirtKeyHash[Qt::Key_Kana_Shift] = XK_Kana_Shift;
     qtKeyToVirtKeyHash[Qt::Key_Eisu_Shift] = XK_Eisu_Shift;
     qtKeyToVirtKeyHash[Qt::Key_Eisu_toggle] = XK_Eisu_toggle;
     qtKeyToVirtKeyHash[Qt::Key_Codeinput] = XK_Kanji_Bangou;
     qtKeyToX11KeySym[Qt::Key_MultipleCandidate] = XK_Zen_Koho;
     qtKeyToX11KeySym[Qt::Key_PreviousCandidate] = XK_Mae_Koho;*/
}

void QtUInputKeyMapper::mapKoreanKeysQtUinput(QHash<int, int> &qtKeyToVirtKeyHash)
{
    qtKeyToVirtKeyHash[Qt::Key_Hangul] = KEY_HANGEUL;
    /* qtKeyToVirtKeyHash[Qt::Key_Hangul_Start] = XK_Hangul_Start;
     qtKeyToVirtKeyHash[Qt::Key_Hangul_End] = XK_Hangul_End;
     qtKeyToVirtKeyHash[Qt::Key_Hangul_Hanja] = KEY_HANJA;
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
     //qtKeyToX11KeySym[Qt::Key_Mode_switch] = XK_Hangul_switch;*/
}

void QtUInputKeyMapper::mapMediaKeysQtUinput(QHash<int, int> &qtKeyToVirtKeyHash)
{
    qtKeyToVirtKeyHash[Qt::Key_VolumeDown] = KEY_VOLUMEDOWN;
    qtKeyToVirtKeyHash[Qt::Key_VolumeMute] = KEY_MUTE;
    qtKeyToVirtKeyHash[Qt::Key_VolumeUp] = KEY_VOLUMEUP;
    qtKeyToVirtKeyHash[Qt::Key_MediaPlay] = KEY_PLAYPAUSE;
    qtKeyToVirtKeyHash[Qt::Key_MediaStop] = KEY_STOPCD;
    qtKeyToVirtKeyHash[Qt::Key_MediaPrevious] = KEY_PREVIOUSSONG;
    qtKeyToVirtKeyHash[Qt::Key_MediaNext] = KEY_NEXTSONG;
    qtKeyToVirtKeyHash[Qt::Key_MediaRecord] = KEY_RECORD;
    qtKeyToVirtKeyHash[Qt::Key_LaunchMedia] = KEY_MEDIA;
}
