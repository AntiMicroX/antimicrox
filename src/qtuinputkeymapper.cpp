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

#include <linux/input.h>
#include <linux/uinput.h>

//#include <QDebug>

#include <QChar>
#include <QList>
#include <QListIterator>

#include "qtuinputkeymapper.h"

QtUInputKeyMapper::QtUInputKeyMapper(QObject *parent) :
    QtKeyMapperBase(parent)
{
    identifier = "uinput";
    populateMappingHashes();
    populateCharKeyInformation();
}

void QtUInputKeyMapper::populateAlphaHashes()
{
    // Map lowercase alpha keys
    qtKeyToVirtualKey[Qt::Key_A] = KEY_A;
    qtKeyToVirtualKey[Qt::Key_B] = KEY_B;
    qtKeyToVirtualKey[Qt::Key_C] = KEY_C;
    qtKeyToVirtualKey[Qt::Key_D] = KEY_D;
    qtKeyToVirtualKey[Qt::Key_E] = KEY_E;
    qtKeyToVirtualKey[Qt::Key_F] = KEY_F;
    qtKeyToVirtualKey[Qt::Key_G] = KEY_G;
    qtKeyToVirtualKey[Qt::Key_H] = KEY_H;
    qtKeyToVirtualKey[Qt::Key_I] = KEY_I;
    qtKeyToVirtualKey[Qt::Key_J] = KEY_J;
    qtKeyToVirtualKey[Qt::Key_K] = KEY_K;
    qtKeyToVirtualKey[Qt::Key_L] = KEY_L;
    qtKeyToVirtualKey[Qt::Key_M] = KEY_M;
    qtKeyToVirtualKey[Qt::Key_N] = KEY_N;
    qtKeyToVirtualKey[Qt::Key_O] = KEY_O;
    qtKeyToVirtualKey[Qt::Key_P] = KEY_P;
    qtKeyToVirtualKey[Qt::Key_Q] = KEY_Q;
    qtKeyToVirtualKey[Qt::Key_R] = KEY_R;
    qtKeyToVirtualKey[Qt::Key_S] = KEY_S;
    qtKeyToVirtualKey[Qt::Key_T] = KEY_T;
    qtKeyToVirtualKey[Qt::Key_U] = KEY_U;
    qtKeyToVirtualKey[Qt::Key_V] = KEY_V;
    qtKeyToVirtualKey[Qt::Key_W] = KEY_W;
    qtKeyToVirtualKey[Qt::Key_X] = KEY_X;
    qtKeyToVirtualKey[Qt::Key_Y] = KEY_Y;
    qtKeyToVirtualKey[Qt::Key_Z] = KEY_Z;
}

void QtUInputKeyMapper::populateFKeyHashes()
{
    // Map F1 - F10
    for (int i=0; i <= (KEY_F10 - KEY_F1); i++)
    {
        qtKeyToVirtualKey[Qt::Key_F1 + i] = KEY_F1 + i;
    }

    // Map F11 and F12
    for (int i=0; i <= (KEY_F12 - KEY_F11); i++)
    {
        qtKeyToVirtualKey[Qt::Key_F11 + i] = KEY_F11 + i;
    }

    // Map F13 - F24
    for (int i=0; i <= (KEY_F24 - KEY_F13); i++)
    {
        qtKeyToVirtualKey[Qt::Key_F13 + i] = KEY_F13 + i;
    }
}

void QtUInputKeyMapper::populateNumPadHashes()
{
    // Map Numpad 0
    qtKeyToVirtualKey[AntKey_KP_0] = KEY_KP0;

    // Map Numpad 1 - 3
    for (int i=0; i <= (KEY_KP3 - KEY_KP1); i++)
    {
        qtKeyToVirtualKey[AntKey_KP_1 + i] = KEY_KP1 + i;
    }

    // Map Numpad 4 - 6
    for (int i=0; i <= (KEY_KP6 - KEY_KP4); i++)
    {
        qtKeyToVirtualKey[AntKey_KP_4 + i] = KEY_KP4 + i;
    }

    // Map Numpad 7 - 9
    for (int i=0; i <= (KEY_KP9 - KEY_KP7); i++)
    {
        qtKeyToVirtualKey[AntKey_KP_7 + i] = KEY_KP7 + i;
    }
}

