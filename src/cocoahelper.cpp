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

#include <QCoreApplication>

#include "cocoahelper.h"
#include <Carbon/Carbon.h>

struct T3{
    const unsigned int code;
    const QString displaystring;
    const QString codestring;
};

struct T2{
    const unsigned int qtcode;
    const unsigned int cocoacode;
};

static const T2 qt_cocoa_map[] = {
    {Qt::Key_1, kVK_ANSI_Keypad1},
    {Qt::Key_2, kVK_ANSI_Keypad2},
    {Qt::Key_3, kVK_ANSI_Keypad3},
    {Qt::Key_4, kVK_ANSI_Keypad4},
    {Qt::Key_5, kVK_ANSI_Keypad5},
    {Qt::Key_6, kVK_ANSI_Keypad6},
    {Qt::Key_7, kVK_ANSI_Keypad7},
    {Qt::Key_8, kVK_ANSI_Keypad8},
    {Qt::Key_9, kVK_ANSI_Keypad9},
    {Qt::Key_0, kVK_ANSI_Keypad0},
    {Qt::Key_Slash, kVK_ANSI_KeypadDivide},
    {Qt::Key_Minus, kVK_ANSI_KeypadMinus},
    {Qt::Key_Period, kVK_ANSI_KeypadDecimal},
    {Qt::Key_Shift, kVK_RightShift},
    {Qt::Key_Meta, kVK_RightControl},
    {Qt::Key_Control, kVK_Command},
    {Qt::Key_Option, kVK_RightOption},
    {Qt::Key_1, kVK_ANSI_1},
    {Qt::Key_2, kVK_ANSI_2},
    {Qt::Key_3, kVK_ANSI_3},
    {Qt::Key_4, kVK_ANSI_4},
    {Qt::Key_5, kVK_ANSI_5},
    {Qt::Key_6, kVK_ANSI_6},
    {Qt::Key_7, kVK_ANSI_7},
    {Qt::Key_8, kVK_ANSI_8},
    {Qt::Key_9, kVK_ANSI_9},
    {Qt::Key_0, kVK_ANSI_0},
    {Qt::Key_A, kVK_ANSI_A},
    {Qt::Key_B, kVK_ANSI_B},
    {Qt::Key_C, kVK_ANSI_C},
    {Qt::Key_D, kVK_ANSI_D},
    {Qt::Key_E, kVK_ANSI_E},
    {Qt::Key_F, kVK_ANSI_F},
    {Qt::Key_G, kVK_ANSI_G},
    {Qt::Key_H, kVK_ANSI_H},
    {Qt::Key_I, kVK_ANSI_I},
    {Qt::Key_J, kVK_ANSI_J},
    {Qt::Key_K, kVK_ANSI_K},
    {Qt::Key_L, kVK_ANSI_L},
    {Qt::Key_M, kVK_ANSI_M},
    {Qt::Key_N, kVK_ANSI_N},
    {Qt::Key_O, kVK_ANSI_O},
    {Qt::Key_P, kVK_ANSI_P},
    {Qt::Key_Q, kVK_ANSI_Q},
    {Qt::Key_R, kVK_ANSI_R},
    {Qt::Key_S, kVK_ANSI_S},
    {Qt::Key_T, kVK_ANSI_T},
    {Qt::Key_U, kVK_ANSI_U},
    {Qt::Key_V, kVK_ANSI_V},
    {Qt::Key_W, kVK_ANSI_W},
    {Qt::Key_X, kVK_ANSI_X},
    {Qt::Key_Y, kVK_ANSI_Y},
    {Qt::Key_Z, kVK_ANSI_Z},
    {Qt::Key_F1, kVK_F1},
    {Qt::Key_F2, kVK_F2},
    {Qt::Key_F3, kVK_F3},
    {Qt::Key_F4, kVK_F4},
    {Qt::Key_F5, kVK_F5},
    {Qt::Key_F6, kVK_F6},
    {Qt::Key_F7, kVK_F7},
    {Qt::Key_F8, kVK_F8},
    {Qt::Key_F9, kVK_F9},
    {Qt::Key_F10, kVK_F10},
    {Qt::Key_F11, kVK_F11},
    {Qt::Key_F12, kVK_F12},
    {Qt::Key_F14, kVK_F14},
    {Qt::Key_F15, kVK_F15},
    {Qt::Key_F16, kVK_F16},
    {Qt::Key_Escape, kVK_Escape},
    {Qt::Key_Dead_Grave, kVK_ANSI_Grave},
    {Qt::Key_Minus, kVK_ANSI_Minus},
    {Qt::Key_Equal, kVK_ANSI_Equal},
    {Qt::Key_Backspace, kVK_Delete},
    {Qt::Key_Tab, kVK_Tab},
    {Qt::Key_BracketLeft, kVK_ANSI_LeftBracket},
    {Qt::Key_BracketRight, kVK_ANSI_RightBracket},
    {Qt::Key_Backslash, kVK_ANSI_Backslash},
    {Qt::Key_CapsLock, kVK_CapsLock},
    {Qt::Key_Semicolon, kVK_ANSI_Semicolon},
    {Qt::Key_QuoteDbl, kVK_ANSI_Quote},
    {Qt::Key_Return, kVK_Return},
    {Qt::Key_Shift, kVK_Shift},
    {Qt::Key_Comma, kVK_ANSI_Comma},
    {Qt::Key_Period, kVK_ANSI_Period},
    {Qt::Key_Slash, kVK_ANSI_Slash},
    {Qt::Key_Meta, kVK_Control},
    {Qt::Key_Control, kVK_Command},
    {Qt::Key_Menu, kVK_Command},
    {Qt::Key_Option, kVK_Option},
    {Qt::Key_Space, kVK_Space},
    {Qt::Key_Up, kVK_UpArrow},
    {Qt::Key_Left, kVK_LeftArrow},
    {Qt::Key_Down, kVK_DownArrow},
    {Qt::Key_Right, kVK_RightArrow},
    {Qt::Key_Help, kVK_Help},
    {Qt::Key_Delete, kVK_ForwardDelete},
    {Qt::Key_Home, kVK_Home},
    {Qt::Key_End, kVK_End},
    {Qt::Key_PageUp, kVK_PageUp},
    {Qt::Key_PageDown, kVK_PageDown},
    {Qt::Key_Enter, kVK_ANSI_KeypadEnter},
    {Qt::Key_Clear, kVK_ANSI_KeypadClear},
    {Qt::Key_Asterisk, kVK_ANSI_KeypadMultiply},
    {Qt::Key_Plus, kVK_ANSI_KeypadPlus},
};

