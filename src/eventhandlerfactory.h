/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
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

#ifndef EVENTHANDLERFACTORY_H
#define EVENTHANDLERFACTORY_H

#include <QObject>
#include <QStringList>

#ifdef WITH_UINPUT
    #include "eventhandlers/uinputeventhandler.h"
#endif

#ifdef WITH_XTEST
    #include "eventhandlers/xtesteventhandler.h"
#endif

#ifdef Q_OS_WIN
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

class BaseEventHandler;

class EventHandlerFactory : public QObject
{
    Q_OBJECT

  public:
    static EventHandlerFactory *getInstance(QString handler = "");
    void deleteInstance();
    BaseEventHandler *handler();
    static QString fallBackIdentifier();
    static QStringList buildEventGeneratorList();
    static QString handlerDisplayName(QString handler);

  protected:
    explicit EventHandlerFactory(QString handler, QObject *parent = nullptr);
    ~EventHandlerFactory();

    BaseEventHandler *eventHandler;
    static EventHandlerFactory *instance;
};

#endif // EVENTHANDLERFACTORY_H
