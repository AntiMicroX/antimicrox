#ifndef GAMECONTROLLERBUTTON_H
#define GAMECONTROLLERBUTTON_H

#include <QObject>

#include <joyaxisbutton.h>

class GameControllerTriggerButton : public JoyAxisButton
{
    Q_OBJECT
public:
    explicit GameControllerTriggerButton(JoyAxis *axis, int index, int originset, QObject *parent = 0);

    virtual QString getXmlName();

    static const QString xmlName;

signals:

public slots:

};

#endif // GAMECONTROLLERBUTTON_H
