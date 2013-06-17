#include <QDebug>

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
