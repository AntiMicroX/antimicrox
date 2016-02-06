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

#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <cmath>

//#include <QDebug>
#include <QStringList>
#include <QStringListIterator>
#include <QFileInfo>
#include <QTimer>
#include <antkeymapper.h>
#include <logger.h>
#include <common.h>

static const QString mouseDeviceName = PadderCommon::mouseDeviceName;
static const QString keyboardDeviceName = PadderCommon::keyboardDeviceName;
static const QString springMouseDeviceName = PadderCommon::springMouseDeviceName;

#ifdef WITH_X11
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
      #include <QApplication>
    #endif

  #include <x11extras.h>
#endif

#include "uinputeventhandler.h"

UInputEventHandler::UInputEventHandler(QObject *parent) :
    BaseEventHandler(parent)
{
    keyboardFileHandler = 0;
    mouseFileHandler = 0;
    springMouseFileHandler = 0;
}

UInputEventHandler::~UInputEventHandler()
{
    cleanup();
}

/**
 * @brief Initialize keyboard and mouse virtual devices. Each device will
 *     use its own file handle with various codes set to distinquish the two
 *     devices.
 * @return
 */
bool UInputEventHandler::init()
{
    bool result = true;

    // Open file handle for keyboard emulation.
    keyboardFileHandler = openUInputHandle();
    if (keyboardFileHandler > 0)
    {
        setKeyboardEvents(keyboardFileHandler);
        populateKeyCodes(keyboardFileHandler);
        createUInputKeyboardDevice(keyboardFileHandler);
    }
    else
    {
        result = false;
    }

    if (result)
    {
        // Open mouse file handle to use for relative mouse emulation.
        mouseFileHandler = openUInputHandle();
        if (mouseFileHandler > 0)
        {
            setRelMouseEvents(mouseFileHandler);
            createUInputMouseDevice(mouseFileHandler);
        }
        else
        {
            result = false;
        }
    }

    if (result)
    {
        // Open mouse file handle to use for absolute mouse emulation.
        springMouseFileHandler = openUInputHandle();
        if (springMouseFileHandler > 0)
        {
            setSpringMouseEvents(springMouseFileHandler);
            createUInputSpringMouseDevice(springMouseFileHandler);
        }
        else
        {
            result = false;
        }
    }

#ifdef WITH_X11
    if (result)
    {
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))

    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
    #endif
    // Some time needs to elapse after device creation before changing
    // pointer settings. Otherwise, settings will not take effect.
    QTimer::singleShot(2000, this, SLOT(x11ResetMouseAccelerationChange()));
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    }
    #endif
    }
#endif

    return result;
}

#ifdef WITH_X11
void UInputEventHandler::x11ResetMouseAccelerationChange()
{
    if (X11Extras::getInstance())
    {
        X11Extras::getInstance()->x11ResetMouseAccelerationChange();
    }
}
#endif

bool UInputEventHandler::cleanup()
{
    if (keyboardFileHandler > 0)
    {
        closeUInputDevice(keyboardFileHandler);
        keyboardFileHandler = 0;
    }

    if (mouseFileHandler > 0)
    {
        closeUInputDevice(mouseFileHandler);
        mouseFileHandler = 0;
    }

    if (springMouseFileHandler > 0)
    {
        closeUInputDevice(springMouseFileHandler);
        springMouseFileHandler = 0;
    }

    return true;
}

void UInputEventHandler::sendKeyboardEvent(JoyButtonSlot *slot, bool pressed)
{
    JoyButtonSlot::JoySlotInputAction device = slot->getSlotMode();
    int code = slot->getSlotCode();

    if (device == JoyButtonSlot::JoyKeyboard)
    {
        write_uinput_event(keyboardFileHandler, EV_KEY, code, pressed ? 1 : 0);
    }
}

