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
//#include <QDebug>
#include <QFileInfo>
#include <QThreadStorage>

#include "common.h"

#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>

#include "x11extras.h"

const QString X11Extras::mouseDeviceName = PadderCommon::mouseDeviceName;
const QString X11Extras::keyboardDeviceName = PadderCommon::keyboardDeviceName;
const QString X11Extras::xtestMouseDeviceName = QString("Virtual core XTEST pointer");

QString X11Extras::_customDisplayString = QString("");

static QThreadStorage<X11Extras*> displays;

X11Extras* X11Extras::_instance = 0;

X11Extras::X11Extras(QObject *parent) :
    QObject(parent),
    knownAliases()
{
    //knownAliases = QHash<QString, QString> ();
    _display = XOpenDisplay(NULL);
    populateKnownAliases();
}

/**
 * @brief Close display connection if one exists
 */
X11Extras::~X11Extras()
{
    if (_display)
    {
        XCloseDisplay(display());
        _display = 0;
        //_customDisplayString = "";
    }
}

X11Extras *X11Extras::getInstance()
{
    /*if (!_instance)
    {
        _instance = new X11Extras();
    }

    return _instance;
    */
    X11Extras *temp = 0;
    if (!displays.hasLocalData())
    {
        temp = new X11Extras();
        displays.setLocalData(temp);
    }
    else
    {
        temp = displays.localData();
    }

    return temp;
}

void X11Extras::deleteInstance()
{
    /*if (_instance)
    {
        delete _instance;
        _instance = 0;
    }
    */
    if (displays.hasLocalData())
    {
        X11Extras *temp = displays.localData();
        delete temp;
        displays.setLocalData(0);
    }
}

/**
 * @brief Get display instance
 * @return Display struct
 */
Display* X11Extras::display()
{
    return _display;
}

bool X11Extras::hasValidDisplay()
{
    bool result = _display != NULL;
    return result;
}

/**
 * @brief CURRENTLY NOT USED
 */
void X11Extras::closeDisplay()
{
    if (_display)
    {
        XCloseDisplay(display());
        _display = 0;
        //_customDisplayString = "";
    }
}

/**
 * @brief Grab instance of active display.
 */
void X11Extras::syncDisplay()
{
    _display = XOpenDisplay(NULL);
    //_customDisplayString = "";
}

/**
 * @brief Grab instance of specified display. Useful for having the GUI
 *     on one display while generating events on another during ssh tunneling.
 * @param Valid display string that X can use
 */
void X11Extras::syncDisplay(QString displayString)
{
    QByteArray tempByteArray = displayString.toLocal8Bit();
    _display = XOpenDisplay(tempByteArray.constData());
    /*if (_display)
    {
        _customDisplayString = displayString;
    }
    else
    {
        _customDisplayString = "";
    }
    */
}

void X11Extras::setCustomDisplay(QString displayString)
{
    _customDisplayString = displayString;
}

/**
 * @brief Return root window for a given X display
 * @param Screen number. If no value is passed, uses screen 1.
 * @return XID of the window
 */
unsigned long X11Extras::appRootWindow(int screen)
{
    return screen == -1 ? XDefaultRootWindow(display()) : XRootWindowOfScreen(XScreenOfDisplay(display(), screen));
}

/**
 * @brief Get appropriate alias for a known KeySym string that might be blank
 *     or contain invalid characters when returned from X.
 * @param QString representation of a KeySym string
 * @return Alias string or a blank QString if no alias was found
 */
QString X11Extras::getDisplayString(QString xcodestring)
{
    QString temp;
    if (knownAliases.contains(xcodestring))
    {
        temp = knownAliases.value(xcodestring);
    }

    return temp;
}

void X11Extras::populateKnownAliases()
{
    // These aliases are needed for xstrings that would
    // return empty space characters from XLookupString
    if (knownAliases.isEmpty())
    {
        knownAliases.insert("Escape", tr("ESC"));
        knownAliases.insert("Tab", tr("Tab"));
        knownAliases.insert("space", tr("Space"));
        knownAliases.insert("Delete", tr("DEL"));
        knownAliases.insert("Return", tr("Return"));
        knownAliases.insert("KP_Enter", tr("KP_Enter"));
        knownAliases.insert("BackSpace", tr("Backspace"));
    }
}

