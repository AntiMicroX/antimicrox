#ifndef SETJOYSTICK_H
#define SETJOYSTICK_H

#include <QObject>
#include <QHash>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <SDL/SDL.h>

#include "joyaxis.h"
#include "joydpad.h"
#include "joybutton.h"

class SetJoystick : public QObject
{
    Q_OBJECT
public:
    explicit SetJoystick(SDL_Joystick *joyhandle, int index, QObject *parent=0);

    SDL_Joystick* getSDLHandle ();
    JoyAxis* getJoyAxis(int index);
    JoyButton* getJoyButton(int index);
    JoyDPad* getJoyDPad(int index);
    int getNumberButtons ();
    int getNumberAxes();
    int getNumberHats();
    void refreshButtons ();
    void refreshAxes();
    void refreshHats();
    void release();

    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

protected:
    bool isSetEmpty();

    QHash<int, JoyButton*> buttons;
    QHash<int, JoyAxis*> axes;
    QHash<int, JoyDPad*> hats;
    int index;
    SDL_Joystick* joyhandle;

signals:
    void setChangeActivated(int index);
    void setAssignmentButtonChanged(int button, int originset, int newset, int mode);
    void setAssignmentAxisChanged(int button, int axis, int originset, int newset, int mode);
    void setAssignmentDPadChanged(int button, int dpad, int originset, int newset, int mode);
    void setAssignmentAxisThrottleChanged(int axis, int originset);

    
public slots:
    void reset();
    void propogateSetChange(int index);
    void propogateSetButtonAssociation(int button, int newset, int mode);
    void propogateSetAxisButtonAssociation(int button, int axis, int newset, int mode);
    void propogateSetDPadButtonAssociation(int button, int dpad, int newset, int mode);


protected slots:
    void propogateSetAxisThrottleSetting(int index);

};

#endif // SETJOYSTICK_H
