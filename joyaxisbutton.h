#ifndef JOYAXISBUTTON_H
#define JOYAXISBUTTON_H

#include "joybutton.h"

class JoyAxis;

class JoyAxisButton : public JoyButton
{
    Q_OBJECT
public:
    explicit JoyAxisButton(JoyAxis *axis, QObject *parent = 0);
    explicit JoyAxisButton(JoyAxis *axis, int index, int originset, QObject *parent=0);
    
    virtual QString getXmlName();
    virtual double getDistanceFromDeadZone();

    static const QString xmlName;

protected:
    JoyAxis *axis;

signals:
    
public slots:
    virtual void mouseEvent();
};

#endif // JOYAXISBUTTON_H
