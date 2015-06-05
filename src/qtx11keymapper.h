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
    typedef struct _charKeyInformation
    {
        Qt::KeyboardModifiers modifiers;
        unsigned int virtualkey;
    } charKeyInformation;

    explicit QtX11KeyMapper(QObject *parent = 0);
    charKeyInformation getCharKeyInformation(QChar value);

protected:
    void populateMappingHashes();
    void populateCharKeyInformation();

    QHash<unsigned int, charKeyInformation> virtualkeyToCharKeyInformation;

signals:

public slots:

};

#endif // QTX11KEYMAPPER_H
