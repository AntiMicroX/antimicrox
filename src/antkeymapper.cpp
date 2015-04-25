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
    temp.append("xtest");
    temp.append("uinput");

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
    }
  #endif

    BACKEND_ELSE_IF (handler == "sendinput")
    {
        internalMapper = &winMapper;
    }

#else
    #ifdef WITH_XTEST
    if (handler == "xtest")
    {
        internalMapper = &x11Mapper;
    }
    #endif

    #ifdef WITH_UINPUT
    if (handler == "uinput")
    {
        internalMapper = &uinputMapper;
    }
    #endif

#endif
}

AntKeyMapper* AntKeyMapper::getInstance(QString handler)
{
    if (!_instance)
    {
        QStringList temp = buildEventGeneratorList();
        if (temp.contains(handler))
        {
            _instance = new AntKeyMapper(handler);
        }
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