void UInputEventHandler::sendMouseButtonEvent(JoyButtonSlot *slot, bool pressed)
{
    JoyButtonSlot::JoySlotInputAction device = slot->getSlotMode();
    int code = slot->getSlotCode();

    if (device == JoyButtonSlot::JoyMouseButton)
    {
        if (code <= 3)
        {
            unsigned int tempcode = BTN_LEFT;
            switch (code)
            {
                case 3:
                {
                    tempcode = BTN_RIGHT;
                    break;
                }
                case 2:
                {
                    tempcode = BTN_MIDDLE;
                    break;
                }
                case 1:
                default:
                {
                    tempcode = BTN_LEFT;
                }
            }

            write_uinput_event(mouseFileHandler, EV_KEY, tempcode, pressed ? 1 : 0);
        }
        else if (code == 4)
        {
            if (pressed)
            {
                write_uinput_event(mouseFileHandler, EV_REL, REL_WHEEL, 1);
            }

        }
        else if (code == 5)
        {
            if (pressed)
            {
                write_uinput_event(mouseFileHandler, EV_REL, REL_WHEEL, -1);
            }
        }
        else if (code == 6)
        {
            if (pressed)
            {
                write_uinput_event(mouseFileHandler, EV_REL, REL_HWHEEL, 1);
            }
        }
        else if (code == 7)
        {
            if (pressed)
            {
                write_uinput_event(mouseFileHandler, EV_REL, REL_HWHEEL, -1);
            }
        }
        else if (code == 8)
        {
            write_uinput_event(mouseFileHandler, EV_KEY, BTN_SIDE, pressed ? 1 : 0);
        }
        else if (code == 9)
        {
            write_uinput_event(mouseFileHandler, EV_KEY, BTN_EXTRA, pressed ? 1 : 0);
        }
    }
}

void UInputEventHandler::sendMouseEvent(int xDis, int yDis)
{
    write_uinput_event(mouseFileHandler, EV_REL, REL_X, xDis, false);
    write_uinput_event(mouseFileHandler, EV_REL, REL_Y, yDis);
}

void UInputEventHandler::sendMouseAbsEvent(int xDis, int yDis, int screen)
{
    Q_UNUSED(screen);

    write_uinput_event(springMouseFileHandler, EV_ABS, ABS_X, xDis, false);
    write_uinput_event(springMouseFileHandler, EV_ABS, ABS_Y, yDis);
}

void UInputEventHandler::sendMouseSpringEvent(unsigned int xDis, unsigned int yDis,
                                              unsigned int width, unsigned int height)
{
    if (width > 0 && height > 0)
    {
        double midwidth = static_cast<double>(width) / 2.0;
        double midheight = static_cast<double>(height) / 2.0;

        int fx = ceil(32767 * ((xDis - midwidth) / midwidth));
        int fy = ceil(32767 * ((yDis - midheight) / midheight));
        sendMouseAbsEvent(fx, fy, -1);
        //write_uinput_event(springMouseFileHandler, EV_ABS, ABS_X, fx, false);
        //write_uinput_event(springMouseFileHandler, EV_ABS, ABS_Y, fy);
    }
}

void UInputEventHandler::sendMouseSpringEvent(int xDis, int yDis)
{
    if (xDis >= -1.0 && xDis <= 1.0 &&
        yDis >= -1.0 && yDis <= 1.0)
    {
        int fx = ceil(32767 * xDis);
        int fy = ceil(32767 * yDis);
        sendMouseAbsEvent(fx, fy, -1);
    }
}

int UInputEventHandler::openUInputHandle()
{
    int filehandle = -1;

    QStringList locations;
    locations.append("/dev/input/uinput");
    locations.append("/dev/uinput");
    locations.append("/dev/misc/uinput");

    QString possibleLocation;
    QStringListIterator iter(locations);
    while (iter.hasNext())
    {
        QString temp = iter.next();
        QFileInfo tempFileInfo(temp);
        if (tempFileInfo.exists())
        {
            possibleLocation = temp;
            iter.toBack();
        }
    }

    if (possibleLocation.isEmpty())
    {
        lastErrorString = tr("Could not find a valid uinput device file.\n"
                             "Please check that you have the uinput module loaded.\n"
                             "lsmod | grep uinput");
    }
    else
    {
        QByteArray tempArray = possibleLocation.toUtf8();
        filehandle = open(tempArray.constData(), O_WRONLY | O_NONBLOCK);
        if (filehandle < 0)
        {
            lastErrorString = tr("Could not open uinput device file\n"
                                 "Please check that you have permission to write to the device");
            lastErrorString.append("\n").append(possibleLocation);
        }
        else
        {
            uinputDeviceLocation = possibleLocation;
        }
    }

    return filehandle;
}


void UInputEventHandler::setKeyboardEvents(int filehandle)
{
    int result = 0;
    result = ioctl(filehandle, UI_SET_EVBIT, EV_KEY);
    result = ioctl(filehandle, UI_SET_EVBIT, EV_SYN);
}

