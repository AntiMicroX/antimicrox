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

X11Info::~X11Info()
{
    if (_display)
    {
        XCloseDisplay(display());
        _display = 0;
    }
}

Display* X11Info::display()
{
    return _instance._display;
}

unsigned long X11Info::appRootWindow(int screen)
{
    return screen == -1 ? XDefaultRootWindow(display()) : XRootWindowOfScreen(XScreenOfDisplay(display(), screen));
}

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

        Window parent = 1;
        Window root = 0;
        Window * children;
        unsigned int num_children;
        bool quitTraversal = false;

        while (!quitTraversal)
        {
            children = 0;
            XQueryTree(display, window, &root, &parent, &children, &num_children);
            if (children) { //must test for null
                XFree(children);
            }
            status = XGetWindowProperty(display, parent, atom, 0, 1024, false, AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &prop);
            if (status == 0 && prop)
            {
                pid = prop[1] << 8;
                pid += prop[0];

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
    }

    return pid;
}

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

Window X11Info::findClientInChildren(Window &window)
{
    Atom actual_type;
    int actual_format = 0;
    unsigned long nitems = 0;
    unsigned long bytes_after = 0;
    unsigned char *prop = 0;
    int status = 0;

    Window parent = 1;
    Window root = 0;
    Window * children;
    unsigned int num_children;
    Window finalwindow = 0;
    Display *display = X11Info::display();

    Atom atom = XInternAtom(display, "WM_STATE", True);
    XQueryTree(display, window, &root, &parent, &children, &num_children);
    if (children)
    {
        for (unsigned int i = 0; i < num_children; i++)
        {
            status = XGetWindowProperty(display, children[i], atom, 0, 1024, false, AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &prop);
            if (status == 0 && prop)
            {
                finalwindow = children[i];
            }
            XFree(prop);
        }
    }

    if (!finalwindow && children)
    {
        for (unsigned int i = 0; i < num_children && !finalwindow; i++)
        {
            finalwindow = X11Info::findClientInChildren(children[i]);
        }
    }

    XFree(children);

    return finalwindow;
}
