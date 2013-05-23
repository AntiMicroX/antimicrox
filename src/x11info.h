#ifndef X11INFO_H
#define X11INFO_H

#include <QObject>
#include <X11/Xlib.h>

class X11Info : public QObject
{
    Q_OBJECT
public:
    ~X11Info();

    static unsigned long appRootWindow(int screen = -1);
    static Display* display();

protected:
    explicit X11Info(QObject *parent = 0);

    Display *_display;
    static X11Info _instance;

signals:
    
public slots:
    
};

#endif // X11INFO_H
