/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
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

#include "antkeymapper.h"
#include "globalvariables.h"
#include "joybuttonslot.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>

#include "x11extras.h"

XTestEventHandler::XTestEventHandler(QObject *parent)
    : BaseEventHandler(parent)
{
}

XTestEventHandler::~XTestEventHandler() {}

bool XTestEventHandler::init()
{
    X11Extras *instance = X11Extras::getInstance();

    if (instance != nullptr)
    {
        instance->x11ResetMouseAccelerationChange(GlobalVariables::X11Extras::xtestMouseDeviceName);
    }

    return true;
}

bool XTestEventHandler::cleanup() { return true; }

void XTestEventHandler::sendKeyboardEvent(JoyButtonSlot *slot, bool pressed)
{
    Display *display = X11Extras::getInstance()->display();

    JoyButtonSlot::JoySlotInputAction device = slot->getSlotMode();

    int code = slot->getSlotCode();

    if (device == JoyButtonSlot::JoyKeyboard)
    {
        int tempcode = XKeysymToKeycode(display, static_cast<KeySym>(code));

        if (tempcode > 0)
        {
            XTestFakeKeyEvent(display, tempcode, pressed, 0);
            XFlush(display);
        }
    }
}

void XTestEventHandler::sendMouseButtonEvent(JoyButtonSlot *slot, bool pressed)
{
    Display *display = X11Extras::getInstance()->display();
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
    Display *display = X11Extras::getInstance()->display();
    XTestFakeRelativeMotionEvent(display, xDis, yDis, 0);
    XFlush(display);
}

void XTestEventHandler::sendMouseAbsEvent(int xDis, int yDis, int screen)
{
    Display *display = X11Extras::getInstance()->display();
    XTestFakeMotionEvent(display, screen, xDis, yDis, 0);
    XFlush(display);
}

QString XTestEventHandler::getName() { return QString("XTest"); }

QString XTestEventHandler::getIdentifier() { return QString("xtest"); }

void XTestEventHandler::sendTextEntryEvent(QString maintext)
{
    AntKeyMapper *mapper = AntKeyMapper::getInstance();

    if ((mapper != nullptr) && mapper->getKeyMapper())
    {
        Display *display = X11Extras::getInstance()->display();
        QtX11KeyMapper *keymapper = qobject_cast<QtX11KeyMapper *>(mapper->getKeyMapper());

        for (int i = 0; i < maintext.size(); i++)
        {
            QtX11KeyMapper::charKeyInformation temp = keymapper->getCharKeyInformation(maintext.at(i));
            int tempcode = XKeysymToKeycode(display, static_cast<KeySym>(temp.virtualkey));

            if (tempcode > 0)
            {
                QList<int> tempList;

                if ((temp.modifiers != Qt::NoModifier) && temp.modifiers.testFlag(Qt::ShiftModifier))
                {
                    int shiftcode = XKeysymToKeycode(display, XK_Shift_L);
                    int modifiercode = shiftcode;
                    XTestFakeKeyEvent(display, modifiercode, 1, 0);
                    tempList.append(modifiercode);
                }

                if ((temp.modifiers != Qt::NoModifier) && temp.modifiers.testFlag(Qt::ControlModifier))
                {
                    int controlcode = XKeysymToKeycode(display, XK_Control_L);
                    int modifiercode = controlcode;
                    XTestFakeKeyEvent(display, modifiercode, 1, 0);
                    tempList.append(modifiercode);
                }

                if ((temp.modifiers != Qt::NoModifier) && temp.modifiers.testFlag(Qt::AltModifier))
                {
                    int altcode = XKeysymToKeycode(display, XK_Alt_L);
                    int modifiercode = altcode;
                    XTestFakeKeyEvent(display, modifiercode, 1, 0);
                    tempList.append(modifiercode);
                }

                if ((temp.modifiers != Qt::NoModifier) && temp.modifiers.testFlag(Qt::MetaModifier))
                {
                    int metacode = XKeysymToKeycode(display, XK_Meta_L);
                    int modifiercode = metacode;
                    XTestFakeKeyEvent(display, modifiercode, 1, 0);
                    tempList.append(modifiercode);
                }

                XTestFakeKeyEvent(display, tempcode, 1, 0);
                tempList.append(tempcode);

                XFlush(display);

                if (tempList.size() > 0)
                {
                    for (auto iter = tempList.crbegin(); iter != tempList.crend(); ++iter)
                    {
                        int currentcode = *iter;
                        XTestFakeKeyEvent(display, currentcode, 0, 0);
                    }

                    XFlush(display);
                }
            }
        }
    }
}

void XTestEventHandler::sendMouseSpringEvent(int xDis, int yDis, int width, int height)
{
    Q_UNUSED(xDis);
    Q_UNUSED(yDis);
    Q_UNUSED(width);
    Q_UNUSED(height);
}

void XTestEventHandler::printPostMessages() {}