static const T3 keys[] = {
    {Qt::Key_1, "KP_1", "KP_1"},
    {Qt::Key_2, "KP_2", "KP_2"},
    {Qt::Key_3, "KP_3", "KP_3"},
    {Qt::Key_4, "KP_4", "KP_4"},
    {Qt::Key_5, "KP_5", "KP_5"},
    {Qt::Key_6, "KP_6", "KP_6"},
    {Qt::Key_7, "KP_7", "KP_7"},
    {Qt::Key_8, "KP_8", "KP_8"},
    {Qt::Key_9, "KP_9", "KP_9"},
    {Qt::Key_0, "KP_0", "KP_0"},
    {Qt::Key_Slash, "/", "KP_Divide"},
    {Qt::Key_Minus, "-", "KP_Subtract"},
    {Qt::Key_Period, ".", "KP_Decimal"},
    {Qt::Key_Shift, "Shift_R", "Shift_R"},
    {Qt::Key_Meta, "Ctrl_R", "Control_R"},
    {Qt::Key_Control, "Super_R", "Super_R"},
    {Qt::Key_Option, "Alt_R", "Alt_R"},
    {Qt::Key_1, "1", "1"},
    {Qt::Key_2, "2", "2"},
    {Qt::Key_3, "3", "3"},
    {Qt::Key_4, "4", "4"},
    {Qt::Key_5, "5", "5"},
    {Qt::Key_6, "6", "6"},
    {Qt::Key_7, "7", "7"},
    {Qt::Key_8, "8", "8"},
    {Qt::Key_9, "9", "9"},
    {Qt::Key_0, "0", "0"},
    {Qt::Key_A, "a", "a"},
    {Qt::Key_B, "b", "b"},
    {Qt::Key_C, "c", "c"},
    {Qt::Key_D, "d", "d"},
    {Qt::Key_E, "e", "e"},
    {Qt::Key_F, "f", "f"},
    {Qt::Key_G, "g", "g"},
    {Qt::Key_H, "h", "h"},
    {Qt::Key_I, "i", "i"},
    {Qt::Key_J, "j", "j"},
    {Qt::Key_K, "k", "k"},
    {Qt::Key_L, "l", "l"},
    {Qt::Key_M, "m", "m"},
    {Qt::Key_N, "n", "n"},
    {Qt::Key_O, "o", "o"},
    {Qt::Key_P, "p", "p"},
    {Qt::Key_Q, "q", "q"},
    {Qt::Key_R, "r", "r"},
    {Qt::Key_S, "s", "s"},
    {Qt::Key_T, "t", "t"},
    {Qt::Key_U, "u", "u"},
    {Qt::Key_V, "v", "v"},
    {Qt::Key_W, "w", "w"},
    {Qt::Key_X, "x", "x"},
    {Qt::Key_Y, "y", "y"},
    {Qt::Key_Z, "z", "z"},
    {Qt::Key_F1, "F1", "F1"},
    {Qt::Key_F2, "F2", "F2"},
    {Qt::Key_F3, "F3", "F3"},
    {Qt::Key_F4, "F4", "F4"},
    {Qt::Key_F5, "F5", "F5"},
    {Qt::Key_F6, "F6", "F6"},
    {Qt::Key_F7, "F7", "F7"},
    {Qt::Key_F8, "F8", "F8"},
    {Qt::Key_F9, "F9", "F9"},
    {Qt::Key_F10, "F10", "F10"},
    {Qt::Key_F11, "F11", "F11"},
    {Qt::Key_F12, "F12", "F12"},
    {Qt::Key_F14, "PrtSc", "Print"},
    {Qt::Key_F15, "SCLK", "Scroll_Lock"},
    {Qt::Key_F16, "Pause", "Pause"},
    {Qt::Key_Escape, "Esc", "Escape"},
    {Qt::Key_Dead_Grave, "`", "grave"},
    {Qt::Key_Minus, "-", "minus"},
    {Qt::Key_Equal, "=", "equal"},
    {Qt::Key_Backspace, "BackSpace", "BackSpace"},
    {Qt::Key_Tab, "Tab", "Tab"},
    {Qt::Key_BracketLeft, "[", "bracketleft"},
    {Qt::Key_BracketRight, "]", "bracketright"},
    {Qt::Key_Backslash, "\\", "backslash"},
    {Qt::Key_CapsLock, "CapsLock", "Caps_Lock"},
    {Qt::Key_Semicolon, ";", "semicolon"},
    {Qt::Key_QuoteDbl, "'", "apostrophe"},
    {Qt::Key_Return, "Enter", "Return"},
    {Qt::Key_Shift, "Shift_L", "Shift_L"},
    {Qt::Key_Comma, ",", "comma"},
    {Qt::Key_Period, ".", "period"},
    {Qt::Key_Slash, "/", "slash"},
    {Qt::Key_Meta, "Ctrl_L", "Control_L"},
    {Qt::Key_Control, "Super_L", "Super_L"},
    {Qt::Key_Menu, "Menu", "Menu"},
    {Qt::Key_Option, "Alt_L", "Alt_L"},
    {Qt::Key_Space, "Space", "space"},
    {Qt::Key_Up, "Up", "Up"},
    {Qt::Key_Left, "Left", "Left"},
    {Qt::Key_Down, "Down", "Down"},
    {Qt::Key_Right, "Right", "Right"},
    {Qt::Key_Help, "Ins", "Insert"},
    {Qt::Key_Delete, "Del", "Delete"},
    {Qt::Key_Home, "Home", "Home"},
    {Qt::Key_End, "End", "End"},
    {Qt::Key_PageUp, "PgUp", "Prior"},
    {Qt::Key_PageDown, "PgDn", "Next"},
    {Qt::Key_Enter, "KP_Enter", "KP_Enter"},
    {Qt::Key_Clear, "NumLock", "Num_Lock"},
    {Qt::Key_Asterisk, "*", "KP_Multiply"},
    {Qt::Key_Plus, "+", "KP_Add"},
};

