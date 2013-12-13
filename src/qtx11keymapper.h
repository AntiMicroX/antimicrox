#ifndef QTX11KEYMAPPER_H
#define QTX11KEYMAPPER_H

#include <QObject>
#include <QHash>

#include "qtkeymapperbase.h"

class QtX11KeyMapper : public QtKeyMapperBase
{
    Q_OBJECT
public:
    static unsigned int returnVirtualKey(unsigned int qkey);
    static unsigned int returnQtKey(unsigned int key);

protected:
    explicit QtX11KeyMapper(QObject *parent = 0);
    void populateMappingHashes();
    unsigned int returnInstanceVirtualKey(unsigned int qkey);
    unsigned int returnInstanceQtKey(unsigned int key);

    static QtX11KeyMapper _instance;
    QHash<unsigned int, unsigned int> qtKeyToVirtualKey;
    QHash<unsigned int, unsigned int> virtualKeyToQtKey;

signals:

public slots:

};

#endif // QTX11KEYMAPPER_H
