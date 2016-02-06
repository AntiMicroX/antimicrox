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

#include <QApplication>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XTest.h>
#include <x11extras.h>
#include <antkeymapper.h>

XTestEventHandler::XTestEventHandler(QObject *parent) :
    BaseEventHandler(parent)
{
}

bool XTestEventHandler::init()
{
    X11Extras *instance = X11Extras::getInstance();
    if (instance)
    {
        instance->x11ResetMouseAccelerationChange(X11Extras::xtestMouseDeviceName);
    }

    return true;
}

bool XTestEventHandler::cleanup()
{
    return true;
}

void XTestEventHandler::sendKeyboardEvent(JoyButtonSlot *slot, bool pressed)
{
    Display* display = X11Extras::getInstance()->display();
    JoyButtonSlot::JoySlotInputAction device = slot->getSlotMode();
    int code = slot->getSlotCode();

    if (device == JoyButtonSlot::JoyKeyboard)
    {
        unsigned int tempcode = XKeysymToKeycode(display, code);
        if (tempcode > 0)
        {
            XTestFakeKeyEvent(display, tempcode, pressed, 0);
            XFlush(display);
        }
    }
}

void XTestEventHandler::sendMouseButtonEvent(JoyButtonSlot *slot, bool pressed)
{
    Display* display = X11Extras::getInstance()->display();
    JoyButtonSlot::JoySlotInputAction device = slot->getSlotMode();
    int code = slot->getSlotCode();

    if (device == JoyButtonSlot::JoyMouseButton)
    {
        XTestFakeButtonEvent(display, code, pressed, 0);
        XFlush(display);
    }
}

void XTestEventHandler::sendMouseEvent(int xDis, int yDis)
{
    Display* display = X11Extras::getInstance()->display();
    XTestFakeRelativeMotionEvent(display, xDis, yDis, 0);
    XFlush(display);
}

void XTestEventHandler::sendMouseAbsEvent(int xDis, int yDis, int screen)
{
    Display* display = X11Extras::getInstance()->display();
    XTestFakeMotionEvent(display, screen, xDis, yDis, 0);
    XFlush(display);
}

QString XTestEventHandler::getName()
{
    return QString("XTest");
}

QString XTestEventHandler::getIdentifier()
{
    return QString("xtest");
}

void XTestEventHandler::sendTextEntryEvent(QString maintext)
{
    AntKeyMapper *mapper = AntKeyMapper::getInstance();

    // Populated as needed.
    unsigned int shiftcode = 0;
    unsigned int controlcode = 0;
    unsigned int metacode = 0;
    unsigned int altcode = 0;

    if (mapper && mapper->getKeyMapper())
    {
        //Qt::KeyboardModifiers originalModifiers = Qt::KeyboardModifiers(QApplication::keyboardModifiers());
        //Qt::KeyboardModifiers currentModifiers = Qt::KeyboardModifiers(QApplication::keyboardModifiers());
        Display* display = X11Extras::getInstance()->display();
        QtX11KeyMapper *keymapper = static_cast<QtX11KeyMapper*>(mapper->getKeyMapper());

        for (int i=0; i < maintext.size(); i++)
        {
            QtX11KeyMapper::charKeyInformation temp = keymapper->getCharKeyInformation(maintext.at(i));
            unsigned int tempcode = XKeysymToKeycode(display, temp.virtualkey);
            if (tempcode > 0)
            {
                QList<unsigned int> tempList;

                if (temp.modifiers != Qt::NoModifier)
                {
                    if (temp.modifiers.testFlag(Qt::ShiftModifier))
                    {
                        if (shiftcode == 0)
                        {
                            shiftcode = XKeysymToKeycode(display, XK_Shift_L);
                        }

                        unsigned int modifiercode = shiftcode;
                        XTestFakeKeyEvent(display, modifiercode, 1, 0);
                        //currentModifiers |= Qt::ShiftModifier;
                        tempList.append(modifiercode);
                    }

                    if (temp.modifiers.testFlag(Qt::ControlModifier))
                    {
                        if (controlcode == 0)
                        {
                            controlcode = XKeysymToKeycode(display, XK_Control_L);
                        }

                        unsigned int modifiercode = controlcode;
                        XTestFakeKeyEvent(display, modifiercode, 1, 0);
                        //currentModifiers |= Qt::ControlModifier;
                        tempList.append(modifiercode);
                    }

                    if (temp.modifiers.testFlag(Qt::AltModifier))
                    {
                        if (altcode == 0)
                        {
                            altcode = XKeysymToKeycode(display, XK_Alt_L);
                        }

                        unsigned int modifiercode = altcode;
                        XTestFakeKeyEvent(display, modifiercode, 1, 0);
                        //currentModifiers |= Qt::AltModifier;
                        tempList.append(modifiercode);
                    }

                    if (temp.modifiers.testFlag(Qt::MetaModifier))
                    {
                        if (metacode == 0)
                        {
                            metacode = XKeysymToKeycode(display, XK_Meta_L);
                        }

                        unsigned int modifiercode = metacode;
                        XTestFakeKeyEvent(display, modifiercode, 1, 0);
                        //currentModifiers |= Qt::MetaModifier;
                        tempList.append(modifiercode);
                    }
                }

                XTestFakeKeyEvent(display, tempcode, 1, 0);
                tempList.append(tempcode);

                XFlush(display);

                if (tempList.size() > 0)
                {
                    QListIterator<unsigned int> tempiter(tempList);
                    tempiter.toBack();
                    while (tempiter.hasPrevious())
                    {
                        unsigned int currentcode = tempiter.previous();
                        XTestFakeKeyEvent(display, currentcode, 0, 0);
                    }

                    XFlush(display);
                }
            }
        }

        // Perform a flush at the end.
        //XFlush(display);

        // Restore modifiers in place
        /*if (originalModifiers != currentModifiers)
        {

        }
        */
    }
}
