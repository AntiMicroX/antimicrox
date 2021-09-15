/* antimicrox Gamepad to KB+M event mapper
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

#include <qt_windows.h>
//#include <QDebug>
#include <QHashIterator>

#include "qtwinkeymapper.h"
#include "winextras.h"

static QHash<unsigned int, unsigned int> initDynamicKeyMapping()
{
    QHash<unsigned int, unsigned int> temp;
    temp[VK_OEM_1] = 0;
    // temp[VK_OEM_PLUS] = 0;
    // temp[VK_OEM_COMMA] = 0;
    // temp[VK_OEM_MINUS] = 0;
    // temp[VK_OEM_PERIOD] = 0;
    temp[VK_OEM_2] = 0;
    temp[VK_OEM_3] = 0;
    temp[VK_OEM_4] = 0;
    temp[VK_OEM_5] = 0;
    temp[VK_OEM_6] = 0;
    temp[VK_OEM_7] = 0;
    temp[VK_OEM_8] = 0;
    temp[VK_OEM_102] = 0;

    return temp;
}

static QHash<QString, unsigned int> intCharToQtKey()
{
    QHash<QString, unsigned int> temp;
    temp.insert(QString('!'), Qt::Key_Exclam);
    temp.insert(QString('"'), Qt::Key_QuoteDbl);
    temp.insert(QString('#'), Qt::Key_NumberSign);
    temp.insert(QString('$'), Qt::Key_Dollar);
    temp.insert(QString('\''), Qt::Key_Apostrophe);
    temp.insert(QString('('), Qt::Key_ParenLeft);
    temp.insert(QString(')'), Qt::Key_ParenRight);
    temp.insert(QString('*'), Qt::Key_Asterisk);
    temp.insert(QString('+'), Qt::Key_Plus);
    temp.insert(QString(','), Qt::Key_Comma);
    temp.insert(QString('-'), Qt::Key_Minus);
    temp.insert(QString('.'), Qt::Key_Period);
    temp.insert(QString('/'), Qt::Key_Slash);
    temp.insert(QString(':'), Qt::Key_Colon);
    temp.insert(QString(';'), Qt::Key_Semicolon);
    temp.insert(QString('<'), Qt::Key_Less);
    temp.insert(QString('='), Qt::Key_Equal);
    temp.insert(QString('>'), Qt::Key_Greater);
    temp.insert(QString('@'), Qt::Key_At);
    temp.insert(QString('['), Qt::Key_BracketLeft);
    temp.insert(QString('\\'), Qt::Key_Backslash);
    temp.insert(QString(']'), Qt::Key_BracketRight);
    temp.insert(QString('^'), Qt::Key_AsciiCircum);
    temp.insert(QString('_'), Qt::Key_Underscore);
    temp.insert(QString('`'), Qt::Key_QuoteLeft);
    temp.insert(QString('{'), Qt::Key_BraceLeft);
    temp.insert(QString('}'), Qt::Key_BraceRight);
    temp.insert(QString::fromUtf8("\u00A1"), Qt::Key_exclamdown);
    temp.insert(QString('~'), Qt::Key_AsciiTilde);
    // temp.insert(QString::fromUtf8("\u20A0"), Qt::Key_)

    return temp;
}

static QHash<QString, unsigned int> initDeadKeyToQtKey()
{
    QHash<QString, unsigned int> temp;
    // temp.insert(QString('`'), Qt::Key_Dead_Grave);
    // temp.insert(QString('\''), Qt::Key_Dead_Acute);
    temp.insert(QString::fromUtf8("\u00B4"), Qt::Key_Dead_Grave);
    // temp.insert(QString('^'), Qt::Key_Dead_Circumflex);
    // temp.insert(QString('~'), Qt::Key_Dead_Tilde);
    temp.insert(QString::fromUtf8("\u02DC"), Qt::Key_Dead_Tilde);
    temp.insert(QString::fromUtf8("\u00AF"), Qt::Key_Dead_Macron);
    temp.insert(QString::fromUtf8("\u02D8"), Qt::Key_Dead_Breve);
    temp.insert(QString::fromUtf8("\u02D9"), Qt::Key_Dead_Abovedot);
    // temp.insert(QString('"'), Qt::Key_Dead_Diaeresis);
    temp.insert(QString::fromUtf8("\u00A8"), Qt::Key_Dead_Diaeresis);
    temp.insert(QString::fromUtf8("\u02DA"), Qt::Key_Dead_Abovering);
    temp.insert(QString::fromUtf8("\u02DD"), Qt::Key_Dead_Doubleacute);
    temp.insert(QString::fromUtf8("\u02C7"), Qt::Key_Dead_Caron);
    // temp.insert(QString(','), Qt::Key_Dead_Cedilla);
    temp.insert(QString::fromUtf8("\u00B8"), Qt::Key_Dead_Cedilla);
    temp.insert(QString::fromUtf8("\u02DB"), Qt::Key_Dead_Ogonek);
    temp.insert(QString::fromUtf8("\u037A"), Qt::Key_Dead_Iota);
    temp.insert(QString::fromUtf8("\u309B"), Qt::Key_Dead_Voiced_Sound);
    temp.insert(QString::fromUtf8("\u309C"), Qt::Key_Dead_Semivoiced_Sound);

    return temp;
}

static QHash<unsigned int, unsigned int> dynamicOEMToQtKeyHash = initDynamicKeyMapping();
static QHash<QString, unsigned int> charToQtKeyHash = intCharToQtKey();
static QHash<QString, unsigned int> deadKeyToQtKeyHash = initDeadKeyToQtKey();

QtWinKeyMapper::QtWinKeyMapper(QObject *parent)
    : QtKeyMapperBase(parent)
{
    identifier = "sendinput";
    populateMappingHashes();
    populateCharKeyInformation();
}

void QtWinKeyMapper::populateMappingHashes()
{
    if (qtKeyToVirtKeyHash.isEmpty())
    {
        qtKeyToVirtKeyHash[Qt::Key_Cancel] = VK_CANCEL;
        qtKeyToVirtKeyHash[Qt::Key_Backspace] = VK_BACK;
        qtKeyToVirtKeyHash[Qt::Key_Tab] = VK_TAB;
        qtKeyToVirtKeyHash[Qt::Key_Clear] = VK_CLEAR;
        qtKeyToVirtKeyHash[Qt::Key_Return] = VK_RETURN;
        qtKeyToVirtKeyHash[Qt::Key_Enter] = VK_RETURN;
        // qtKeyToWinVirtualKey[Qt::Key_Shift] = VK_SHIFT;
        // qtKeyToWinVirtualKey[Qt::Key_Control] = VK_CONTROL;
        // qtKeyToWinVirtualKey[Qt::Key_Alt] = VK_MENU;
        qtKeyToVirtKeyHash[Qt::Key_Pause] = VK_PAUSE;
        qtKeyToVirtKeyHash[Qt::Key_CapsLock] = VK_CAPITAL;
        qtKeyToVirtKeyHash[Qt::Key_Escape] = VK_ESCAPE;
        qtKeyToVirtKeyHash[Qt::Key_Mode_switch] = VK_MODECHANGE;
        qtKeyToVirtKeyHash[Qt::Key_Space] = VK_SPACE;
        qtKeyToVirtKeyHash[Qt::Key_PageUp] = VK_PRIOR;
        qtKeyToVirtKeyHash[Qt::Key_PageDown] = VK_NEXT;
        qtKeyToVirtKeyHash[Qt::Key_End] = VK_END;
        qtKeyToVirtKeyHash[Qt::Key_Home] = VK_HOME;
        qtKeyToVirtKeyHash[Qt::Key_Left] = VK_LEFT;
        qtKeyToVirtKeyHash[Qt::Key_Up] = VK_UP;
        qtKeyToVirtKeyHash[Qt::Key_Right] = VK_RIGHT;
        qtKeyToVirtKeyHash[Qt::Key_Down] = VK_DOWN;
        qtKeyToVirtKeyHash[Qt::Key_Select] = VK_SELECT;
        qtKeyToVirtKeyHash[Qt::Key_Printer] = VK_PRINT;
        qtKeyToVirtKeyHash[Qt::Key_Execute] = VK_EXECUTE;
        qtKeyToVirtKeyHash[Qt::Key_Print] = VK_SNAPSHOT;
        qtKeyToVirtKeyHash[Qt::Key_Insert] = VK_INSERT;
        qtKeyToVirtKeyHash[Qt::Key_Delete] = VK_DELETE;
        qtKeyToVirtKeyHash[Qt::Key_Help] = VK_HELP;
        qtKeyToVirtKeyHash[Qt::Key_Meta] = VK_LWIN;
        // qtKeyToWinVirtualKey[Qt::Key_Meta] = VK_RWIN;
        qtKeyToVirtKeyHash[Qt::Key_Menu] = VK_APPS;
        qtKeyToVirtKeyHash[Qt::Key_Sleep] = VK_SLEEP;

        qtKeyToVirtKeyHash[AntKey_KP_Multiply] = VK_MULTIPLY;
        // qtKeyToVirtKeyHash[Qt::Key_Asterisk] = VK_MULTIPLY;
        qtKeyToVirtKeyHash[AntKey_KP_Add] = VK_ADD;
        // qtKeyToVirtKeyHash[Qt::Key_Comma] = VK_SEPARATOR;
        qtKeyToVirtKeyHash[AntKey_KP_Subtract] = VK_SUBTRACT;
        qtKeyToVirtKeyHash[AntKey_KP_Decimal] = VK_DECIMAL;
        qtKeyToVirtKeyHash[AntKey_KP_Divide] = VK_DIVIDE;

        qtKeyToVirtKeyHash[Qt::Key_NumLock] = VK_NUMLOCK;
        qtKeyToVirtKeyHash[Qt::Key_ScrollLock] = VK_SCROLL;
        qtKeyToVirtKeyHash[Qt::Key_Massyo] = VK_OEM_FJ_MASSHOU;
        qtKeyToVirtKeyHash[Qt::Key_Touroku] = VK_OEM_FJ_TOUROKU;

        qtKeyToVirtKeyHash[Qt::Key_Shift] = VK_LSHIFT;
        // qtKeyToWinVirtualKey[Qt::Key_Shift] = VK_RSHIFT;
        qtKeyToVirtKeyHash[Qt::Key_Control] = VK_LCONTROL;
        // qtKeyToWinVirtualKey[Qt::Key_Control] = VK_RCONTROL;
        qtKeyToVirtKeyHash[Qt::Key_Alt] = VK_LMENU;
        // qtKeyToWinVirtualKey[Qt::Key_Alt] = VK_RMENU;
        qtKeyToVirtKeyHash[Qt::Key_Back] = VK_BROWSER_BACK;
        qtKeyToVirtKeyHash[Qt::Key_Forward] = VK_BROWSER_FORWARD;
        qtKeyToVirtKeyHash[Qt::Key_Refresh] = VK_BROWSER_REFRESH;
        qtKeyToVirtKeyHash[Qt::Key_Stop] = VK_BROWSER_STOP;
        qtKeyToVirtKeyHash[Qt::Key_Search] = VK_BROWSER_SEARCH;
        qtKeyToVirtKeyHash[Qt::Key_Favorites] = VK_BROWSER_FAVORITES;
        qtKeyToVirtKeyHash[Qt::Key_HomePage] = VK_BROWSER_HOME;
        qtKeyToVirtKeyHash[Qt::Key_VolumeMute] = VK_VOLUME_MUTE;
        qtKeyToVirtKeyHash[Qt::Key_VolumeDown] = VK_VOLUME_DOWN;
        qtKeyToVirtKeyHash[Qt::Key_VolumeUp] = VK_VOLUME_UP;
        qtKeyToVirtKeyHash[Qt::Key_MediaNext] = VK_MEDIA_NEXT_TRACK;
        qtKeyToVirtKeyHash[Qt::Key_MediaPrevious] = VK_MEDIA_PREV_TRACK;
        qtKeyToVirtKeyHash[Qt::Key_MediaStop] = VK_MEDIA_STOP;
        qtKeyToVirtKeyHash[Qt::Key_MediaPlay] = VK_MEDIA_PLAY_PAUSE;
        qtKeyToVirtKeyHash[Qt::Key_LaunchMail] = VK_LAUNCH_MAIL;
        qtKeyToVirtKeyHash[Qt::Key_LaunchMedia] = VK_LAUNCH_MEDIA_SELECT;
        qtKeyToVirtKeyHash[Qt::Key_Launch0] = VK_LAUNCH_APP1;
        qtKeyToVirtKeyHash[Qt::Key_Launch1] = VK_LAUNCH_APP2;
        qtKeyToVirtKeyHash[Qt::Key_Kanji] = VK_KANJI;

        // The following VK_OEM_* keys are consistent across all
        // keyboard layouts.
        qtKeyToVirtKeyHash[Qt::Key_Equal] = VK_OEM_PLUS;
        qtKeyToVirtKeyHash[Qt::Key_Minus] = VK_OEM_MINUS;
        qtKeyToVirtKeyHash[Qt::Key_Period] = VK_OEM_PERIOD;
        qtKeyToVirtKeyHash[Qt::Key_Comma] = VK_OEM_COMMA;
        /*qtKeyToVirtKeyHash[Qt::Key_Semicolon] = VK_OEM_1;
        qtKeyToVirtKeyHash[Qt::Key_Slash] = VK_OEM_2;
        qtKeyToVirtKeyHash[Qt::Key_Equal] = VK_OEM_PLUS;
        qtKeyToVirtKeyHash[Qt::Key_Minus] = VK_OEM_MINUS;
        qtKeyToVirtKeyHash[Qt::Key_Period]  = VK_OEM_PERIOD;
        qtKeyToVirtKeyHash[Qt::Key_QuoteLeft] = VK_OEM_3;
        qtKeyToVirtKeyHash[Qt::Key_BracketLeft] = VK_OEM_4;
        qtKeyToVirtKeyHash[Qt::Key_Backslash] = VK_OEM_5;
        qtKeyToVirtKeyHash[Qt::Key_BracketRight] = VK_OEM_6;
        qtKeyToVirtKeyHash[Qt::Key_Apostrophe] = VK_OEM_7;*/

        qtKeyToVirtKeyHash[Qt::Key_Play] = VK_PLAY;
        qtKeyToVirtKeyHash[Qt::Key_Zoom] = VK_ZOOM;
        // qtKeyToWinVirtualKey[Qt::Key_Clear] = VK_OEM_CLEAR;

        // Map 0-9 ASCII codes
        for (int i = 0; i <= (0x39 - 0x30); i++)
        {
            qtKeyToVirtKeyHash[Qt::Key_0 + i] = 0x30 + i;
        }

        // Map A-Z ASCII codes
        for (int i = 0; i <= (0x5a - 0x41); i++)
        {
            qtKeyToVirtKeyHash[Qt::Key_A + i] = 0x41 + i;
        }

        // Map function keys
        for (int i = 0; i <= (VK_F24 - VK_F1); i++)
        {
            qtKeyToVirtKeyHash[Qt::Key_F1 + i] = VK_F1 + i;
        }

        // Map numpad keys
        for (int i = 0; i <= (VK_NUMPAD9 - VK_NUMPAD0); i++)
        {
            qtKeyToVirtKeyHash[AntKey_KP_0 + i] = VK_NUMPAD0 + i;
        }

        // Map custom keys
        qtKeyToVirtKeyHash[AntKey_Alt_R] = VK_RMENU;
        qtKeyToVirtKeyHash[AntKey_Meta_R] = VK_RWIN;
        qtKeyToVirtKeyHash[AntKey_Shift_R] = VK_RSHIFT;
        qtKeyToVirtKeyHash[AntKey_Control_R] = VK_RCONTROL;

        // Go through VK_OEM_* values and find the appropriate association
        // with a key defined in Qt. Association is decided based on char
        // returned from Windows for the VK_OEM_* key.
        QHashIterator<unsigned int, unsigned int> iterDynamic(dynamicOEMToQtKeyHash);
        while (iterDynamic.hasNext())
        {
            iterDynamic.next();

            byte ks[256];
            char cbuf[2] = {'\0', '\0'};
            GetKeyboardState(ks);
            unsigned int oemkey = iterDynamic.key();
            unsigned int scancode = MapVirtualKey(oemkey, 0);
            int charlength = ToAscii(oemkey, scancode, ks, (WORD *)cbuf, 0);
            if (charlength < 0)
            {
                charlength = ToAscii(VK_SPACE, scancode, ks, (WORD *)cbuf, 0);
                QString temp = QString::fromUtf8(cbuf);
                if (temp.length() > 0)
                {
                    QHashIterator<QString, unsigned int> tempiter(charToQtKeyHash);
                    while (tempiter.hasNext())
                    {
                        tempiter.next();
                        QString currentChar = tempiter.key();
                        if (currentChar == temp)
                        {
                            dynamicOEMToQtKeyHash[oemkey] = tempiter.value();
                            tempiter.toBack();
                        }
                    }
                }
            } else if (charlength == 1)
            {
                QString temp = QString::fromUtf8(cbuf);
                QHashIterator<QString, unsigned int> tempiter(charToQtKeyHash);
                while (tempiter.hasNext())
                {
                    tempiter.next();
                    QString currentChar = tempiter.key();
                    if (currentChar == temp)
                    {
                        dynamicOEMToQtKeyHash[oemkey] = tempiter.value();
                        tempiter.toBack();
                    }
                }
            }
        }

        // Populate hash with values found for the VK_OEM_* keys.
        // Values will likely be different across various keyboard
        // layouts.
        iterDynamic = QHashIterator<unsigned int, unsigned int>(dynamicOEMToQtKeyHash);
        while (iterDynamic.hasNext())
        {
            iterDynamic.next();
            unsigned int tempvalue = iterDynamic.value();
            if (tempvalue != 0 && !qtKeyToVirtKeyHash.contains(tempvalue))
            {
                qtKeyToVirtKeyHash.insert(tempvalue, iterDynamic.key());
            }
        }

        // Populate other hash. Flip key and value so mapping
        // goes VK -> Qt Key.
        QHashIterator<int, int> iter(qtKeyToVirtKeyHash);
        while (iter.hasNext())
        {
            iter.next();
            virtKeyToQtKeyHash[iter.value()] = iter.key();
        }

        // Override current item for VK_RETURN
        virtKeyToQtKeyHash[VK_RETURN] = Qt::Key_Return;

        // Insert more aliases that would have resulted in
        // overwrites in other hash.
        virtKeyToQtKeyHash[VK_SHIFT] = Qt::Key_Shift;
        virtKeyToQtKeyHash[VK_CONTROL] = Qt::Key_Control;
        virtKeyToQtKeyHash[VK_MENU] = Qt::Key_Alt;
    }
}

