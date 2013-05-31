#ifndef JOYCONTROLSTICK_H
#define JOYCONTROLSTICK_H

#include <QObject>
#include <QHash>

#include "joyaxis.h"
#include "joybutton.h"
#include "joycontrolstickbutton.h"

class JoyControlStick : public QObject
{
    Q_OBJECT
public:
    explicit JoyControlStick(JoyAxis *axis1, JoyAxis *axis2, int originset = 0, QObject *parent = 0);
    ~JoyControlStick();

    void joyEvent(bool ignoresets=false);
    bool inDeadZone();

    int getDeadZone();
    int getDiagonalRange();
    double getDistanceFromDeadZone();

    enum JoyStickDirections {
        StickCentered = 0, StickUp = 1, StickRight = 2,
        StickDown = 4, StickLeft = 8, StickRightUp = 3,
        StickRightDown = 6, StickLeftUp = 9, StickLeftDown = 12
    };

    static double PI;

protected:
    void populateButtons();
    void createDeskEvent(bool ignoresets = false);
    double calculateBearing();
    void changeButtonEvent(JoyControlStickButton *eventbutton, JoyControlStickButton *&activebutton, bool ignoresets);

    JoyAxis *axis1;
    JoyAxis *axis2;
    int originset;
    int deadZone;
    int diagonalRange;
    int maxZone;
    bool isActive;
    JoyControlStickButton *activeButton1;
    JoyControlStickButton *activeButton2;
    bool safezone;

    QHash<JoyStickDirections, JoyControlStickButton*> buttons;


signals:
    void moved(int xaxis, int yaxis);
    void active(int xaxis, int yaxis);
    void released(int axis, int yaxis);

public slots:
    
};

#endif // JOYCONTROLSTICK_H
