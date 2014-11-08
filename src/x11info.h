#ifndef X11INFO_H
#define X11INFO_H

#include <QObject>
#include <QString>
#include <QHash>
#include <X11/Xlib.h>

class X11Info : public QObject
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
    void closeDisplay();
    void syncDisplay();
    void syncDisplay(QString displayString);
    QString getXDisplayString();

    static X11Info* getInstance();
    static void deleteInstance();

protected:
    explicit X11Info(QObject *parent = 0);
    ~X11Info();

    void populateKnownAliases();
    bool windowHasProperty(Display *display, Window window, Atom atom);
    bool windowIsViewable(Display *display, Window window);

    Display *_display;
    static X11Info *_instance;
    QHash<QString, QString> knownAliases;
    QString _customDisplayString;

signals:
    
public slots:
    
};

#endif // X11INFO_H
