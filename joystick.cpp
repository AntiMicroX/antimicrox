#include <QDebug>

#include "joystick.h"

Joystick::Joystick(QObject *parent) :
    QObject(parent)
{
    joyhandle = 0;
    buttons = QHash<int, JoyButton*> ();
    axes = QHash<int, JoyAxis*> ();
}

Joystick::Joystick(SDL_Joystick *joyhandle, QObject *parent) :
    QObject(parent)
{
    this->joyhandle = joyhandle;
    buttons = QHash<int, JoyButton*> ();
    axes = QHash<int, JoyAxis*> ();
}

int Joystick::getNumberButtons()
{
    return buttons.count();
}

int Joystick::getNumberAxes()
{
    return axes.count();
}

int Joystick::getNumberHats()
{
    return hats.count();
}

SDL_Joystick* Joystick::getSDLHandle()
{
    return joyhandle;
}

int Joystick::getJoyNumber()
{
    int joynumber = SDL_JoystickIndex(joyhandle);
    return joynumber;
}

void Joystick::refreshButtons()
{
    buttons.clear();

    for (int i=0; i < SDL_JoystickNumButtons(joyhandle); i++)
    {
        JoyButton *button = new JoyButton (i, this);
        buttons.insert(i, button);
    }
}

void Joystick::refreshAxes()
{
    axes.clear();

    for (int i=0; i < SDL_JoystickNumAxes(joyhandle); i++)
    {
        JoyAxis *axis = new JoyAxis(i, this);
        axes.insert(i, axis);
    }
}

void Joystick::refreshHats()
{
    hats.clear();

    for (int i=0; i < SDL_JoystickNumHats(joyhandle); i++)
    {
        JoyDPad *dpad = new JoyDPad(i, this);
        hats.insert(i, dpad);
    }
}

int Joystick::getRealJoyNumber()
{
    int joynumber = getJoyNumber();
    return joynumber + 1;
}

JoyButton* Joystick::getJoyButton(int index)
{
    return buttons.value(index);
}

JoyAxis* Joystick::getJoyAxis(int index)
{
    return axes.value(index);
}

JoyDPad* Joystick::getJoyDPad(int index)
{
    return hats.value(index);
}

QString Joystick::getName()
{
    return QString("Joystick ").append(QString::number(getRealJoyNumber()));
}

void Joystick::reset()
{
    refreshAxes();
    refreshButtons();
    refreshHats();
}
