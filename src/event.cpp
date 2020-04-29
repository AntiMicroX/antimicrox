/* antimicroX Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail.com>
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


#include <QVariant>
#include <QApplication>
#include <cmath>
#include <QFileInfo>
#include <QStringList>
#include <QCursor>
#include <QDesktopWidget>
#include <QProcess>
#include <QDebug>
#include <QMessageBox>

#include "event.h"
#include "globalvariables.h"
#include "messagehandler.h"
#include "eventhandlerfactory.h"
#include "joybutton.h"


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



// TODO: Implement function for determining final mouse pointer position
// based around a fixed bounding box resolution.
void fakeAbsMouseCoordinates(double springX, double springY,
                             int width, int height,
                             int &finalx, int &finaly, int screen)
{

    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int screenWidth = 0;
    int screenHeight = 0;
    int screenMidwidth = 0;
    int screenMidheight = 0;

    int destSpringWidth = 0;
    int destSpringHeight = 0;
    int destMidWidth = 0;
    int destMidHeight = 0;

    QRect deskRect = PadderCommon::mouseHelperObj.getDesktopWidget()
            ->screenGeometry(screen);

    screenWidth = deskRect.width();
    screenHeight = deskRect.height();

    screenMidwidth = screenWidth / 2;
    screenMidheight = screenHeight / 2;

    if ((width >= 2) && (height >= 2))
    {
        destSpringWidth = qMin(width, screenWidth);
        destSpringHeight = qMin(height, screenHeight);
    }
    else
    {
        destSpringWidth = screenWidth;
        destSpringHeight = screenHeight;
    }

    destMidWidth = destSpringWidth / 2;
    destMidHeight = destSpringHeight / 2;

    finalx = (screenMidwidth + (springX * destMidWidth) + deskRect.x());
    finaly = (screenMidheight + (springY * destMidHeight) + deskRect.y());
}

// Create the event used by the operating system.
void sendevent(JoyButtonSlot *slot, bool pressed)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButtonSlot::JoySlotInputAction device = slot->getSlotMode();

    if (device == JoyButtonSlot::JoyKeyboard)
    {
        EventHandlerFactory::getInstance()->handler()->sendKeyboardEvent(slot, pressed);
    }
    else if (device == JoyButtonSlot::JoyMouseButton)
    {
        EventHandlerFactory::getInstance()->handler()->sendMouseButtonEvent(slot, pressed);
    }
    else if ((device == JoyButtonSlot::JoyTextEntry) && pressed && !slot->getTextData().isEmpty())
    {
        EventHandlerFactory::getInstance()->handler()->sendTextEntryEvent(slot->getTextData());
    }
    else if ((device == JoyButtonSlot::JoyExecute) && pressed && !slot->getTextData().isEmpty())
    {
        if (slot->getExtraData().canConvert<QString>())
        {
            QString argumentsString = slot->getExtraData().toString();
            //QStringList argumentsTempList(PadderCommon::parseArgumentsString(argumentsString));
            bool success = QProcess::startDetached(QString("%1 %2 %3").arg(detectedScriptExt(slot->getTextData())).arg(slot->getTextData()).arg(argumentsString));
            if (!success) qDebug() << "Script cannot be executed";
        }
        else
        {
            bool success = QProcess::startDetached(QString("%1 %2").arg(detectedScriptExt(slot->getTextData())).arg(slot->getTextData()));
            if (!success) qDebug() << "Script cannot be executed";

        }
    }
}


QString detectedScriptExt(QString file)
{
    QFileInfo fileinfo(file);
    QFile inputFile(file);

    QString firstLine = QString();

    if (inputFile.open(QIODevice::ReadOnly))
    {
       QTextStream in(&inputFile);
       firstLine = in.readLine();
       inputFile.close();
    }

    /*
     * shell scripts work, but I am not sure about other extensions
     * neither start method form QProcess nor QProcess::startDetached
     * don't work as expected
     * need to find another way
     */

    if (fileinfo.completeSuffix() == "sh" || firstLine.contains("bin/bash")) return "/bin/sh";
    else if (fileinfo.completeSuffix() == "py" && firstLine.contains("python3")) return "python3";
    else if (fileinfo.completeSuffix() == "py" && firstLine.contains("python")) return "python";
    else if (fileinfo.completeSuffix() == "pl" || firstLine.contains("usr/bin/perl")) return "perl";
    else if (fileinfo.completeSuffix() == "php" || firstLine.contains("/php")) return "php";
    else if (fileinfo.completeSuffix() == "rb" || firstLine.contains("ruby")) return "ruby";

    // when run "chmod +x file_name"
    return "";
}

// Create the relative mouse event used by the operating system.
void sendevent(int code1, int code2)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    EventHandlerFactory::getInstance()->handler()->sendMouseEvent(code1, code2);
}

