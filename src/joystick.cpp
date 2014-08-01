#include <typeinfo>

#include <QDebug>

#include "joystick.h"

const QString Joystick::xmlName = "joystick";

Joystick::Joystick(SDL_Joystick *joyhandle, int deviceIndex, QObject *parent) :
    InputDevice(deviceIndex, parent)
{
    this->joyhandle = joyhandle;
#ifdef USE_SDL_2
    joystickID = SDL_JoystickInstanceID(joyhandle);
#else
    joyNumber = SDL_JoystickIndex(joyhandle);
#endif

    for (int i=0; i < NUMBER_JOYSETS; i++)
    {
        SetJoystick *setstick = new SetJoystick(this, i, this);
        joystick_sets.insert(i, setstick);
        enableSetConnections(setstick);
    }
}

QString Joystick::getName()
{
    return QString(tr("Joystick")).append(" ").append(QString::number(getRealJoyNumber()));
}

QString Joystick::getSDLName()
{
    QString temp;
#ifdef USE_SDL_2
    if (joyhandle)
    {
        temp = SDL_JoystickName(joyhandle);
    }
#else
    temp = SDL_JoystickName(joyNumber);
#endif
    return temp;
}

QString Joystick::getGUIDString()
{
    QString temp;
#ifdef USE_SDL_2
    SDL_JoystickGUID tempGUID = SDL_JoystickGetGUID(joyhandle);
    char guidString[65] = {'0'};
    SDL_JoystickGetGUIDString(tempGUID, guidString, sizeof(guidString));
    temp = QString(guidString);
#endif
    // Not available on SDL 1.2. Return empty string in that case.
    return temp;
}

QString Joystick::getXmlName()
{
    return this->xmlName;
}

void Joystick::closeSDLDevice()
{
#ifdef USE_SDL_2
    if (joyhandle && SDL_JoystickGetAttached(joyhandle))
    {
        SDL_JoystickClose(joyhandle);
    }
#else
    if (joyhandle && SDL_JoystickOpened(joyNumber))
    {
        SDL_JoystickClose(joyhandle);
    }
#endif
}

int Joystick::getNumberRawButtons()
{
    int numbuttons = SDL_JoystickNumButtons(joyhandle);
    return numbuttons;
}

int Joystick::getNumberRawAxes()
{
    int numaxes = SDL_JoystickNumAxes(joyhandle);
    return numaxes;
}

int Joystick::getNumberRawHats()
{
    int numhats = SDL_JoystickNumHats(joyhandle);
    return numhats;
}

#ifdef USE_SDL_2
SDL_JoystickID Joystick::getSDLJoystickID()
{
    return joystickID;
}
#endif
