//#include <QDebug>
#include <QDataStream>

#include <X11/Xlib.h>
#include <X11/cursorfont.h> // for XGrabPointer
#include "x11extras.h"
#include "qtx11keymapper.h"
#include "unixcapturewindowutility.h"

static QtX11KeyMapper x11KeyMapper;

UnixCaptureWindowUtility::UnixCaptureWindowUtility(QObject *parent) :
    QObject(parent)
{
    targetPath = "";
    failed = false;
}

/**
 * @brief Attempt to capture window selected with the mouse
 */
void UnixCaptureWindowUtility::attemptWindowCapture()
{
    targetPath = "";
    failed = false;
    bool escaped = false;

    Cursor cursor;
    Window target_window = None;
    int status = 0;
    Display *display = 0;

    QString potentialXDisplayString = X11Extras::getInstance()->getXDisplayString();
    if (!potentialXDisplayString.isEmpty())
    {
        QByteArray tempByteArray = potentialXDisplayString.toLocal8Bit();
        display = XOpenDisplay(tempByteArray.constData());
    }
    else
    {
        display = XOpenDisplay(NULL);
    }

    Window rootWin = XDefaultRootWindow(display);

    cursor = XCreateFontCursor(display, XC_crosshair);
    status = XGrabPointer(display, rootWin, False, ButtonPressMask,
                 GrabModeSync, GrabModeAsync, None,
                 cursor, CurrentTime);
    if (status == Success)
    {
        XGrabKey(display, XKeysymToKeycode(display, x11KeyMapper.returnVirtualKey(Qt::Key_Escape)), 0, rootWin,
                 true, GrabModeAsync, GrabModeAsync);

        XEvent event;
        XAllowEvents(display, SyncPointer, CurrentTime);
        XWindowEvent(display, rootWin, ButtonPressMask|KeyPressMask, &event);
        switch (event.type)
        {
            case (ButtonPress):
                target_window = event.xbutton.subwindow;
                if (target_window == None)
                {
                    target_window = event.xbutton.window;
                }

                //qDebug() << QString::number(target_window, 16);
                break;

            case (KeyPress):
            {
                escaped = true;
                break;
            }
        }

        XUngrabKey(display, XKeysymToKeycode(display, x11KeyMapper.returnVirtualKey(Qt::Key_Escape)),
                   0, rootWin);
        XUngrabPointer(display, CurrentTime);
        XFlush(display);
    }

    if (target_window != None)
    {
        targetWindow = target_window;
    }
    else if (!escaped)
    {
        failed = true;
    }

    XCloseDisplay(display);
    emit captureFinished();
}

/**
 * @brief Get the saved path for a window
 * @return Program path
 */
QString UnixCaptureWindowUtility::getTargetPath()
{
    return targetPath;
}

/**
 * @brief Check if attemptWindowCapture failed to obtain an application
 * @return Error status
 */
bool UnixCaptureWindowUtility::hasFailed()
{
    return failed;
}

unsigned long UnixCaptureWindowUtility::getTargetWindow()
{
    return targetWindow;
}
