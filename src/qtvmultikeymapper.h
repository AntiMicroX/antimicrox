#ifndef QTVMULTIKEYMAPPER_H
#define QTVMULTIKEYMAPPER_H

#include <QObject>
#include <QHash>

#include "qtkeymapperbase.h"

class QtVMultiKeyMapper : public QtKeyMapperBase
{
    Q_OBJECT
public:
    explicit QtVMultiKeyMapper(QObject *parent = 0);

protected:
    void populateMappingHashes();

signals:

public slots:
};

#endif // QTVMULTIKEYMAPPER_H
