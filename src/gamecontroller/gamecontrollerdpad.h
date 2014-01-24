#ifndef GAMECONTROLLERDPAD_H
#define GAMECONTROLLERDPAD_H

#include <QObject>

#include "vdpad.h"

class GameControllerDPad : public VDPad
{
    Q_OBJECT
public:
    explicit GameControllerDPad(JoyButton *upButton, JoyButton *downButton, JoyButton *leftButton, JoyButton *rightButton,
                                int index, int originset, SetJoystick *parentSet, QObject *parent = 0);

    virtual QString getName(bool forceFullFormat, bool displayName);
    virtual QString getXmlName();

    static const QString xmlName;

signals:

public slots:

};

#endif // GAMECONTROLLERDPAD_H
