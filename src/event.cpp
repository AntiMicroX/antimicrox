#include <QDebug>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XTest.h>
#include <cmath>

#include "event.h"
#include "x11info.h"

Display* display;
MouseHelper mouseHelperObj;

//actually creates an XWindows event  :)
void sendevent( int code, bool pressed, JoyButtonSlot::JoySlotInputAction device) {

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
}

void sendevent(int code1, int code2)
{
    display = X11Info::display();

    XLockDisplay (display);

    XTestFakeRelativeMotionEvent(display, code1, code2, 0);

    XFlush(display);
    XUnlockDisplay (display);
}

void sendSpringEvent(double xcoor, double ycoor, int springWidth, int springHeight)
{
    display = X11Info::display();

    XLockDisplay(display);
    mouseHelperObj.mouseTimer.stop();

    if (xcoor >= -2.0 && xcoor <= 1.0 &&
        ycoor >= -2.0 && ycoor <= 1.0)
    {
        int xmovecoor = 0;
        int ymovecoor = 0;
        int width = 0;
        int height = 0;
        int midwidth = 0;
        int midheight = 0;
        int destSpringWidth = 0;
        int destSpringHeight = 0;
        int destMidWidth = 0;
        int destMidHeight = 0;

        XEvent mouseEvent;
        Window wid = DefaultRootWindow(display);
        XWindowAttributes xwAttr;

        XQueryPointer(display, wid,
            &mouseEvent.xbutton.root, &mouseEvent.xbutton.window,
            &mouseEvent.xbutton.x_root, &mouseEvent.xbutton.y_root,
            &mouseEvent.xbutton.x, &mouseEvent.xbutton.y,
            &mouseEvent.xbutton.state);

        XGetWindowAttributes(display, wid, &xwAttr);
        width = xwAttr.width;
        height = xwAttr.height;
        midwidth = width / 2;
        midheight = height / 2;

        if (springWidth >= 2 && springHeight >= 2)
        {
            destSpringWidth = qMin(springWidth, width);
            destSpringHeight = qMin(springHeight, height);
        }
        else
        {
            destSpringWidth = width;
            destSpringHeight = height;
        }

        destMidWidth = destSpringWidth / 2;
        destMidHeight = destSpringHeight / 2;

        xmovecoor = (xcoor >= -1.0) ? (midwidth + (xcoor * destMidWidth)): mouseEvent.xbutton.x_root;
        ymovecoor = (ycoor >= -1.0) ? (midheight + (ycoor * destMidHeight)) : mouseEvent.xbutton.y_root;

        if (xmovecoor != mouseEvent.xbutton.x_root || ymovecoor != mouseEvent.xbutton.y_root)
        {
            double diffx = abs(mouseEvent.xbutton.x_root - xmovecoor);
            double diffy = abs(mouseEvent.xbutton.y_root - ymovecoor);
            //double finaldiff = sqrt((diffx*diffx)+(diffy*diffy));
            if (!mouseHelperObj.springMouseMoving && (diffx >= destSpringWidth*.0066 || diffy >= destSpringHeight*.0066))
            {
                mouseHelperObj.springMouseMoving = true;
                XTestFakeMotionEvent(display, -1, xmovecoor, ymovecoor, 0);
                mouseHelperObj.mouseTimer.start(8);
            }
            else if (mouseHelperObj.springMouseMoving && (diffx < 2 && diffy < 2))
            {
                mouseHelperObj.springMouseMoving = false;
            }
            else if (mouseHelperObj.springMouseMoving)
            {
                XTestFakeMotionEvent(display, -1, xmovecoor, ymovecoor, 0);
                mouseHelperObj.mouseTimer.start(8);
            }

            mouseHelperObj.previousCursorLocation[0] = mouseEvent.xbutton.x_root;
            mouseHelperObj.previousCursorLocation[1] = mouseEvent.xbutton.y_root;
        }
        else if (mouseHelperObj.previousCursorLocation[0] == xmovecoor &&
                 mouseHelperObj.previousCursorLocation[1] == ymovecoor)
        {
            mouseHelperObj.springMouseMoving = false;
        }
        else
        {
            mouseHelperObj.previousCursorLocation[0] = mouseEvent.xbutton.x_root;
            mouseHelperObj.previousCursorLocation[1] = mouseEvent.xbutton.y_root;
            mouseHelperObj.mouseTimer.start(8);
        }
    }
    else
    {
        mouseHelperObj.springMouseMoving = false;
    }

    XFlush(display);
    XUnlockDisplay(display);
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
