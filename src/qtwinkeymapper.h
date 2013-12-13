#ifndef QTWINKEYMAPPER_H
#define QTWINKEYMAPPER_H

#include <QObject>
#include <QHash>

#include "qtkeymapperbase.h"

class QtWinKeyMapper : public QtKeyMapperBase
{
    Q_OBJECT
public:
    static unsigned int returnVirtualKey(unsigned int qkey);
    static unsigned int returnQtKey(unsigned int key);

protected:
    explicit QtWinKeyMapper(QObject *parent = 0);
    void populateMappingHashes();
    unsigned int returnInstanceVirtualKey(unsigned int qkey);
    unsigned int returnInstanceQtKey(unsigned int key);

    static QtWinKeyMapper _instance;
    QHash<unsigned int, unsigned int> qtKeyToVirtualKey;
    QHash<unsigned int, unsigned int> virtualKeyToQtKey;

signals:

public slots:

};

#endif // QTWINKEYMAPPER_H
