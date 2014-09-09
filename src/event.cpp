#include <QDebug>
#include <QVariant>
#include <QApplication>
#include <QTime>
#include <cmath>
#include <QFileInfo>
#include <QStringList>
#include <QCursor>
#include <QDesktopWidget>

#include "event.h"

#if defined(Q_OS_UNIX)
#include "eventhandlerfactory.h"

    #if defined(WITH_X11)

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include "x11info.h"

        #ifdef WITH_XTEST
#include <X11/extensions/XTest.h>
        #endif
    #endif

    #if defined(WITH_UINPUT)
#include "uinputhelper.h"
    #endif

#elif defined (Q_OS_WIN)
#include <qt_windows.h>
#include "wininfo.h"

#endif

static MouseHelper *mouseHelperObj = 0;

#ifdef Q_OS_UNIX
    #ifdef WITH_XTEST
    static void finalSpringEvent(Display *display, unsigned int xmovecoor, unsigned int ymovecoor)
    {
        //XTestFakeMotionEvent(display, -1, xmovecoor, ymovecoor, 0);
    }
    #endif

#elif defined(Q_OS_WIN)
    static void finalSpringEvent(INPUT *event, unsigned int xmovecoor, unsigned int ymovecoor, unsigned int width, unsigned int height)
    {
        int fx = ceil(xmovecoor * (65535.0/(double)width));
        int fy = ceil(ymovecoor * (65535.0/(double)height));
        event[0].mi.dx = fx;
        event[0].mi.dy = fy;
        SendInput(1, event, sizeof(INPUT));
    }
#endif

