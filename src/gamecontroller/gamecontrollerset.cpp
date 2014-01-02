#include "gamecontrollerset.h"
#include <inputdevice.h>

GameControllerSet::GameControllerSet(InputDevice *device, int index, QObject *parent) :
    SetJoystick(device, index, false, parent)
{
    reset();
}

void GameControllerSet::reset()
{
    SetJoystick::reset();
    populateSticksDPad();
}

void GameControllerSet::populateSticksDPad()
{
    // Left Stick Assignment
    JoyAxis *axisX = getJoyAxis(SDL_CONTROLLER_AXIS_LEFTX);
    JoyAxis *axisY = getJoyAxis(SDL_CONTROLLER_AXIS_LEFTY);
    addControlStick(0, new JoyControlStick(axisX, axisY, 0, index, this));

    // Right Stick Assignment
    axisX = getJoyAxis(SDL_CONTROLLER_AXIS_RIGHTX);
    axisY = getJoyAxis(SDL_CONTROLLER_AXIS_RIGHTY);
    addControlStick(1, new JoyControlStick(axisX, axisY, 1, index, this));

    // Assign DPad buttons as a virtual DPad. Allows rougelike controls
    // to be assigned.
    JoyButton *buttonUp = getJoyButton(SDL_CONTROLLER_BUTTON_DPAD_UP);
    JoyButton *buttonDown = getJoyButton(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    JoyButton *buttonLeft = getJoyButton(SDL_CONTROLLER_BUTTON_DPAD_LEFT);
    JoyButton *buttonRight = getJoyButton(SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
    GameControllerDPad *controllerDPad = new GameControllerDPad(buttonUp, buttonDown, buttonLeft, buttonRight, 0, index, this);
    addVDPad(0, controllerDPad);

    // Give default names to buttons
    getJoyButton(SDL_CONTROLLER_BUTTON_A)->setDefaultButtonName("A");
    getJoyButton(SDL_CONTROLLER_BUTTON_B)->setDefaultButtonName("B");
    getJoyButton(SDL_CONTROLLER_BUTTON_X)->setDefaultButtonName("X");
    getJoyButton(SDL_CONTROLLER_BUTTON_Y)->setDefaultButtonName("Y");
    getJoyButton(SDL_CONTROLLER_BUTTON_BACK)->setDefaultButtonName("Back");
    getJoyButton(SDL_CONTROLLER_BUTTON_GUIDE)->setDefaultButtonName("Guide");
    getJoyButton(SDL_CONTROLLER_BUTTON_START)->setDefaultButtonName("Start");
    getJoyButton(SDL_CONTROLLER_BUTTON_LEFTSTICK)->setDefaultButtonName("LS Click");
    getJoyButton(SDL_CONTROLLER_BUTTON_RIGHTSTICK)->setDefaultButtonName("RS Click");
    getJoyButton(SDL_CONTROLLER_BUTTON_LEFTSHOULDER)->setDefaultButtonName("LB");
    getJoyButton(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)->setDefaultButtonName("RB");

    // Give default names to triggers
    getJoyAxis(SDL_CONTROLLER_AXIS_TRIGGERLEFT)->setDefaultAxisName("Left Trigger");
    getJoyAxis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT)->setDefaultAxisName("Right Trigger");
}

void GameControllerSet::readConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && xml->name() == "set")
    {
        //reset();

        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != "set"))
        {
            if (xml->name() == "button" && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyButton *button = getJoyButton(index-1);
                if (button)
                {
                    button->readConfig(xml);
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if (xml->name() == "trigger" && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                GameControllerTrigger *axis = qobject_cast<GameControllerTrigger*>(getJoyAxis((index-1) + SDL_CONTROLLER_AXIS_TRIGGERLEFT));
                if (axis)
                {
                    axis->readConfig(xml);
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if (xml->name() == "stick" && xml->isStartElement())
            {
                int stickIndex = xml->attributes().value("index").toString().toInt();

                if (stickIndex > 0)
                {
                    stickIndex -= 1;
                    JoyControlStick *stick = getJoyStick(stickIndex);
                    if (stick)
                    {
                        stick->readConfig(xml);
                    }
                    else
                    {
                        xml->skipCurrentElement();
                    }
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if (xml->name() == "dpad" && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                GameControllerDPad *vdpad = qobject_cast<GameControllerDPad*>(getVDPad(index-1));
                if (vdpad)
                {
                    vdpad->readConfig(xml);
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else
            {
                // If none of the above, skip the element
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }
    }
}

void GameControllerSet::refreshAxes()
{
    deleteAxes();

    for (int i=0; i < device->getNumberRawAxes(); i++)
    {
        if (i == SDL_CONTROLLER_AXIS_TRIGGERLEFT ||
            i == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
        {
            GameControllerTrigger *trigger = new GameControllerTrigger(i, index, this);
            axes.insert(i, trigger);
            enableAxisConnections(trigger);
        }
        else
        {
            JoyAxis *axis = new JoyAxis(i, index, this);
            axes.insert(i, axis);
            enableAxisConnections(axis);
        }
    }
}
