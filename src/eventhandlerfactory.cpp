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
#include "logger.h"

#include "eventhandlers/baseeventhandler.h"

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
    static QString temp = "xtest";
    static bool identifier_obtained = false;
    if (identifier_obtained)
        return temp;
    QString detected_xdg_session = qgetenv("XDG_SESSION_TYPE");

    bool compiled_with_x11 = false;
    bool compiled_with_uinput = false;
#if defined(WITH_XTEST)
    compiled_with_x11 = true;
#endif
#if defined(WITH_UINPUT)
    compiled_with_uinput = true;
#endif

    if (detected_xdg_session == "wayland")
    {
        if (compiled_with_uinput)
        {
            PRINT_STDOUT() << "Selecting uinput as a default event generator.";
            qInfo() << "uinput is default for wayland";
            temp = "uinput";
        } else
        {
            qWarning() << "Detected wayland session, but there is no support for uinput detected, defaulting to xtest.";
            temp = "xtest";
        }
    }
    if (!compiled_with_uinput && !compiled_with_x11)
        qWarning() << "Neither uinput nor xtest support is detected.";
    identifier_obtained = true;
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
