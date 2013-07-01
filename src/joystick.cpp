#include <typeinfo>

#include <QDebug>
#include <QHashIterator>

#include "joystick.h"

const int Joystick::NUMBER_JOYSETS = 8;

Joystick::Joystick(SDL_Joystick *joyhandle, QObject *parent) :
    QObject(parent)
{
    this->joyhandle = joyhandle;
    joyNumber= SDL_JoystickIndex(joyhandle);
    joystick_sets = QHash<int, SetJoystick*> ();
    for (int i=0; i < NUMBER_JOYSETS; i++)
    {
        SetJoystick *setstick = new SetJoystick(joyhandle, i, this);
        joystick_sets.insert(i, setstick);
        connect(setstick, SIGNAL(setChangeActivated(int)), this, SLOT(setActiveSetNumber(int)));
        connect(setstick, SIGNAL(setChangeActivated(int)), this, SLOT(propogateSetChange(int)));
        connect(setstick, SIGNAL(setAssignmentButtonChanged(int,int,int,int)), this, SLOT(changeSetButtonAssociation(int,int,int,int)));

        connect(setstick, SIGNAL(setAssignmentAxisChanged(int,int,int,int,int)), this, SLOT(changeSetAxisButtonAssociation(int,int,int,int,int)));
        connect(setstick, SIGNAL(setAssignmentDPadChanged(int,int,int,int,int)), this, SLOT(changeSetDPadButtonAssociation(int,int,int,int,int)));
        connect(setstick, SIGNAL(setAssignmentStickChanged(int,int,int,int,int)), this, SLOT(changeSetStickButtonAssociation(int,int,int,int,int)));

        connect(setstick, SIGNAL(setAssignmentAxisThrottleChanged(int,int)), this, SLOT(propogateSetAxisThrottleChange(int, int)));
    }

    active_set = 0;
}

Joystick::~Joystick()
{
    QHashIterator<int, SetJoystick*> iter(joystick_sets);
    while (iter.hasNext())
    {
        SetJoystick *setjoystick = iter.next().value();
        if (setjoystick)
        {
            delete setjoystick;
            setjoystick = 0;
        }
    }

    joystick_sets.clear();
}

SDL_Joystick* Joystick::getSDLHandle()
{
    return joyhandle;
}

int Joystick::getJoyNumber()
{
    return joyNumber;
}

int Joystick::getRealJoyNumber()
{
    int joynumber = getJoyNumber();
    return joynumber + 1;
}

QString Joystick::getName()
{
    return QString(tr("Joystick")).append(" ").append(QString::number(getRealJoyNumber()));
}

void Joystick::reset()
{
    for (int i=0; i < NUMBER_JOYSETS; i++)
    {
        SetJoystick* set = joystick_sets.value(i);
        set->reset();
    }
}

void Joystick::setActiveSetNumber(int index)
{
    if ((index >= 0 && index < NUMBER_JOYSETS) && (index != active_set))
    {
        QList<bool> buttonstates;
        QList<int> axesstates;
        QList<int> dpadstates;

        SetJoystick *current_set = joystick_sets.value(active_set);
        for (int i = 0; i < current_set->getNumberButtons(); i++)
        {
            JoyButton *button = current_set->getJoyButton(i);
            buttonstates.append(button->getButtonState());
        }

        for (int i = 0; i < current_set->getNumberAxes(); i++)
        {
            JoyAxis *axis = current_set->getJoyAxis(i);
            axesstates.append(axis->getCurrentRawValue());
        }

        for (int i = 0; i < current_set->getNumberHats(); i++)
        {
            JoyDPad *dpad = current_set->getJoyDPad(i);
            dpadstates.append(dpad->getCurrentDirection());
        }

        joystick_sets.value(active_set)->release();
        active_set = index;

        current_set = joystick_sets.value(active_set);
        for (int i = 0; i < current_set->getNumberButtons(); i++)
        {
            bool value = buttonstates.at(i);
            JoyButton *button = current_set->getJoyButton(i);
            button->joyEvent(value, true);
        }

        for (int i = 0; i < current_set->getNumberAxes(); i++)
        {
            int value = axesstates.at(i);
            JoyAxis *axis = current_set->getJoyAxis(i);
            axis->joyEvent(value, true);
        }

        for (int i = 0; i < current_set->getNumberHats(); i++)
        {
            int value = dpadstates.at(i);
            JoyDPad *dpad = current_set->getJoyDPad(i);
            dpad->joyEvent(value, true);
        }
    }
}

