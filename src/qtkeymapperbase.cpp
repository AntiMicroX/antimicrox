#include "qtkeymapperbase.h"

const unsigned int QtKeyMapperBase::customQtKeyPrefix;
const unsigned int QtKeyMapperBase::customKeyPrefix;
const unsigned int QtKeyMapperBase::nativeKeyPrefix;

QtKeyMapperBase::QtKeyMapperBase(QObject *parent) :
    QObject(parent)
{
}
