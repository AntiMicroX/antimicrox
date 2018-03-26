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

#include "qtkeymapperbase.h"

#include <QDebug>

const int QtKeyMapperBase::customQtKeyPrefix;
const int QtKeyMapperBase::customKeyPrefix;
const int QtKeyMapperBase::nativeKeyPrefix;

QtKeyMapperBase::QtKeyMapperBase(QObject *parent) :
    QObject(parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
}

int QtKeyMapperBase::returnQtKey(int key, int scancode)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    Q_UNUSED(scancode);

    return virtualKeyToQtKey.value(key);
}

int QtKeyMapperBase::returnVirtualKey(int qkey)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return qtKeyToVirtualKey.value(qkey);
}

bool QtKeyMapperBase::isModifier(int qkey)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    bool modifier = false;
    int qtKeyValue = qkey & 0x0FFFFFFF;

    if (qtKeyValue == Qt::Key_Shift)
    {
        modifier = true;
    }
    else if (qtKeyValue == Qt::Key_Control)
    {
        modifier = true;
    }
    else if (qtKeyValue == Qt::Key_Alt)
    {
        modifier = true;
    }
    else if (qtKeyValue == Qt::Key_Meta)
    {
        modifier = true;
    }

    return modifier;
}

QtKeyMapperBase::charKeyInformation QtKeyMapperBase::getCharKeyInformation(QChar value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    charKeyInformation temp;
    temp.virtualkey = 0;
    temp.modifiers = Qt::NoModifier;

    if (virtualkeyToCharKeyInformation.contains(value.unicode()))
    {
        temp = virtualkeyToCharKeyInformation.value(value.unicode());
    }

    return temp;
}

/**
 * @brief Obtain identifier string for key mapper.
 * @return Identifier string.
 */
QString QtKeyMapperBase::getIdentifier()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    return identifier;
}
