/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QHash>

#include "eventhandlerfactory.h"

static QHash<QString, QString> buildDisplayNames()
{
    QHash<QString, QString> temp;
#ifdef Q_OS_WIN
    temp.insert("sendinput", "SendInput");
  #ifdef WITH_VMULTI
    temp.insert("vmulti", "Vmulti");
  #endif
#else
    temp.insert("xtest", "Xtest");
    temp.insert("uinput", "uinput");
#endif
    return temp;
}

QHash<QString, QString> handlerDisplayNames = buildDisplayNames();

EventHandlerFactory* EventHandlerFactory::instance = 0;

EventHandlerFactory::EventHandlerFactory(QString handler, QObject *parent) :
    QObject(parent)
{
#ifdef Q_OS_UNIX
    #ifdef WITH_UINPUT
    if (handler == "uinput")
    {
        eventHandler = new UInputEventHandler(this);
    }
    #endif

    #ifdef WITH_XTEST
    if (handler == "xtest")
    {
        eventHandler = new XTestEventHandler(this);
    }
    #endif
#elif defined(Q_OS_WIN)
    if (handler == "sendinput")
    {
        eventHandler = new WinSendInputEventHandler(this);
    }
  #ifdef WITH_VMULTI
    else if (handler == "vmulti")
    {
        eventHandler = new WinVMultiEventHandler(this);
    }
  #endif
#endif
}

EventHandlerFactory::~EventHandlerFactory()
{
    if (eventHandler)
    {
        delete eventHandler;
        eventHandler = 0;
    }
}

EventHandlerFactory* EventHandlerFactory::getInstance(QString handler)
{
    if (!instance)
    {
        QStringList temp = buildEventGeneratorList();
        if (!handler.isEmpty() && temp.contains(handler))
        {
            instance = new EventHandlerFactory(handler);
        }
        else
        {
            instance = new EventHandlerFactory(fallBackIdentifier());
        }
    }

    return instance;
}

void EventHandlerFactory::deleteInstance()
{
    if (instance)
    {
        delete instance;
        instance = 0;
    }
}

BaseEventHandler* EventHandlerFactory::handler()
{
    return eventHandler;
}

QString EventHandlerFactory::fallBackIdentifier()
{
    QString temp;
#ifdef Q_OS_UNIX
  #if defined(WITH_XTEST)
    temp = "xtest";
  #elif defined(WITH_UINPUT)
    temp = "uinput";
  #else
    temp = "xtest";
  #endif
#elif defined(Q_OS_WIN)
    temp = "sendinput";
#endif

    return temp;
}

QStringList EventHandlerFactory::buildEventGeneratorList()
{
    QStringList temp;

#ifdef Q_OS_WIN
    temp.append("sendinput");
  #ifdef WITH_VMULTI
    temp.append("vmulti");
  #endif
#else
    temp.append("xtest");
    temp.append("uinput");
#endif
    return temp;
}

QString EventHandlerFactory::handlerDisplayName(QString handler)
{
    QString temp;
    if (handlerDisplayNames.contains(handler))
    {
        temp = handlerDisplayNames.value(handler);
    }

    return temp;
}
