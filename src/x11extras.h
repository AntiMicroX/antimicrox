#ifndef X11EXTRAS_H
#define X11EXTRAS_H

#include <QObject>
#include <QString>
#include <QHash>
#include <X11/Xlib.h>

class X11Extras : public QObject
{
    Q_OBJECT
public:

    unsigned long appRootWindow(int screen = -1);
    Display* display();
    bool hasValidDisplay();
    QString getDisplayString(QString xcodestring);
    int getApplicationPid(Window window);
    QString getApplicationLocation(int pid);
    Window findClientWindow(Window window);
    Window findParentClient(Window window);
    void closeDisplay();
    void syncDisplay();
    void syncDisplay(QString displayString);
    QString getXDisplayString();
    QString getWindowTitle(Window window);
    QString getWindowClass(Window window);
    unsigned long getWindowInFocus();

    static X11Extras* getInstance();
    static void deleteInstance();

protected:
    explicit X11Extras(QObject *parent = 0);
    ~X11Extras();

    void populateKnownAliases();
    bool windowHasProperty(Display *display, Window window, Atom atom);
    bool windowIsViewable(Display *display, Window window);
    bool isWindowRelevant(Display *display, Window window);

    Display *_display;
    static X11Extras *_instance;
    QHash<QString, QString> knownAliases;
    QString _customDisplayString;

signals:
    
public slots:
    
};

#endif // X11EXTRAS_H
