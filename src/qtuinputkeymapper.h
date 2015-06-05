#ifndef QTUINPUTKEYMAPPER_H
#define QTUINPUTKEYMAPPER_H

#include <QObject>
#include <QHash>

#include "qtkeymapperbase.h"

class QtUInputKeyMapper : public QtKeyMapperBase
{
    Q_OBJECT
public:
    typedef struct _charKeyInformation
    {
        Qt::KeyboardModifiers modifiers;
        unsigned int virtualkey;
    } charKeyInformation;

    explicit QtUInputKeyMapper(QObject *parent = 0);
    charKeyInformation getCharKeyInformation(QChar value);

protected:
    void populateMappingHashes();
    void populateCharKeyInformation();

    void populateAlphaHashes();
    void populateFKeyHashes();
    void populateNumPadHashes();
    void populateSpecialCharHashes();

    QHash<unsigned int, charKeyInformation> virtualkeyToCharKeyInformation;

signals:

public slots:

};

#endif // QTUINPUTKEYMAPPER_H
