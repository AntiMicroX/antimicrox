#ifndef ANTKEYMAPPER_H
#define ANTKEYMAPPER_H

#include <QObject>

#ifdef Q_OS_WIN
#include "qtwinkeymapper.h"
#else

#if defined(WITH_XTEST)
#include "qtx11keymapper.h"
#elif defined(WITH_UINPUT)
#include "qtuinputkeymapper.h"
#endif

#endif

class AntKeyMapper : public QObject
{
    Q_OBJECT
public:
    static unsigned int returnVirtualKey(unsigned int qkey);
    static unsigned int returnQtKey(unsigned int key, unsigned int scancode=0);
    static bool isModifierKey(unsigned int qkey);

protected:
    explicit AntKeyMapper(QObject *parent = 0);

    static AntKeyMapper _instance;

#ifdef Q_OS_WIN
    QtWinKeyMapper internalMapper;
#else

#if defined(WITH_XTEST)
    QtX11KeyMapper internalMapper;
#elif defined(WITH_UINPUT)
    QtUInputKeyMapper internalMapper;
#endif

#endif

signals:

public slots:

};

#endif // ANTKEYMAPPER_H
