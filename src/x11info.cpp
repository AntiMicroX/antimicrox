#include <QDebug>
#include <unistd.h>

#include "x11info.h"

X11Info X11Info::_instance;

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
    }
}

/**
 * @brief Get display instance
 * @return Display struct
 */
Display* X11Info::display()
{
    return _instance._display;
}

/**
 * @brief CURRENTLY NOT USED
 */
void X11Info::closeDisplay()
{
    if (_instance._display)
    {
        XCloseDisplay(display());
        _instance._display = 0;
    }
}

/**
 * @brief Grab instance of active display.
 */
void X11Info::syncDisplay()
{
    _instance._display = XOpenDisplay(NULL);
}

/**
 * @brief Grab instance of specified display. Useful for having the GUI
 *     on one display while generating events on another during ssh tunneling.
 * @param Valid display string that X can use
 */
void X11Info::syncDisplay(QString displayString)
{
    QByteArray tempByteArray = displayString.toLocal8Bit();
    _instance._display = XOpenDisplay(tempByteArray.constData());
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
    if (_instance.knownAliases.contains(xcodestring))
    {
        temp = _instance.knownAliases.value(xcodestring);
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

/**
 * @brief Check window and any children for the window property "_NET_WM_PID"
 * @param Window XID for window of interest
 * @return PID of the application instance corresponding to the window
 */
int X11Info::getApplicationPid(Window &window)
{
    Atom atom, actual_type;
    int actual_format = 0;
    unsigned long nitems = 0;
    unsigned long bytes_after = 0;
    unsigned char *prop = 0;
    int status = 0;
    int pid = 0;

    Display *display = X11Info::display();
    atom = XInternAtom(display, "_NET_WM_PID", True);
    status = XGetWindowProperty(display, window, atom, 0, 1024, false, AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &prop);
    if (status == 0 && prop)
    {
        pid = prop[1] << 8;
        pid += prop[0];
        XFree(prop);
    }
    else if (status == 0)
    {
        XFree(prop);

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
    }

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
        QString temp = QString::fromLocal8Bit(buf);
        if (!temp.isEmpty())
        {
            exepath = temp;
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
Window X11Info::findClientWindow(Window &window)
{
    Atom actual_type;
    int actual_format = 0;
    unsigned long nitems = 0;
    unsigned long bytes_after = 0;
    unsigned char *prop = 0;
    int status = 0;

    Window parent = 1;
    Window root = 0;
    Window *children = 0;
    unsigned int num_children = 0;
    Window finalwindow = 0;
    Display *display = X11Info::display();

    Atom wmStateAtom = XInternAtom(display, "WM_STATE", True);
    // WM_STATE is not available from an app when running a game
    // in the SteamOS BPM X Session. Make a special case for SteamOS.
    // TODO: Find a better Atom variable to discover.
    Atom steamCheckAtom = XInternAtom(display, "STEAM_GAME", True);
    // Put in a check for WM_NAME. Attempt to ensure that a valid window has
    // been obtained.
    Atom wmNameAtom = XInternAtom(display, "WM_NAME", True);

    QList<Atom> atomChecks;
    atomChecks.append(wmStateAtom);
    atomChecks.append(steamCheckAtom);
    QListIterator<Atom> iter(atomChecks);
    while (iter.hasNext())
    {
        Atom tmpAtom = iter.next();
        status = XGetWindowProperty(display, window, tmpAtom, 0, 1024, false, AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &prop);
        if (status == 0 && prop)
        {
            if (tmpAtom == steamCheckAtom)
            {
                // A Steam game has been detected. Try to find the WM_NAME
                // property to ensure that a root window has not been detected.
                if (prop)
                {
                    XFree(prop);
                    prop = 0;
                }

                status = XGetWindowProperty(display, window, wmNameAtom, 0, 1024, false, AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &prop);
                if (status == 0 && prop)
                {
                    finalwindow = window;
                    iter.toBack();
                }
            }
            else
            {
                finalwindow = window;
                iter.toBack();
            }
        }

        if (prop)
        {
            XFree(prop);
            prop = 0;
        }
    }

    if (!finalwindow)
    {
        XQueryTree(display, window, &root, &parent, &children, &num_children);
        if (children)
        {
            for (unsigned int i = 0; i < num_children && !finalwindow; i++)
            {
                iter.toFront();
                while (iter.hasNext())
                {
                    Atom tmpAtom = iter.next();
                    status = XGetWindowProperty(display, children[i], tmpAtom, 0, 1024, false, AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &prop);
                    if (status == 0 && prop)
                    {
                        if (tmpAtom == steamCheckAtom)
                        {
                            // A Steam game has been detected. Try to find the WM_NAME
                            // property to ensure that a root window has not been detected.
                            if (prop)
                            {
                                XFree(prop);
                                prop = 0;
                            }

                            status = XGetWindowProperty(display, children[i], wmNameAtom, 0, 1024, false, AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &prop);
                            if (status == 0 && prop)
                            {
                                finalwindow = children[i];
                                iter.toBack();
                            }
                        }
                        else
                        {
                            finalwindow = children[i];
                            iter.toBack();
                        }
                    }

                    if (prop)
                    {
                        XFree(prop);
                        prop = 0;
                    }
                }
            }
        }

        if (!finalwindow && children)
        {
            for (unsigned int i = 0; i < num_children && !finalwindow; i++)
            {
                finalwindow = X11Info::findClientWindow(children[i]);
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
