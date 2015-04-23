#ifndef JOYKEYREPEATHELPER_H
#define JOYKEYREPEATHELPER_H

#include <QObject>
#include <QTimer>

#include "joybuttonslot.h"

class JoyKeyRepeatHelper : public QObject
{
    Q_OBJECT
public:
    explicit JoyKeyRepeatHelper(QObject *parent = 0);
    QTimer* getRepeatTimer();

    void setLastActiveKey(JoyButtonSlot *slot);
    JoyButtonSlot* getLastActiveKey();

    //void setKeyRepeatDelay(unsigned int repeatDelay);
    //unsigned int getKeyRepeatDelay();

    void setKeyRepeatRate(unsigned int repeatRate);
    unsigned int getKeyRepeatRate();

protected:
    QTimer keyRepeatTimer;
    JoyButtonSlot *lastActiveKey;
    unsigned int keyRepeatDelay;
    unsigned int keyRepeatRate;

signals:

private slots:
    void repeatKeysEvent();
};

#endif // JOYKEYREPEATHELPER_H
