#ifndef ANTKEYMAPPER_H
#define ANTKEYMAPPER_H

#include <QObject>

#ifdef Q_OS_WIN
#include "qtwinkeymapper.h"
#else
#include "qtx11keymapper.h"
#endif

class AntKeyMapper : public QObject
{
    Q_OBJECT
public:
    static unsigned int returnVirtualKey(unsigned int qkey);
    static unsigned int returnQtKey(unsigned int key, unsigned int scancode=0);

protected:
    explicit AntKeyMapper(QObject *parent = 0);

    static AntKeyMapper _instance;

#ifdef Q_OS_WIN
    QtWinKeyMapper internalMapper;
#else
    QtX11KeyMapper internalMapper;
#endif

signals:

public slots:

};

#endif // ANTKEYMAPPER_H
