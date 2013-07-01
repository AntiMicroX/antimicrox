#include <QDebug>
#include <QHashIterator>

#include "setjoystick.h"

SetJoystick::SetJoystick(SDL_Joystick *joyhandle, int index, QObject *parent) :
    QObject(parent)
{
    this->joyhandle = joyhandle;
    this->index = index;
    this->reset();
}

SetJoystick::~SetJoystick()
{
    deleteSticks();
    deleteVDpads();
    deleteButtons();
    deleteAxes();
    deleteHats();
}

JoyButton* SetJoystick::getJoyButton(int index)
{
    return buttons.value(index);
}

JoyAxis* SetJoystick::getJoyAxis(int index)
{
    return axes.value(index);
}

JoyDPad* SetJoystick::getJoyDPad(int index)
{
    return hats.value(index);
}

VDPad* SetJoystick::getVDPad(int index)
{
    return vdpads.value(index);
}

JoyControlStick* SetJoystick::getJoyStick(int index)
{
    return sticks.value(index);
}

void SetJoystick::refreshButtons()
{
    deleteButtons();

    for (int i=0; i < SDL_JoystickNumButtons(joyhandle); i++)
    {
        JoyButton *button = new JoyButton (i, index, this);
        buttons.insert(i, button);
        connect(button, SIGNAL(setChangeActivated(int)), this, SLOT(propogateSetChange(int)));
        connect(button, SIGNAL(setAssignmentChanged(int,int,int)), this, SLOT(propogateSetButtonAssociation(int,int,int)));
    }
}

void SetJoystick::refreshAxes()
{
    deleteAxes();

    for (int i=0; i < SDL_JoystickNumAxes(joyhandle); i++)
    {
        JoyAxis *axis = new JoyAxis(i, index, this);
        axes.insert(i, axis);

        connect(axis, SIGNAL(throttleChangePropogated(int)), this, SLOT(propogateSetAxisThrottleSetting(int)));

        JoyAxisButton *button = axis->getNAxisButton();
        connect(button, SIGNAL(setChangeActivated(int)), this, SLOT(propogateSetChange(int)));
        connect(button, SIGNAL(setAssignmentChanged(int,int,int,int)), this, SLOT(propogateSetAxisButtonAssociation(int,int,int,int)));

        button = axis->getPAxisButton();
        connect(button, SIGNAL(setChangeActivated(int)), this, SLOT(propogateSetChange(int)));
        connect(button, SIGNAL(setAssignmentChanged(int,int,int,int)), this, SLOT(propogateSetAxisButtonAssociation(int,int,int,int)));
    }
}

void SetJoystick::refreshHats()
{
    deleteHats();

    for (int i=0; i < SDL_JoystickNumHats(joyhandle); i++)
    {
        JoyDPad *dpad = new JoyDPad(i, index, this);
        hats.insert(i, dpad);
        QHash<int, JoyDPadButton*> *buttons = dpad->getJoyButtons();
        QHashIterator<int, JoyDPadButton*> iter(*buttons);
        while (iter.hasNext())
        {
            JoyDPadButton *button = iter.next().value();
            connect(button, SIGNAL(setChangeActivated(int)), this, SLOT(propogateSetChange(int)));

            connect(button, SIGNAL(setAssignmentChanged(int,int,int,int)), this, SLOT(propogateSetDPadButtonAssociation(int,int,int,int)));
        }
    }
}

void SetJoystick::deleteButtons()
{
    QHashIterator<int, JoyButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyButton *button = iter.next().value();
        if (button)
        {
            delete button;
            button = 0;
        }
    }

    buttons.clear();
}

void SetJoystick::deleteAxes()
{
    QHashIterator<int, JoyAxis*> iter(axes);
    while (iter.hasNext())
    {
        JoyAxis *axis = iter.next().value();
        if (axis)
        {
            delete axis;
            axis = 0;
        }
    }

    axes.clear();
}

void SetJoystick::deleteSticks()
{
    QHashIterator<int, JoyControlStick*> iter(sticks);
    while (iter.hasNext())
    {
        JoyControlStick *stick = iter.next().value();
        if (stick)
        {
            delete stick;
            stick = 0;
        }
    }

    sticks.clear();
}

void SetJoystick::deleteVDpads()
{
    QHashIterator<int, VDPad*> iter(vdpads);
    while (iter.hasNext())
    {
        VDPad *dpad = iter.next().value();
        if (dpad)
        {
            delete dpad;
            dpad = 0;
        }
    }

    vdpads.clear();
}

