#include "joydpadbutton.h"
#include "joydpad.h"
#include "event.h"

const QString JoyDPadButton::xmlName = "dpadbutton";

// Initially, qualify direction as the button's index
JoyDPadButton::JoyDPadButton(int direction, JoyDPad* dpad, QObject *parent) :
    JoyButton(direction, parent)
{
    this->direction = direction;
    this->dpad = dpad;
    this->keycode = 0;
    this->mousecode = 0;
}

QString JoyDPadButton::getDirectionName()
{
    QString label = QString ();
    if (direction & DpadUp)
    {
        label.append("Up");
    }
    else if (direction & DpadDown)
    {
        label.append("Down");
    }
    else if (direction & DpadLeft)
    {
        label.append("Left");
    }
    else if (direction & DpadRight)
    {
        label.append("Right");
    }
    else if (direction & DpadLeftUp)
    {
        label.append("Up+Left");
    }
    else if (direction & DpadLeftDown)
    {
        label.append("Down+Left");
    }
    else if (direction & DpadRightUp)
    {
        label.append("Up+Left");
    }
    else if (direction & DpadRightDown)
    {
        label.append("Down+Right");
    }

    return label;
}

QString JoyDPadButton::getXmlName()
{
    return this->xmlName;
}

int JoyDPadButton::getRealJoyNumber()
{
    return index;
}

QString JoyDPadButton::getPartialName()
{
    return dpad->getName().append(" - ").append(getDirectionName());
}


void JoyDPadButton::reset()
{
    JoyButton::reset();
    this->keycode = 0;
    this->mousecode = 0;
}

void JoyDPadButton::reset(int index)
{
    reset();
}