Window X11Extras::findParentClient(Window window)
{
    Window parent = 0;
    Window root = 0;
    Window *children = 0;
    unsigned int num_children = 0;
    Window finalwindow = 0;
    Display *display = this->display();

    if (windowIsViewable(display, window) &&
        isWindowRelevant(display, window))
    {
        finalwindow = window;
    }
    else
    {
        bool quitTraversal = false;
        while (!quitTraversal)
        {
            children = 0;

            if (XQueryTree(display, window, &root, &parent, &children, &num_children))
            {
                if (children)
                {
                    // must test for NULL
                    XFree(children);
                }

                if (parent)
                {
                    if (windowIsViewable(display, parent) &&
                        isWindowRelevant(display, parent))
                    {
                        quitTraversal = true;
                        finalwindow = parent;
                    }
                    else if (parent == 0)
                    {
                        quitTraversal = true;
                    }
                    else if (parent == root)
                    {
                        quitTraversal = true;
                    }
                    else
                    {
                        window = parent;
                    }
                }
                else
                {
                    quitTraversal = true;
                }
            }
            else
            {
                quitTraversal = true;
            }
        }
    }

    return finalwindow;
}

/**
 * @brief Check window and any parents for the window property "_NET_WM_PID"
 * @param Window XID for window of interest
 * @return PID of the application instance corresponding to the window
 */
int X11Extras::getApplicationPid(Window window)
{
    Atom atom, actual_type;
    int actual_format = 0;
    unsigned long nitems = 0;
    unsigned long bytes_after = 0;
    unsigned char *prop = 0;
    int status = 0;
    int pid = 0;
    Window finalwindow = 0;

    Display *display = this->display();
    atom = XInternAtom(display, "_NET_WM_PID", True);
    if (windowHasProperty(display, window, atom))
    {
        finalwindow = window;
    }
    else
    {
        Window parent = 0;
        Window root = 0;
        Window *children;
        unsigned int num_children;
        bool quitTraversal = false;

        while (!quitTraversal)
        {
            children = 0;

            if (XQueryTree(display, window, &root, &parent, &children, &num_children))
            {
                if (children)
                {
                    // must test for NULL
                    XFree(children);
                }

                if (parent)
                {
                    if (windowHasProperty(display, parent, atom))
                    {
                        quitTraversal = true;
                        finalwindow = parent;
                    }
                    else if (parent == 0)
                    {
                        quitTraversal = true;
                    }
                    else if (parent == root)
                    {
                        quitTraversal = true;
                    }
                    else
                    {
                        window = parent;
                    }
                }
                else
                {
                    quitTraversal = true;
                }
            }
            else
            {
                quitTraversal = true;
            }
        }
    }

    if (finalwindow)
    {
        status = XGetWindowProperty(display, finalwindow, atom, 0, 1024, false, AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &prop);
        if (status == 0 && prop)
        {
            pid = prop[1] << 8;
            pid += prop[0];
            XFree(prop);
        }
    }

    return pid;
}

/**
 * @brief Find the application file location for a given PID
 * @param PID of window
 * @return File location of application
 */
QString X11Extras::getApplicationLocation(int pid)
{
    QString exepath;
    if (pid > 0)
    {
        QString procString = QString("/proc/%1/exe").arg(pid);
        QFileInfo procFileInfo(procString);
        if (procFileInfo.exists())
        {
            char buf[1024];
            QByteArray tempByteArray = procString.toLocal8Bit();
            ssize_t len = readlink(tempByteArray.constData(), buf, sizeof(buf)-1);
            if (len != -1)
            {
                buf[len] = '\0';
            }

            if (len > 0)
            {
                QString temp = QString::fromUtf8(buf);
                if (!temp.isEmpty())
                {
                    exepath = temp;
                }
            }
        }
    }

    return exepath;
}

/**
 * @brief Find the proper client window within a hierarchy. This check is needed
 *     in some environments where the window that has been selected is actually
 *     a child to a transparent parent window which was the one that was
 *     actually grabbed
 * @param Top window to check
 * @return Client window XID or 0 if no appropriate window was found
 */
Window X11Extras::findClientWindow(Window window)
{
    Window parent = 1;
    Window root = 0;
    Window *children = 0;
    unsigned int num_children = 0;
    Window finalwindow = 0;
    Display *display = this->display();

    if (windowIsViewable(display, window) &&
        isWindowRelevant(display, window))
    {
        finalwindow = window;
    }
    else
    {
        XQueryTree(display, window, &root, &parent, &children, &num_children);
        if (children)
        {
            for (unsigned int i = 0; i < num_children && !finalwindow; i++)
            {
                if (windowIsViewable(display, children[i]) &&
                    isWindowRelevant(display, window))
                {
                    finalwindow = children[i];
                }
            }
        }

        if (!finalwindow && children)
        {
            for (unsigned int i = 0; i < num_children && !finalwindow; i++)
            {
                finalwindow = findClientWindow(children[i]);
            }
        }

        if (children)
        {
            XFree(children);
            children = 0;
        }
    }

    return finalwindow;
}