void UInputEventHandler::setRelMouseEvents(int filehandle)
{
    int result = 0;
    result = ioctl(filehandle, UI_SET_EVBIT, EV_KEY);
    result = ioctl(filehandle, UI_SET_EVBIT, EV_SYN);
    result = ioctl(filehandle, UI_SET_EVBIT, EV_REL);

    result = ioctl(filehandle, UI_SET_RELBIT, REL_X);
    result = ioctl(filehandle, UI_SET_RELBIT, REL_Y);
    result = ioctl(filehandle, UI_SET_RELBIT, REL_WHEEL);
    result = ioctl(filehandle, UI_SET_RELBIT, REL_HWHEEL);

    result = ioctl(filehandle, UI_SET_KEYBIT, BTN_LEFT);
    result = ioctl(filehandle, UI_SET_KEYBIT, BTN_RIGHT);
    result = ioctl(filehandle, UI_SET_KEYBIT, BTN_MIDDLE);
    result = ioctl(filehandle, UI_SET_KEYBIT, BTN_SIDE);
    result = ioctl(filehandle, UI_SET_KEYBIT, BTN_EXTRA);
}

void UInputEventHandler::setSpringMouseEvents(int filehandle)
{
    int result = 0;
    result = ioctl(filehandle, UI_SET_EVBIT, EV_KEY);
    result = ioctl(filehandle, UI_SET_EVBIT, EV_SYN);

    result = ioctl(filehandle, UI_SET_KEYBIT, BTN_LEFT);
    result = ioctl(filehandle, UI_SET_KEYBIT, BTN_RIGHT);
    result = ioctl(filehandle, UI_SET_KEYBIT, BTN_MIDDLE);
    result = ioctl(filehandle, UI_SET_KEYBIT, BTN_SIDE);
    result = ioctl(filehandle, UI_SET_KEYBIT, BTN_EXTRA);

    result = ioctl(filehandle, UI_SET_EVBIT, EV_ABS);
    result = ioctl(filehandle, UI_SET_ABSBIT, ABS_X);
    result = ioctl(filehandle, UI_SET_ABSBIT, ABS_Y);
    result = ioctl(filehandle, UI_SET_KEYBIT, BTN_TOUCH);
    // BTN_TOOL_PEN is required for the mouse to be seen as an
    // absolute mouse as opposed to a relative mouse.
    result = ioctl(filehandle, UI_SET_KEYBIT, BTN_TOOL_PEN);
}

void UInputEventHandler::populateKeyCodes(int filehandle)
{
    int result = 0;
    for (unsigned int i=KEY_ESC; i <= KEY_MICMUTE; i++)
    {
        result = ioctl(filehandle, UI_SET_KEYBIT, i);
    }
}

void UInputEventHandler::createUInputKeyboardDevice(int filehandle)
{
    struct uinput_user_dev uidev;

    memset(&uidev, 0, sizeof(uidev));
    QByteArray temp = keyboardDeviceName.toUtf8();
    strncpy(uidev.name, temp.constData(), UINPUT_MAX_NAME_SIZE);
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x0;
    uidev.id.product = 0x0;
    uidev.id.version = 1;

    int result = 0;
    result = write(filehandle, &uidev, sizeof(uidev));
    result = ioctl(filehandle, UI_DEV_CREATE);
}

void UInputEventHandler::createUInputMouseDevice(int filehandle)
{
    struct uinput_user_dev uidev;

    memset(&uidev, 0, sizeof(uidev));
    QByteArray temp = mouseDeviceName.toUtf8();
    strncpy(uidev.name, temp.constData(), UINPUT_MAX_NAME_SIZE);
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x0;
    uidev.id.product = 0x0;
    uidev.id.version = 1;

    int result = 0;
    result = write(filehandle, &uidev, sizeof(uidev));
    result = ioctl(filehandle, UI_DEV_CREATE);
}

void UInputEventHandler::createUInputSpringMouseDevice(int filehandle)
{
    struct uinput_user_dev uidev;

    memset(&uidev, 0, sizeof(uidev));
    QByteArray temp = springMouseDeviceName.toUtf8();
    strncpy(uidev.name, temp.constData(), UINPUT_MAX_NAME_SIZE);
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x0;
    uidev.id.product = 0x0;
    uidev.id.version = 1;

    uidev.absmin[ABS_X] = -32767;
    uidev.absmax[ABS_X] = 32767;
    uidev.absflat[ABS_X] = 0;

    uidev.absmin[ABS_Y] = -32767;
    uidev.absmax[ABS_Y] = 32767;
    uidev.absflat[ABS_Y] = 0;

    int result = 0;
    result = write(filehandle, &uidev, sizeof(uidev));
    result = ioctl(filehandle, UI_DEV_CREATE);
}

void UInputEventHandler::closeUInputDevice(int filehandle)
{
    int result = 0;
    result = ioctl(filehandle, UI_DEV_DESTROY);
    result = close(filehandle);
}


