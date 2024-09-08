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

#include <cmath>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <unistd.h>

#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QStringList>
#include <QTimer>

#include <antkeymapper.h>
#include <common.h>
#include <joybuttonslot.h>
#include <logger.h>

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

UInputEventHandler::UInputEventHandler(QObject *parent)
    : BaseEventHandler(parent)
#if defined(Q_OS_UNIX)
    , is_problem_with_opening_uinput_present(false)
#endif
{
    keyboardFileHandler = 0;
    mouseFileHandler = 0;
    springMouseFileHandler = 0;
}

UInputEventHandler::~UInputEventHandler() { cleanupUinputEvHand(); }

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
    initDevice(keyboardFileHandler, "keyboardFileHandler", result);

    // two separate statements for "result" are needed anyway because of possible changing its
    // statement in "initDevice" method
    if (result)
    {
        // Open mouse file handle to use for relative mouse emulation.
        initDevice(mouseFileHandler, "mouseFileHandler", result);
    }

    if (result)
    {
        // Open mouse file handle to use for absolute mouse emulation.
        initDevice(springMouseFileHandler, "springMouseFileHandler", result);
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

void UInputEventHandler::initDevice(int &device, QString name, bool &result)
{
    device = openUInputHandle();

    if (device > 0)
    {
        if (name == "springMouseFileHandler")
        {
            QString detected_xdg_session = qgetenv("XDG_SESSION_TYPE");
            if (detected_xdg_session == "x11")
                qWarning()
                    << "uinput event handle may not work properly with absolute mouse events (like spring mouse) for X11";
            // https://stackoverflow.com/questions/5190921/simulating-absolute-mouse-movements-in-linux-using-uinput
            setSpringMouseEvents(device);
            createUInputSpringMouseDevice(device);
        } else if (name == "mouseFileHandler")
        {
            setRelMouseEvents(device);
            createUInputMouseDevice(device);
        } else if (name == "keyboardFileHandler")
        {
            setKeyboardEvents(device);
            populateKeyCodes(device);
            createUInputKeyboardDevice(device);
        }
    } else
    {
        result = false;
    }
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

bool UInputEventHandler::cleanup() { return cleanupUinputEvHand(); }

bool UInputEventHandler::cleanupUinputEvHand()
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
            unsigned int tempcode;
            switch (code)
            {
            case 3: {
                tempcode = BTN_RIGHT;
                break;
            }
            case 2: {
                tempcode = BTN_MIDDLE;
                break;
            }
            case 1:
            default: {
                tempcode = BTN_LEFT;
            }
            }

            write_uinput_event(mouseFileHandler, EV_KEY, tempcode, pressed ? 1 : 0);
        } else if (code == 4)
        {
            if (pressed)
            {
                write_uinput_event(mouseFileHandler, EV_REL, REL_WHEEL, 1);
            }

        } else if (code == 5)
        {
            if (pressed)
            {
                write_uinput_event(mouseFileHandler, EV_REL, REL_WHEEL, -1);
            }
        } else if (code == 6)
        {
            if (pressed)
            {
                write_uinput_event(mouseFileHandler, EV_REL, REL_HWHEEL, -1);
            }
        } else if (code == 7)
        {
            if (pressed)
            {
                write_uinput_event(mouseFileHandler, EV_REL, REL_HWHEEL, 1);
            }
        } else if (code == 8)
        {
            write_uinput_event(mouseFileHandler, EV_KEY, BTN_SIDE, pressed ? 1 : 0);
        } else if (code == 9)
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

void UInputEventHandler::sendMouseSpringEvent(int xDis, int yDis, int width, int height)
{
    if ((width > 0) && (height > 0))
    {
        double midwidth = width / 2.0;
        double midheight = height / 2.0;

        int fx = ceil(32767 * ((xDis - midwidth) / midwidth));
        int fy = ceil(32767 * ((yDis - midheight) / midheight));
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

    for (auto &&temp : locations)
    {
        QFileInfo tempFileInfo(temp);
        if (tempFileInfo.exists())
        {
            possibleLocation = temp;
            break;
        }
    }

    if (possibleLocation.isEmpty())
    {
        lastErrorString = tr("Could not find a valid uinput device file.\n"
                             "Please check that you have the uinput module loaded.\n"
                             "lsmod | grep uinput");
    } else
    {
        QByteArray tempArray = possibleLocation.toUtf8();
        filehandle = open(tempArray.constData(), O_WRONLY | O_NONBLOCK);

        if (filehandle < 0)
        {
            lastErrorString = tr("Could not open uinput device file\n"
                                 "Please check that you have permission to write to the device");
            lastErrorString.append(": ").append(possibleLocation).append('\n');
#if defined(Q_OS_UNIX)
            is_problem_with_opening_uinput_present = true;
#endif
        } else
        {
            uinputDeviceLocation = possibleLocation;
        }
    }

    return filehandle;
}

void UInputEventHandler::setKeyboardEvents(int filehandle)
{
    ioctl(filehandle, UI_SET_EVBIT, EV_KEY);
    ioctl(filehandle, UI_SET_EVBIT, EV_SYN);
}

void UInputEventHandler::setRelMouseEvents(int filehandle)
{
    ioctl(filehandle, UI_SET_EVBIT, EV_KEY);
    ioctl(filehandle, UI_SET_EVBIT, EV_SYN);
    ioctl(filehandle, UI_SET_EVBIT, EV_REL);

    ioctl(filehandle, UI_SET_RELBIT, REL_X);
    ioctl(filehandle, UI_SET_RELBIT, REL_Y);
    ioctl(filehandle, UI_SET_RELBIT, REL_WHEEL);
    ioctl(filehandle, UI_SET_RELBIT, REL_HWHEEL);

    ioctl(filehandle, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(filehandle, UI_SET_KEYBIT, BTN_RIGHT);
    ioctl(filehandle, UI_SET_KEYBIT, BTN_MIDDLE);
    ioctl(filehandle, UI_SET_KEYBIT, BTN_SIDE);
    ioctl(filehandle, UI_SET_KEYBIT, BTN_EXTRA);
}

void UInputEventHandler::setSpringMouseEvents(int filehandle)
{
    ioctl(filehandle, UI_SET_EVBIT, EV_KEY);
    ioctl(filehandle, UI_SET_EVBIT, EV_SYN);

    ioctl(filehandle, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(filehandle, UI_SET_KEYBIT, BTN_RIGHT);
    ioctl(filehandle, UI_SET_KEYBIT, BTN_MIDDLE);
    ioctl(filehandle, UI_SET_KEYBIT, BTN_SIDE);
    ioctl(filehandle, UI_SET_KEYBIT, BTN_EXTRA);

    ioctl(filehandle, UI_SET_EVBIT, EV_ABS);
    ioctl(filehandle, UI_SET_ABSBIT, ABS_X);
    ioctl(filehandle, UI_SET_ABSBIT, ABS_Y);
    ioctl(filehandle, UI_SET_KEYBIT, BTN_TOUCH);
}

void UInputEventHandler::populateKeyCodes(int filehandle)
{
    for (unsigned int i = KEY_ESC; i <= KEY_MICMUTE; i++)
    {
        ioctl(filehandle, UI_SET_KEYBIT, i);
    }
}

void UInputEventHandler::createUInputKeyboardDevice(int filehandle)
{
    struct uinput_user_dev uidev;

    memset(&uidev, 0, sizeof(uidev));
    QByteArray temp = keyboardDeviceName.toUtf8();
    strncpy(uidev.name, temp.constData(), UINPUT_MAX_NAME_SIZE);
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor = 0x0;
    uidev.id.product = 0x0;
    uidev.id.version = 1;

    write(filehandle, &uidev, sizeof(uidev));
    ioctl(filehandle, UI_DEV_CREATE);
}

void UInputEventHandler::createUInputMouseDevice(int filehandle)
{
    struct uinput_user_dev uidev;

    memset(&uidev, 0, sizeof(uidev));
    QByteArray temp = mouseDeviceName.toUtf8();
    strncpy(uidev.name, temp.constData(), UINPUT_MAX_NAME_SIZE);
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor = 0x0;
    uidev.id.product = 0x0;
    uidev.id.version = 1;

    write(filehandle, &uidev, sizeof(uidev));
    ioctl(filehandle, UI_DEV_CREATE);
}

void UInputEventHandler::createUInputSpringMouseDevice(int filehandle)
{
    struct uinput_user_dev uidev;

    memset(&uidev, 0, sizeof(uidev));
    QByteArray temp = springMouseDeviceName.toUtf8();
    strncpy(uidev.name, temp.constData(), UINPUT_MAX_NAME_SIZE);
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor = 0x0;
    uidev.id.product = 0x0;
    uidev.id.version = 1;

    uidev.absmin[ABS_X] = -32767;
    uidev.absmax[ABS_X] = 32767;
    uidev.absflat[ABS_X] = 0;

    uidev.absmin[ABS_Y] = -32767;
    uidev.absmax[ABS_Y] = 32767;
    uidev.absflat[ABS_Y] = 0;

    write(filehandle, &uidev, sizeof(uidev));
    ioctl(filehandle, UI_DEV_CREATE);
}

void UInputEventHandler::closeUInputDevice(int filehandle)
{
    ioctl(filehandle, UI_DEV_DESTROY);
    close(filehandle);
}

void UInputEventHandler::write_uinput_event(int filehandle, int type, int code, int value, bool syn)
{
    struct input_event ev;
    struct input_event ev2;

    memset(&ev, 0, sizeof(struct input_event));
    gettimeofday(&ev.time, nullptr);
    ev.type = type;
    ev.code = code;
    ev.value = value;

    write(filehandle, &ev, sizeof(struct input_event));

    if (syn)
    {
        memset(&ev2, 0, sizeof(struct input_event));
        gettimeofday(&ev2.time, nullptr);
        ev2.type = EV_SYN;
        ev2.code = SYN_REPORT;
        ev2.value = 0;

        write(filehandle, &ev2, sizeof(struct input_event));
    }
}

QString UInputEventHandler::getName() { return QString("uinput"); }

QString UInputEventHandler::getIdentifier() { return getName(); }

/**
 * @brief Print extra help messages to stdout.
 */
void UInputEventHandler::printPostMessages()
{
    if (!lastErrorString.isEmpty())
    {
        PRINT_STDERR() << lastErrorString;
#if defined(Q_OS_UNIX)
        if (is_problem_with_opening_uinput_present)
        {
            QMessageBox msgBox;
            msgBox.setTextFormat(Qt::RichText);
            msgBox.setText(
                QObject::tr("Unable to open uinput files, this may cause problems with generating events.\nTo check "
                            "possible solutions please visit: ")
                    .append(
                        "<a href='https://github.com/AntiMicroX/antimicrox/wiki/Open-uinput-error'>AntiMicroX Wiki</a>"));
            msgBox.setDetailedText(lastErrorString);
            msgBox.exec();
        }
#endif
    }

    if (!uinputDeviceLocation.isEmpty())
    {
        qInfo() << tr("Using uinput device file %1").arg(uinputDeviceLocation);
    }
}

void UInputEventHandler::sendTextEntryEvent(QString maintext)
{
    AntKeyMapper *mapper = AntKeyMapper::getInstance();

    if ((mapper != nullptr) && mapper->getKeyMapper())
    {
        QtUInputKeyMapper *keymapper = qobject_cast<QtUInputKeyMapper *>(mapper->getKeyMapper());
#ifdef WITH_X11
        QtX11KeyMapper *nativeWinKeyMapper = nullptr;

        if (mapper->getNativeKeyMapper())
        {
            nativeWinKeyMapper = qobject_cast<QtX11KeyMapper *>(mapper->getNativeKeyMapper());
        }
#endif

        QList<unsigned int> tempList;

        for (int i = 0; i < maintext.size(); i++)
        {
            tempList.clear();

            QtUInputKeyMapper::charKeyInformation temp;
            temp.virtualkey = 0;
            temp.modifiers = Qt::NoModifier;

#ifdef WITH_X11
            if (nativeWinKeyMapper != nullptr)
            {
                QtX11KeyMapper::charKeyInformation tempX11 = nativeWinKeyMapper->getCharKeyInformation(maintext.at(i));
                tempX11.virtualkey = X11Extras::getInstance()->getGroup1KeySym(tempX11.virtualkey);
                unsigned int tempQtKey = nativeWinKeyMapper->returnQtKey(tempX11.virtualkey);

                if (tempQtKey > 0)
                {
                    temp.virtualkey = keymapper->returnVirtualKey(tempQtKey);
                    temp.modifiers = tempX11.modifiers;
                } else
                {
                    temp = keymapper->getCharKeyInformation(maintext.at(i));
                }
            } else
            {
#endif
                temp = keymapper->getCharKeyInformation(maintext.at(i));
#ifdef WITH_X11
            }
#endif

            if (temp.virtualkey > KEY_RESERVED)
            {
                if (temp.modifiers != Qt::NoModifier)
                {
                    testAndAppend(temp.modifiers.testFlag(Qt::ShiftModifier), tempList, KEY_LEFTSHIFT);
                    testAndAppend(temp.modifiers.testFlag(Qt::ControlModifier), tempList, KEY_LEFTCTRL);
                    testAndAppend(temp.modifiers.testFlag(Qt::AltModifier), tempList, KEY_LEFTALT);
                    testAndAppend(temp.modifiers.testFlag(Qt::MetaModifier), tempList, KEY_LEFTMETA);
                }

                tempList.append(temp.virtualkey);
                write_uinput_event(keyboardFileHandler, EV_KEY, temp.virtualkey, 1, true);
            }

            if (tempList.size() > 0)
            {
                for (auto iter = tempList.crbegin(); iter != tempList.crend(); ++iter)
                {
                    unsigned int currentcode = *iter;
                    bool sync = std::next(iter) == tempList.crend();
                    write_uinput_event(keyboardFileHandler, EV_KEY, currentcode, 0, sync);
                }
            }
        }
    }
}

void UInputEventHandler::testAndAppend(bool tested, QList<unsigned int> &tempList, unsigned int key)
{
    if (tested)
    {
        tempList.append(key);
        write_uinput_event(keyboardFileHandler, EV_KEY, key, 1, false);
    }
}

int UInputEventHandler::getKeyboardFileHandler() { return keyboardFileHandler; }

int UInputEventHandler::getMouseFileHandler() { return mouseFileHandler; }

int UInputEventHandler::getSpringMouseFileHandler() { return springMouseFileHandler; }

const QString UInputEventHandler::getUinputDeviceLocation() { return uinputDeviceLocation; }
