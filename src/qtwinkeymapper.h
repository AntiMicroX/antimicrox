#ifndef QTWINKEYMAPPER_H
#define QTWINKEYMAPPER_H

#include <QObject>
#include <QHash>

#include "qtkeymapperbase.h"

class QtWinKeyMapper : public QtKeyMapperBase
{
    Q_OBJECT
public:
    explicit QtWinKeyMapper(QObject *parent = 0);

protected:
    void populateMappingHashes();

signals:

public slots:

};

#endif // QTWINKEYMAPPER_H