// Create the event used by the operating system.
void sendevent(JoyButtonSlot *slot, bool pressed)
{
    //int code = slot->getSlotCode();
    JoyButtonSlot::JoySlotInputAction device = slot->getSlotMode();

#if defined (Q_OS_UNIX)
    //Display* display = X11Info::getInstance()->display();

    if (device == JoyButtonSlot::JoyKeyboard)
    {
        EventHandlerFactory::getInstance()->handler()->sendKeyboardEvent(slot, pressed);
        //write_uinput_event(fd, EV_KEY, code, pressed ? 1 : 0);
        //write_uinput_event(fd, EV_KEY, KEY_D, pressed ? 1 : 0);
        //unsigned int tempcode = XKeysymToKeycode(display, code);
        //if (tempcode > 0)
        //{
        //    XTestFakeKeyEvent(display, tempcode, pressed, 0);
        //}
    }
    else if (device == JoyButtonSlot::JoyMouseButton)
    {
        EventHandlerFactory::getInstance()->handler()->sendMouseButtonEvent(slot, pressed);
        //XTestFakeButtonEvent(display, code, pressed, 0);
    }

    //XFlush(display);

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
    //Display* display = X11Info::getInstance()->display();

    EventHandlerFactory::getInstance()->handler()->sendMouseEvent(code1, code2);

    //XTestFakeRelativeMotionEvent(display, code1, code2, 0);
    //XFlush(display);

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

void sendSpringEvent(PadderCommon::springModeInfo *fullSpring, PadderCommon::springModeInfo *relativeSpring)
{
    if (!mouseHelperObj)
    {
        mouseHelperObj = new MouseHelper();
        QObject::connect(qApp, SIGNAL(aboutToQuit()), mouseHelperObj, SLOT(deleteLater()));
    }

    mouseHelperObj->mouseTimer.stop();

    if ((fullSpring->displacementX >= -2.0 && fullSpring->displacementX <= 1.0 &&
        fullSpring->displacementY >= -2.0 && fullSpring->displacementY <= 1.0) ||
        (relativeSpring->displacementX >= -2.0 && relativeSpring->displacementX <= 1.0 &&
        relativeSpring->displacementY >= -2.0 && relativeSpring->displacementY <= 1.0))
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
        QDesktopWidget deskWid;
        QRect deskRect = deskWid.screenGeometry();
        width = deskRect.width();
        height = deskRect.height();
        QPoint currentPoint = QCursor::pos();
        currentMouseX = currentPoint.x();
        currentMouseY = currentPoint.y();

        //qDebug() << "Current Mouse X: " << currentMouseX;
        //qDebug() << "Current Mouse Y: " << currentMouseY;
        //qDebug() << "WIDTH: " << width;
        //qDebug() << "HEIGHT: " << height;

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

        int springWidth = fullSpring->width;
        int springHeight = fullSpring->height;

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

        unsigned int pivotX = midwidth;
        unsigned int pivotY = midheight;
        if (mouseHelperObj->pivotPoint[0] != -1)
        {
            pivotX = mouseHelperObj->pivotPoint[0];
        }
        else
        {
            pivotX = currentMouseX;
        }

        if (mouseHelperObj->pivotPoint[1] != -1)
        {
            pivotY = mouseHelperObj->pivotPoint[1];
        }
        else
        {
            pivotY = currentMouseY;
        }

        xmovecoor = (fullSpring->displacementX >= -1.0) ? (midwidth + (fullSpring->displacementX * destMidWidth)): pivotX;
        ymovecoor = (fullSpring->displacementY >= -1.0) ? (midheight + (fullSpring->displacementY * destMidHeight)) : pivotY;

        unsigned int fullSpringDestX = xmovecoor;
        unsigned int fullSpringDestY = ymovecoor;

        unsigned int destRelativeWidth = 0;
        unsigned int destRelativeHeight = 0;
        if (relativeSpring && relativeSpring->width >= 2 && relativeSpring->height >= 2)
        {
            destRelativeWidth = relativeSpring->width;
            destRelativeHeight = relativeSpring->height;

            int xRelativeMoovCoor = 0;
            if (relativeSpring->displacementX >= -1.0)
            {
                xRelativeMoovCoor = (relativeSpring->displacementX * destRelativeWidth) / 2;
            }

            int yRelativeMoovCoor = 0;
            if (relativeSpring->displacementY >= -1.0)
            {
                yRelativeMoovCoor = (relativeSpring->displacementY * destRelativeHeight) / 2;
            }

            xmovecoor += xRelativeMoovCoor;
            ymovecoor += yRelativeMoovCoor;
        }

        if (xmovecoor != currentMouseX || ymovecoor != currentMouseY)
        {
            //qDebug() << "XMOVECOOR: " << xmovecoor;
            //qDebug() << "YMOVECOOR: " << ymovecoor;

            double diffx = abs(currentMouseX - xmovecoor);
            double diffy = abs(currentMouseY - ymovecoor);

            //qDebug() << "DIFFX: " << diffx;
            //qDebug() << "DIFFY: " << diffy;

            //qDebug() << "SHITX: " << xmovecoor - currentMouseX;
            //qDebug() << "SHITY: " << ymovecoor - currentMouseY;
            //double finaldiff = sqrt((diffx*diffx)+(diffy*diffy));

#ifdef Q_OS_WIN
            INPUT temp[1] = {};
            temp[0].type = INPUT_MOUSE;
            temp[0].mi.mouseData = 0;
            temp[0].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;

#endif

            //write_uinput_event(fd, EV_REL, REL_X, xmovecoor - currentMouseX, false);
            //write_uinput_event(fd, EV_REL, REL_Y, ymovecoor - currentMouseY);

            // If either position is set to center, force update.
            if (xmovecoor == midwidth || ymovecoor == midheight)
            {
#if defined(Q_OS_UNIX)
                EventHandlerFactory::getInstance()->handler()->sendMouseEvent(xmovecoor - currentMouseX,
                                                                              ymovecoor - currentMouseY);
                //write_uinput_event(fd, EV_REL, REL_X, xmovecoor - currentMouseX);
                //write_uinput_event(fd, EV_REL, REL_Y, ymovecoor - currentMouseY);
                //XTestFakeMotionEvent(display, -1, xmovecoor, ymovecoor, 0);

#elif defined(Q_OS_WIN)
                finalSpringEvent(temp, xmovecoor, ymovecoor, width, height);
#endif
            }
            else if (!mouseHelperObj->springMouseMoving && relativeSpring &&
                (relativeSpring->displacementX >= -1.0 || relativeSpring->displacementY >= -1.0) &&
                (diffx >= destRelativeWidth*.013 || diffy >= destRelativeHeight*.013))
            {
                mouseHelperObj->springMouseMoving = true;
#if defined(Q_OS_UNIX)
                EventHandlerFactory::getInstance()->handler()->sendMouseEvent(xmovecoor - currentMouseX,
                                                                              ymovecoor - currentMouseY);
                //write_uinput_event(fd, EV_REL, REL_X, xmovecoor - currentMouseX);
                //write_uinput_event(fd, EV_REL, REL_Y, ymovecoor - currentMouseY);
                //XTestFakeMotionEvent(display, -1, xmovecoor, ymovecoor, 0);

#elif defined(Q_OS_WIN)
                finalSpringEvent(temp, xmovecoor, ymovecoor, width, height);
#endif
                mouseHelperObj->mouseTimer.start(8);
            }
            else if (!mouseHelperObj->springMouseMoving && (diffx >= destSpringWidth*.013 || diffy >= destSpringHeight*.013))
            {
                mouseHelperObj->springMouseMoving = true;
#if defined(Q_OS_UNIX)
                EventHandlerFactory::getInstance()->handler()->sendMouseEvent(xmovecoor - currentMouseX,
                                                                              ymovecoor - currentMouseY);
                //write_uinput_event(fd, EV_REL, REL_X, xmovecoor - currentMouseX);
                //write_uinput_event(fd, EV_REL, REL_Y, ymovecoor - currentMouseY);
                //XTestFakeMotionEvent(display, -1, xmovecoor, ymovecoor, 0);

#elif defined(Q_OS_WIN)
                finalSpringEvent(temp, xmovecoor, ymovecoor, width, height);
#endif

                //qDebug() << QTime::currentTime();
                //qDebug() << "X: " << xmovecoor;
                //qDebug() << "Y: " << ymovecoor;

                //mouseHelperObj->mouseTimer.start(8);
            }

            else if (mouseHelperObj->springMouseMoving && (diffx < 2 && diffy < 2))
            {
                mouseHelperObj->springMouseMoving = false;
            }
            else if (mouseHelperObj->springMouseMoving)
            {
#if defined(Q_OS_UNIX)
                EventHandlerFactory::getInstance()->handler()->sendMouseEvent(xmovecoor - currentMouseX,
                                                                              ymovecoor - currentMouseY);
                //write_uinput_event(fd, EV_REL, REL_X, xmovecoor - currentMouseX);
                //write_uinput_event(fd, EV_REL, REL_Y, ymovecoor - currentMouseY);
                //XTestFakeMotionEvent(display, -1, xmovecoor, ymovecoor, 0);

#elif defined(Q_OS_WIN)
                finalSpringEvent(temp, xmovecoor, ymovecoor, width, height);
#endif


                mouseHelperObj->mouseTimer.start(8);
            }

            mouseHelperObj->previousCursorLocation[0] = currentMouseX;
            mouseHelperObj->previousCursorLocation[1] = currentMouseY;
            mouseHelperObj->pivotPoint[0] = fullSpringDestX;
            mouseHelperObj->pivotPoint[1] = fullSpringDestY;
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
            mouseHelperObj->pivotPoint[0] = fullSpringDestX;
            mouseHelperObj->pivotPoint[1] = fullSpringDestY;

            mouseHelperObj->mouseTimer.start(8);
        }
    }
    else
    {
        mouseHelperObj->springMouseMoving = false;
        mouseHelperObj->pivotPoint[0] = -1;
        mouseHelperObj->pivotPoint[1] = -1;
    }
}

