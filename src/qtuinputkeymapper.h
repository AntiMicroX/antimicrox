#ifndef QTUINPUTKEYMAPPER_H
#define QTUINPUTKEYMAPPER_H

#include <QObject>
#include <QHash>

#include "qtkeymapperbase.h"

class QtUInputKeyMapper : public QtKeyMapperBase
{
    Q_OBJECT
public:
    explicit QtUInputKeyMapper(QObject *parent = 0);
    charKeyInformation getCharKeyInformation(QChar value);

protected:
    void populateMappingHashes();
    void populateCharKeyInformation();

    void populateAlphaHashes();
    void populateFKeyHashes();
    void populateNumPadHashes();
    void populateSpecialCharHashes();

signals:

public slots:

};

#endif // QTUINPUTKEYMAPPER_H
