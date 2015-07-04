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

#include <qt_windows.h>
#include <cmath>

#include "winsendinputeventhandler.h"
#include <winextras.h>
#include <antkeymapper.h>

WinSendInputEventHandler::WinSendInputEventHandler(QObject *parent) :
    BaseEventHandler(parent)
{
}

bool WinSendInputEventHandler::init()
{
    return true;
}

bool WinSendInputEventHandler::cleanup()
{
    return true;
}

void WinSendInputEventHandler::sendKeyboardEvent(JoyButtonSlot *slot, bool pressed)
{
    int code = slot->getSlotCode();
    INPUT temp[1] = {};

    unsigned int scancode = WinExtras::scancodeFromVirtualKey(code, slot->getSlotCodeAlias());
    int extended = (scancode & WinExtras::EXTENDED_FLAG) != 0;
    int tempflags = extended ? KEYEVENTF_EXTENDEDKEY : 0;

    temp[0].type = INPUT_KEYBOARD;
    //temp[0].ki.wScan = MapVirtualKey(code, MAPVK_VK_TO_VSC);
    temp[0].ki.wScan = scancode;
    temp[0].ki.time = 0;
    temp[0].ki.dwExtraInfo = 0;

    temp[0].ki.wVk = code;
    temp[0].ki.dwFlags = pressed ? tempflags : (tempflags | KEYEVENTF_KEYUP); // 0 for key press
    SendInput(1, temp, sizeof(INPUT));
}

void WinSendInputEventHandler::sendMouseButtonEvent(JoyButtonSlot *slot, bool pressed)
{
    int code = slot->getSlotCode();
    INPUT temp[1] = {};

    temp[0].type = INPUT_MOUSE;
    if (code == 1)
    {
        temp[0].mi.dwFlags = pressed ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
    }
    else if (code == 2)
    {
        temp[0].mi.dwFlags = pressed ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
    }
    else if (code == 3)
    {
        temp[0].mi.dwFlags = pressed ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
    }
    else if (code == 4)
    {
        temp[0].mi.dwFlags = MOUSEEVENTF_WHEEL;
        temp[0].mi.mouseData = pressed ? WHEEL_DELTA : 0;
    }
    else if (code == 5)
    {
        temp[0].mi.dwFlags = MOUSEEVENTF_WHEEL;
        temp[0].mi.mouseData = pressed ? -WHEEL_DELTA : 0;
    }
    else if (code == 6)
    {
        temp[0].mi.dwFlags = 0x01000;
        temp[0].mi.mouseData = pressed ? -WHEEL_DELTA : 0;
    }
    else if (code == 7)
    {
        temp[0].mi.dwFlags = 0x01000;
        temp[0].mi.mouseData = pressed ? WHEEL_DELTA : 0;
    }
    else if (code == 8)
    {
        temp[0].mi.dwFlags = pressed ? MOUSEEVENTF_XDOWN : MOUSEEVENTF_XUP;
        temp[0].mi.mouseData = XBUTTON1;
    }
    else if (code == 9)
    {
        temp[0].mi.dwFlags = pressed ? MOUSEEVENTF_XDOWN : MOUSEEVENTF_XUP;
        temp[0].mi.mouseData = XBUTTON2;
    }

    SendInput(1, temp, sizeof(INPUT));
}

void WinSendInputEventHandler::sendMouseEvent(int xDis, int yDis)
{
    INPUT temp[1] = {};
    temp[0].type = INPUT_MOUSE;
    temp[0].mi.mouseData = 0;
    temp[0].mi.dwFlags   =  MOUSEEVENTF_MOVE;
    temp[0].mi.dx = xDis;
    temp[0].mi.dy = yDis;
    SendInput(1, temp, sizeof(INPUT));
}

QString WinSendInputEventHandler::getName()
{
    return QString("SendInput");
}

QString WinSendInputEventHandler::getIdentifier()
{
    return QString("sendinput");
}

