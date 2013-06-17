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
    ~X11Info();

    static unsigned long appRootWindow(int screen = -1);
    static Display* display();
    static QString getDisplayString(QString xcodestring);

protected:
    explicit X11Info(QObject *parent = 0);
    void populateKnownAliases();

    Display *_display;
    static X11Info _instance;
    QHash<QString, QString> knownAliases;

signals:
    
public slots:
    
};

#endif // X11INFO_H
