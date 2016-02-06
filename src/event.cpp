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
#include <QVariant>
#include <QApplication>
#include <QTime>
#include <cmath>
#include <QFileInfo>
#include <QStringList>
#include <QCursor>
#include <QDesktopWidget>
#include <QProcess>

#include "event.h"
#include "eventhandlerfactory.h"
#include "joybutton.h"

#if defined(Q_OS_UNIX)

    #if defined(WITH_X11)

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include "x11extras.h"

        #ifdef WITH_XTEST
#include <X11/extensions/XTest.h>
        #endif
    #endif

    #if defined(WITH_UINPUT)
#include "uinputhelper.h"
    #endif

#elif defined (Q_OS_WIN)
#include <qt_windows.h>
#include "winextras.h"

#endif

// TODO: Implement function for determining final mouse pointer position
// based around a fixed bounding box resolution.
void fakeAbsMouseCoordinates(double springX, double springY,
                             unsigned int width, unsigned int height,
                             unsigned int &finalx, unsigned int &finaly, int screen=-1)
{
    //Q_UNUSED(finalx);
    //Q_UNUSED(finaly);
    //Q_UNUSED(width);
    //Q_UNUSED(height);

    int screenWidth = 0;
    int screenHeight = 0;
    int screenMidwidth = 0;
    int screenMidheight = 0;

    int destSpringWidth = 0;
    int destSpringHeight = 0;
    int destMidWidth = 0;
    int destMidHeight = 0;
    //int currentMouseX = 0;
    //int currentMouseY = 0;

    QRect deskRect = PadderCommon::mouseHelperObj.getDesktopWidget()
            ->screenGeometry(screen);

    screenWidth = deskRect.width();
    screenHeight = deskRect.height();

    screenMidwidth = screenWidth / 2;
    screenMidheight = screenHeight / 2;

    if (width >= 2 && height >= 2)
    {
        destSpringWidth = qMin(static_cast<int>(width), screenWidth);
        destSpringHeight = qMin(static_cast<int>(height), screenHeight);
    }
    else
    {
        destSpringWidth = screenWidth;
        destSpringHeight = screenHeight;
    }

/*#if defined(Q_OS_UNIX) && defined(WITH_X11)
    QPoint currentPoint;
  #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
  #endif
        currentPoint = X11Extras::getInstance()->getPos();
  #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    }
    else
    {
        currentPoint = QCursor::pos();
    }
  #endif

#else
    QPoint currentPoint = QCursor::pos();
#endif
*/

    destMidWidth = destSpringWidth / 2;
    destMidHeight = destSpringHeight / 2;

    finalx = (screenMidwidth + (springX * destMidWidth) + deskRect.x());
    finaly = (screenMidheight + (springY * destMidHeight) + deskRect.y());
}

// Create the event used by the operating system.
void sendevent(JoyButtonSlot *slot, bool pressed)
{
    //int code = slot->getSlotCode();
    JoyButtonSlot::JoySlotInputAction device = slot->getSlotMode();

    if (device == JoyButtonSlot::JoyKeyboard)
    {
        EventHandlerFactory::getInstance()->handler()->sendKeyboardEvent(slot, pressed);
    }
    else if (device == JoyButtonSlot::JoyMouseButton)
    {
        EventHandlerFactory::getInstance()->handler()->sendMouseButtonEvent(slot, pressed);
    }
    else if (device == JoyButtonSlot::JoyTextEntry && pressed && !slot->getTextData().isEmpty())
    {
        EventHandlerFactory::getInstance()->handler()->sendTextEntryEvent(slot->getTextData());
    }
    else if (device == JoyButtonSlot::JoyExecute && pressed && !slot->getTextData().isEmpty())
    {
        QString execString = slot->getTextData();
        if (slot->getExtraData().canConvert<QString>())
        {
            QString argumentsString = slot->getExtraData().toString();
            QStringList argumentsTempList(PadderCommon::parseArgumentsString(argumentsString));
            QProcess::startDetached(execString, argumentsTempList);
        }
        else
        {
            QProcess::startDetached(execString);
        }
    }
}

// Create the relative mouse event used by the operating system.
void sendevent(int code1, int code2)
{
    EventHandlerFactory::getInstance()->handler()->sendMouseEvent(code1, code2);
}

