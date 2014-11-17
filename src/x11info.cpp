#include <unistd.h>

//#include <QDebug>
#include "x11info.h"

#include <X11/Xatom.h>

X11Info* X11Info::_instance = 0;

X11Info::X11Info(QObject *parent) :
    QObject(parent)
{
    knownAliases = QHash<QString, QString> ();
    _display = XOpenDisplay(NULL);
    populateKnownAliases();
}

/**
 * @brief Close display connection if one exists
 */
X11Info::~X11Info()
{
    if (_display)
    {
        XCloseDisplay(display());
        _display = 0;
        _customDisplayString = "";
    }
}

X11Info *X11Info::getInstance()
{
    if (!_instance)
    {
        _instance = new X11Info();
    }

    return _instance;
}

void X11Info::deleteInstance()
{
    if (_instance)
    {
        delete _instance;
        _instance = 0;
    }
}

/**
 * @brief Get display instance
 * @return Display struct
 */
Display* X11Info::display()
{
    return _display;
}

bool X11Info::hasValidDisplay()
{
    bool result = _display != NULL;
    return result;
}

/**
 * @brief CURRENTLY NOT USED
 */
void X11Info::closeDisplay()
{
    if (_display)
    {
        XCloseDisplay(display());
        _display = 0;
        _customDisplayString = "";
    }
}

/**
 * @brief Grab instance of active display.
 */
void X11Info::syncDisplay()
{
    _display = XOpenDisplay(NULL);
    _customDisplayString = "";
}

/**
 * @brief Grab instance of specified display. Useful for having the GUI
 *     on one display while generating events on another during ssh tunneling.
 * @param Valid display string that X can use
 */
void X11Info::syncDisplay(QString displayString)
{
    QByteArray tempByteArray = displayString.toLocal8Bit();
    _display = XOpenDisplay(tempByteArray.constData());
    if (_display)
    {
        _customDisplayString = displayString;
    }
    else
    {
        _customDisplayString = "";
    }
}

/**
 * @brief Return root window for a given X display
 * @param Screen number. If no value is passed, uses screen 1.
 * @return XID of the window
 */
unsigned long X11Info::appRootWindow(int screen)
{
    return screen == -1 ? XDefaultRootWindow(display()) : XRootWindowOfScreen(XScreenOfDisplay(display(), screen));
}

/**
 * @brief Get appropriate alias for a known KeySym string that might be blank
 *     or contain invalid characters when returned from X.
 * @param QString representation of a KeySym string
 * @return Alias string or a blank QString if no alias was found
 */
QString X11Info::getDisplayString(QString xcodestring)
{
    QString temp;
    if (knownAliases.contains(xcodestring))
    {
        temp = knownAliases.value(xcodestring);
    }

    return temp;
}

void X11Info::populateKnownAliases()
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

Window X11Info::findParentClient(Window window)
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
int X11Info::getApplicationPid(Window window)
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
    //status = XGetWindowProperty(display, window, atom, 0, 1024, false, AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &prop);
    /*if (status == 0 && prop)
    {
        pid = prop[1] << 8;
        pid += prop[0];
        XFree(prop);
    }
    */
    //else if (status == 0)
    //{
        //XFree(prop);

        /*Window parent = 0;
        Window root = 0;
        Window *children;
        unsigned int num_children;
        bool quitTraversal = false;

        while (!quitTraversal)
        {
            children = 0;

            if (XQueryTree(display, window, &root, &parent, &children, &num_children))
            {
                if (children) { // must test for null
                    XFree(children);
                }

                if (parent)
                {
                    status = XGetWindowProperty(display, parent, atom, 0, 1024, false, AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &prop);
                    if (status == 0 && prop)
                    {
                        pid = prop[1] << 8;
                        pid += prop[0];

                        quitTraversal = true;
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

                    XFree(prop);
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
        */
    //}

    return pid;
}

/**
 * @brief Find the application file location for a given PID
 * @param PID of window
 * @return File location of application
 */
QString X11Info::getApplicationLocation(int pid)
{
    QString exepath;
    if (pid > 0)
    {
        QString procString = QString("/proc/%1/exe").arg(pid);
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
Window X11Info::findClientWindow(Window window)
{
    /*Atom actual_type;
    int actual_format = 0;
    unsigned long nitems = 0;
    unsigned long bytes_after = 0;
    unsigned char *prop = 0;
    int status = 0;
    */

    Window parent = 1;
    Window root = 0;
    Window *children = 0;
    unsigned int num_children = 0;
    Window finalwindow = 0;
    Display *display = this->display();

    //Atom wm_state_atom = XInternAtom(display, "WM_STATE", True);
    //Atom net_wm_state_atom = XInternAtom(display, "_NET_WM_STATE", True);
    //Atom pidAtom = XInternAtom(display, "_NET_WM_PID", True);
    //Atom wm_class = XInternAtom(display, "WM_CLASS", True);

    if (windowIsViewable(display, window) &&
        isWindowRelevant(display, window))
    {
        finalwindow = window;
    }

    //status = XGetWindowProperty(display, window, pidAtom, 0, 1024, false, AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &prop);

    //Atom pidAtom = XInternAtom(display, "_NET_WM_PID", True);
    //status = XGetWindowProperty(display, window, pidAtom, 0, 1024, false, AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &prop);
    //if (status == 0 && prop)
    //{
    //    finalwindow = window;
    //}

    /*if (prop)
    {
        XFree(prop);
        prop = 0;
    }
    */

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

                //status = XGetWindowProperty(display, children[i], wm_state_atom, 0, 1024, false, AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &prop);
                /*if (status == 0 && prop)
                {
                    finalwindow = children[i];
                }
                */

                /*if (prop)
                {
                    XFree(prop);
                    prop = 0;
                }
                */
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

    /*if (finalwindow)
    {
        if (!windowHasProperty(display, finalwindow, pidAtom))
        {
            finalwindow = 0;
        }
    }
    */

    return finalwindow;
}

bool X11Info::windowHasProperty(Display *display, Window window, Atom atom)
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

bool X11Info::windowIsViewable(Display *display, Window window)
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
bool X11Info::isWindowRelevant(Display *display, Window window)
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

QString X11Info::getWindowTitle(Window window)
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

QString X11Info::getWindowClass(Window window)
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

unsigned long X11Info::getWindowInFocus()
{
    unsigned long result = 0;

    Window currentWindow = 0;
    int focusState = 0;

    Display *display = this->display();
    XGetInputFocus(display, &currentWindow, &focusState);

    if (currentWindow > 0)
    {
        result = (unsigned long)currentWindow;
    }

    return result;
}

/**
 * @brief Get QString representation of currently utilized X display.
 * @return
 */
QString X11Info::getXDisplayString()
{
    return _customDisplayString;
}
