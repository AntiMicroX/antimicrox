#include <QDataStream>
//#include <QDebug>

#include <X11/Xlib.h>
#include <X11/cursorfont.h> // for XGrabPointer
#include "x11info.h"
#include "antkeymapper.h"

#include "unixcapturewindowutility.h"

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
    cursor = XCreateFontCursor(X11Info::display(), XC_crosshair);

    status = XGrabPointer(X11Info::display(), X11Info::appRootWindow(), False, ButtonPressMask,
                 GrabModeSync, GrabModeAsync, None,
                 cursor, CurrentTime);
    if (status == Success)
    {
        XGrabKey(X11Info::display(), XKeysymToKeycode(X11Info::display(), AntKeyMapper::returnVirtualKey(Qt::Key_Escape)), 0, X11Info::appRootWindow(),
                 true, GrabModeAsync, GrabModeAsync);

        XEvent event;
        XAllowEvents(X11Info::display(), SyncPointer, CurrentTime);
        XWindowEvent(X11Info::display(), X11Info::appRootWindow(), ButtonPressMask|KeyPressMask, &event);
        switch (event.type)
        {
            case (ButtonPress):
                target_window = event.xbutton.subwindow;
                if (target_window == None) {
                    target_window = event.xbutton.window;
                }

                // Attempt to find the appropriate window below the root window
                // that was clicked.
                //qDebug() << QString::number(target_window, 16);
                target_window = X11Info::findClientWindow(target_window);
                break;

            case (KeyPress):
            {
                escaped = true;
                break;
            }
        }

        XUngrabKey(X11Info::display(), XKeysymToKeycode(X11Info::display(), AntKeyMapper::returnVirtualKey(Qt::Key_Escape)),
                   0, X11Info::appRootWindow());
        XUngrabPointer(X11Info::display(), CurrentTime);
        XFlush(X11Info::display());
    }

    if (target_window != None)
    {
        int pid = X11Info::getApplicationPid(target_window);
        if (pid > 0)
        {
            QString exepath = X11Info::getApplicationLocation(pid);

            if (!exepath.isEmpty())
            {
                targetPath = exepath;
            }
            else
            {
                failed = true;
            }
        }
        else
        {
            failed = true;
        }
    }
    else if (!escaped)
    {
        failed = true;
    }

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
