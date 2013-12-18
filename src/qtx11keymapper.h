#ifndef QTX11KEYMAPPER_H
#define QTX11KEYMAPPER_H

#include <QObject>
#include <QHash>

#include "qtkeymapperbase.h"

class QtX11KeyMapper : public QtKeyMapperBase
{
    Q_OBJECT
public:
    explicit QtX11KeyMapper(QObject *parent = 0);

protected:
    void populateMappingHashes();

signals:

public slots:

};

#endif // QTX11KEYMAPPER_H
