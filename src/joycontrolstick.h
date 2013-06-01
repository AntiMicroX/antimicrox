#ifndef JOYCONTROLSTICK_H
#define JOYCONTROLSTICK_H

#include <QObject>
#include <QHash>

#include "joyaxis.h"
#include "joybutton.h"
#include "joycontrolstickdirectionstype.h"
#include "joycontrolstickbutton.h"

class JoyControlStick : public QObject, public JoyStickDirectionsType
{
    Q_OBJECT
public:
    explicit JoyControlStick(JoyAxis *axis1, JoyAxis *axis2, int index, int originset = 0, QObject *parent = 0);
    ~JoyControlStick();

    void joyEvent(bool ignoresets=false);
    bool inDeadZone();
    int getDeadZone();
    int getDiagonalRange();
    double getDistanceFromDeadZone();
    int getIndex();
    void setIndex(int index);
    int getRealJoyIndex();
    int getMaxZone();
    int getCurrentlyAssignedSet();
    QString getName();
    JoyStickDirections getCurrentDirection();

    JoyControlStickButton* getDirectionButton(JoyStickDirections direction);
    double calculateNormalizedAxis1Placement();
    double calculateNormalizedAxis2Placement();
    double calculateDirectionalDistance(JoyControlStickButton *button);

    void readConfig(QXmlStreamReader *xml);
    void writeConfig(QXmlStreamWriter *xml);

    static double PI;

protected:
    void populateButtons();
    void createDeskEvent(bool ignoresets = false);
    double calculateBearing();
    void changeButtonEvent(JoyControlStickButton *eventbutton, JoyControlStickButton *&activebutton, bool ignoresets);
    void refreshButtons();
    void deleteButtons();
    void resetButtons();

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
    int index;
    JoyStickDirections currentDirection;

    QHash<JoyStickDirections, JoyControlStickButton*> buttons;


signals:
    void moved(int xaxis, int yaxis);
    void active(int xaxis, int yaxis);
    void released(int axis, int yaxis);

public slots:
    void reset();
    void setDeadZone(int value);
    void setMaxZone(int value);
    void setDiagonalRange(int value);
};

#endif // JOYCONTROLSTICK_H
