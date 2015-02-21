#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>

//#include <QDebug>
#include <QTextStream>
#include <QStringList>
#include <QStringListIterator>
#include <QFileInfo>
#include <QTimer>


#ifdef WITH_X11
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QApplication>
    #endif

#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>

#include <x11extras.h>

#endif

#include "uinputeventhandler.h"

static const QString mouseDeviceName("antimicro Mouse Emulation");
static const QString keyboardDeviceName("antimicro Keyboard Emulation");

UInputEventHandler::UInputEventHandler(QObject *parent) :
    BaseEventHandler(parent)
{
    keyboardFileHandler = 0;
    mouseFileHandler = 0;
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
        // Open mouse file handle to use for mouse emulation.
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

#ifdef WITH_X11
    if (result)
    {
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))

    if (QApplication::platformName() == QStringLiteral("xcb"))
    {
    #endif
    // Some time needs to elapse after device creation before changing
    // pointer settings. Otherwise, settings will not take effect.
    QTimer::singleShot(1000, this, SLOT(x11ResetMouseAccelerationChange()));
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
    QTextStream out(stdout);

    int xi_opcode, event, error;
    xi_opcode = event = error = 0;
    Display *display = X11Extras::getInstance()->display();

    bool result = XQueryExtension(display, "XInputExtension", &xi_opcode, &event, &error);
    if (!result)
    {
        out << tr("xinput extension was not found. No mouse acceleration changes will occur.") << endl;
    }
    else
    {
        int ximajor = 2, ximinor = 0;
        if (XIQueryVersion(display, &ximajor, &ximinor) != Success)
        {
            out << tr("xinput version must be at least 2.0. No mouse acceleration changes will occur.") << endl;
        }
    }

    if (result)
    {
        XIDeviceInfo *all_devices = 0;
        XIDeviceInfo *current_devices = 0;
        XIDeviceInfo *mouse_device = 0;

        int num_devices = 0;
        all_devices = XIQueryDevice(display, XIAllDevices, &num_devices);
        for (int i=0; i < num_devices; i++)
        {
            current_devices = &all_devices[i];
            if (current_devices->use == XISlavePointer && QString::fromUtf8(current_devices->name) == mouseDeviceName)
            {
                out << tr("Virtual pointer found with id=%1.").arg(current_devices->deviceid)
                    << endl;
                mouse_device = current_devices;
            }
        }

        if (mouse_device)
        {
            XDevice *device = XOpenDevice(display, mouse_device->deviceid);

            int num_feedbacks = 0;
            int feedback_id = -1;
            XFeedbackState *feedbacks = XGetFeedbackControl(display, device, &num_feedbacks);
            XFeedbackState *temp = feedbacks;
            for (int i=0; (i < num_feedbacks) && (feedback_id == -1); i++)
            {
                if (temp->c_class == PtrFeedbackClass)
                {
                    feedback_id = temp->id;
                }

                if (i+1 < num_feedbacks)
                {
                    temp = (XFeedbackState*) ((char*) temp + temp->length);
                }
            }

            XFree(feedbacks);
            feedbacks = temp = 0;

            if (feedback_id <= -1)
            {
                out << tr("PtrFeedbackClass was not found for virtual pointer."
                          "No change to mouse acceleration will occur for device with id=%1").arg(device->device_id)
                    << endl;
                result = false;
            }
            else
            {
                out << tr("Changing mouse acceleration for device with id=%1").arg(device->device_id)
                    << endl;

                XPtrFeedbackControl	feedback;
                feedback.c_class = PtrFeedbackClass;
                feedback.length = sizeof(XPtrFeedbackControl);
                feedback.id = feedback_id;
                feedback.threshold = 0;
                feedback.accelNum = 1;
                feedback.accelDenom = 1;

                XChangeFeedbackControl(display, device, DvAccelNum|DvAccelDenom|DvThreshold,
                           (XFeedbackControl*) &feedback);

                XSync(display, false);
            }

            XCloseDevice(display, device);
        }

        if (all_devices)
        {
            XIFreeDeviceInfo(all_devices);
        }
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
        QByteArray tempArray = possibleLocation.toUtf8();
        filehandle = open(tempArray.constData(), O_WRONLY | O_NONBLOCK);
        if (filehandle < 0)
        {
            lastErrorString = tr("Could not open uinput device file\n"
                                 "Please check that you have permission to write to the device");
            lastErrorString.append("\n").append(possibleLocation);
            err << lastErrorString << endl << endl;
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

void UInputEventHandler::closeUInputDevice(int filehandle)
{
    int result = 0;
    result = ioctl(filehandle, UI_DEV_DESTROY);
    result = close(filehandle);
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
    QTextStream out(stdout);

    if (!uinputDeviceLocation.isEmpty())
    {
        out << tr("Using uinput device file %1").arg(uinputDeviceLocation) << endl;
    }
}