bool X11Extras::windowHasProperty(Display *display, Window window, Atom atom)
{
    bool result = false;

    Atom actual_type;
    int actual_format = 0;
    unsigned long nitems = 0;
    unsigned long bytes_after = 0;
    unsigned char *prop = 0;
    int status = 0;
    status = XGetWindowProperty(display, window, atom, 0, 1024, false, AnyPropertyType,
                                &actual_type, &actual_format, &nitems, &bytes_after,
                                &prop);

    if (status == Success && prop)
    {
        result = true;
    }

    if (prop)
    {
        XFree(prop);
        prop = 0;
    }

    return result;
}

bool X11Extras::windowIsViewable(Display *display, Window window)
{
    bool result = false;
    XWindowAttributes xwa;
    XGetWindowAttributes(display, window, &xwa);
    if (xwa.c_class == InputOutput && xwa.map_state == IsViewable)
    {
        result = true;
    }

    return result;
}

/**
 * @brief Go through a window's properties and search for an Atom
 *     from a defined list. If an Atom is found in a window's properties,
 *     that window should be considered relevant and one that should be grabbed.
 * @param Display*
 * @param Window
 * @return If a window has a relevant Atom in its properties.
 */
bool X11Extras::isWindowRelevant(Display *display, Window window)
{
    bool result = false;

    QList<Atom> temp;
    temp.append(XInternAtom(display, "WM_STATE", True));
    temp.append(XInternAtom(display, "_NW_WM_STATE", True));
    temp.append(XInternAtom(display, "_NW_WM_NAME", True));

    QListIterator<Atom> iter(temp);
    while (iter.hasNext())
    {
        Atom current_atom = iter.next();
        if (windowHasProperty(display, window, current_atom))
        {
            iter.toBack();
            result = true;
        }
    }

    return result;
}

QString X11Extras::getWindowTitle(Window window)
{
    QString temp;

    Atom atom, actual_type;
    int actual_format = 0;
    unsigned long nitems = 0;
    unsigned long bytes_after = 0;
    unsigned char *prop = 0;
    int status = 0;

    //qDebug() << "WIN: 0x" << QString::number(window, 16);

    Display *display = this->display();
    Atom wm_name = XInternAtom(display, "WM_NAME", True);
    Atom net_wm_name = XInternAtom(display, "_NET_WM_NAME", True);
    atom = wm_name;

    QList<Atom> tempList;
    tempList.append(wm_name);
    tempList.append(net_wm_name);
    QListIterator<Atom> iter(tempList);
    while (iter.hasNext())
    {
        Atom temp_atom = iter.next();
        if (windowHasProperty(display, window, temp_atom))
        {
            iter.toBack();
            atom = temp_atom;
        }
    }

    status = XGetWindowProperty(display, window, atom, 0, 1024, false, AnyPropertyType,
                                &actual_type, &actual_format, &nitems, &bytes_after,
                                &prop);

    if (status == Success && prop)
    {
        char *tempprop = (char*)prop;
        temp.append(QString::fromUtf8(tempprop));
        //qDebug() << temp;
    }

    if (prop)
    {
        XFree(prop);
        prop = 0;
    }

    return temp;
}

QString X11Extras::getWindowClass(Window window)
{
    QString temp;

    Atom atom, actual_type;
    int actual_format = 0;
    unsigned long nitems = 0;
    unsigned long bytes_after = 0;
    unsigned char *prop = 0;
    int status = 0;

    Display *display = this->display();
    atom = XInternAtom(display, "WM_CLASS", True);
    status = XGetWindowProperty(display, window, atom, 0, 1024, false, AnyPropertyType,
                                &actual_type, &actual_format, &nitems, &bytes_after,
                                &prop);

    if (status == Success && prop)
    {
        //qDebug() << nitems;
        char *null_char = strchr((char*)prop, '\0');
        if ((char*)prop + nitems - 1 > null_char)
        {
            *(null_char) = ' ';
        }

        char *tempprop = (char*)prop;
        temp.append(QString::fromUtf8(tempprop));
        //qDebug() << temp;
        //qDebug() << (char*)prop;
    }

    if (prop)
    {
        XFree(prop);
        prop = 0;
    }

    return temp;
}

