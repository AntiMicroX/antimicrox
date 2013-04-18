#include <QHashIterator>

#include "setjoystick.h"

SetJoystick::SetJoystick(SDL_Joystick *joyhandle, int index, QObject *parent) :
    QObject(parent)
{
    this->joyhandle = joyhandle;
    this->index = index;
    this->reset();
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

void SetJoystick::refreshButtons()
{
    buttons.clear();

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
    axes.clear();

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
    hats.clear();

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

void SetJoystick::reset()
{
    refreshAxes();
    refreshButtons();
    refreshHats();
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

    /*for (int i=0; i < SDL_JoystickNumAxes(joyhandle); i++)
    {
        JoyAxis *axis = new JoyAxis(i, this);
        axes.insert(i, axis);
        JoyButton *button = axis->getNAxisButton();
        connect(button, SIGNAL(setChangeActivated(int)), this, SLOT(propogateSetChange(int)));
        button = axis->getPAxisButton();
        connect(button, SIGNAL(setChangeActivated(int)), this, SLOT(propogateSetChange(int)));
    }*/
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

        for (int i=0; i < getNumberAxes(); i++)
        {
            JoyAxis *axis = getJoyAxis(i);
            axis->writeConfig(xml);
        }

        for (int i=0; i < getNumberHats(); i++)
        {
            JoyDPad *dpad = getJoyDPad(i);
            dpad->writeConfig(xml);
        }

        for (int i=0; i < getNumberButtons(); i++)
        {
            JoyButton *button = getJoyButton(i);
            button->writeConfig(xml);
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