// TODO: Re-implement spring event generation to simplify the process
// and reduce overhead. Refactor old function to only be used when an absmouse
// position must be faked.
void sendSpringEventRefactor(PadderCommon::springModeInfo *fullSpring,
                     PadderCommon::springModeInfo *relativeSpring,
                     int* const mousePosX, int* const mousePosY)
{
    Q_UNUSED(relativeSpring)
    Q_UNUSED(mousePosX)
    Q_UNUSED(mousePosY)

    qInstallMessageHandler(MessageHandler::myMessageOutput);

    PadderCommon::mouseHelperObj.mouseTimer.stop();

    if (fullSpring != nullptr)
    {
        int xmovecoor = 0;
        int ymovecoor = 0;

        double displacementX = 0.0;
        double displacementY = 0.0;

        PadderCommon::mouseHelperObj.mouseTimer.stop();
        BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();

        if ((fullSpring->screen >= -1) &&
            (fullSpring->screen >= PadderCommon::mouseHelperObj.getDesktopWidget()->screenCount()))
        {
            fullSpring->screen = -1;
        }

        int springWidth = fullSpring->width;
        int springHeight = fullSpring->height;

        if ((springWidth >= 2) && (springHeight >= 2))
        {
            displacementX = fullSpring->displacementX;
            displacementY = fullSpring->displacementY;
        }
        else
        {
            displacementX = fullSpring->displacementX;
            displacementY = fullSpring->displacementY;
        }

        if (relativeSpring && (relativeSpring->width >= 2) && (relativeSpring->height >= 2))
        {
            int pivotX = 0;
            int pivotY = 0;

            if (PadderCommon::mouseHelperObj.pivotPoint[0] != -1)
                pivotX = PadderCommon::mouseHelperObj.pivotPoint[0];

            if (PadderCommon::mouseHelperObj.pivotPoint[1] != -1)
                pivotY = PadderCommon::mouseHelperObj.pivotPoint[1];

            if ((pivotX >= 0) && (pivotY >= 0))
            {

                // Find a use for this routine in this context.
                int destRelativeWidth = relativeSpring->width;
                int destRelativeHeight = relativeSpring->height;
                int xRelativeMoovCoor = 0;
                int yRelativeMoovCoor = 0;

                if (relativeSpring->displacementX >= -1.0)
                    xRelativeMoovCoor = (relativeSpring->displacementX * destRelativeWidth) / 2;

                if (relativeSpring->displacementY >= -1.0)
                    yRelativeMoovCoor = (relativeSpring->displacementY * destRelativeHeight) / 2;

                xmovecoor += xRelativeMoovCoor;
                ymovecoor += yRelativeMoovCoor;
            }
        }

        if (handler->getIdentifier() == "xtest")
        {
            fakeAbsMouseCoordinates(displacementX, displacementY,
                                    springWidth, springHeight, xmovecoor, ymovecoor,
                                    fullSpring->screen);
        }
        else if (handler->getIdentifier() == "uinput")
        {
            fakeAbsMouseCoordinates(displacementX, displacementY,
                                    springWidth, springHeight, xmovecoor, ymovecoor,
                                    fullSpring->screen);
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

    qInstallMessageHandler(MessageHandler::myMessageOutput);

    PadderCommon::mouseHelperObj.mouseTimer.stop();

    if (((fullSpring->displacementX >= -2.0) && (fullSpring->displacementX <= 1.0) &&
        (fullSpring->displacementY >= -2.0) && (fullSpring->displacementY <= 1.0)) ||
        (relativeSpring && ((relativeSpring->displacementX >= -2.0) && (relativeSpring->displacementX <= 1.0) &&
        (relativeSpring->displacementY >= -2.0) && (relativeSpring->displacementY <= 1.0))))
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

        if ((fullSpring->screen >= -1) &&
            (fullSpring->screen >= PadderCommon::mouseHelperObj.getDesktopWidget()->screenCount()))
        {
            fullSpring->screen = -1;
        }

        QRect deskRect = PadderCommon::mouseHelperObj.getDesktopWidget()
                ->screenGeometry(fullSpring->screen);

        width = deskRect.width();
        height = deskRect.height();

#if defined(WITH_X11)
        QPoint currentPoint;
        if (QApplication::platformName() == QStringLiteral("xcb"))
        {
            currentPoint = X11Extras::getInstance()->getPos();
        }
        else
        {
            currentPoint = QCursor::pos();
        }
#endif

        currentMouseX = currentPoint.x();
        currentMouseY = currentPoint.y();

        midwidth = width / 2;
        midheight = height / 2;

        int springWidth = fullSpring->width;
        int springHeight = fullSpring->height;

        if ((springWidth >= 2) && (springHeight >= 2))
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

        int pivotX = currentMouseX;
        int pivotY = currentMouseY;

        if (relativeSpring != nullptr)
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

        int fullSpringDestX = xmovecoor;
        int fullSpringDestY = ymovecoor;

        int destRelativeWidth = 0;
        int destRelativeHeight = 0;

        if ((relativeSpring != nullptr) && (relativeSpring->width >= 2) && (relativeSpring->height >= 2))
        {
            destRelativeWidth = relativeSpring->width;
            destRelativeHeight = relativeSpring->height;

            int xRelativeMoovCoor = 0;
            int yRelativeMoovCoor = 0;

            if (relativeSpring->displacementX >= -1.0)
                xRelativeMoovCoor = (relativeSpring->displacementX * destRelativeWidth) / 2;

            if (relativeSpring->displacementY >= -1.0)
                yRelativeMoovCoor = (relativeSpring->displacementY * destRelativeHeight) / 2;

            xmovecoor += xRelativeMoovCoor;
            ymovecoor += yRelativeMoovCoor;
        }

        if (mousePosX) *mousePosX = xmovecoor;

        if (mousePosY) *mousePosY = ymovecoor;

        if ((xmovecoor != currentMouseX) || (ymovecoor != currentMouseY))
        {
            double diffx = abs(currentMouseX - xmovecoor);
            double diffy = abs(currentMouseY - ymovecoor);

            // If either position is set to center, force update.
            if ((xmovecoor == (deskRect.x() + midwidth)) || (ymovecoor == (deskRect.y() + midheight)))
            {

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
            }
            else if (!PadderCommon::mouseHelperObj.springMouseMoving && relativeSpring &&
                ((relativeSpring->displacementX >= -1.0) || (relativeSpring->displacementY >= -1.0)) &&
                ((diffx >= (destRelativeWidth * .013)) || (diffy >= (destRelativeHeight * .013))))
            {
                PadderCommon::mouseHelperObj.springMouseMoving = true;
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

                PadderCommon::mouseHelperObj.mouseTimer.start(
                            qMax(GlobalVariables::JoyButton::mouseRefreshRate,
                                 GlobalVariables::JoyButton::gamepadRefreshRate) + 1);
            }
            else if (!PadderCommon::mouseHelperObj.springMouseMoving &&
                     ((diffx >= (destSpringWidth * .013)) || (diffy >= (destSpringHeight * .013))))
            {
                PadderCommon::mouseHelperObj.springMouseMoving = true;
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


                PadderCommon::mouseHelperObj.mouseTimer.start(
                            qMax(GlobalVariables::JoyButton::mouseRefreshRate,
                                 GlobalVariables::JoyButton::gamepadRefreshRate) + 1);
            }

            else if (PadderCommon::mouseHelperObj.springMouseMoving && ((diffx < 2) && (diffy < 2)))
            {
                PadderCommon::mouseHelperObj.springMouseMoving = false;
            }

            else if (PadderCommon::mouseHelperObj.springMouseMoving)
            {
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

                PadderCommon::mouseHelperObj.mouseTimer.start(
                            qMax(GlobalVariables::JoyButton::mouseRefreshRate,
                                 GlobalVariables::JoyButton::gamepadRefreshRate) + 1);
            }


            PadderCommon::mouseHelperObj.previousCursorLocation[0] = currentMouseX;
            PadderCommon::mouseHelperObj.previousCursorLocation[1] = currentMouseY;
            PadderCommon::mouseHelperObj.pivotPoint[0] = fullSpringDestX;
            PadderCommon::mouseHelperObj.pivotPoint[1] = fullSpringDestY;
        }
        else if ((PadderCommon::mouseHelperObj.previousCursorLocation[0] == xmovecoor) &&
                 (PadderCommon::mouseHelperObj.previousCursorLocation[1] == ymovecoor))
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
                        qMax(GlobalVariables::JoyButton::mouseRefreshRate,
                             GlobalVariables::JoyButton::gamepadRefreshRate) + 1);
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int tempcode = 0;

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

    return tempcode;
}


QString keycodeToKeyString(int keycode, int alias)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString newkey = QString();

    Q_UNUSED(alias)

    qDebug() << "keycode is: " << keycode;

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
            QString tempkey = XKeysymToString(XkbKeycodeToKeysym(display, static_cast<KeyCode>(keycode), 0, 0));
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
                int numchars = XLookupString(&tempevent, tempstring, bitestoreturn, nullptr, nullptr);

                if (numchars > 0)
                {
                    tempstring[numchars] = '\0';
                    newkey = QString::fromUtf8(tempstring);

                    qDebug() << "NEWKEY:" << newkey << endl;
                    qDebug() << "NEWKEY LEGNTH:" << numchars << endl;
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

            if (!tempalias.isEmpty()) newkey = tempalias;
            else newkey = QString("0x%1").arg(keycode, 0, 16);
        }
#endif
    }

    return newkey;
}

int X11KeyCodeToX11KeySym(int keycode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    #ifdef WITH_X11

    Display* display = X11Extras::getInstance()->display();
    return XkbKeycodeToKeysym(display, static_cast<KeyCode>(keycode), 0, 0);

    #else

    Q_UNUSED(keycode)
    return 0;

    #endif
}

QString keysymToKeyString(int keysym, int alias)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString newkey = QString();

    #ifdef WITH_X11
    Q_UNUSED(alias)

    BaseEventHandler *handler = EventHandlerFactory::getInstance()->handler();

    if (handler->getIdentifier() == "xtest")
    {
        Display* display = X11Extras::getInstance()->display();
        int keycode = 0;

        if (keysym > 0)
            keycode = XKeysymToKeycode(display, static_cast<KeySym>(keysym));

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

void sendKeybEvent(JoyButtonSlot *slot, bool pressed)
{
    EventHandlerFactory::getInstance()->handler()->sendKeyboardEvent(slot, pressed);
}
