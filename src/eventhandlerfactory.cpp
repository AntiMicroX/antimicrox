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

#include "eventhandlerfactory.h"
#include "eventhandlers/baseeventhandler.h"

#include <QHash>
#include <QDebug>


static QHash<QString, QString> buildDisplayNames()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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

EventHandlerFactory* EventHandlerFactory::instance = nullptr;

EventHandlerFactory::EventHandlerFactory(QString handler, QObject *parent) :
    QObject(parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (eventHandler != nullptr)
    {
        delete eventHandler;
        eventHandler = nullptr;
    }
}

EventHandlerFactory* EventHandlerFactory::getInstance(QString handler)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (instance == nullptr)
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    if (instance != nullptr)
    {
        delete instance;
        instance = nullptr;
    }
}

BaseEventHandler* EventHandlerFactory::handler()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return eventHandler;
}

QString EventHandlerFactory::fallBackIdentifier()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QString temp = QString();
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QStringList temp = QStringList();

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    QString temp = QString();
    if (handlerDisplayNames.contains(handler))
    {
        temp = handlerDisplayNames.value(handler);
    }

    return temp;
}