void SetJoystick::deleteHats()
{
    QHashIterator<int, JoyDPad*> iter(hats);
    while (iter.hasNext())
    {
        JoyDPad *dpad = iter.next().value();
        if (dpad)
        {
            delete dpad;
            dpad = 0;
        }
    }

    hats.clear();
}

int SetJoystick::getNumberButtons()
{
    return buttons.count();
}

int SetJoystick::getNumberAxes()
{
    return axes.count();
}

int SetJoystick::getNumberHats()
{
    return hats.count();
}

int SetJoystick::getNumberSticks()
{
    return sticks.size();
}

int SetJoystick::getNumberVDPads()
{
    return vdpads.size();
}

void SetJoystick::reset()
{
    deleteSticks();
    deleteVDpads();
    refreshAxes();
    refreshButtons();
    refreshHats();

    /*if (axes.contains(6) && axes.contains(7))
    {
        JoyButton *upButton = axes.value(7)->getNAxisButton();
        JoyButton *downButton = axes.value(7)->getPAxisButton();
        JoyButton *leftButton = axes.value(6)->getNAxisButton();
        JoyButton *rightButton = axes.value(6)->getPAxisButton();

        VDPad *dpad = new VDPad(upButton, downButton, leftButton, rightButton, 0, 0, this);
        vdpads.insert(0, dpad);
    }
    */

    /*if (axes.contains(0) && axes.contains(1))
    {
        JoyControlStick *stick = new JoyControlStick(axes.value(0), axes.value(1), 0, index, this);
        stick->getDirectionButton(JoyControlStick::StickUp)->setAssignedSlot(25);
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::StickUp);
        connect(button, SIGNAL(setChangeActivated(int)), this, SLOT(propogateSetChange(int)));
        connect(button, SIGNAL(setAssignmentChanged(int,int,int,int)), this, SLOT(propogateSetStickButtonAssociation(int,int,int,int)));

        stick->getDirectionButton(JoyControlStick::StickDown)->setAssignedSlot(39);
        button = stick->getDirectionButton(JoyControlStick::StickDown);
        connect(button, SIGNAL(setChangeActivated(int)), this, SLOT(propogateSetChange(int)));
        connect(button, SIGNAL(setAssignmentChanged(int,int,int,int)), this, SLOT(propogateSetStickButtonAssociation(int,int,int,int)));

        stick->getDirectionButton(JoyControlStick::StickLeft)->setAssignedSlot(38);
        button = stick->getDirectionButton(JoyControlStick::StickLeft);
        connect(button, SIGNAL(setChangeActivated(int)), this, SLOT(propogateSetChange(int)));
        connect(button, SIGNAL(setAssignmentChanged(int,int,int,int)), this, SLOT(propogateSetStickButtonAssociation(int,int,int,int)));

        stick->getDirectionButton(JoyControlStick::StickRight)->setAssignedSlot(40);
        button = stick->getDirectionButton(JoyControlStick::StickRight);
        connect(button, SIGNAL(setChangeActivated(int)), this, SLOT(propogateSetChange(int)));
        connect(button, SIGNAL(setAssignmentChanged(int,int,int,int)), this, SLOT(propogateSetStickButtonAssociation(int,int,int,int)));

        sticks.insert(0, stick);

        //axes.value(0)->setControlStick(stick);
        //axes.value(1)->setControlStick(stick);
    }

    if (axes.contains(3) && axes.contains(4))
    {
        JoyControlStick *stick = new JoyControlStick(axes.value(3), axes.value(4), 1, index, this);
        stick->getDirectionButton(JoyControlStick::StickUp)->setAssignedSlot(JoyButtonSlot::MouseUp, JoyButtonSlot::JoyMouseMovement);

        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::StickUp);
        connect(button, SIGNAL(setChangeActivated(int)), this, SLOT(propogateSetChange(int)));
        connect(button, SIGNAL(setAssignmentChanged(int,int,int,int)), this, SLOT(propogateSetStickButtonAssociation(int,int,int,int)));

        //stick->getDirectionButton(JoyControlStick::StickUp)->setMouseSpeedX(10);
        //stick->getDirectionButton(JoyControlStick::StickUp)->setMouseSpeedY(10);

        stick->getDirectionButton(JoyControlStick::StickDown)->setAssignedSlot(JoyButtonSlot::MouseDown, JoyButtonSlot::JoyMouseMovement);

        button = stick->getDirectionButton(JoyControlStick::StickDown);
        connect(button, SIGNAL(setChangeActivated(int)), this, SLOT(propogateSetChange(int)));
        connect(button, SIGNAL(setAssignmentChanged(int,int,int,int)), this, SLOT(propogateSetStickButtonAssociation(int,int,int,int)));

        //stick->getDirectionButton(JoyControlStick::StickDown)->setMouseSpeedX(10);
        //stick->getDirectionButton(JoyControlStick::StickDown)->setMouseSpeedY(10);

        stick->getDirectionButton(JoyControlStick::StickLeft)->setAssignedSlot(JoyButtonSlot::MouseLeft, JoyButtonSlot::JoyMouseMovement);

        button = stick->getDirectionButton(JoyControlStick::StickLeft);
        connect(button, SIGNAL(setChangeActivated(int)), this, SLOT(propogateSetChange(int)));
        connect(button, SIGNAL(setAssignmentChanged(int,int,int,int)), this, SLOT(propogateSetStickButtonAssociation(int,int,int,int)));

        //stick->getDirectionButton(JoyControlStick::StickLeft)->setMouseSpeedX(10);
        //stick->getDirectionButton(JoyControlStick::StickLeft)->setMouseSpeedY(10);

        stick->getDirectionButton(JoyControlStick::StickRight)->setAssignedSlot(JoyButtonSlot::MouseRight, JoyButtonSlot::JoyMouseMovement);

        button = stick->getDirectionButton(JoyControlStick::StickRight);
        connect(button, SIGNAL(setChangeActivated(int)), this, SLOT(propogateSetChange(int)));
        connect(button, SIGNAL(setAssignmentChanged(int,int,int,int)), this, SLOT(propogateSetStickButtonAssociation(int,int,int,int)));

        //stick->getDirectionButton(JoyControlStick::StickRight)->setMouseSpeedX(10);
        //stick->getDirectionButton(JoyControlStick::StickRight)->setMouseSpeedY(10);

        sticks.insert(1, stick);

        //axes.value(3)->setControlStick(stick);
        //axes.value(4)->setControlStick(stick);
    }*/
}