unsigned long X11Extras::getWindowInFocus()
{
    unsigned long result = 0;

    Window currentWindow = 0;
    int focusState = 0;

    Display *display = this->display();
    XGetInputFocus(display, &currentWindow, &focusState);

    if (currentWindow > 0)
    {
        result = static_cast<unsigned long>(currentWindow);
    }

    return result;
}

/**
 * @brief Get QString representation of currently utilized X display.
 * @return
 */
QString X11Extras::getXDisplayString()
{
    return _customDisplayString;
}

unsigned int X11Extras::getGroup1KeySym(unsigned int virtualkey)
{
    unsigned int result = 0;
    Display *display = this->display();

    unsigned int temp = XKeysymToKeycode(display, virtualkey);
    result = XkbKeycodeToKeysym(display, temp, 0, 0);

    return result;
}

void X11Extras::x11ResetMouseAccelerationChange(QString pointerName)
 {
    int xi_opcode, event, error;
    xi_opcode = event = error = 0;
    Display *display = this->display();

    bool result = XQueryExtension(display, "XInputExtension", &xi_opcode, &event, &error);
    if (!result)
    {
        Logger::LogInfo(tr("xinput extension was not found. No mouse acceleration changes will occur."));
    }
    else
    {
        int ximajor = 2, ximinor = 0;
        if (XIQueryVersion(display, &ximajor, &ximinor) != Success)
        {
            Logger::LogInfo(tr("xinput version must be at least 2.0. No mouse acceleration changes will occur."));
            result = false;
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
            if (current_devices->use == XISlavePointer &&
                QString::fromUtf8(current_devices->name) == pointerName)
            {
                Logger::LogInfo(tr("Virtual pointer found with id=%1.").arg(current_devices->deviceid));
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
                Logger::LogInfo(tr("PtrFeedbackClass was not found for virtual pointer."
                                   "No change to mouse acceleration will occur for device with id=%1").arg(device->device_id));

                result = false;
            }
            else
            {
                Logger::LogInfo(tr("Changing mouse acceleration for device with id=%1").arg(device->device_id));

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

void X11Extras::x11ResetMouseAccelerationChange()
{
    x11ResetMouseAccelerationChange(mouseDeviceName);
}

struct X11Extras::ptrInformation X11Extras::getPointInformation(QString pointerName)
{
    struct ptrInformation tempInfo;

    int xi_opcode, event, error;
    xi_opcode = event = error = 0;
    Display *display = this->display();

    bool result = XQueryExtension(display, "XInputExtension", &xi_opcode, &event, &error);
    if (result)
    {
        int ximajor = 2, ximinor = 0;
        if (XIQueryVersion(display, &ximajor, &ximinor) != Success)
        {
            Logger::LogInfo(tr("xinput version must be at least 2.0. No mouse acceleration changes will occur."));
            result = false;
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
            if (current_devices->use == XISlavePointer &&
                QString::fromUtf8(current_devices->name) == pointerName)
            {
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

                if (feedback_id == -1 && (i+1 < num_feedbacks))
                {
                    temp = (XFeedbackState*) ((char*) temp + temp->length);
                }
            }

            if (feedback_id <= -1)
            {
                result = false;
            }
            else
            {
                XPtrFeedbackState *tempPtrFeedback = reinterpret_cast<XPtrFeedbackState*>(temp);
                tempInfo.id = feedback_id;
                tempInfo.accelNum = tempPtrFeedback->accelNum;
                tempInfo.accelDenom = tempPtrFeedback->accelDenom;
                tempInfo.threshold = tempPtrFeedback->threshold;
            }

            XFree(feedbacks);
            feedbacks = temp = 0;
            XCloseDevice(display, device);
        }

        if (all_devices)
        {
            XIFreeDeviceInfo(all_devices);
        }
    }

    return tempInfo;
}

struct X11Extras::ptrInformation X11Extras::getPointInformation()
{
    return getPointInformation(mouseDeviceName);
}

QPoint X11Extras::getPos()
{
    XEvent mouseEvent;
    Window wid = DefaultRootWindow(display());
    XWindowAttributes xwAttr;

    XQueryPointer(display(), wid,
                  &mouseEvent.xbutton.root, &mouseEvent.xbutton.window,
                  &mouseEvent.xbutton.x_root, &mouseEvent.xbutton.y_root,
                  &mouseEvent.xbutton.x, &mouseEvent.xbutton.y,
                  &mouseEvent.xbutton.state);

    XGetWindowAttributes(display(), wid, &xwAttr);
    QPoint currentPoint(mouseEvent.xbutton.x_root, mouseEvent.xbutton.y_root);
    return currentPoint;
}
