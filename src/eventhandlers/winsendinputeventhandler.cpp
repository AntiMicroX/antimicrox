#include <qt_windows.h>

#include "winsendinputeventhandler.h"
#include "winextras.h"

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