// TODO: Re-implement spring event generation to simplify the process
// and reduce overhead. Refactor old function to only be used when an absmouse
// position must be faked.
void sendSpringEventRefactor(PadderCommon::springModeInfo *fullSpring,
                     PadderCommon::springModeInfo *relativeSpring,
                     int* const mousePosX, int* const mousePosY)
{
    Q_UNUSED(relativeSpring);
    Q_UNUSED(mousePosX);
    Q_UNUSED(mousePosY);

    PadderCommon::mouseHelperObj.mouseTimer.stop();

    if (fullSpring)
    {
        unsigned int xmovecoor = 0;
        unsigned int ymovecoor = 0;
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

        double displacementX = 0.0;
        double displacementY = 0.0;
        bool useFullScreen = true;

        PadderCommon::mouseHelperObj.mouseTimer.stop();
        BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();

        if (fullSpring->screen >= -1 &&
            fullSpring->screen >= PadderCommon::mouseHelperObj.getDesktopWidget()->screenCount())
        {
            fullSpring->screen = -1;
        }

        int springWidth = fullSpring->width;
        int springHeight = fullSpring->height;

        if (springWidth >= 2 && springHeight >= 2)
        {
            useFullScreen = false;
            displacementX = fullSpring->displacementX;
            displacementY = fullSpring->displacementY;
        }
        else
        {
            useFullScreen = true;
            displacementX = fullSpring->displacementX;
            displacementY = fullSpring->displacementY;
        }

        unsigned int pivotX = 0;
        unsigned int pivotY = 0;
        if (relativeSpring && relativeSpring->width >= 2 && relativeSpring->height >= 2)
        {
            if (PadderCommon::mouseHelperObj.pivotPoint[0] != -1)
            {
                pivotX = PadderCommon::mouseHelperObj.pivotPoint[0];
            }

            if (PadderCommon::mouseHelperObj.pivotPoint[1] != -1)
            {
                pivotY = PadderCommon::mouseHelperObj.pivotPoint[1];
            }

            if (pivotX >= 0 && pivotY >= 0)
            {
                // Find a use for this routine in this context.
                int destRelativeWidth = relativeSpring->width;
                int destRelativeHeight = relativeSpring->height;

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
        }

        if (handler->getIdentifier() == "xtest")
        {
            fakeAbsMouseCoordinates(displacementX, displacementY,
                                    springWidth, springHeight, xmovecoor, ymovecoor,
                                    fullSpring->screen);

            //EventHandlerFactory::getInstance()->handler()->sendMouseAbsEvent(xmovecoor,
            //                                                                 ymovecoor);
        }
        else if (handler->getIdentifier() == "uinput")
        {
            //EventHandlerFactory::getInstance()->handler()->sendMouseAbsEvent(displacementX,
            //                                                                 displacementY);

            fakeAbsMouseCoordinates(displacementX, displacementY,
                                    springWidth, springHeight, xmovecoor, ymovecoor,
                                    fullSpring->screen);
            //EventHandlerFactory::getInstance()->handler()
            //        ->sendMouseSpringEvent(xmovecoor, ymovecoor, width, height);
        }
    }
    else
    {
        PadderCommon::mouseHelperObj.springMouseMoving = false;
        PadderCommon::mouseHelperObj.pivotPoint[0] = -1;
        PadderCommon::mouseHelperObj.pivotPoint[1] = -1;
    }
}

// TODO: Change to only use this routine when using a relative mouse
// pointer to fake absolute mouse moves. Otherwise, don't worry about
// current position of the mouse and just send an absolute mouse pointer
// event.
void sendSpringEvent(PadderCommon::springModeInfo *fullSpring,
                     PadderCommon::springModeInfo *relativeSpring,
                     int* const mousePosX, int* const mousePosY)
{
    PadderCommon::mouseHelperObj.mouseTimer.stop();

    if ((fullSpring->displacementX >= -2.0 && fullSpring->displacementX <= 1.0 &&
        fullSpring->displacementY >= -2.0 && fullSpring->displacementY <= 1.0) ||
        (relativeSpring && (relativeSpring->displacementX >= -2.0 && relativeSpring->displacementX <= 1.0 &&
        relativeSpring->displacementY >= -2.0 && relativeSpring->displacementY <= 1.0)))
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

        //QDesktopWidget deskWid;
        if (fullSpring->screen >= -1 &&
            fullSpring->screen >= PadderCommon::mouseHelperObj.getDesktopWidget()->screenCount())
        {
            fullSpring->screen = -1;
        }

        QRect deskRect = PadderCommon::mouseHelperObj.getDesktopWidget()
                ->screenGeometry(fullSpring->screen);

        width = deskRect.width();
        height = deskRect.height();

#if defined(Q_OS_UNIX) && defined(WITH_X11)
        QPoint currentPoint;
  #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        if (QApplication::platformName() == QStringLiteral("xcb"))
        {
  #endif
            currentPoint = X11Extras::getInstance()->getPos();
  #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        }
        else
        {
            currentPoint = QCursor::pos();
        }
  #endif
#else
        QPoint currentPoint = QCursor::pos();
#endif

        currentMouseX = currentPoint.x();
        currentMouseY = currentPoint.y();

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

        unsigned int pivotX = currentMouseX;
        unsigned int pivotY = currentMouseY;
        if (relativeSpring)
        {
            if (PadderCommon::mouseHelperObj.pivotPoint[0] != -1)
            {
                pivotX = PadderCommon::mouseHelperObj.pivotPoint[0];
            }
            else
            {
                pivotX = currentMouseX;
            }

            if (PadderCommon::mouseHelperObj.pivotPoint[1] != -1)
            {
                pivotY = PadderCommon::mouseHelperObj.pivotPoint[1];
            }
            else
            {
                pivotY = currentMouseY;
            }
        }

        xmovecoor = (fullSpring->displacementX >= -1.0) ? (midwidth + (fullSpring->displacementX * destMidWidth) + deskRect.x()): pivotX;
        ymovecoor = (fullSpring->displacementY >= -1.0) ? (midheight + (fullSpring->displacementY * destMidHeight) + deskRect.y()) : pivotY;

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

        if (mousePosX)
        {
            *mousePosX = xmovecoor;
        }

        if (mousePosY)
        {
            *mousePosY = ymovecoor;
        }

        if (xmovecoor != currentMouseX || ymovecoor != currentMouseY)
        {
            double diffx = abs(currentMouseX - xmovecoor);
            double diffy = abs(currentMouseY - ymovecoor);

            // If either position is set to center, force update.
            if (xmovecoor == (deskRect.x() + midwidth) || ymovecoor == (deskRect.y() + midheight))
            {
#if defined(Q_OS_UNIX)
                BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();
                if (fullSpring->screen <= -1)
                {
                    if (handler->getIdentifier() == "xtest")
                    {
                        EventHandlerFactory::getInstance()->handler()->sendMouseAbsEvent(xmovecoor,
                                                                                         ymovecoor,
                                                                                         -1);
                    }
                    else if (handler->getIdentifier() == "uinput")
                    {
                        EventHandlerFactory::getInstance()->handler()
                                ->sendMouseSpringEvent(xmovecoor, ymovecoor,
                                                       width + deskRect.x(), height + deskRect.y());
                    }
                }
                else
                {
                    EventHandlerFactory::getInstance()->handler()->sendMouseEvent(xmovecoor - currentMouseX,
                                                                                  ymovecoor - currentMouseY);
                }

#elif defined(Q_OS_WIN)
                if (fullSpring->screen <= -1)
                {
                    EventHandlerFactory::getInstance()->handler()
                            ->sendMouseSpringEvent(xmovecoor, ymovecoor,
                                                   width + deskRect.x(), height + deskRect.y());
                }
                else
                {
                    sendevent(xmovecoor - currentMouseX, ymovecoor - currentMouseY);
                }
#endif

            }
            else if (!PadderCommon::mouseHelperObj.springMouseMoving && relativeSpring &&
                (relativeSpring->displacementX >= -1.0 || relativeSpring->displacementY >= -1.0) &&
                (diffx >= destRelativeWidth*.013 || diffy >= destRelativeHeight*.013))
            {
                PadderCommon::mouseHelperObj.springMouseMoving = true;
#if defined(Q_OS_UNIX)
                BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();
                if (fullSpring->screen <= -1)
                {
                    if (handler->getIdentifier() == "xtest")
                    {
                        EventHandlerFactory::getInstance()->handler()->sendMouseAbsEvent(xmovecoor,
                                                                                         ymovecoor,
                                                                                         -1);
                    }
                    else if (handler->getIdentifier() == "uinput")
                    {
                        EventHandlerFactory::getInstance()->handler()
                                ->sendMouseSpringEvent(xmovecoor, ymovecoor,
                                                       width + deskRect.x(), height + deskRect.y());
                    }
                }
                else
                {
                    EventHandlerFactory::getInstance()->handler()
                            ->sendMouseEvent(xmovecoor - currentMouseX, ymovecoor - currentMouseY);
                }

#elif defined(Q_OS_WIN)
                if (fullSpring->screen <= -1)
                {
                    EventHandlerFactory::getInstance()->handler()
                            ->sendMouseSpringEvent(xmovecoor, ymovecoor,
                                                   width + deskRect.x(), height + deskRect.y());
                }
                else
                {
                    sendevent(xmovecoor - currentMouseX, ymovecoor - currentMouseY);
                }
#endif
                PadderCommon::mouseHelperObj.mouseTimer.start(
                            qMax(JoyButton::getMouseRefreshRate(),
                                 JoyButton::getGamepadRefreshRate()) + 1);
            }
            else if (!PadderCommon::mouseHelperObj.springMouseMoving &&
                     (diffx >= destSpringWidth*.013 || diffy >= destSpringHeight*.013))
            {
                PadderCommon::mouseHelperObj.springMouseMoving = true;
#if defined(Q_OS_UNIX)

                BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();
                if (fullSpring->screen <= -1)
                {
                    if (handler->getIdentifier() == "xtest")
                    {
                        EventHandlerFactory::getInstance()->handler()->sendMouseAbsEvent(xmovecoor,
                                                                                         ymovecoor,
                                                                                         -1);
                    }
                    else if (handler->getIdentifier() == "uinput")
                    {
                        EventHandlerFactory::getInstance()->handler()
                                ->sendMouseSpringEvent(xmovecoor, ymovecoor,
                                                       width + deskRect.x(), height + deskRect.y());
                    }
                }
                else
                {
                    EventHandlerFactory::getInstance()->handler()
                            ->sendMouseEvent(xmovecoor - currentMouseX,
                                             ymovecoor - currentMouseY);
                }


#elif defined(Q_OS_WIN)
                if (fullSpring->screen <= -1)
                {
                    EventHandlerFactory::getInstance()->handler()
                            ->sendMouseSpringEvent(xmovecoor, ymovecoor,
                                                   width + deskRect.x(), height + deskRect.y());
                }
                else
                {
                    sendevent(xmovecoor - currentMouseX, ymovecoor - currentMouseY);
                }
#endif

                PadderCommon::mouseHelperObj.mouseTimer.start(
                            qMax(JoyButton::getMouseRefreshRate(),
                                 JoyButton::getGamepadRefreshRate()) + 1);
            }

            else if (PadderCommon::mouseHelperObj.springMouseMoving && (diffx < 2 && diffy < 2))
            {
                PadderCommon::mouseHelperObj.springMouseMoving = false;
            }

            else if (PadderCommon::mouseHelperObj.springMouseMoving)
            {
#if defined(Q_OS_UNIX)
                BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();
                if (fullSpring->screen <= -1)
                {
                    if (handler->getIdentifier() == "xtest")
                    {
                        EventHandlerFactory::getInstance()->handler()->sendMouseAbsEvent(xmovecoor,
                                                                                         ymovecoor,
                                                                                         -1);
                    }
                    else if (handler->getIdentifier() == "uinput")
                    {
                        EventHandlerFactory::getInstance()->handler()
                                ->sendMouseSpringEvent(xmovecoor, ymovecoor,
                                                       width + deskRect.x(), height + deskRect.y());
                    }
                }
                else
                {
                    EventHandlerFactory::getInstance()->handler()
                            ->sendMouseEvent(xmovecoor - currentMouseX,
                                             ymovecoor - currentMouseY);
                }

#elif defined(Q_OS_WIN)
                if (fullSpring->screen <= -1)
                {
                    EventHandlerFactory::getInstance()->handler()
                            ->sendMouseSpringEvent(xmovecoor, ymovecoor,
                                                   width + deskRect.x(), height + deskRect.y());
                }
                else
                {
                    sendevent(xmovecoor - currentMouseX, ymovecoor - currentMouseY);
                }
#endif

                PadderCommon::mouseHelperObj.mouseTimer.start(
                            qMax(JoyButton::getMouseRefreshRate(),
                                 JoyButton::getGamepadRefreshRate()) + 1);
            }


            PadderCommon::mouseHelperObj.previousCursorLocation[0] = currentMouseX;
            PadderCommon::mouseHelperObj.previousCursorLocation[1] = currentMouseY;
            PadderCommon::mouseHelperObj.pivotPoint[0] = fullSpringDestX;
            PadderCommon::mouseHelperObj.pivotPoint[1] = fullSpringDestY;
        }
        else if (PadderCommon::mouseHelperObj.previousCursorLocation[0] == xmovecoor &&
                 PadderCommon::mouseHelperObj.previousCursorLocation[1] == ymovecoor)
        {
            PadderCommon::mouseHelperObj.springMouseMoving = false;
        }
        else
        {
            PadderCommon::mouseHelperObj.previousCursorLocation[0] = currentMouseX;
            PadderCommon::mouseHelperObj.previousCursorLocation[1] = currentMouseY;
            PadderCommon::mouseHelperObj.pivotPoint[0] = fullSpringDestX;
            PadderCommon::mouseHelperObj.pivotPoint[1] = fullSpringDestY;

            PadderCommon::mouseHelperObj.mouseTimer.start(
                        qMax(JoyButton::getMouseRefreshRate(),
                             JoyButton::getGamepadRefreshRate()) + 1);
        }
    }
    else
    {
        PadderCommon::mouseHelperObj.springMouseMoving = false;
        PadderCommon::mouseHelperObj.pivotPoint[0] = -1;
        PadderCommon::mouseHelperObj.pivotPoint[1] = -1;
    }
}

int X11KeySymToKeycode(QString key)
{
    int tempcode = 0;
#if defined (Q_OS_UNIX)

    BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();

    if (key.length() > 0)
    {
#ifdef WITH_XTEST
        if (handler->getIdentifier() == "xtest")
        {
            Display* display = X11Extras::getInstance()->display();
            tempcode = XKeysymToKeycode(display, XStringToKeysym(key.toUtf8().data()));
        }
#endif

#ifdef WITH_UINPUT
        if (handler->getIdentifier() == "uinput")
        {
            tempcode = UInputHelper::getInstance()->getVirtualKey(key);
        }
#endif
    }

#elif defined (Q_OS_WIN)
    if (key.length() > 0)
    {
        tempcode = WinExtras::getVirtualKey(key);
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
        BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();

#ifdef WITH_XTEST
        if (handler->getIdentifier() == "xtest")
        {
            Display* display = X11Extras::getInstance()->display();
            newkey = QString("0x%1").arg(keycode, 0, 16);
            QString tempkey = XKeysymToString(XkbKeycodeToKeysym(display, keycode, 0, 0));
            QString tempalias = X11Extras::getInstance()->getDisplayString(tempkey);
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
#endif

#ifdef WITH_UINPUT
        if (handler->getIdentifier() == "uinput")
        {
            QString tempalias = UInputHelper::getInstance()->getDisplayString(keycode);
            if (!tempalias.isEmpty())
            {
                newkey = tempalias;
            }
            else
            {
                newkey = QString("0x%1").arg(keycode, 0, 16);
            }
        }
#endif
    }

#elif defined (Q_OS_WIN)
    wchar_t buffer[50] = {0};

    QString tempalias = WinExtras::getDisplayString(keycode);
    if (!tempalias.isEmpty())
    {
        newkey = tempalias;
    }
    else
    {
        int scancode = WinExtras::scancodeFromVirtualKey(keycode, alias);

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
    Display* display = X11Extras::getInstance()->display();
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

    BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();

    if (handler->getIdentifier() == "xtest")
    {
        Display* display = X11Extras::getInstance()->display();
        unsigned int keycode = 0;
        if (keysym > 0)
        {
            keycode = XKeysymToKeycode(display, keysym);
        }
        newkey = keycodeToKeyString(keycode);
    }
    else if (handler->getIdentifier() == "uinput")
    {
        newkey = keycodeToKeyString(keysym);
    }

#else
    newkey = keycodeToKeyString(keysym, alias);
#endif
    return newkey;
}