int X11KeySymToKeycode(QString key)
{
    int tempcode = 0;
#if defined (Q_OS_UNIX)

    if (key.length() > 0)
    {
    #if defined(WITH_XTEST)
        Display* display = X11Info::getInstance()->display();
        tempcode = XKeysymToKeycode(display, XStringToKeysym(key.toUtf8().data()));
    #elif defined(WITH_UINPUT)
        tempcode = UInputHelper::getInstance()->getVirtualKey(key);
    #endif
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

QString keycodeToKeyString(int keycode, unsigned int alias)
{
    QString newkey;

#if defined (Q_OS_UNIX)
    Q_UNUSED(alias);

    if (keycode <= 0)
    {
        newkey = "[NO KEY]";
    }
    else
    {
    #if defined(WITH_XTEST)
        Display* display = X11Info::getInstance()->display();
        newkey = QString("0x%1").arg(keycode, 0, 16);
        QString tempkey = XKeysymToString(XkbKeycodeToKeysym(display, keycode, 0, 0));
        QString tempalias = X11Info::getInstance()->getDisplayString(tempkey);
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

    #elif defined(WITH_UINPUT)
        QString tempalias = UInputHelper::getInstance()->getDisplayString(keycode);
        if (!tempalias.isEmpty())
        {
            newkey = tempalias;
        }
        else
        {
            newkey = QString("0x%1").arg(keycode, 0, 16);
        }

    #endif
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
    #ifdef WITH_X11
    Display* display = X11Info::getInstance()->display();
    unsigned int tempcode = XkbKeycodeToKeysym(display, keycode, 0, 0);
    return tempcode;
    #else

    Q_UNUSED(keycode);
    return 0;
    #endif
#endif
}

QString keysymToKeyString(int keysym, unsigned int alias)
{
    QString newkey;
#if defined (Q_OS_UNIX)
    Q_UNUSED(alias);

    #if defined(WITH_XTEST)
    Display* display = X11Info::getInstance()->display();
    unsigned int keycode = 0;
    if (keysym > 0)
    {
        keycode = XKeysymToKeycode(display, keysym);
    }
    newkey = keycodeToKeyString(keycode);

    #elif defined(WITH_UINPUT)
    newkey = keycodeToKeyString(keysym);

    #endif

#else
    newkey = keycodeToKeyString(keysym, alias);
#endif
    return newkey;
}
