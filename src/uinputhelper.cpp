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
#include <QCoreApplication>

#include "uinputhelper.h"

UInputHelper* UInputHelper::_instance = 0;

UInputHelper::UInputHelper(QObject *parent) :
    QObject(parent)
{
    populateKnownAliases();
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));
}

UInputHelper::~UInputHelper()
{
    _instance = 0;
}

void UInputHelper::populateKnownAliases()
{
    if (knownAliasesX11SymVK.isEmpty())
    {
        knownAliasesX11SymVK.insert("a", KEY_A);
        knownAliasesX11SymVK.insert("b", KEY_B);
        knownAliasesX11SymVK.insert("c", KEY_C);
        knownAliasesX11SymVK.insert("d", KEY_D);
        knownAliasesX11SymVK.insert("e", KEY_E);
        knownAliasesX11SymVK.insert("f", KEY_F);
        knownAliasesX11SymVK.insert("g", KEY_G);
        knownAliasesX11SymVK.insert("h", KEY_H);
        knownAliasesX11SymVK.insert("i", KEY_I);
        knownAliasesX11SymVK.insert("j", KEY_J);
        knownAliasesX11SymVK.insert("k", KEY_K);
        knownAliasesX11SymVK.insert("l", KEY_L);
        knownAliasesX11SymVK.insert("m", KEY_M);
        knownAliasesX11SymVK.insert("n", KEY_N);
        knownAliasesX11SymVK.insert("o", KEY_O);
        knownAliasesX11SymVK.insert("p", KEY_P);
        knownAliasesX11SymVK.insert("q", KEY_Q);
        knownAliasesX11SymVK.insert("r", KEY_R);
        knownAliasesX11SymVK.insert("s", KEY_S);
        knownAliasesX11SymVK.insert("t", KEY_T);
        knownAliasesX11SymVK.insert("u", KEY_U);
        knownAliasesX11SymVK.insert("v", KEY_V);
        knownAliasesX11SymVK.insert("w", KEY_W);
        knownAliasesX11SymVK.insert("x", KEY_X);
        knownAliasesX11SymVK.insert("y", KEY_Y);
        knownAliasesX11SymVK.insert("z", KEY_Z);

        knownAliasesX11SymVK.insert("Escape", KEY_ESC);
        knownAliasesX11SymVK.insert("F1", KEY_F1);
        knownAliasesX11SymVK.insert("F2", KEY_F2);
        knownAliasesX11SymVK.insert("F3", KEY_F3);
        knownAliasesX11SymVK.insert("F4", KEY_F4);
        knownAliasesX11SymVK.insert("F5", KEY_F5);
        knownAliasesX11SymVK.insert("F6", KEY_F6);
        knownAliasesX11SymVK.insert("F7", KEY_F7);
        knownAliasesX11SymVK.insert("F8", KEY_F8);
        knownAliasesX11SymVK.insert("F9", KEY_F9);
        knownAliasesX11SymVK.insert("F10", KEY_F10);
        knownAliasesX11SymVK.insert("F11", KEY_F11);
        knownAliasesX11SymVK.insert("F12", KEY_F12);

        knownAliasesX11SymVK.insert("grave", KEY_GRAVE);
        knownAliasesX11SymVK.insert("1", KEY_1);
        knownAliasesX11SymVK.insert("2", KEY_2);
        knownAliasesX11SymVK.insert("3", KEY_3);
        knownAliasesX11SymVK.insert("4", KEY_4);
        knownAliasesX11SymVK.insert("5", KEY_5);
        knownAliasesX11SymVK.insert("6", KEY_6);
        knownAliasesX11SymVK.insert("7", KEY_7);
        knownAliasesX11SymVK.insert("8", KEY_8);
        knownAliasesX11SymVK.insert("9", KEY_9);
        knownAliasesX11SymVK.insert("0", KEY_0);
        knownAliasesX11SymVK.insert("minus", KEY_MINUS);
        knownAliasesX11SymVK.insert("equal", KEY_EQUAL);
        knownAliasesX11SymVK.insert("BackSpace", KEY_BACKSPACE);
        knownAliasesX11SymVK.insert("Tab", KEY_TAB);
        knownAliasesX11SymVK.insert("bracketleft", KEY_LEFTBRACE);
        knownAliasesX11SymVK.insert("bracketright", KEY_RIGHTBRACE);
        knownAliasesX11SymVK.insert("backslash", KEY_BACKSLASH);
        knownAliasesX11SymVK.insert("Caps_Lock", KEY_CAPSLOCK);
        knownAliasesX11SymVK.insert("semicolon", KEY_SEMICOLON);
        knownAliasesX11SymVK.insert("apostrophe", KEY_APOSTROPHE);
        knownAliasesX11SymVK.insert("Return", KEY_ENTER);
        knownAliasesX11SymVK.insert("Shift_L", KEY_LEFTSHIFT);
        knownAliasesX11SymVK.insert("comma", KEY_COMMA);
        knownAliasesX11SymVK.insert("period", KEY_DOT);
        knownAliasesX11SymVK.insert("slash", KEY_SLASH);
        knownAliasesX11SymVK.insert("Control_L", KEY_LEFTCTRL);
        knownAliasesX11SymVK.insert("Super_L", KEY_MENU);
        knownAliasesX11SymVK.insert("Alt_L", KEY_LEFTALT);
        knownAliasesX11SymVK.insert("space", KEY_SPACE);
        knownAliasesX11SymVK.insert("Alt_R", KEY_RIGHTALT);
        knownAliasesX11SymVK.insert("Menu", KEY_COMPOSE);
        knownAliasesX11SymVK.insert("Control_R", KEY_RIGHTCTRL);
        knownAliasesX11SymVK.insert("Shift_R", KEY_RIGHTSHIFT);

        knownAliasesX11SymVK.insert("Up", KEY_UP);
        knownAliasesX11SymVK.insert("Left", KEY_LEFT);
        knownAliasesX11SymVK.insert("Down", KEY_DOWN);
        knownAliasesX11SymVK.insert("Right", KEY_RIGHT);
        knownAliasesX11SymVK.insert("Print", KEY_PRINT);
        knownAliasesX11SymVK.insert("Insert", KEY_INSERT);
        knownAliasesX11SymVK.insert("Delete", KEY_DELETE);
        knownAliasesX11SymVK.insert("Home", KEY_HOME);
        knownAliasesX11SymVK.insert("End", KEY_END);
        knownAliasesX11SymVK.insert("Prior", KEY_PAGEUP);
        knownAliasesX11SymVK.insert("Next", KEY_PAGEDOWN);

        knownAliasesX11SymVK.insert("Num_Lock", KEY_NUMLOCK);
        knownAliasesX11SymVK.insert("KP_Divide", KEY_KPSLASH);
        knownAliasesX11SymVK.insert("KP_Multiply", KEY_KPASTERISK);
        knownAliasesX11SymVK.insert("KP_Subtract", KEY_KPMINUS);
        knownAliasesX11SymVK.insert("KP_Add", KEY_KPPLUS);
        knownAliasesX11SymVK.insert("KP_Enter", KEY_KPENTER);

        knownAliasesX11SymVK.insert("KP_1", KEY_KP1);
        knownAliasesX11SymVK.insert("KP_2", KEY_KP2);
        knownAliasesX11SymVK.insert("KP_3", KEY_KP3);
        knownAliasesX11SymVK.insert("KP_4", KEY_KP4);
        knownAliasesX11SymVK.insert("KP_5", KEY_KP5);
        knownAliasesX11SymVK.insert("KP_6", KEY_KP6);
        knownAliasesX11SymVK.insert("KP_7", KEY_KP7);
        knownAliasesX11SymVK.insert("KP_8", KEY_KP8);
        knownAliasesX11SymVK.insert("KP_9", KEY_KP9);
        knownAliasesX11SymVK.insert("KP_0", KEY_KP0);
        knownAliasesX11SymVK.insert("KP_Decimal", KEY_KPDOT);

        knownAliasesX11SymVK.insert("Scroll_Lock", KEY_SCROLLLOCK);
        knownAliasesX11SymVK.insert("Pause", KEY_PAUSE);
        knownAliasesX11SymVK.insert("Multi_key", KEY_RIGHTALT);
    }

    if (knownAliasesVKStrings.isEmpty())
    {
        knownAliasesVKStrings.insert(KEY_A, tr("a"));
        knownAliasesVKStrings.insert(KEY_B, tr("b"));
        knownAliasesVKStrings.insert(KEY_C, tr("c"));
        knownAliasesVKStrings.insert(KEY_D, tr("d"));
        knownAliasesVKStrings.insert(KEY_E, tr("e"));
        knownAliasesVKStrings.insert(KEY_F, tr("f"));
        knownAliasesVKStrings.insert(KEY_G, tr("g"));
        knownAliasesVKStrings.insert(KEY_H, tr("h"));
        knownAliasesVKStrings.insert(KEY_I, tr("i"));
        knownAliasesVKStrings.insert(KEY_J, tr("j"));
        knownAliasesVKStrings.insert(KEY_K, tr("k"));
        knownAliasesVKStrings.insert(KEY_L, tr("l"));
        knownAliasesVKStrings.insert(KEY_M, tr("m"));
        knownAliasesVKStrings.insert(KEY_N, tr("n"));
        knownAliasesVKStrings.insert(KEY_O, tr("o"));
        knownAliasesVKStrings.insert(KEY_P, tr("p"));
        knownAliasesVKStrings.insert(KEY_Q, tr("q"));
        knownAliasesVKStrings.insert(KEY_R, tr("r"));
        knownAliasesVKStrings.insert(KEY_S, tr("s"));
        knownAliasesVKStrings.insert(KEY_T, tr("t"));
        knownAliasesVKStrings.insert(KEY_U, tr("u"));
        knownAliasesVKStrings.insert(KEY_V, tr("v"));
        knownAliasesVKStrings.insert(KEY_W, tr("w"));
        knownAliasesVKStrings.insert(KEY_X, tr("x"));
        knownAliasesVKStrings.insert(KEY_Y, tr("y"));
        knownAliasesVKStrings.insert(KEY_Z, tr("z"));

        knownAliasesVKStrings.insert(KEY_ESC, tr("Esc"));
        knownAliasesVKStrings.insert(KEY_F1, tr("F1"));
        knownAliasesVKStrings.insert(KEY_F2, tr("F2"));
        knownAliasesVKStrings.insert(KEY_F3, tr("F3"));
        knownAliasesVKStrings.insert(KEY_F4, tr("F4"));
        knownAliasesVKStrings.insert(KEY_F5, tr("F5"));
        knownAliasesVKStrings.insert(KEY_F6, tr("F6"));
        knownAliasesVKStrings.insert(KEY_F7, tr("F7"));
        knownAliasesVKStrings.insert(KEY_F8, tr("F8"));
        knownAliasesVKStrings.insert(KEY_F9, tr("F9"));
        knownAliasesVKStrings.insert(KEY_F10, tr("F10"));
        knownAliasesVKStrings.insert(KEY_F11, tr("F11"));
        knownAliasesVKStrings.insert(KEY_F12, tr("F12"));

        knownAliasesVKStrings.insert(KEY_GRAVE, tr("`"));
        knownAliasesVKStrings.insert(KEY_1, tr("1"));
        knownAliasesVKStrings.insert(KEY_2, tr("2"));
        knownAliasesVKStrings.insert(KEY_3, tr("3"));
        knownAliasesVKStrings.insert(KEY_4, tr("4"));
        knownAliasesVKStrings.insert(KEY_5, tr("5"));
        knownAliasesVKStrings.insert(KEY_6, tr("6"));
        knownAliasesVKStrings.insert(KEY_7, tr("7"));
        knownAliasesVKStrings.insert(KEY_8, tr("8"));
        knownAliasesVKStrings.insert(KEY_9, tr("9"));
        knownAliasesVKStrings.insert(KEY_0, tr("0"));
        knownAliasesVKStrings.insert(KEY_MINUS, tr("-"));
        knownAliasesVKStrings.insert(KEY_EQUAL, tr("="));
        knownAliasesVKStrings.insert(KEY_BACKSPACE, tr("BackSpace"));
        knownAliasesVKStrings.insert(KEY_TAB, tr("Tab"));
        knownAliasesVKStrings.insert(KEY_LEFTBRACE, tr("["));
        knownAliasesVKStrings.insert(KEY_RIGHTBRACE, tr("]"));
        knownAliasesVKStrings.insert(KEY_BACKSLASH, tr("\\"));
        knownAliasesVKStrings.insert(KEY_CAPSLOCK, tr("CapsLock"));
        knownAliasesVKStrings.insert(KEY_SEMICOLON, tr(";"));
        knownAliasesVKStrings.insert(KEY_APOSTROPHE, tr("'"));
        knownAliasesVKStrings.insert(KEY_ENTER, tr("Enter"));
        knownAliasesVKStrings.insert(KEY_LEFTSHIFT, tr("Shift_L"));
        knownAliasesVKStrings.insert(KEY_COMMA, tr(","));
        knownAliasesVKStrings.insert(KEY_DOT, tr("."));
        knownAliasesVKStrings.insert(KEY_SLASH, tr("/"));
        knownAliasesVKStrings.insert(KEY_LEFTCTRL, tr("Ctrl_L"));
        knownAliasesVKStrings.insert(KEY_MENU, tr("Super_L"));
        knownAliasesVKStrings.insert(KEY_LEFTALT, tr("Alt_L"));
        knownAliasesVKStrings.insert(KEY_SPACE, tr("Space"));
        knownAliasesVKStrings.insert(KEY_RIGHTALT, tr("Alt_R"));
        knownAliasesVKStrings.insert(KEY_COMPOSE, tr("Menu"));
        knownAliasesVKStrings.insert(KEY_RIGHTCTRL, tr("Ctrl_R"));
        knownAliasesVKStrings.insert(KEY_RIGHTSHIFT, tr("Shift_R"));

        knownAliasesVKStrings.insert(KEY_UP, tr("Up"));
        knownAliasesVKStrings.insert(KEY_LEFT, tr("Left"));
        knownAliasesVKStrings.insert(KEY_DOWN, tr("Down"));
        knownAliasesVKStrings.insert(KEY_RIGHT, tr("Right"));
        knownAliasesVKStrings.insert(KEY_PRINT, tr("PrtSc"));
        knownAliasesVKStrings.insert(KEY_INSERT, tr("Ins"));
        knownAliasesVKStrings.insert(KEY_DELETE, tr("Del"));
        knownAliasesVKStrings.insert(KEY_HOME, tr("Home"));
        knownAliasesVKStrings.insert(KEY_END, tr("End"));
        knownAliasesVKStrings.insert(KEY_PAGEUP, tr("PgUp"));
        knownAliasesVKStrings.insert(KEY_PAGEDOWN, tr("PgDn"));

        knownAliasesVKStrings.insert(KEY_NUMLOCK, tr("NumLock"));
        knownAliasesVKStrings.insert(KEY_KPSLASH, tr("/"));
        knownAliasesVKStrings.insert(KEY_KPASTERISK, tr("*"));
        knownAliasesVKStrings.insert(KEY_KPMINUS, tr("-"));
        knownAliasesVKStrings.insert(KEY_KPPLUS, tr("+"));
        knownAliasesVKStrings.insert(KEY_KPENTER, tr("KP_Enter"));

        knownAliasesVKStrings.insert(KEY_KP1, tr("KP_1"));
        knownAliasesVKStrings.insert(KEY_KP2, tr("KP_2"));
        knownAliasesVKStrings.insert(KEY_KP3, tr("KP_3"));
        knownAliasesVKStrings.insert(KEY_KP4, tr("KP_4"));
        knownAliasesVKStrings.insert(KEY_KP5, tr("KP_5"));
        knownAliasesVKStrings.insert(KEY_KP6, tr("KP_6"));
        knownAliasesVKStrings.insert(KEY_KP7, tr("KP_7"));
        knownAliasesVKStrings.insert(KEY_KP8, tr("KP_8"));
        knownAliasesVKStrings.insert(KEY_KP9, tr("KP_9"));
        knownAliasesVKStrings.insert(KEY_KP0, tr("KP_0"));

        knownAliasesVKStrings.insert(KEY_SCROLLLOCK, tr("SCLK"));
        knownAliasesVKStrings.insert(KEY_PAUSE, tr("Pause"));
        knownAliasesVKStrings.insert(KEY_KPDOT, tr("."));
        knownAliasesVKStrings.insert(KEY_LEFTMETA, tr("Super_L"));
        knownAliasesVKStrings.insert(KEY_RIGHTMETA, tr("Super_R"));
        knownAliasesVKStrings.insert(KEY_MUTE, tr("Mute"));
        knownAliasesVKStrings.insert(KEY_VOLUMEDOWN, tr("VolDn"));
        knownAliasesVKStrings.insert(KEY_VOLUMEUP, tr("VolUp"));
        knownAliasesVKStrings.insert(KEY_PLAYPAUSE, tr("Play"));
        knownAliasesVKStrings.insert(KEY_STOPCD, tr("Stop"));
        knownAliasesVKStrings.insert(KEY_PREVIOUSSONG, tr("Prev"));
        knownAliasesVKStrings.insert(KEY_NEXTSONG, tr("Next"));
    }
}

UInputHelper* UInputHelper::getInstance()
{
    if (!_instance)
    {
        _instance = new UInputHelper();
    }

    return _instance;
}

void UInputHelper::deleteInstance()
{
    if (_instance)
    {
        delete _instance;
        _instance = 0;
    }
}

QString UInputHelper::getDisplayString(unsigned int virtualkey)
{
    QString temp;
    if (virtualkey <= 0)
    {
        temp = tr("[NO KEY]");
    }
    else if (knownAliasesVKStrings.contains(virtualkey))
    {
        temp = knownAliasesVKStrings.value(virtualkey);
    }

    return temp;
}

unsigned int UInputHelper::getVirtualKey(QString codestring)
{
    int temp = 0;
    if (knownAliasesX11SymVK.contains(codestring))
    {
        temp = knownAliasesX11SymVK.value(codestring);
    }

    return temp;
}
