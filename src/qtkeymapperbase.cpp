#include "qtkeymapperbase.h"

const unsigned int QtKeyMapperBase::customQtKeyPrefix;
const unsigned int QtKeyMapperBase::customKeyPrefix;
const unsigned int QtKeyMapperBase::nativeKeyPrefix;

QtKeyMapperBase::QtKeyMapperBase(QObject *parent) :
    QObject(parent)
{
}

unsigned int QtKeyMapperBase::returnQtKey(unsigned int key, unsigned int scancode)
{
    Q_UNUSED(scancode);

    return virtualKeyToQtKey.value(key);
}

unsigned int QtKeyMapperBase::returnVirtualKey(unsigned int qkey)
{
    return qtKeyToVirtualKey.value(qkey);
}

bool QtKeyMapperBase::isModifier(unsigned int qkey)
{
    bool modifier = false;
    unsigned int qtKeyValue = qkey & 0x0FFFFFFF;

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
    return identifier;
}