void QtUInputKeyMapper::populateSpecialCharHashes()
{
    qtKeyToVirtualKey[Qt::Key_QuoteLeft] = KEY_GRAVE;
    qtKeyToVirtualKey[Qt::Key_Minus] = KEY_MINUS;
    qtKeyToVirtualKey[Qt::Key_Equal] = KEY_EQUAL;
    qtKeyToVirtualKey[Qt::Key_BracketLeft] = KEY_LEFTBRACE;
    qtKeyToVirtualKey[Qt::Key_BracketRight] = KEY_RIGHTBRACE;
    qtKeyToVirtualKey[Qt::Key_Semicolon] = KEY_SEMICOLON;
    qtKeyToVirtualKey[Qt::Key_Apostrophe] = KEY_APOSTROPHE;
    qtKeyToVirtualKey[Qt::Key_Comma] = KEY_COMMA;
    qtKeyToVirtualKey[Qt::Key_Period] = KEY_DOT;
    qtKeyToVirtualKey[Qt::Key_Slash] = KEY_SLASH;
    qtKeyToVirtualKey[Qt::Key_Backslash] = KEY_BACKSLASH;
}

void QtUInputKeyMapper::populateMappingHashes()
{
    if (qtKeyToVirtualKey.isEmpty())
    {
        // misc keys
        qtKeyToVirtualKey[Qt::Key_Escape] = KEY_ESC;
        qtKeyToVirtualKey[Qt::Key_Tab] = KEY_TAB;
        qtKeyToVirtualKey[Qt::Key_Backspace] = KEY_BACKSPACE;
        qtKeyToVirtualKey[Qt::Key_Return] = KEY_ENTER;
        qtKeyToVirtualKey[Qt::Key_Insert] = KEY_INSERT;
        qtKeyToVirtualKey[Qt::Key_Delete] = KEY_DELETE;
        qtKeyToVirtualKey[Qt::Key_Pause] = KEY_PAUSE;
        qtKeyToVirtualKey[Qt::Key_Print] = KEY_PRINT;
        qtKeyToVirtualKey[Qt::Key_Space] = KEY_SPACE;

        qtKeyToVirtualKey[Qt::Key_SysReq] = KEY_SYSRQ;
        qtKeyToVirtualKey[Qt::Key_PowerOff] = KEY_POWER;
        qtKeyToVirtualKey[Qt::Key_Stop] = KEY_STOP;
        qtKeyToVirtualKey[Qt::Key_Refresh] = KEY_REFRESH;
        qtKeyToVirtualKey[Qt::Key_Copy] = KEY_COPY;
        qtKeyToVirtualKey[Qt::Key_Paste] = KEY_PASTE;
        //qtKeyToVirtualKey[Qt::Key_Search] = KEY_FIND;
        qtKeyToVirtualKey[Qt::Key_Cut] = KEY_CUT;
        qtKeyToVirtualKey[Qt::Key_Sleep] = KEY_SLEEP;
        //qtKeyToVirtualKey[Qt::Key_Calculator] = KEY_CALC;
        qtKeyToVirtualKey[Qt::Key_Launch0] = KEY_COMPUTER;
        qtKeyToVirtualKey[Qt::Key_Launch1] = KEY_CALC;
        qtKeyToVirtualKey[Qt::Key_Launch2] = KEY_PROG1;
        qtKeyToVirtualKey[Qt::Key_Launch3] = KEY_PROG2;
        qtKeyToVirtualKey[Qt::Key_Launch4] = KEY_PROG3;
        qtKeyToVirtualKey[Qt::Key_Launch5] = KEY_PROG4;
        qtKeyToVirtualKey[Qt::Key_HomePage] = KEY_HOMEPAGE;
        qtKeyToVirtualKey[Qt::Key_LaunchMail] = KEY_MAIL;
        qtKeyToVirtualKey[Qt::Key_Back] = KEY_BACK;
        qtKeyToVirtualKey[Qt::Key_Favorites] = KEY_FAVORITES;
        qtKeyToVirtualKey[Qt::Key_Forward] = KEY_FORWARD;
        qtKeyToVirtualKey[Qt::Key_Suspend] = KEY_SUSPEND;
        qtKeyToVirtualKey[Qt::Key_Close] = KEY_CLOSE;
        //qtKeyToVirtualKey[Qt::Key_Search] = KEY_SEARCH;
        qtKeyToVirtualKey[Qt::Key_Camera] = KEY_CAMERA;
        qtKeyToVirtualKey[Qt::Key_MonBrightnessUp] = KEY_BRIGHTNESSUP;
        qtKeyToVirtualKey[Qt::Key_MonBrightnessDown] = KEY_BRIGHTNESSDOWN;
        qtKeyToVirtualKey[Qt::Key_Send] = KEY_SEND;
        qtKeyToVirtualKey[Qt::Key_Reply] = KEY_REPLY;
        qtKeyToVirtualKey[Qt::Key_Forward] = KEY_FORWARDMAIL;
        qtKeyToVirtualKey[Qt::Key_Save] = KEY_SAVE;
        qtKeyToVirtualKey[Qt::Key_Documents] = KEY_DOCUMENTS;
        qtKeyToVirtualKey[Qt::Key_Battery] = KEY_BATTERY;
        qtKeyToVirtualKey[Qt::Key_Bluetooth] = KEY_BLUETOOTH;
        qtKeyToVirtualKey[Qt::Key_WLAN] = KEY_WLAN;
        qtKeyToVirtualKey[Qt::Key_Cancel] = KEY_CANCEL;
        qtKeyToVirtualKey[Qt::Key_Shop] = KEY_SHOP;
        qtKeyToVirtualKey[Qt::Key_Finance]  = KEY_FINANCE;
        qtKeyToVirtualKey[Qt::Key_Question] = KEY_QUESTION;
        qtKeyToVirtualKey[Qt::Key_BassBoost] = KEY_BASSBOOST;

        // cursor movement
        qtKeyToVirtualKey[Qt::Key_Home] = KEY_HOME;
        qtKeyToVirtualKey[Qt::Key_End] = KEY_END;
        qtKeyToVirtualKey[Qt::Key_Left] = KEY_LEFT;
        qtKeyToVirtualKey[Qt::Key_Up] = KEY_UP;
        qtKeyToVirtualKey[Qt::Key_Right] = KEY_RIGHT;
        qtKeyToVirtualKey[Qt::Key_Down] = KEY_DOWN;
        qtKeyToVirtualKey[Qt::Key_PageUp] = KEY_PAGEUP;
        qtKeyToVirtualKey[Qt::Key_PageDown] = KEY_PAGEDOWN;

        // modifiers
        qtKeyToVirtualKey[Qt::Key_Shift] = KEY_LEFTSHIFT;
        qtKeyToVirtualKey[Qt::Key_Control] = KEY_LEFTCTRL;
        qtKeyToVirtualKey[Qt::Key_Alt] = KEY_LEFTALT;

        qtKeyToVirtualKey[Qt::Key_CapsLock] = KEY_CAPSLOCK;
        qtKeyToVirtualKey[Qt::Key_NumLock] = KEY_NUMLOCK;
        qtKeyToVirtualKey[Qt::Key_ScrollLock] = KEY_SCROLLLOCK;
        qtKeyToVirtualKey[Qt::Key_Meta] = KEY_LEFTMETA;
        qtKeyToVirtualKey[AntKey_Meta_R] = KEY_RIGHTMETA;
        qtKeyToVirtualKey[Qt::Key_Menu] = KEY_COMPOSE;
        qtKeyToVirtualKey[Qt::Key_Help] = KEY_HELP;

        // media keys
        qtKeyToVirtualKey[Qt::Key_VolumeDown] = KEY_VOLUMEDOWN;
        qtKeyToVirtualKey[Qt::Key_VolumeMute] = KEY_MUTE;
        qtKeyToVirtualKey[Qt::Key_VolumeUp] = KEY_VOLUMEUP;
        qtKeyToVirtualKey[Qt::Key_MediaPlay] = KEY_PLAYPAUSE;
        qtKeyToVirtualKey[Qt::Key_MediaStop] = KEY_STOPCD;
        qtKeyToVirtualKey[Qt::Key_MediaPrevious] = KEY_PREVIOUSSONG;
        qtKeyToVirtualKey[Qt::Key_MediaNext] = KEY_NEXTSONG;
        qtKeyToVirtualKey[Qt::Key_MediaRecord] = KEY_RECORD;
        qtKeyToVirtualKey[Qt::Key_LaunchMedia] = KEY_MEDIA;

        // Map 0-9 keys
        for (unsigned int i=0; i <= (KEY_9 - KEY_1); i++)
        {
            qtKeyToVirtualKey[Qt::Key_1 + i] = KEY_1 + i;
        }
        qtKeyToVirtualKey[Qt::Key_0] = KEY_0;

        populateSpecialCharHashes();
        populateAlphaHashes();
        populateFKeyHashes();
        populateNumPadHashes();

        // Map custom defined keys
        qtKeyToVirtualKey[AntKey_Shift_R] = KEY_RIGHTSHIFT;
        qtKeyToVirtualKey[AntKey_Control_R] = KEY_RIGHTCTRL;
        qtKeyToVirtualKey[AntKey_Alt_R] = KEY_RIGHTALT;
        qtKeyToVirtualKey[AntKey_KP_Multiply] = KEY_KPASTERISK;

        // numeric and function keypad keys
        qtKeyToVirtualKey[Qt::Key_Enter] = KEY_KPENTER;
        qtKeyToVirtualKey[AntKey_KP_Home] = KEY_KP7;
        qtKeyToVirtualKey[AntKey_KP_Left] = KEY_KP4;
        qtKeyToVirtualKey[AntKey_KP_Up] = KEY_KP8;
        qtKeyToVirtualKey[AntKey_KP_Right] = KEY_KP6;
        qtKeyToVirtualKey[AntKey_KP_Down] = KEY_KP2;
        qtKeyToVirtualKey[AntKey_KP_Prior] = KEY_KP9;
        qtKeyToVirtualKey[AntKey_KP_Next] = KEY_KP3;
        qtKeyToVirtualKey[AntKey_KP_End] = KEY_KP1;

        qtKeyToVirtualKey[AntKey_KP_Begin] = KEY_KP5;
        qtKeyToVirtualKey[AntKey_KP_Insert] = KEY_KP0;
        qtKeyToVirtualKey[AntKey_KP_Add] = KEY_KPPLUS;
        qtKeyToVirtualKey[AntKey_KP_Subtract] = KEY_KPMINUS;
        qtKeyToVirtualKey[AntKey_KP_Decimal] = KEY_KPDOT;
        qtKeyToVirtualKey[AntKey_KP_Divide] = KEY_KPSLASH;

        /*
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
        */

        // Populate other hash. Flip key and value so mapping
        // goes VK -> Qt Key.
        QHashIterator<unsigned int, unsigned int> iter(qtKeyToVirtualKey);
        while (iter.hasNext())
        {
            iter.next();
            virtualKeyToQtKey[iter.value()] = iter.key();
        }

        /*int j = 0;
        for (int i=KEY_ESC; i < KEY_UNKNOWN; i++)
        {
            if (!virtualKeyToQtKey.contains(i) && i != 84)
            {
                qDebug() << "KEY MISSING: " << QString::number(i);
                j++;
            }
        }

        qDebug() << "TOTAL MISSING: " << j;
        */

        // Override some entries.
        virtualKeyToQtKey[KEY_KP0] = AntKey_KP_0;
        virtualKeyToQtKey[KEY_KP1] = AntKey_KP_1;
        virtualKeyToQtKey[KEY_KP2] = AntKey_KP_2;
        virtualKeyToQtKey[KEY_KP3] = AntKey_KP_3;
        virtualKeyToQtKey[KEY_KP4] = AntKey_KP_4;
        virtualKeyToQtKey[KEY_KP5] = AntKey_KP_5;
        virtualKeyToQtKey[KEY_KP6] = AntKey_KP_6;
        virtualKeyToQtKey[KEY_KP7] = AntKey_KP_7;
        virtualKeyToQtKey[KEY_KP8] = AntKey_KP_8;
        virtualKeyToQtKey[KEY_KP9] = AntKey_KP_9;
        virtualKeyToQtKey[KEY_CALC] = Qt::Key_Launch1;
    }
}

