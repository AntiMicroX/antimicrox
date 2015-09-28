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

//#include <QDebug>
#include <QDataStream>

#include <X11/Xlib.h>
#include <X11/cursorfont.h> // for XGrabPointer
#include "x11extras.h"
#include "qtx11keymapper.h"
#include "unixcapturewindowutility.h"

UnixCaptureWindowUtility::UnixCaptureWindowUtility(QObject *parent) :
    QObject(parent)
{
    targetPath = "";
    failed = false;
    targetWindow = None;
}

/**
 * @brief Attempt to capture window selected with the mouse
 */
void UnixCaptureWindowUtility::attemptWindowCapture()
{
    // Only create instance when needed.
    static QtX11KeyMapper x11KeyMapper;

    targetPath = "";
    targetWindow = None;
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
