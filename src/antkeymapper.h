#ifndef ANTKEYMAPPER_H
#define ANTKEYMAPPER_H

#include <QObject>

#ifdef Q_OS_WIN
  #include "qtwinkeymapper.h"

  #ifdef WITH_VMULTI
    #include "qtvmultikeymapper.h"
  #endif
#else

  #if defined(WITH_XTEST)
    #include "qtx11keymapper.h"
  #endif

  #if defined(WITH_UINPUT)
    #include "qtuinputkeymapper.h"
  #endif
#endif

class AntKeyMapper : public QObject
{
    Q_OBJECT
public:
    static AntKeyMapper* getInstance(QString handler = "");
    void deleteInstance();

    unsigned int returnVirtualKey(unsigned int qkey);
    unsigned int returnQtKey(unsigned int key, unsigned int scancode=0);
    bool isModifierKey(unsigned int qkey);
    QtKeyMapperBase* getNativeKeyMapper();

protected:
    explicit AntKeyMapper(QString handler = "", QObject *parent = 0);

    static AntKeyMapper *_instance;
    QtKeyMapperBase *internalMapper;
    QtKeyMapperBase *nativeKeyMapper;

#ifdef Q_OS_WIN
    QtWinKeyMapper winMapper;

  #ifdef WITH_VMULTI
    QtVMultiKeyMapper vmultiMapper;
  #endif

#else
  #if defined(WITH_XTEST)
    QtX11KeyMapper x11Mapper;
  #endif

  #if defined(WITH_UINPUT)
    QtUInputKeyMapper uinputMapper;
  #endif

#endif

signals:

public slots:

};

#endif // ANTKEYMAPPER_H
