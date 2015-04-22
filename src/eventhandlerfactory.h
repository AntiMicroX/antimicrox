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
#elif defined(Q_OS_WIN)
  #include "eventhandlers/winsendinputeventhandler.h"

  #ifdef WITH_VMULTI
    #include "eventhandlers/winvmultieventhandler.h"
  #endif
#endif

#ifdef Q_OS_WIN
  #define ADD_SENDINPUT 1
  #ifdef WITH_VMULTI
    #define ADD_VMULTI 1
  #else
    #define ADD_VMULTI 0
  #endif

  #define NUM_BACKENDS (ADD_SENDINPUT + ADD_VMULTI)
#else
  #ifdef WITH_XTEST
    #define ADD_XTEST 1
  #else
    #define ADD_XTEST 0
  #endif

  #ifdef WITH_UINPUT
    #define ADD_UINPUT 1
  #else
    #define ADD_UINPUT 0
  #endif

  #define NUM_BACKENDS (ADD_XTEST + ADD_UINPUT)
#endif

#if (NUM_BACKENDS > 1)
  #define BACKEND_ELSE_IF else if
#else
  #define BACKEND_ELSE_IF if
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