int Joystick::getActiveSetNumber()
{
    return active_set;
}

SetJoystick* Joystick::getActiveSetJoystick()
{
    return joystick_sets.value(active_set);
}

int Joystick::getNumberButtons()
{
    return getActiveSetJoystick()->getNumberButtons();
}

int Joystick::getNumberAxes()
{
    return getActiveSetJoystick()->getNumberAxes();
}

int Joystick::getNumberHats()
{
    return getActiveSetJoystick()->getNumberHats();
}

int Joystick::getNumberSticks()
{
    return getActiveSetJoystick()->getNumberSticks();
}

int Joystick::getNumberVDPads()
{
    return getActiveSetJoystick()->getNumberVDPads();
}

SetJoystick* Joystick::getSetJoystick(int index)
{
    return joystick_sets.value(index);
}

void Joystick::propogateSetChange(int index)
{
    emit setChangeActivated(index);
}

void Joystick::changeSetButtonAssociation(int button_index, int originset, int newset, int mode)
{
    JoyButton *button = joystick_sets.value(newset)->getJoyButton(button_index);
    JoyButton::SetChangeCondition tempmode = (JoyButton::SetChangeCondition)mode;
    button->setChangeSetSelection(originset);
    button->setChangeSetCondition(tempmode, true);
}

