/* antimicroX Gamepad to KB+M event mapper
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

#include "antkeymapper.h"

#include "messagehandler.h"
#include "eventhandlerfactory.h"

#include <QDebug>
#include <QtGlobal>
#include <QStringList>


AntKeyMapper* AntKeyMapper::_instance = nullptr;

static QStringList buildEventGeneratorList()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QStringList temp = QStringList();

  #ifdef WITH_XTEST
    temp.append("xtest");
  #endif
  #ifdef WITH_UINPUT
    temp.append("uinput");
  #endif

    return temp;
}

AntKeyMapper::AntKeyMapper(QString handler, QObject *parent) :
    QObject(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    internalMapper = nullptr;

#if defined(Q_OS_UNIX)
    #ifdef WITH_XTEST
    if (handler == "xtest")
    {
        internalMapper = &x11Mapper;
        nativeKeyMapper = nullptr;
    }
    #endif

    #ifdef WITH_UINPUT
    if (handler == "uinput")
    {
        internalMapper = &uinputMapper;
#ifdef WITH_XTEST
        nativeKeyMapper = &x11Mapper;
#else
        nativeKeyMapper = nullptr;
#endif
    }
    #endif

#endif
}

AntKeyMapper* AntKeyMapper::getInstance(QString handler)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (_instance == nullptr)
    {
        Q_ASSERT(!handler.isEmpty());
        QStringList temp = buildEventGeneratorList();
        Q_ASSERT(temp.contains(handler));
        _instance = new AntKeyMapper(handler);
    }

    return _instance;
}

void AntKeyMapper::deleteInstance()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (_instance != nullptr)
    {
        delete _instance;
        _instance = nullptr;
    }
}

int AntKeyMapper::returnQtKey(int key, int scancode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return internalMapper->returnQtKey(key, scancode);
}

int AntKeyMapper::returnVirtualKey(int qkey)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return internalMapper->returnVirtualKey(qkey);
}

bool AntKeyMapper::isModifierKey(int qkey)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return internalMapper->isModifier(qkey);
}

QtKeyMapperBase* AntKeyMapper::getNativeKeyMapper() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return nativeKeyMapper;
}

QtKeyMapperBase* AntKeyMapper::getKeyMapper() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return internalMapper;
}

bool AntKeyMapper::hasNativeKeyMapper()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = (nativeKeyMapper != nullptr);
    return result;
}
