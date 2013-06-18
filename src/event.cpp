#include <QDebug>
#include <QMutex>
#include <QChar>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XTest.h>

#include "event.h"
#include "x11info.h"

QMutex mutex;
Display* display;

//actually creates an XWindows event  :)
void sendevent( int code, bool pressed, JoyButtonSlot::JoySlotInputAction device) {

    //mutex.lock();
    display = X11Info::display();
    XLockDisplay (display);

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
    display = X11Info::display();

    XLockDisplay (display);

    XTestFakeRelativeMotionEvent(display, code1, code2, 0);

    XFlush(display);
    XUnlockDisplay (display);
}

int keyToKeycode (QString key)
{
    int tempcode = -1;
    Display* display = X11Info::display();

    if (key.length() > 0)
    {
        tempcode = XKeysymToKeycode(display, XStringToKeysym(key.toUtf8().data()));
    }
    return tempcode;
}

QString keycodeToKey(int keycode)
{
    display = X11Info::display();
    QString newkey;
    if (keycode <= 0)
    {
        newkey = "[NO KEY]";
    }
    else
    {
        QString tempkey = XKeysymToString(XkbKeycodeToKeysym(display, keycode, 0, 0));
        QString tempalias = X11Info::getDisplayString(tempkey);
        if (!tempalias.isEmpty())
        {
            newkey = tempalias;
        }
        else
        {
            XKeyPressedEvent tempevent;
            tempevent.keycode = keycode;
            tempevent.type = KeyPress;
            tempevent.display = display;
            tempevent.state = 0;

            char tempstring[256];
            memset(tempstring, 0, sizeof(tempstring));
            int bitestoreturn = sizeof(tempstring) - 1;
            int numchars = XLookupString(&tempevent, tempstring, bitestoreturn, NULL, NULL);
            if (numchars > 0)
            {
                tempstring[numchars] = '\0';
                newkey = QString::fromUtf8(tempstring);
                //qDebug() << "NEWKEY:" << newkey << endl;
                //qDebug() << "NEWKEY LEGNTH:" << numchars << endl;
            }
            else
            {
                newkey = tempkey;
            }
        }
    }

    return newkey;
}