CocoaHelper* CocoaHelper::_instance = 0;

CocoaHelper::CocoaHelper(QObject *parent) :
    QObject(parent)
{
    populateKnownAliases();
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));
}

CocoaHelper::~CocoaHelper()
{
    _instance = 0;
}

void CocoaHelper::populateKnownAliases()
{
    if (knownAliasesX11SymVK.isEmpty())
    {
        for (int i = 0; i < sizeof(keys)/sizeof(T3); i++){
            knownAliasesX11SymVK.insert(keys[i].codestring, keys[i].code);
        }
    }

    if (knownAliasesVKStrings.isEmpty())
    {
        for (int i = 0; i < sizeof(keys)/sizeof(T3); i++){
            knownAliasesVKStrings.insert(keys[i].code, tr(keys[i].displaystring.toStdString().c_str()));
        }
    }

    if (knownCocoaCode.isEmpty())
    {
        for (int i = 0; i < sizeof(qt_cocoa_map)/sizeof(T2); i++){
            knownCocoaCode.insert(qt_cocoa_map[i].qtcode, qt_cocoa_map[i].cocoacode);
        }
    }
}

CocoaHelper* CocoaHelper::getInstance()
{
    if (!_instance)
    {
        _instance = new CocoaHelper();
    }

    return _instance;
}

void CocoaHelper::deleteInstance()
{
    if (_instance)
    {
        delete _instance;
        _instance = 0;
    }
}

QString CocoaHelper::getDisplayString(unsigned int virtualkey)
{
    QString temp;

    if (knownAliasesVKStrings.contains(virtualkey))
    {
        temp = knownAliasesVKStrings.value(virtualkey);
    }
    else
    {
        temp = tr("[NO KEY]");
    }

    return temp;
}

unsigned int CocoaHelper::getVirtualKey(QString codestring)
{
    int temp = 0;
    if (knownAliasesX11SymVK.contains(codestring))
    {
        temp = knownAliasesX11SymVK.value(codestring);
    }

    return temp;
}

unsigned int CocoaHelper::getCocoaVirtualKey(int qtcode)
{
    unsigned int temp = -1;
    if (knownCocoaCode.contains(qtcode))
    {
        temp = knownCocoaCode.value(qtcode);
    }

    return temp;
}
