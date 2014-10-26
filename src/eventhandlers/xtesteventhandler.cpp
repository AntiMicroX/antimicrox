#include "xtesteventhandler.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XTest.h>
#include <x11info.h>

XTestEventHandler::XTestEventHandler(QObject *parent) :
    BaseEventHandler(parent)
{
}

bool XTestEventHandler::init()
{
    return true;
}

bool XTestEventHandler::cleanup()
{
    return true;
}

void XTestEventHandler::sendKeyboardEvent(JoyButtonSlot *slot, bool pressed)
{
    Display* display = X11Info::getInstance()->display();
    JoyButtonSlot::JoySlotInputAction device = slot->getSlotMode();
    int code = slot->getSlotCode();

    if (device == JoyButtonSlot::JoyKeyboard)
    {
        unsigned int tempcode = XKeysymToKeycode(display, code);
        if (tempcode > 0)
        {
            XTestFakeKeyEvent(display, tempcode, pressed, 0);
            XFlush(display);
        }
    }
}

void XTestEventHandler::sendMouseButtonEvent(JoyButtonSlot *slot, bool pressed)
{
    Display* display = X11Info::getInstance()->display();
    JoyButtonSlot::JoySlotInputAction device = slot->getSlotMode();
    int code = slot->getSlotCode();

    if (device == JoyButtonSlot::JoyMouseButton)
    {
        XTestFakeButtonEvent(display, code, pressed, 0);
        XFlush(display);
    }
}

void XTestEventHandler::sendMouseEvent(int xDis, int yDis)
{
    Display* display = X11Info::getInstance()->display();
    XTestFakeRelativeMotionEvent(display, xDis, yDis, 0);
    XFlush(display);
}

QString XTestEventHandler::getName()
{
    return QString("XTest");
}

QString XTestEventHandler::getIdentifier()
{
    return QString("xtest");
}
