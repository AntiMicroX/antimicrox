#include <QDebug>
#include <QVariant>
#include <QApplication>
#include <cmath>

#include "event.h"

#if defined(Q_OS_UNIX)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XTest.h>
#include "x11info.h"

#elif defined (Q_OS_WIN)
#include <qt_windows.h>
#include "wininfo.h"

#endif



static MouseHelper *mouseHelperObj = 0;

//actually creates an XWindows event  :)
void sendevent(JoyButtonSlot *slot, bool pressed)
{
    int code = slot->getSlotCode();
    JoyButtonSlot::JoySlotInputAction device = slot->getSlotMode();

#if defined (Q_OS_UNIX)
    Display* display = X11Info::display();

    if (device == JoyButtonSlot::JoyKeyboard)
    {
        unsigned int tempcode = XKeysymToKeycode(display, code);
        if (tempcode > 0)
        {
            XTestFakeKeyEvent(display, tempcode, pressed, 0);
        }
    }
    else if (device == JoyButtonSlot::JoyMouseButton)
    {
        XTestFakeButtonEvent(display, code, pressed, 0);
    }

    XFlush(display);

#elif defined (Q_OS_WIN)
    INPUT temp[1] = {};
    if (device == JoyButtonSlot::JoyKeyboard)
    {
        unsigned int scancode = WinInfo::scancodeFromVirtualKey(code, slot->getSlotCodeAlias());
        int extended = (scancode & WinInfo::EXTENDED_FLAG) != 0;
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
    else if (device == JoyButtonSlot::JoyMouseButton)
    {
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

#endif
}

void sendevent(int code1, int code2)
{
#if defined (Q_OS_UNIX)
    Display* display = X11Info::display();

    XTestFakeRelativeMotionEvent(display, code1, code2, 0);
    XFlush(display);

#elif defined (Q_OS_WIN)
    INPUT temp[1] = {};
    temp[0].type = INPUT_MOUSE;
    temp[0].mi.mouseData = 0;
    temp[0].mi.dwFlags   =  MOUSEEVENTF_MOVE;
    temp[0].mi.dx = code1;
    temp[0].mi.dy = code2;
    SendInput(1, temp, sizeof(INPUT));

#endif
}

void sendSpringEvent(double xcoor, double ycoor, int springWidth, int springHeight)
{
#ifdef Q_OS_UNIX
    Display* display = X11Info::display();
#endif

    if (!mouseHelperObj)
    {
        mouseHelperObj = new MouseHelper();
        QObject::connect(qApp, SIGNAL(aboutToQuit()), mouseHelperObj, SLOT(deleteLater()));
    }

    mouseHelperObj->mouseTimer.stop();

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
        int currentMouseX = 0;
        int currentMouseY = 0;

#if defined (Q_OS_UNIX)
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
        currentMouseX = mouseEvent.xbutton.x_root;
        currentMouseY = mouseEvent.xbutton.y_root;

#elif defined (Q_OS_WIN)
        POINT cursorPoint;
        GetCursorPos(&cursorPoint);

        width = GetSystemMetrics(SM_CXSCREEN);
        height = GetSystemMetrics(SM_CYSCREEN);
        currentMouseX = cursorPoint.x;
        currentMouseY = cursorPoint.y;

#endif

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

        xmovecoor = (xcoor >= -1.0) ? (midwidth + (xcoor * destMidWidth)): currentMouseX;
        ymovecoor = (ycoor >= -1.0) ? (midheight + (ycoor * destMidHeight)) : currentMouseY;

        if (xmovecoor != currentMouseX || ymovecoor != currentMouseY)
        {
            double diffx = abs(currentMouseX - xmovecoor);
            double diffy = abs(currentMouseY - ymovecoor);
            //double finaldiff = sqrt((diffx*diffx)+(diffy*diffy));

#ifdef Q_OS_WIN
            INPUT temp[1] = {};
            temp[0].type = INPUT_MOUSE;
            temp[0].mi.mouseData = 0;
            temp[0].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;

#endif

            if (!mouseHelperObj->springMouseMoving && (diffx >= destSpringWidth*.0066 || diffy >= destSpringHeight*.0066))
            {
                mouseHelperObj->springMouseMoving = true;

#if defined (Q_OS_UNIX)
                XTestFakeMotionEvent(display, -1, xmovecoor, ymovecoor, 0);

#elif defined (Q_OS_WIN)
                int fx = ceil(xmovecoor * (65535.0/(double)width));
                int fy = ceil(ymovecoor * (65535.0/(double)height));
                temp[0].mi.dx = fx;
                temp[0].mi.dy = fy;
                SendInput(1, temp, sizeof(INPUT));
#endif

                mouseHelperObj->mouseTimer.start(8);
            }
            else if (mouseHelperObj->springMouseMoving && (diffx < 2 && diffy < 2))
            {
                mouseHelperObj->springMouseMoving = false;
            }
            else if (mouseHelperObj->springMouseMoving)
            {
#if defined (Q_OS_UNIX)
                XTestFakeMotionEvent(display, -1, xmovecoor, ymovecoor, 0);

#elif defined (Q_OS_WIN)
                int fx = ceil(xmovecoor * (65535.0/(double)width));
                int fy = ceil(ymovecoor * (65535.0/(double)height));
                temp[0].mi.dx = fx;
                temp[0].mi.dy = fy;
                SendInput(1, temp, sizeof(INPUT));

#endif

                mouseHelperObj->mouseTimer.start(8);

            }

            mouseHelperObj->previousCursorLocation[0] = currentMouseX;
            mouseHelperObj->previousCursorLocation[1] = currentMouseY;
        }
        else if (mouseHelperObj->previousCursorLocation[0] == xmovecoor &&
                 mouseHelperObj->previousCursorLocation[1] == ymovecoor)
        {
            mouseHelperObj->springMouseMoving = false;
        }
        else
        {
            mouseHelperObj->previousCursorLocation[0] = currentMouseX;
            mouseHelperObj->previousCursorLocation[1] = currentMouseY;
            mouseHelperObj->mouseTimer.start(8);
        }
    }
    else
    {
        mouseHelperObj->springMouseMoving = false;
    }

#ifdef Q_OS_UNIX
    XFlush(display);
#endif
}

int X11KeySymToKeycode(QString key)
{
    int tempcode = 0;
#if defined (Q_OS_UNIX)
    Display* display = X11Info::display();

    if (key.length() > 0)
    {
        tempcode = XKeysymToKeycode(display, XStringToKeysym(key.toUtf8().data()));
    }

#elif defined (Q_OS_WIN)
    if (key.length() > 0)
    {
        tempcode = WinInfo::getVirtualKey(key);
        if (tempcode <= 0 && key.length() == 1)
        {
            //qDebug() << "KEY: " << key;
            //int oridnal = key.toUtf8().constData()[0];
            int ordinal = QVariant(key.toUtf8().constData()[0]).toInt();
            tempcode = VkKeyScan(ordinal);
            int modifiers = tempcode >> 8;
            tempcode = tempcode & 0xff;
            if ((modifiers & 1) != 0) tempcode |= VK_SHIFT;
            if ((modifiers & 2) != 0) tempcode |= VK_CONTROL;
            if ((modifiers & 4) != 0) tempcode |= VK_MENU;
            //tempcode = VkKeyScan(QVariant(key.constData()).toInt());
            //tempcode = OemKeyScan(key.toUtf8().toInt());
            //tempcode = OemKeyScan(ordinal);
        }
    }

#endif

    return tempcode;
}

QString keycodeToKey(int keycode, unsigned int alias)
{
    QString newkey;

#if defined (Q_OS_UNIX)
    Q_UNUSED(alias);

    Display* display = X11Info::display();
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

#elif defined (Q_OS_WIN)
    wchar_t buffer[50] = {0};

    QString tempalias = WinInfo::getDisplayString(keycode);
    if (!tempalias.isEmpty())
    {
        newkey = tempalias;
    }
    else
    {
        int scancode = WinInfo::scancodeFromVirtualKey(keycode, alias);

        if (keycode >= VK_BROWSER_BACK && keycode <= VK_LAUNCH_APP2)
        {
            newkey.append(QString("0x%1").arg(keycode, 0, 16));
        }
        else
        {
            int length = GetKeyNameTextW(scancode << 16, buffer, sizeof(buffer));
            if (length > 0)
            {
                newkey = QString::fromWCharArray(buffer);
            }
            else
            {
                newkey.append(QString("0x%1").arg(keycode, 0, 16));
            }
        }
    }

#endif

    return newkey;
}

unsigned int X11KeyCodeToX11KeySym(unsigned int keycode)
{
#ifdef Q_OS_WIN
    Q_UNUSED(keycode);
    return 0;
#else
    Display* display = X11Info::display();
    unsigned int tempcode = XkbKeycodeToKeysym(display, keycode, 0, 0);
    return tempcode;
#endif
}

QString keysymToKey(int keysym, unsigned int alias)
{
    QString newkey;
#if defined (Q_OS_UNIX)
    Q_UNUSED(alias);

    Display* display = X11Info::display();
    unsigned int keycode = 0;
    if (keysym > 0)
    {
        keycode = XKeysymToKeycode(display, keysym);
    }

    newkey = keycodeToKey(keycode);
#else
    newkey = keycodeToKey(keysym, alias);
#endif
    return newkey;
}