int QtWinKeyMapper::returnQtKey(int key, int scancode)
{
    int tempkey = virtKeyToQtKeyHash.value(key);
    int extended = scancode & WinExtras::EXTENDED_FLAG;
    if (key == VK_RETURN && extended)
    {
        tempkey = Qt::Key_Enter;
    }

    return tempkey;
}

void QtWinKeyMapper::populateCharKeyInformation()
{
    virtkeyToCharKeyInfo.clear();

    unsigned int total = 0;
    // BYTE ks[256];
    // GetKeyboardState(ks);
    /*for (int x=0; x <= 255; x++)
    {
        if (ks[x] != 0)
        {
            qDebug() << "TEST: " << QString::number(x)
                     << " | " << QString::number(ks[x]);
        }
    }
    */

    for (int i = VK_SPACE; i <= VK_OEM_CLEAR; i++)
    {
        unsigned int scancode = MapVirtualKey(i, 0);

        for (int j = 0; j <= 3; j++)
        {
            WCHAR cbuf[256];
            BYTE tempks[256];
            memset(tempks, 0, sizeof(tempks));

            Qt::KeyboardModifiers dicis;
            if (j >= 2)
            {
                dicis |= Qt::MetaModifier;
                tempks[VK_LWIN] = 1 << 7;
                // tempks[VK_RWIN] = 1 << 7;
            }

            if (j == 1 || j == 3)
            {
                dicis |= Qt::ShiftModifier;
                tempks[VK_LSHIFT] = 1 << 7;
                tempks[VK_SHIFT] = 1 << 7;
                // qDebug() << "NEVER ME: ";
            }

            int charlength = ToUnicode(i, scancode, tempks, cbuf, 255, 0);
            if (charlength == 1 || charlength < 0)
            {
                QString temp = QString::fromWCharArray(cbuf);
                if (temp.size() > 0)
                {
                    QChar tempchar(temp.at(0));
                    charKeyInformation tempinfo;
                    tempinfo.modifiers = dicis;
                    tempinfo.virtualkey = i;
                    if (!virtkeyToCharKeyInfo.contains(tempchar.unicode()))
                    {
                        virtkeyToCharKeyInfo.insert(tempchar.unicode(), tempinfo);
                        total++;
                    }
                }
            }
        }
    }

    qDebug() << "TOTAL: " << total;
}
