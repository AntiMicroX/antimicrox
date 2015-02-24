#ifndef INPUTDEVICESTATUSEVENT_H
#define INPUTDEVICESTATUSEVENT_H

#include <QObject>
#include <QList>
#include <QBitArray>

#include "inputdevice.h"

class InputDeviceBitArrayStatus : public QObject
{
    Q_OBJECT
public:
    explicit InputDeviceBitArrayStatus(InputDevice *device, bool readCurrent = true, QObject *parent = 0);

    void changeAxesStatus(int axisIndex, bool value);
    void changeButtonStatus(int buttonIndex, bool value);
    void changeHatStatus(int hatIndex, bool value);

    QBitArray generateFinalBitArray();
    void clearStatusValues();

protected:
    QList<bool> axesStatus;
    QList<bool> hatButtonStatus;
    QBitArray buttonStatus;

signals:

public slots:

};

#endif // INPUTDEVICESTATUSEVENT_H
