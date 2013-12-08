#ifndef VDPAD_H
#define VDPAD_H

#include "joydpad.h"
#include "joybutton.h"

class VDPad : public JoyDPad
{
    Q_OBJECT
public:
    explicit VDPad(int index, int originset, QObject *parent = 0);
    explicit VDPad(JoyButton *upButton, JoyButton *downButton, JoyButton *leftButton, JoyButton *rightButton,
                   int index, int originset, QObject *parent = 0);
    ~VDPad();

    void joyEvent (bool pressed, bool ignoresets=false);
    void addVButton(JoyDPadButton::JoyDPadDirections direction, JoyButton *button);
    void removeVButton(JoyDPadButton::JoyDPadDirections direction);
    void removeVButton(JoyButton *button);
    JoyButton* getVButton(JoyDPadButton::JoyDPadDirections direction);
    bool isEmpty();
    virtual QString getName(bool forceFullFormat=false, bool displayName=false);
    virtual QString getXmlName();

    static const QString xmlName;

protected:
    JoyButton *upButton;
    JoyButton *downButton;
    JoyButton *leftButton;
    JoyButton *rightButton;

signals:

public slots:
    
};

#endif // VDPAD_H
