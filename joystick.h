#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <QObject>
#include <QHash>
#include <SDL/SDL.h>

#include "joyaxis.h"
#include "joydpad.h"
#include "joybutton.h"

class Joystick : public QObject
{
    Q_OBJECT
public:
    explicit Joystick(QObject *parent = 0);
    explicit Joystick(SDL_Joystick *joyhandle, QObject *parent=0);

    int getNumberButtons ();
    int getNumberAxes();
    int getNumberHats();
    SDL_Joystick* getSDLHandle ();
    int getJoyNumber ();
    int getRealJoyNumber ();
    void refreshButtons ();
    void refreshAxes();
    void refreshHats();
    JoyAxis* getJoyAxis(int index);
    JoyButton* getJoyButton(int index);
    JoyDPad* getJoyDPad(int index);
    QString getName();

protected:
    QHash<int, JoyButton*> buttons;
    QHash<int, JoyAxis*> axes;
    QHash<int, JoyDPad*> hats;
    SDL_Joystick* joyhandle;

signals:
    
public slots:
    void reset();
};

#endif // JOYSTICK_H