SDL_Joystick* SetJoystick::getSDLHandle()
{
    return joyhandle;
}

void SetJoystick::propogateSetChange(int index)
{
    emit setChangeActivated(index);
}

void SetJoystick::propogateSetButtonAssociation(int button, int newset, int mode)
{
    if (newset != index)
    {
        emit setAssignmentButtonChanged(button, index, newset, mode);
    }
}

void SetJoystick::propogateSetAxisButtonAssociation(int button, int axis, int newset, int mode)
{
    if (newset != index)
    {
        emit setAssignmentAxisChanged(button, axis, index, newset, mode);
    }
}

void SetJoystick::propogateSetStickButtonAssociation(int button, int stick, int newset, int mode)
{
    if (newset != index)
    {
        emit setAssignmentStickChanged(button, stick, index, newset, mode);
    }
}

void SetJoystick::propogateSetDPadButtonAssociation(int button, int dpad, int newset, int mode)
{
    if (newset != index)
    {
        emit setAssignmentDPadChanged(button, dpad, index, newset, mode);
    }
}

void SetJoystick::release()
{
    QHashIterator<int, JoyButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyButton *button = iter.next().value();
        button->joyEvent(false, true);
    }

    QHashIterator<int, JoyAxis*> iter2(axes);
    while (iter2.hasNext())
    {
        JoyAxis *axis = iter2.next().value();
        axis->joyEvent(axis->getCurrentThrottledDeadValue(), true);
    }

    QHashIterator<int, JoyDPad*> iter3(hats);
    while (iter3.hasNext())
    {
        JoyDPad *dpad = iter3.next().value();
        dpad->joyEvent(0, true);
    }
}

