/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail.com>
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
#include "messagehandler.h"

#include <QDebug>
#include <QHash>

static QHash<QString, QString> buildDisplayNames()
{
    QHash<QString, QString> temp;

    temp.insert("xtest", "Xtest");
    temp.insert("uinput", "uinput");

    return temp;
}

QHash<QString, QString> handlerDisplayNames = buildDisplayNames();

EventHandlerFactory *EventHandlerFactory::instance = nullptr;

EventHandlerFactory::EventHandlerFactory(QString handler, QObject *parent)
    : QObject(parent)
{
#ifdef WITH_UINPUT

    if (handler == "uinput")
        eventHandler = new UInputEventHandler(this);

#endif

#ifdef WITH_XTEST

    if (handler == "xtest")
        eventHandler = new XTestEventHandler(this);

#endif
}

EventHandlerFactory::~EventHandlerFactory()
{
    if (eventHandler != nullptr)
    {
        delete eventHandler;
        eventHandler = nullptr;
    }
}

EventHandlerFactory *EventHandlerFactory::getInstance(QString handler)
{
    if (instance == nullptr)
    {
        QStringList temp = buildEventGeneratorList();

        if (!handler.isEmpty() && temp.contains(handler))
            instance = new EventHandlerFactory(handler);
        else
            instance = new EventHandlerFactory(fallBackIdentifier());
    }

    return instance;
}

void EventHandlerFactory::deleteInstance()
{
    if (instance != nullptr)
    {
        delete instance;
        instance = nullptr;
    }
}

BaseEventHandler *EventHandlerFactory::handler() { return eventHandler; }

QString EventHandlerFactory::fallBackIdentifier()
{
    QString temp = QString();

#if defined(WITH_XTEST)
    temp = "xtest";
#elif defined(WITH_UINPUT)
    temp = "uinput";
#else
    temp = "xtest";
#endif

    return temp;
}

QStringList EventHandlerFactory::buildEventGeneratorList()
{
    QStringList temp = QStringList();

    temp.append("xtest");
    temp.append("uinput");

    return temp;
}

QString EventHandlerFactory::handlerDisplayName(QString handler)
{
    QString handlerDispName = QString();

    if (handlerDisplayNames.contains(handler))
        handlerDispName = handlerDisplayNames.value(handler);

    return handlerDispName;
}
