#include "qtkeymapperbase.h"

const unsigned int QtKeyMapperBase::customQtKeyPrefix;
const unsigned int QtKeyMapperBase::customKeyPrefix;
const unsigned int QtKeyMapperBase::nativeKeyPrefix;

QtKeyMapperBase::QtKeyMapperBase(QObject *parent) :
    QObject(parent)
{
}

unsigned int QtKeyMapperBase::returnQtKey(unsigned int key)
{
    return virtualKeyToQtKey.value(key);
}

unsigned int QtKeyMapperBase::returnVirtualKey(unsigned int qkey)
{
    return qtKeyToVirtualKey.value(qkey);
}
