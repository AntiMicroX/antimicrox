#ifndef QTVMULTIKEYMAPPER_H
#define QTVMULTIKEYMAPPER_H

#include <QObject>
#include <QHash>

#include "qtkeymapperbase.h"
#include "qtwinkeymapper.h"

class QtVMultiKeyMapper : public QtKeyMapperBase
{
    Q_OBJECT
public:
    explicit QtVMultiKeyMapper(QObject *parent = 0);

    static const unsigned int consumerUsagePagePrefix = 0x12000;

protected:
    void populateMappingHashes();

    static QtWinKeyMapper nativeKeyMapper;

signals:

public slots:
};

#endif // QTVMULTIKEYMAPPER_H
