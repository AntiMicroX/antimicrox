#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>

//#include <QDebug>
#include <QTextStream>
#include <QStringList>
#include <QStringListIterator>
#include <QFileInfo>

#include "uinputeventhandler.h"

UInputEventHandler::UInputEventHandler(QObject *parent) :
    BaseEventHandler(parent)
{
    keyboardFileHandler = 0;
    mouseFileHandler = 0;
}

bool UInputEventHandler::init()
{
    bool result = true;

    keyboardFileHandler = openUInputHandle();
    if (keyboardFileHandler > 0)
    {
        setKeyboardEvents(keyboardFileHandler);
        populateKeyCodes(keyboardFileHandler);
        createUInputDevice(keyboardFileHandler);
    }
    else
    {
        result = false;
    }

    if (result)
    {
        mouseFileHandler = openUInputHandle();
        if (mouseFileHandler > 0)
        {
            setMouseEvents(mouseFileHandler);
            createUInputMouseDevice(mouseFileHandler);
        }
        else
        {
            result = false;
        }
    }

    return result;
}

bool UInputEventHandler::cleanup()
{
    if (keyboardFileHandler > 0)
    {
        closeUInputDevice(keyboardFileHandler);
    }

    if (mouseFileHandler > 0)
    {
        closeUInputDevice(mouseFileHandler);
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
            unsigned int tempcode = (BTN_LEFT | code);
            if (code == 3)
            {
                tempcode -= 2;
            }
            else
            {
                tempcode -= 1;
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

int UInputEventHandler::openUInputHandle()
{
    int filehandle = -1;
    QTextStream err(stderr);
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
        err << lastErrorString << endl << endl;
    }
    else
    {
        //qDebug() << "LOCATION: " << possibleLocation;
        QByteArray tempArray = possibleLocation.toUtf8();
        filehandle = open(tempArray.constData(), O_WRONLY | O_NONBLOCK);
        if (filehandle < 0)
        {
            lastErrorString = tr("Could not open uinput device file\n"
                                 "Please check that you have permission to write to the device");
            lastErrorString.append("\n").append(possibleLocation);
            err << lastErrorString << endl << endl;
            //err << possibleLocation << endl << endl;
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

void UInputEventHandler::setMouseEvents(int filehandle)
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

void UInputEventHandler::populateKeyCodes(int filehandle)
{
    for (unsigned int i=KEY_ESC; i < KEY_UNKNOWN; i++)
    {
        ioctl(filehandle, UI_SET_KEYBIT, i);
    }
}

void UInputEventHandler::createUInputDevice(int filehandle)
{
    struct uinput_user_dev uidev;

    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "AntiMicro Keyboard Emulation");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x0;
    uidev.id.product = 0x0;
    uidev.id.version = 1;

    write(filehandle, &uidev, sizeof(uidev));
    ioctl(filehandle, UI_DEV_CREATE);
}

void UInputEventHandler::createUInputMouseDevice(int filehandle)
{
    struct uinput_user_dev uidev;

    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "AntiMicro Mouse Emulation");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x0;
    uidev.id.product = 0x0;
    uidev.id.version = 1;

    write(filehandle, &uidev, sizeof(uidev));
    ioctl(filehandle, UI_DEV_CREATE);
}

void UInputEventHandler::closeUInputDevice(int filehandle)
{
    ioctl(filehandle, UI_DEV_DESTROY);
    close(filehandle);
}


void UInputEventHandler::write_uinput_event(int filehandle, unsigned int type, unsigned int code, int value, bool syn)
{
    struct input_event ev;
    struct input_event ev2;

    memset(&ev, 0, sizeof(struct input_event));
    gettimeofday(&ev.time, 0);
    ev.type = type;
    ev.code = code;
    ev.value = value;

    write(filehandle, &ev, sizeof(struct input_event));

    if (syn)
    {
        memset(&ev2, 0, sizeof(struct input_event));
        gettimeofday(&ev2.time, 0);
        ev2.type = EV_SYN;
        ev2.code = SYN_REPORT;
        ev2.value = 0;

        write(filehandle, &ev2, sizeof(struct input_event));
    }
}
