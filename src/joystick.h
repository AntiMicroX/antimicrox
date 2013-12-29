#ifndef JOYSTICK_H
#define JOYSTICK_H

/*#include <QObject>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#ifdef USE_SDL_2
#include <SDL2/SDL_joystick.h>
#else
#include <SDL/SDL_joystick.h>
#endif

#include "joyaxis.h"
#include "joydpad.h"
#include "joybutton.h"
#include "setjoystick.h"
#include "common.h"
*/

#include <QObject>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "inputdevice.h"

class Joystick : public InputDevice
{
    Q_OBJECT
public:
    explicit Joystick(SDL_Joystick *joyhandle, QObject *parent=0);

    virtual QString getName();
    virtual QString getSDLName();
    virtual QString getGUIDString(); // GUID available on SDL 2.
    virtual QString getXmlName();
    virtual void closeSDLDevice();
#ifdef USE_SDL_2
    virtual SDL_JoystickID getSDLJoystickID();
#endif

    static const QString xmlName;

protected:
    virtual int getNumberRawButtons();
    virtual int getNumberRawAxes();
    virtual int getNumberRawHats();

    SDL_Joystick *joyhandle;

signals:

public slots:

};

Q_DECLARE_METATYPE(Joystick*)

#endif // JOYSTICK_H
