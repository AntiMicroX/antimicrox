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

#include "xtesteventhandler.h"

#include "joybuttonslot.h"
#include "antkeymapper.h"
#include "messagehandler.h"

#include <QApplication>
#include <QDebug>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XTest.h>

#include "x11extras.h"





XTestEventHandler::XTestEventHandler(QObject *parent) :
    BaseEventHandler(parent)
{
qInstallMessageHandler(MessageHandler::myMessageOutput);
}


XTestEventHandler::~XTestEventHandler()
{
qInstallMessageHandler(MessageHandler::myMessageOutput);
}


bool XTestEventHandler::init()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);
    X11Extras *instance = X11Extras::getInstance();
    if (instance != nullptr)
    {
        instance->x11ResetMouseAccelerationChange(X11Extras::xtestMouseDeviceName);
    }

    return true;
}

bool XTestEventHandler::cleanup()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);
    return true;
}

void XTestEventHandler::sendKeyboardEvent(JoyButtonSlot *slot, bool pressed)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);
    Display* display = X11Extras::getInstance()->display();
    JoyButtonSlot::JoySlotInputAction device = slot->getSlotMode();
    int code = slot->getSlotCode();

    if (device == JoyButtonSlot::JoyKeyboard)
    {
        int tempcode = XKeysymToKeycode(display, static_cast<KeySym>(code));
        if (tempcode > 0)
        {
            XTestFakeKeyEvent(display, static_cast<unsigned int>(tempcode), pressed, 0);
            XFlush(display);
        }
    }
}

void XTestEventHandler::sendMouseButtonEvent(JoyButtonSlot *slot, bool pressed)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);
    Display* display = X11Extras::getInstance()->display();
    JoyButtonSlot::JoySlotInputAction device = slot->getSlotMode();
    int code = slot->getSlotCode();

    if (device == JoyButtonSlot::JoyMouseButton)
    {
        XTestFakeButtonEvent(display, static_cast<unsigned int>(code), pressed, 0);
        XFlush(display);
    }
}

void XTestEventHandler::sendMouseEvent(int xDis, int yDis)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);
    Display* display = X11Extras::getInstance()->display();
    XTestFakeRelativeMotionEvent(display, xDis, yDis, 0);
    XFlush(display);
}

void XTestEventHandler::sendMouseAbsEvent(int xDis, int yDis, int screen)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);
    Display* display = X11Extras::getInstance()->display();
    XTestFakeMotionEvent(display, screen, xDis, yDis, 0);
    XFlush(display);
}

QString XTestEventHandler::getName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);
    return QString("XTest");
}

QString XTestEventHandler::getIdentifier()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);
    return QString("xtest");
}

void XTestEventHandler::sendTextEntryEvent(QString maintext)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);
    AntKeyMapper *mapper = AntKeyMapper::getInstance();

    if ((mapper != nullptr) && mapper->getKeyMapper())
    {
        Display* display = X11Extras::getInstance()->display();
        QtX11KeyMapper *keymapper = qobject_cast<QtX11KeyMapper*>(mapper->getKeyMapper()); // static_cast

        for (int i=0; i < maintext.size(); i++)
        {
            QtX11KeyMapper::charKeyInformation temp = keymapper->getCharKeyInformation(maintext.at(i));
            int tempcode = XKeysymToKeycode(display, static_cast<KeySym>(temp.virtualkey));
            if (tempcode > 0)
            {
                QList<int> tempList;

                if (temp.modifiers != Qt::NoModifier)
                {
                    if (temp.modifiers.testFlag(Qt::ShiftModifier))
                    {
                        int shiftcode = 0;

                        if (shiftcode == 0)
                        {
                            shiftcode = XKeysymToKeycode(display, XK_Shift_L);
                        }

                        int modifiercode = shiftcode;
                        XTestFakeKeyEvent(display, static_cast<unsigned int>(modifiercode), 1, 0);
                        tempList.append(modifiercode);
                    }

                    if (temp.modifiers.testFlag(Qt::ControlModifier))
                    {
                        int controlcode = 0;

                        if (controlcode == 0)
                        {
                            controlcode = XKeysymToKeycode(display, XK_Control_L);
                        }

                        int modifiercode = controlcode;
                        XTestFakeKeyEvent(display, static_cast<unsigned int>(modifiercode), 1, 0);
                        tempList.append(modifiercode);
                    }

                    if (temp.modifiers.testFlag(Qt::AltModifier))
                    {
                        int altcode = 0;

                        if (altcode == 0)
                        {
                            altcode = XKeysymToKeycode(display, XK_Alt_L);
                        }

                        int modifiercode = altcode;
                        XTestFakeKeyEvent(display, static_cast<unsigned int>(modifiercode), 1, 0);
                        tempList.append(modifiercode);
                    }

                    if (temp.modifiers.testFlag(Qt::MetaModifier))
                    {
                        int metacode = 0;

                        if (metacode == 0)
                        {
                            metacode = XKeysymToKeycode(display, XK_Meta_L);
                        }

                        int modifiercode = metacode;
                        XTestFakeKeyEvent(display, static_cast<unsigned int>(modifiercode), 1, 0);
                        tempList.append(modifiercode);
                    }
                }

                XTestFakeKeyEvent(display, static_cast<unsigned int>(tempcode), 1, 0);
                tempList.append(tempcode);

                XFlush(display);

                if (tempList.size() > 0)
                {
                    QListIterator<int> tempiter(tempList);
                    tempiter.toBack();
                    while (tempiter.hasPrevious())
                    {
                        int currentcode = tempiter.previous();
                        XTestFakeKeyEvent(display, static_cast<unsigned int>(currentcode), 0, 0);
                    }

                    XFlush(display);
                }
            }
        }
    }
}


void XTestEventHandler::sendMouseSpringEvent(int xDis, int yDis, int width, int height) {

    qInstallMessageHandler(MessageHandler::myMessageOutput);
    Q_UNUSED(xDis);
    Q_UNUSED(yDis);
    Q_UNUSED(width);
    Q_UNUSED(height);
}


void XTestEventHandler::sendMouseSpringEvent(int, int) {

    qInstallMessageHandler(MessageHandler::myMessageOutput);

}


void XTestEventHandler::printPostMessages() {

    qInstallMessageHandler(MessageHandler::myMessageOutput);

}


