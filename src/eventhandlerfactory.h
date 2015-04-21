#ifndef EVENTHANDLERFACTORY_H
#define EVENTHANDLERFACTORY_H

#include <QObject>
#include <QStringList>

#ifdef Q_OS_UNIX
  #ifdef WITH_UINPUT
    #include "eventhandlers/uinputeventhandler.h"
  #endif

  #ifdef WITH_XTEST
    #include "eventhandlers/xtesteventhandler.h"
  #endif
#elif Q_OS_WIN
  #include "eventhandlers/winsendinputeventhandler.h"

  #ifdef WITH_VMULTI
    #include "eventhandlers/winvmultieventhandler.h"
  #endif
#endif

class EventHandlerFactory : public QObject
{
    Q_OBJECT
public:
    static EventHandlerFactory* getInstance(QString handler = "");
    void deleteInstance();
    BaseEventHandler* handler();
    static QString fallBackIdentifier();
    static QStringList buildEventGeneratorList();
    static QString handlerDisplayName(QString handler);

protected:
    explicit EventHandlerFactory(QString handler, QObject *parent = 0);
    ~EventHandlerFactory();

    BaseEventHandler *eventHandler;
    static EventHandlerFactory *instance;

signals:

public slots:

};

#endif // EVENTHANDLERFACTORY_H
