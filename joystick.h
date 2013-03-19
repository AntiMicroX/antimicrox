#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <QObject>
#include <QHash>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <SDL/SDL.h>

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

    int getNumberButtons ();
    int getNumberAxes();
    int getNumberHats();
    SDL_Joystick* getSDLHandle ();
    int getJoyNumber ();
    int getRealJoyNumber ();
    //void refreshButtons ();
    //void refreshAxes();
    //void refreshHats();
    /*JoyAxis* getJoyAxis(int index);
    JoyButton* getJoyButton(int index);
    JoyDPad* getJoyDPad(int index);*/
    QString getName();
    int getActiveSetNumber();
    SetJoystick* getActiveSetJoystick();
    SetJoystick* getSetJoystick(int index);

    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

    static const int NUMBER_JOYSETS;

protected:
    QHash<int, JoyButton*> buttons;
    QHash<int, JoyAxis*> axes;
    QHash<int, JoyDPad*> hats;

    SDL_Joystick* joyhandle;
    QHash<int, SetJoystick*> joystick_sets;
    int active_set;

signals:
    void setChangeActivated(int index);

public slots:
    void reset();
    void setActiveSetNumber(int index);
    void changeSetButtonAssociation(int button_index, int originset, int newset, int mode);
    void changeSetAxisButtonAssociation(int button_index, int axis_index, int originset, int newset, int mode);
    void changeSetDPadButtonAssociation(int button_index, int dpad_index, int originset, int newset, int mode);


private slots:
    void propogateSetChange(int index);

};

#endif // JOYSTICK_H
