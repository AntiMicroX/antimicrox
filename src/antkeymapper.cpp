#include "antkeymapper.h"

AntKeyMapper AntKeyMapper::_instance;

AntKeyMapper::AntKeyMapper(QObject *parent) :
    QObject(parent)
{
}

unsigned int AntKeyMapper::returnQtKey(unsigned int key, unsigned int scancode)
{
    return _instance.internalMapper.returnQtKey(key, scancode);
}

unsigned int AntKeyMapper::returnVirtualKey(unsigned int qkey)
{
    return _instance.internalMapper.returnVirtualKey(qkey);
}

bool AntKeyMapper::isModifierKey(unsigned int qkey)
{
    return _instance.internalMapper.isModifier(qkey);
}