void SetJoystick::readConfig(QXmlStreamReader *xml)
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
            else if (xml->name() == "axis" && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyAxis *axis = getJoyAxis(index-1);
                if (axis)
                {
                    axis->readConfig(xml);
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if (xml->name() == "dpad" && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyDPad *dpad = getJoyDPad(index-1);
                if (dpad)
                {
                    dpad->readConfig(xml);
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
            else if (xml->name() == "vdpad" && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                VDPad *vdpad = getVDPad(index-1);
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

void SetJoystick::writeConfig(QXmlStreamWriter *xml)
{
    if (!isSetEmpty())
    {
        xml->writeStartElement("set");

        xml->writeAttribute("index", QString::number(index+1));

        for (int i=0; i < getNumberSticks(); i++)
        {
            JoyControlStick *stick = getJoyStick(i);
            stick->writeConfig(xml);
        }

        for (int i=0; i < getNumberVDPads(); i++)
        {
            VDPad *vdpad = getVDPad(i);
            if (vdpad)
            {
                vdpad->writeConfig(xml);
            }
        }

        for (int i=0; i < getNumberAxes(); i++)
        {
            JoyAxis *axis = getJoyAxis(i);
            if (!axis->isPartControlStick() && axis->hasControlOfButtons())
            {
                axis->writeConfig(xml);
            }
        }

        for (int i=0; i < getNumberHats(); i++)
        {
            JoyDPad *dpad = getJoyDPad(i);
            dpad->writeConfig(xml);
        }

        for (int i=0; i < getNumberButtons(); i++)
        {
            JoyButton *button = getJoyButton(i);
            if (button && !button->isPartVDPad())
            {
                button->writeConfig(xml);
            }
        }

        xml->writeEndElement();
    }
}

bool SetJoystick::isSetEmpty()
{
    bool result = true;
    QHashIterator<int, JoyButton*> iter(buttons);
    while (iter.hasNext() && result)
    {
        JoyButton *button = iter.next().value();
        if (button->getAssignedSlots()->size() > 0)
        {
            result = false;
        }
    }

    QHashIterator<int, JoyAxis*> iter2(axes);
    while (iter2.hasNext() && result)
    {
        JoyAxis *axis = iter2.next().value();
        if (axis->getPAxisButton()->getAssignedSlots()->size() > 0)
        {
            result = false;
        }
        else if (axis->getNAxisButton()->getAssignedSlots()->size() > 0)
        {
            result = false;
        }
    }

    QHashIterator<int, JoyDPad*> iter3(hats);
    while (iter3.hasNext() && result)
    {
        JoyDPad *dpad = iter3.next().value();
        QHash<int, JoyDPadButton*> *dpadButtons = dpad->getJoyButtons();
        QHashIterator<int, JoyDPadButton*> dpaditer(*dpadButtons);
        while (dpaditer.hasNext() && result)
        {
            JoyDPadButton *button = dpaditer.next().value();
            if (button->getAssignedSlots()->size() > 0)
            {
                result = false;
            }
        }
    }

    QHashIterator<int, JoyControlStick*> iter4(sticks);
    while (iter4.hasNext() && result)
    {
        JoyControlStick *stick = iter4.next().value();
        QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton*> *stickbuttons = stick->getButtons();
        QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> stickiter(*stickbuttons);
        while (stickiter.hasNext() && result)
        {
            JoyControlStickButton *button = stickiter.next().value();
            if (button->getAssignedSlots()->size() > 0)
            {
                result = false;
            }
        }
    }

    QHashIterator<int, VDPad*> iter5(vdpads);
    while (iter5.hasNext() && result)
    {
        VDPad *vdpad = iter5.next().value();
        QHash<int, JoyDPadButton*> *dpadButtons = vdpad->getJoyButtons();
        QHashIterator<int, JoyDPadButton*> dpaditer(*dpadButtons);
        while (dpaditer.hasNext() && result)
        {
            JoyDPadButton *button = dpaditer.next().value();
            if (button->getAssignedSlots()->size() > 0)
            {
                result = false;
            }
        }
    }

    return result;
}

void SetJoystick::propogateSetAxisThrottleSetting(int index)
{
    JoyAxis *axis = axes.value(index);
    if (axis)
    {
        emit setAssignmentAxisThrottleChanged(index, axis->getCurrentlyAssignedSet());
    }
}

void SetJoystick::addControlStick(int index, JoyControlStick *stick)
{
    sticks.insert(index, stick);
}

void SetJoystick::removeControlStick(int index)
{
    if (sticks.contains(index))
    {
        JoyControlStick *stick = sticks.value(index);
        sticks.remove(index);
        delete stick;
        stick = 0;
    }
}

void SetJoystick::addVDPad(int index, VDPad *vdpad)
{
    vdpads.insert(index, vdpad);
}

void SetJoystick::removeVDPad(int index)
{
    if (vdpads.contains(index))
    {
        VDPad *vdpad = vdpads.value(index);
        vdpads.remove(index);
        delete vdpad;
        vdpad = 0;
    }
}

int SetJoystick::getIndex()
{
    return index;
}
