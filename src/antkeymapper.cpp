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

//#include <QDebug>
#include <QtGlobal>
#include <QStringList>

#include "antkeymapper.h"
#include "eventhandlerfactory.h"

AntKeyMapper* AntKeyMapper::_instance = 0;

static QStringList buildEventGeneratorList()
{
    QStringList temp;

#ifdef Q_OS_WIN
    temp.append("sendinput");
  #ifdef WITH_VMULTI
    temp.append("vmulti");
  #endif

#else
  #ifdef WITH_XTEST
    temp.append("xtest");
  #endif
  #ifdef WITH_UINPUT
    temp.append("uinput");
  #endif

#endif
    return temp;
}

AntKeyMapper::AntKeyMapper(QString handler, QObject *parent) :
    QObject(parent)
{
    internalMapper = 0;

#ifdef Q_OS_WIN
  #ifdef WITH_VMULTI
    if (handler == "vmulti")
    {
        internalMapper = &vmultiMapper;
        nativeKeyMapper = &winMapper;
    }
  #endif

    BACKEND_ELSE_IF (handler == "sendinput")
    {
        internalMapper = &winMapper;
        nativeKeyMapper = 0;
    }

#else
    #ifdef WITH_XTEST
    if (handler == "xtest")
    {
        internalMapper = &x11Mapper;
        nativeKeyMapper = 0;
    }
    #endif

    #ifdef WITH_UINPUT
    if (handler == "uinput")
    {
        internalMapper = &uinputMapper;
#ifdef WITH_XTEST
        nativeKeyMapper = &x11Mapper;
#else
        nativeKeyMapper = 0;
#endif
    }
    #endif

#endif
}

AntKeyMapper* AntKeyMapper::getInstance(QString handler)
{
    if (!_instance)
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
    if (_instance)
    {
        delete _instance;
        _instance = 0;
    }
}

unsigned int AntKeyMapper::returnQtKey(unsigned int key, unsigned int scancode)
{
    return internalMapper->returnQtKey(key, scancode);
}

unsigned int AntKeyMapper::returnVirtualKey(unsigned int qkey)
{
    return internalMapper->returnVirtualKey(qkey);
}

bool AntKeyMapper::isModifierKey(unsigned int qkey)
{
    return internalMapper->isModifier(qkey);
}

QtKeyMapperBase* AntKeyMapper::getNativeKeyMapper()
{
    return nativeKeyMapper;
}

QtKeyMapperBase* AntKeyMapper::getKeyMapper()
{
    return internalMapper;
}

bool AntKeyMapper::hasNativeKeyMapper()
{
    bool result = nativeKeyMapper != 0;
    return result;
}