void UInputEventHandler::write_uinput_event(int filehandle, unsigned int type,
                                            unsigned int code, int value, bool syn)
{
    struct input_event ev;
    struct input_event ev2;

    memset(&ev, 0, sizeof(struct input_event));
    gettimeofday(&ev.time, 0);
    ev.type = type;
    ev.code = code;
    ev.value = value;

    int result = 0;
    result = write(filehandle, &ev, sizeof(struct input_event));

    if (syn)
    {
        memset(&ev2, 0, sizeof(struct input_event));
        gettimeofday(&ev2.time, 0);
        ev2.type = EV_SYN;
        ev2.code = SYN_REPORT;
        ev2.value = 0;

        result = write(filehandle, &ev2, sizeof(struct input_event));
    }
}

QString UInputEventHandler::getName()
{
    return QString("uinput");
}

QString UInputEventHandler::getIdentifier()
{
    return getName();
}

/**
 * @brief Print extra help messages to stdout.
 */
void UInputEventHandler::printPostMessages()
{
    //QTextStream out(stdout);

    if (!lastErrorString.isEmpty())
    {
        Logger::LogInfo(lastErrorString);
    }

    if (!uinputDeviceLocation.isEmpty())
    {
        Logger::LogInfo(tr("Using uinput device file %1").arg(uinputDeviceLocation));
    }
}

void UInputEventHandler::sendTextEntryEvent(QString maintext)
{
    AntKeyMapper *mapper = AntKeyMapper::getInstance();

    if (mapper && mapper->getKeyMapper())
    {
        QtUInputKeyMapper *keymapper = static_cast<QtUInputKeyMapper*>(mapper->getKeyMapper());
        QtX11KeyMapper *nativeWinKeyMapper = 0;
        if (mapper->getNativeKeyMapper())
        {
            nativeWinKeyMapper = static_cast<QtX11KeyMapper*>(mapper->getNativeKeyMapper());
        }

        QList<unsigned int> tempList;
        for (int i=0; i < maintext.size(); i++)
        {
            tempList.clear();

            QtUInputKeyMapper::charKeyInformation temp;
            temp.virtualkey = 0;
            temp.modifiers = Qt::NoModifier;

            if (nativeWinKeyMapper)
            {
                QtX11KeyMapper::charKeyInformation tempX11 = nativeWinKeyMapper->getCharKeyInformation(maintext.at(i));
                tempX11.virtualkey = X11Extras::getInstance()->getGroup1KeySym(tempX11.virtualkey);
                unsigned int tempQtKey = nativeWinKeyMapper->returnQtKey(tempX11.virtualkey);
                if (tempQtKey > 0)
                {
                    temp.virtualkey = keymapper->returnVirtualKey(tempQtKey);
                    temp.modifiers = tempX11.modifiers;
                }
                else
                {
                    temp = keymapper->getCharKeyInformation(maintext.at(i));
                }
            }
            else
            {
                temp = keymapper->getCharKeyInformation(maintext.at(i));
            }

            if (temp.virtualkey > KEY_RESERVED)
            {
                if (temp.modifiers != Qt::NoModifier)
                {
                    if (temp.modifiers.testFlag(Qt::ShiftModifier))
                    {
                        tempList.append(KEY_LEFTSHIFT);
                        write_uinput_event(keyboardFileHandler, EV_KEY, KEY_LEFTSHIFT, 1, false);
                    }

                    if (temp.modifiers.testFlag(Qt::ControlModifier))
                    {
                        tempList.append(KEY_LEFTCTRL);
                        write_uinput_event(keyboardFileHandler, EV_KEY, KEY_LEFTCTRL, 1, false);
                    }

                    if (temp.modifiers.testFlag(Qt::AltModifier))
                    {
                        tempList.append(KEY_LEFTALT);
                        write_uinput_event(keyboardFileHandler, EV_KEY, KEY_LEFTALT, 1, false);
                    }

                    if (temp.modifiers.testFlag(Qt::MetaModifier))
                    {
                        tempList.append(KEY_LEFTMETA);
                        write_uinput_event(keyboardFileHandler, EV_KEY, KEY_LEFTMETA, 1, false);
                    }
                }

                tempList.append(temp.virtualkey);
                write_uinput_event(keyboardFileHandler, EV_KEY, temp.virtualkey, 1, true);
            }

            if (tempList.size() > 0)
            {
                QListIterator<unsigned int> tempiter(tempList);
                tempiter.toBack();
                while (tempiter.hasPrevious())
                {
                    unsigned int currentcode = tempiter.previous();
                    bool sync = !tempiter.hasPrevious() ? true : false;
                    write_uinput_event(keyboardFileHandler, EV_KEY, currentcode, 0, sync);
                }
            }
        }
    }
}
