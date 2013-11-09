#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <QObject>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <SDL/SDL_joystick.h>

#include "joyaxis.h"
#include "joydpad.h"
#include "joybutton.h"
#include "setjoystick.h"
#include "common.h"

class Joystick : public QObject
{
    Q_OBJECT
public:
    explicit Joystick(SDL_Joystick *joyhandle, QObject *parent=0);
    ~Joystick();

    int getNumberButtons ();
    int getNumberAxes();
    int getNumberHats();
    int getNumberSticks();
    int getNumberVDPads();
    SDL_Joystick* getSDLHandle ();
    int getJoyNumber ();
    int getRealJoyNumber ();
    QString getName();
    int getActiveSetNumber();
    SetJoystick* getActiveSetJoystick();
    SetJoystick* getSetJoystick(int index);
    void removeControlStick(int index);

    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

    static const int NUMBER_JOYSETS;

protected:
    SDL_Joystick* joyhandle;
    QHash<int, SetJoystick*> joystick_sets;
    int active_set;
    int joyNumber;

signals:
    void setChangeActivated(int index);
    void setAxisThrottleActivated(int index);

public slots:
    void reset();
    void setActiveSetNumber(int index);
    void changeSetButtonAssociation(int button_index, int originset, int newset, int mode);
    void changeSetAxisButtonAssociation(int button_index, int axis_index, int originset, int newset, int mode);
    void changeSetStickButtonAssociation(int button_index, int stick_index, int originset, int newset, int mode);
    void changeSetDPadButtonAssociation(int button_index, int dpad_index, int originset, int newset, int mode);


private slots:
    void propogateSetChange(int index);
    void propogateSetAxisThrottleChange(int index, int originset);
};

Q_DECLARE_METATYPE(Joystick*)

#endif // JOYSTICK_H