void QtUInputKeyMapper::populateCharKeyInformation()
{
    virtualkeyToCharKeyInformation.clear();

    unsigned int unicodeTempValue = 0;
    unsigned int listIndex = 0;

    charKeyInformation temp;
    temp.modifiers = Qt::NoModifier;
    temp.virtualkey = 0;

    // Map 0-9 keys
    for (unsigned int i=QChar('1').unicode(); i <= QChar('9').unicode(); i++)
    {
        temp.virtualkey = KEY_1 + i;
        virtualkeyToCharKeyInformation.insert(i, temp);
    }

    temp.virtualkey = KEY_0;
    virtualkeyToCharKeyInformation.insert(QChar('0').unicode(), temp);

    temp.virtualkey = KEY_MINUS;
    virtualkeyToCharKeyInformation.insert(QChar('-').unicode(), temp);

    temp.virtualkey = KEY_EQUAL;
    virtualkeyToCharKeyInformation.insert(QChar('=').unicode(), temp);

    QList<unsigned int> tempKeys;
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
    QListIterator<unsigned int> tempIter(tempKeys);
    while (tempIter.hasNext())
    {
        temp.virtualkey = tempIter.next();
        virtualkeyToCharKeyInformation.insert(unicodeTempValue, temp);
        unicodeTempValue++;
    }

    tempIter.toFront();

    temp.modifiers = Qt::ShiftModifier;
    unicodeTempValue = QChar('A').unicode();
    while (tempIter.hasNext())
    {
        temp.virtualkey = tempIter.next();
        virtualkeyToCharKeyInformation.insert(unicodeTempValue, temp);
        unicodeTempValue++;
    }

    tempKeys.clear();

    temp.modifiers = Qt::ShiftModifier;
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

    tempIter = QListIterator<unsigned int>(tempKeys);
    listIndex = 0;
    while (tempIter.hasNext())
    {
        unicodeTempValue = tempIter.next();
        temp.virtualkey = KEY_1 + listIndex;
        virtualkeyToCharKeyInformation.insert(unicodeTempValue, temp);
        listIndex++;
    }

    tempKeys.clear();

    temp.modifiers = Qt::NoModifier;
    temp.virtualkey = KEY_SPACE;
    unicodeTempValue = QChar(' ').unicode();
    virtualkeyToCharKeyInformation.insert(unicodeTempValue, temp);

    temp.virtualkey = KEY_LEFTBRACE;
    unicodeTempValue = QChar('[').unicode();
    virtualkeyToCharKeyInformation.insert(unicodeTempValue, temp);

    temp.virtualkey = KEY_RIGHTBRACE;
    unicodeTempValue = QChar(']').unicode();
    virtualkeyToCharKeyInformation.insert(unicodeTempValue, temp);

    temp.virtualkey = KEY_BACKSLASH;
    unicodeTempValue = QChar('\\').unicode();
    virtualkeyToCharKeyInformation.insert(unicodeTempValue, temp);

    temp.virtualkey = KEY_SEMICOLON;
    unicodeTempValue = QChar(';').unicode();
    virtualkeyToCharKeyInformation.insert(unicodeTempValue, temp);

    temp.virtualkey = KEY_APOSTROPHE;
    unicodeTempValue = QChar('\'').unicode();
    virtualkeyToCharKeyInformation.insert(unicodeTempValue, temp);

    temp.virtualkey = KEY_COMMA;
    unicodeTempValue = QChar(',').unicode();
    virtualkeyToCharKeyInformation.insert(unicodeTempValue, temp);

    temp.virtualkey = KEY_DOT;
    unicodeTempValue = QChar('.').unicode();
    virtualkeyToCharKeyInformation.insert(unicodeTempValue, temp);

    temp.virtualkey = KEY_SLASH;
    unicodeTempValue = QChar('/').unicode();
    virtualkeyToCharKeyInformation.insert(unicodeTempValue, temp);


    temp.modifiers = Qt::ShiftModifier;

    temp.virtualkey = KEY_LEFTBRACE;
    unicodeTempValue = QChar('{').unicode();
    virtualkeyToCharKeyInformation.insert(unicodeTempValue, temp);

    temp.virtualkey = KEY_RIGHTBRACE;
    unicodeTempValue = QChar('}').unicode();
    virtualkeyToCharKeyInformation.insert(unicodeTempValue, temp);

    temp.virtualkey = KEY_BACKSLASH;
    unicodeTempValue = QChar('|').unicode();
    virtualkeyToCharKeyInformation.insert(unicodeTempValue, temp);

    temp.virtualkey = KEY_SEMICOLON;
    unicodeTempValue = QChar(':').unicode();
    virtualkeyToCharKeyInformation.insert(unicodeTempValue, temp);

    temp.virtualkey = KEY_APOSTROPHE;
    unicodeTempValue = QChar('"').unicode();
    virtualkeyToCharKeyInformation.insert(unicodeTempValue, temp);

    temp.virtualkey = KEY_COMMA;
    unicodeTempValue = QChar('<').unicode();
    virtualkeyToCharKeyInformation.insert(unicodeTempValue, temp);

    temp.virtualkey = KEY_DOT;
    unicodeTempValue = QChar('>').unicode();
    virtualkeyToCharKeyInformation.insert(unicodeTempValue, temp);

    temp.virtualkey = KEY_SLASH;
    unicodeTempValue = QChar('?').unicode();
    virtualkeyToCharKeyInformation.insert(unicodeTempValue, temp);
}