void Joystick::readConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && xml->name() == "joystick")
    {
        reset();

        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != "joystick"))
        {
            if (xml->name() == "sets" && xml->isStartElement())
            {
                xml->readNextStartElement();

                while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != "sets"))
                {
                    if (xml->name() == "set" && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        index = index - 1;
                        if (index >= 0 && index < joystick_sets.size())
                        {
                            joystick_sets.value(index)->readConfig(xml);
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
            else if (xml->name() == "button" && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyButton *button = joystick_sets.value(0)->getJoyButton(index-1);
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
                JoyAxis *axis = joystick_sets.value(0)->getJoyAxis(index-1);
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
                JoyDPad *dpad = joystick_sets.value(0)->getJoyDPad(index-1);
                if (dpad)
                {
                    dpad->readConfig(xml);
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if (xml->name() == "stickAxisAssociation" && xml->isStartElement())
            {
                int stickIndex = xml->attributes().value("index").toString().toInt();
                int xAxis = xml->attributes().value("xAxis").toString().toInt();
                int yAxis = xml->attributes().value("yAxis").toString().toInt();

                if (stickIndex > 0 && xAxis > 0 && yAxis > 0)
                {
                    xAxis -= 1;
                    yAxis -= 1;
                    stickIndex -= 1;

                    for (int i=0; i <joystick_sets.size(); i++)
                    {
                        SetJoystick *currentset = joystick_sets.value(i);
                        JoyAxis *axis1 = currentset->getJoyAxis(xAxis);
                        JoyAxis *axis2 = currentset->getJoyAxis(yAxis);
                        if (axis1 && axis2)
                        {
                            JoyControlStick *stick = new JoyControlStick(axis1, axis2, stickIndex, i, this);
                            currentset->addControlStick(stickIndex, stick);
                        }
                    }

                    xml->readNext();
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

                    JoyControlStick *stick = joystick_sets.value(0)->getJoyStick(stickIndex);
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
            else if (xml->name() == "vdpadButtonAssociations" && xml->isStartElement())
            {
                int vdpadIndex = xml->attributes().value("index").toString().toInt();
                if (vdpadIndex > 0)
                {
                    for (int i=0; i <joystick_sets.size(); i++)
                    {
                        SetJoystick *currentset = joystick_sets.value(i);
                        VDPad *vdpad = currentset->getVDPad(vdpadIndex-1);
                        if (!vdpad)
                        {
                            vdpad = new VDPad(vdpadIndex-1, i, currentset);
                            currentset->addVDPad(vdpadIndex-1, vdpad);
                        }
                    }

                    xml->readNextStartElement();
                    while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != "vdpadButtonAssociations"))
                    {
                        if (xml->name() == "vdpadButtonAssociation" && xml->isStartElement())
                        {
                            int vdpadAxisIndex = xml->attributes().value("axis").toString().toInt();
                            int vdpadButtonIndex = xml->attributes().value("button").toString().toInt();
                            int vdpadDirection = xml->attributes().value("direction").toString().toInt();

                            if (vdpadAxisIndex > 0 && vdpadDirection > 0)
                            {
                                vdpadAxisIndex -= 1;
                                for (int i=0; i < joystick_sets.size(); i++)
                                {
                                    SetJoystick *currentset = joystick_sets.value(i);
                                    VDPad *vdpad = currentset->getVDPad(vdpadIndex-1);
                                    if (vdpad)
                                    {
                                        JoyAxis *axis = currentset->getJoyAxis(vdpadAxisIndex);
                                        if (axis)
                                        {
                                            JoyButton *button = 0;
                                            if (vdpadButtonIndex == 0)
                                            {
                                                button = axis->getNAxisButton();
                                            }
                                            else if (vdpadButtonIndex == 1)
                                            {
                                                button = axis->getPAxisButton();
                                            }

                                            if (button)
                                            {
                                                vdpad->addVButton((JoyDPadButton::JoyDPadDirections)vdpadDirection, button);
                                            }
                                        }
                                    }
                                }
                            }
                            else if (vdpadButtonIndex > 0 && vdpadDirection > 0)
                            {
                                vdpadButtonIndex -= 1;

                                for (int i=0; i < joystick_sets.size(); i++)
                                {
                                    SetJoystick *currentset = joystick_sets.value(i);
                                    VDPad *vdpad = currentset->getVDPad(vdpadIndex-1);
                                    if (vdpad)
                                    {
                                        JoyButton *button = currentset->getJoyButton(vdpadButtonIndex);
                                        if (button)
                                        {
                                            vdpad->addVButton((JoyDPadButton::JoyDPadDirections)vdpadDirection, button);
                                        }
                                    }
                                }
                            }
                            xml->readNext();
                        }
                        else
                        {
                            xml->skipCurrentElement();
                        }

                        xml->readNextStartElement();
                    }
                }
            }
            else if (xml->name() == "vdpad" && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                VDPad *vdpad = joystick_sets.value(0)->getVDPad(index-1);
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

void Joystick::writeConfig(QXmlStreamWriter *xml)
{
    xml->writeStartElement("joystick");
    xml->writeAttribute("configversion", QString::number(PadderCommon::LATESTCONFIGFILEVERSION));

    for (int i=0; i < getNumberSticks(); i++)
    {
        JoyControlStick *stick = getActiveSetJoystick()->getJoyStick(i);
        xml->writeStartElement("stickAxisAssociation");
        xml->writeAttribute("index", QString::number(stick->getRealJoyIndex()));
        xml->writeAttribute("xAxis", QString::number(stick->getAxisX()->getRealJoyIndex()));
        xml->writeAttribute("yAxis", QString::number(stick->getAxisY()->getRealJoyIndex()));
        xml->writeEndElement();
    }

    for (int i=0; i < getNumberVDPads(); i++)
    {
        VDPad *vdpad = getActiveSetJoystick()->getVDPad(i);
        xml->writeStartElement("vdpadButtonAssociations");
        xml->writeAttribute("index", QString::number(vdpad->getRealJoyNumber()));

        JoyButton *button = vdpad->getVButton(JoyDPadButton::DpadUp);
        if (button)
        {
            xml->writeStartElement("vdpadButtonAssociation");

            if (typeid(*button) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = static_cast<JoyAxisButton*>(button);
                xml->writeAttribute("axis", QString::number(axisbutton->getAxis()->getRealJoyIndex()));
                xml->writeAttribute("button", QString::number(button->getJoyNumber()));
            }
            else
            {
                xml->writeAttribute("axis", QString::number(0));
                xml->writeAttribute("button", QString::number(button->getRealJoyNumber()));
            }

            xml->writeAttribute("direction", QString::number(JoyDPadButton::DpadUp));
            xml->writeEndElement();
        }

        button = vdpad->getVButton(JoyDPadButton::DpadDown);
        if (button)
        {
            xml->writeStartElement("vdpadButtonAssociation");

            if (typeid(*button) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = static_cast<JoyAxisButton*>(button);
                xml->writeAttribute("axis", QString::number(axisbutton->getAxis()->getRealJoyIndex()));
                xml->writeAttribute("button", QString::number(button->getJoyNumber()));
            }
            else
            {
                xml->writeAttribute("axis", QString::number(0));
                xml->writeAttribute("button", QString::number(button->getRealJoyNumber()));
            }

            xml->writeAttribute("direction", QString::number(JoyDPadButton::DpadDown));
            xml->writeEndElement();
        }

        button = vdpad->getVButton(JoyDPadButton::DpadLeft);
        if (button)
        {
            xml->writeStartElement("vdpadButtonAssociation");

            if (typeid(*button) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = static_cast<JoyAxisButton*>(button);
                xml->writeAttribute("axis", QString::number(axisbutton->getAxis()->getRealJoyIndex()));
                xml->writeAttribute("button", QString::number(button->getJoyNumber()));
            }
            else
            {
                xml->writeAttribute("axis", QString::number(0));
                xml->writeAttribute("button", QString::number(button->getRealJoyNumber()));
            }

            xml->writeAttribute("direction", QString::number(JoyDPadButton::DpadLeft));
            xml->writeEndElement();
        }

        button = vdpad->getVButton(JoyDPadButton::DpadRight);
        if (button)
        {
            xml->writeStartElement("vdpadButtonAssociation");

            if (typeid(*button) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = static_cast<JoyAxisButton*>(button);
                xml->writeAttribute("axis", QString::number(axisbutton->getAxis()->getRealJoyIndex()));
                xml->writeAttribute("button", QString::number(button->getJoyNumber()));
            }
            else
            {
                xml->writeAttribute("axis", QString::number(0));
                xml->writeAttribute("button", QString::number(button->getRealJoyNumber()));
            }

            xml->writeAttribute("direction", QString::number(JoyDPadButton::DpadRight));
            xml->writeEndElement();
        }

        xml->writeEndElement();
    }

    xml->writeStartElement("sets");
    for (int i=0; i < joystick_sets.size(); i++)
    {
        joystick_sets.value(i)->writeConfig(xml);
    }
    xml->writeEndElement();

    xml->writeEndElement();
}

void Joystick::changeSetAxisButtonAssociation(int button_index, int axis_index, int originset, int newset, int mode)
{
    JoyAxisButton *button = 0;
    if (button_index == 0)
    {
        button = joystick_sets.value(newset)->getJoyAxis(axis_index)->getNAxisButton();
    }
    else if (button_index == 1)
    {
        button = joystick_sets.value(newset)->getJoyAxis(axis_index)->getPAxisButton();
    }

    JoyButton::SetChangeCondition tempmode = (JoyButton::SetChangeCondition)mode;
    button->setChangeSetSelection(originset);
    button->setChangeSetCondition(tempmode, true);
}

void Joystick::changeSetStickButtonAssociation(int button_index, int stick_index, int originset, int newset, int mode)
{
    JoyControlStickButton *button = joystick_sets.value(newset)->getJoyStick(stick_index)->getDirectionButton((JoyControlStick::JoyStickDirections)button_index);

    JoyButton::SetChangeCondition tempmode = (JoyButton::SetChangeCondition)mode;
    button->setChangeSetSelection(originset);
    button->setChangeSetCondition(tempmode, true);
}

void Joystick::changeSetDPadButtonAssociation(int button_index, int dpad_index, int originset, int newset, int mode)
{
    JoyDPadButton *button = joystick_sets.value(newset)->getJoyDPad(dpad_index)->getJoyButton(button_index);

    JoyButton::SetChangeCondition tempmode = (JoyButton::SetChangeCondition)mode;
    button->setChangeSetSelection(originset);
    button->setChangeSetCondition(tempmode, true);
}

void Joystick::propogateSetAxisThrottleChange(int index, int originset)
{
    SetJoystick *currentSet = joystick_sets.value(originset);
    if (currentSet)
    {
        JoyAxis *axis = currentSet->getJoyAxis(index);
        if (axis)
        {
            int throttleSetting = axis->getThrottle();

            QHashIterator<int, SetJoystick*> iter(joystick_sets);
            while (iter.hasNext())
            {
                iter.next();
                SetJoystick *temp = iter.value();
                // Ignore change for set axis that initiated the change
                if (temp != currentSet)
                {
                    temp->getJoyAxis(index)->setThrottle(throttleSetting);
                }
            }
        }
    }
}
