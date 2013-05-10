#include <QDebug>

#include "x11info.h"

X11Info X11Info::_instance;

X11Info::X11Info(QObject *parent) :
    QObject(parent)
{
    _display = XOpenDisplay(NULL);
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
