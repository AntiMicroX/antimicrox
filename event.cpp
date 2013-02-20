#include <QX11Info>
#include <QDebug>
#include <QMutex>
#include <X11/extensions/XTest.h>

#include "event.h"

QMutex mutex;
Display* display;

//actually creates an XWindows event  :)
void sendevent( int code, bool pressed, JoyButtonSlot::JoySlotInputAction device) {
    //mutex.lock();
    display = QX11Info::display();

    XLockDisplay (display);

    //qDebug () << "IN SENDEVENT: " << code << endl;


    if (device == JoyButtonSlot::JoyKeyboard)
    {
        XTestFakeKeyEvent(display, code, pressed, 0);
    }
    else if (device == JoyButtonSlot::JoyMouseButton)
    {
        XTestFakeButtonEvent(display, code, pressed, 0);
    }

    XFlush(display);
    XUnlockDisplay (display);

    //mutex.unlock();
}

void sendevent(int code1, int code2)
{
    display = QX11Info::display();

    XLockDisplay (display);

    XTestFakeRelativeMotionEvent(display, code1, code2, 0);

    XFlush(display);
    XUnlockDisplay (display);
}

int keyToKeycode (QString key)
{
    int tempcode = -1;
    Display* display = QX11Info::display();
    if (key.length() > 0)
    {
        tempcode = XKeysymToKeycode(display, XStringToKeysym(key.toUtf8().data()));
    }
    return tempcode;
}

QString keycodeToKey(int keycode)
{
    display = QX11Info::display();
    QString newkey;
    if (keycode <= 0)
    {
        newkey = "[NO KEY]";
    }
    else
    {
        newkey = XKeysymToString(XKeycodeToKeysym(display, keycode,0));
    }

    return newkey;
}
