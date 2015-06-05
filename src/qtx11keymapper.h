#ifndef QTX11KEYMAPPER_H
#define QTX11KEYMAPPER_H

#include <QObject>
#include <QHash>
#include <QChar>

#include "qtkeymapperbase.h"

class QtX11KeyMapper : public QtKeyMapperBase
{
    Q_OBJECT
public:
    explicit QtX11KeyMapper(QObject *parent = 0);
    charKeyInformation getCharKeyInformation(QChar value);

protected:
    void populateMappingHashes();
    void populateCharKeyInformation();

signals:

public slots:

};

#endif // QTX11KEYMAPPER_H