void WinSendInputEventHandler::sendMouseSpringEvent(unsigned int xDis, unsigned int yDis,
                                                    unsigned int width, unsigned int height)
{
    if (width > 0 && height > 0)
    {
        INPUT temp[1] = {};
        temp[0].type = INPUT_MOUSE;
        temp[0].mi.mouseData = 0;
        temp[0].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;

        int fx = ceil(xDis * (65535.0/static_cast<double>(width)));
        int fy = ceil(yDis * (65535.0/static_cast<double>(height)));
        temp[0].mi.dx = fx;
        temp[0].mi.dy = fy;
        SendInput(1, temp, sizeof(INPUT));
    }
}

void WinSendInputEventHandler::sendTextEntryEvent(QString maintext)
{
    AntKeyMapper *mapper = AntKeyMapper::getInstance();

    if (mapper && mapper->getKeyMapper())
    {
        QtWinKeyMapper *keymapper = static_cast<QtWinKeyMapper*>(mapper->getKeyMapper());

        for (int i=0; i < maintext.size(); i++)
        {
            QtWinKeyMapper::charKeyInformation temp = keymapper->getCharKeyInformation(maintext.at(i));
            QList<unsigned int> tempList;

            if (temp.modifiers != Qt::NoModifier)
            {
                if (temp.modifiers.testFlag(Qt::ShiftModifier))
                {
                    tempList.append(VK_LSHIFT);
                }

                if (temp.modifiers.testFlag(Qt::ControlModifier))
                {
                    tempList.append(VK_LCONTROL);
                }

                if (temp.modifiers.testFlag(Qt::AltModifier))
                {
                    tempList.append(VK_LMENU);
                }

                if (temp.modifiers.testFlag(Qt::MetaModifier))
                {
                    tempList.append(VK_LWIN);
                }
            }

            tempList.append(temp.virtualkey);

            if (tempList.size() > 0)
            {
                INPUT tempBuffer[tempList.size()] = {0};

                QListIterator<unsigned int> tempiter(tempList);
                unsigned int j = 0;
                while (tempiter.hasNext())
                {
                    unsigned int tempcode = tempiter.next();
                    unsigned int scancode = WinExtras::scancodeFromVirtualKey(tempcode);
                    int extended = (scancode & WinExtras::EXTENDED_FLAG) != 0;
                    int tempflags = extended ? KEYEVENTF_EXTENDEDKEY : 0;

                    tempBuffer[j].type = INPUT_KEYBOARD;
                    tempBuffer[j].ki.wScan = scancode;
                    tempBuffer[j].ki.time = 0;
                    tempBuffer[j].ki.dwExtraInfo = 0;

                    tempBuffer[j].ki.wVk = tempcode;
                    tempBuffer[j].ki.dwFlags = tempflags;
                    j++;
                }

                SendInput(j, tempBuffer, sizeof(INPUT));

                tempiter.toBack();
                j = 0;
                memset(tempBuffer, 0, sizeof(tempBuffer));
                //INPUT tempBuffer2[tempList.size()] = {0};
                while (tempiter.hasPrevious())
                {
                    unsigned int tempcode = tempiter.previous();
                    unsigned int scancode = WinExtras::scancodeFromVirtualKey(tempcode);
                    int extended = (scancode & WinExtras::EXTENDED_FLAG) != 0;
                    int tempflags = extended ? KEYEVENTF_EXTENDEDKEY : 0;

                    tempBuffer[j].type = INPUT_KEYBOARD;
                    tempBuffer[j].ki.wScan = scancode;
                    tempBuffer[j].ki.time = 0;
                    tempBuffer[j].ki.dwExtraInfo = 0;

                    tempBuffer[j].ki.wVk = tempcode;
                    tempBuffer[j].ki.dwFlags = tempflags | KEYEVENTF_KEYUP;
                    j++;
                }

                SendInput(j, tempBuffer, sizeof(INPUT));
            }
        }
    }
}
