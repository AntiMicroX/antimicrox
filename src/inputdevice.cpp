#include <typeinfo>

#include "inputdevice.h"

const int InputDevice::NUMBER_JOYSETS = 8;

InputDevice::InputDevice(int deviceIndex, QObject *parent) :
    QObject(parent)
{
    buttonDownCount = 0;
    joyNumber = deviceIndex;
    active_set = 0;
    joystickID = 0;
}

InputDevice::~InputDevice()
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

int InputDevice::getJoyNumber()
{
    return joyNumber;
}

int InputDevice::getRealJoyNumber()
{
    int joynumber = getJoyNumber();
    return joynumber + 1;
}

void InputDevice::reset()
{
    for (int i=0; i < NUMBER_JOYSETS; i++)
    {
        SetJoystick* set = joystick_sets.value(i);
        set->reset();
    }

    buttonDownCount = 0;
}

void InputDevice::setActiveSetNumber(int index)
{
    if ((index >= 0 && index < NUMBER_JOYSETS) && (index != active_set))
    {
        QList<bool> buttonstates;
        QList<int> axesstates;
        QList<int> dpadstates;

        SetJoystick *current_set = joystick_sets.value(active_set);
        SetJoystick *old_set = current_set;
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
            bool tempignore = true;
            JoyButton *button = current_set->getJoyButton(i);
            JoyButton *oldButton = old_set->getJoyButton(i);
            if (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld)
            {
                if (value)
                {
                    if (oldButton->getChangeSetCondition() == JoyButton::SetChangeWhileHeld && oldButton->getWhileHeldStatus())
                    {
                        button->setWhileHeldStatus(true);
                    }
                    else if (!button->getWhileHeldStatus())
                    {
                        tempignore = false;
                    }
                }
                else
                {
                    button->setWhileHeldStatus(false);
                }
            }

            //button->joyEvent(value, true);
            button->joyEvent(value, tempignore);
        }

        for (int i = 0; i < current_set->getNumberAxes(); i++)
        {
            int value = axesstates.at(i);
            bool tempignore = true;
            JoyAxis *axis = current_set->getJoyAxis(i);
            JoyAxisButton *oldButton = old_set->getJoyAxis(i)->getAxisButtonByValue(value);
            JoyAxisButton *button = axis->getAxisButtonByValue(value);

            if (button && oldButton)
            {
                if (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld)
                {
                    if (value)
                    {
                        if (oldButton->getChangeSetCondition() == JoyButton::SetChangeWhileHeld && oldButton->getWhileHeldStatus())
                        {
                            button->setWhileHeldStatus(true);
                        }
                        else if (!button->getWhileHeldStatus())
                        {
                            tempignore = false;
                        }
                    }
                }
            }
            else if (!button)
            {
                axis->getPAxisButton()->setWhileHeldStatus(false);
                axis->getNAxisButton()->setWhileHeldStatus(false);
            }

            axis->joyEvent(value, tempignore);
        }

        for (int i = 0; i < current_set->getNumberHats(); i++)
        {
            int value = dpadstates.at(i);
            bool tempignore = true;
            JoyDPad *dpad = current_set->getJoyDPad(i);
            JoyDPadButton *button = dpad->getJoyButton(value);
            JoyDPadButton *oldButton = old_set->getJoyDPad(i)->getJoyButton(value);

            if (button && oldButton)
            {
                if (button->getChangeSetCondition() == JoyButton::SetChangeWhileHeld)
                {
                    if (value)
                    {
                        if (oldButton->getChangeSetCondition() == JoyButton::SetChangeWhileHeld && oldButton->getWhileHeldStatus())
                        {
                            button->setWhileHeldStatus(true);
                        }
                        else if (!button->getWhileHeldStatus())
                        {
                            tempignore = false;
                        }
                    }
                }
            }
            else if (!button)
            {
                QHashIterator<int, JoyDPadButton*> iter(*dpad->getJoyButtons());
                while (iter.hasNext())
                {
                    JoyDPadButton *button = iter.next().value();
                    button->setWhileHeldStatus(false);
                }
            }

            dpad->joyEvent(value, tempignore);
        }
    }
}

int InputDevice::getActiveSetNumber()
{
    return active_set;
}

SetJoystick* InputDevice::getActiveSetJoystick()
{
    return joystick_sets.value(active_set);
}

int InputDevice::getNumberButtons()
{
    return getActiveSetJoystick()->getNumberButtons();
}

int InputDevice::getNumberAxes()
{
    return getActiveSetJoystick()->getNumberAxes();
}

int InputDevice::getNumberHats()
{
    return getActiveSetJoystick()->getNumberHats();
}

int InputDevice::getNumberSticks()
{
    return getActiveSetJoystick()->getNumberSticks();
}

int InputDevice::getNumberVDPads()
{
    return getActiveSetJoystick()->getNumberVDPads();
}

SetJoystick* InputDevice::getSetJoystick(int index)
{
    return joystick_sets.value(index);
}

void InputDevice::propogateSetChange(int index)
{
    emit setChangeActivated(index);
}

void InputDevice::changeSetButtonAssociation(int button_index, int originset, int newset, int mode)
{
    JoyButton *button = joystick_sets.value(newset)->getJoyButton(button_index);
    JoyButton::SetChangeCondition tempmode = (JoyButton::SetChangeCondition)mode;
    button->setChangeSetSelection(originset);
    button->setChangeSetCondition(tempmode, true);
}

void InputDevice::readConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && xml->name() == getXmlName())
    {
        reset();

        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != getXmlName()))
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

                for (int i=0; i < joystick_sets.size(); i++)
                {
                    SetJoystick *currentset = joystick_sets.value(i);
                    for (int j=0; j < currentset->getNumberVDPads(); j++)
                    {
                        VDPad *vdpad = currentset->getVDPad(j);
                        if (vdpad && vdpad->isEmpty())
                        {
                            currentset->removeVDPad(j);
                        }
                    }
                }
            }
            else if (xml->name() == "names" && xml->isStartElement())
            {
                xml->readNextStartElement();
                while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != "names"))
                {
                    if (xml->name() == "buttonname" && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if (index >= 0 && !temp.isEmpty())
                        {
                            setButtonName(index, temp);
                        }
                    }
                    else if (xml->name() == "axisbuttonname" && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        int buttonIndex = xml->attributes().value("button").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        buttonIndex = buttonIndex - 1;
                        if (index >= 0 && !temp.isEmpty())
                        {
                            setAxisButtonName(index, buttonIndex, temp);
                        }
                    }
                    else if (xml->name() == "controlstickbuttonname" && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        int buttonIndex = xml->attributes().value("button").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if (index >= 0 && !temp.isEmpty())
                        {
                            setStickButtonName(index, buttonIndex, temp);
                        }
                    }
                    else if (xml->name() == "dpadbuttonname" && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        int buttonIndex = xml->attributes().value("button").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if (index >= 0 && !temp.isEmpty())
                        {
                            setDPadButtonName(index, buttonIndex, temp);
                        }
                    }
                    else if (xml->name() == "vdpadbuttonname" && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        int buttonIndex = xml->attributes().value("button").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if (index >= 0 && !temp.isEmpty())
                        {
                            setVDPadButtonName(index, buttonIndex, temp);
                        }
                    }
                    else if (xml->name() == "axisname" && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if (index >= 0 && !temp.isEmpty())
                        {
                            setAxisName(index, temp);
                        }
                    }
                    else if (xml->name() == "controlstickname" && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if (index >= 0 && !temp.isEmpty())
                        {
                            setStickName(index, temp);
                        }
                    }
                    else if (xml->name() == "dpadname" && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if (index >= 0 && !temp.isEmpty())
                        {
                            setDPadName(index, temp);
                        }
                    }
                    else if (xml->name() == "vdpadname" && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if (index >= 0 && !temp.isEmpty())
                        {
                            setVDPadName(index, temp);
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
            else
            {
                // If none of the above, skip the element
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }
    }
}

void InputDevice::writeConfig(QXmlStreamWriter *xml)
{
    xml->writeStartElement(getXmlName());
    xml->writeAttribute("configversion", QString::number(PadderCommon::LATESTCONFIGFILEVERSION));
    xml->writeAttribute("appversion", PadderCommon::programVersion);

    xml->writeComment("The SDL name for a joystick is included for informational purposes only.");
    xml->writeTextElement("sdlname", getSDLName());

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

    xml->writeStartElement("names"); // <name>

    SetJoystick *tempSet = getActiveSetJoystick();
    for (int i=0; i < getNumberButtons(); i++)
    {
        JoyButton *button = tempSet->getJoyButton(i);
        if (button && !button->getButtonName().isEmpty())
        {
            xml->writeStartElement("buttonname");
            xml->writeAttribute("index", QString::number(button->getRealJoyNumber()));
            xml->writeCharacters(button->getButtonName());
            xml->writeEndElement();
        }
    }

    for (int i=0; i < getNumberAxes(); i++)
    {
        JoyAxis *axis = tempSet->getJoyAxis(i);
        if (axis)
        {
            if (!axis->getAxisName().isEmpty())
            {
                xml->writeStartElement("axisname");
                xml->writeAttribute("index", QString::number(axis->getRealJoyIndex()));
                xml->writeCharacters(axis->getAxisName());
                xml->writeEndElement();
            }

            JoyAxisButton *naxisbutton = axis->getNAxisButton();
            if (!naxisbutton->getButtonName().isEmpty())
            {
                xml->writeStartElement("axisbuttonname");
                xml->writeAttribute("index", QString::number(axis->getRealJoyIndex()));
                xml->writeAttribute("button", QString::number(naxisbutton->getRealJoyNumber()));
                xml->writeCharacters(naxisbutton->getButtonName());
                xml->writeEndElement();
            }

            JoyAxisButton *paxisbutton = axis->getPAxisButton();
            if (!paxisbutton->getButtonName().isEmpty())
            {
                xml->writeStartElement("axisbuttonname");
                xml->writeAttribute("index", QString::number(axis->getRealJoyIndex()));
                xml->writeAttribute("button", QString::number(paxisbutton->getRealJoyNumber()));
                xml->writeCharacters(paxisbutton->getButtonName());
                xml->writeEndElement();
            }
        }
    }

    for (int i=0; i < getNumberSticks(); i++)
    {
        JoyControlStick *stick = tempSet->getJoyStick(i);
        if (stick)
        {
            if (!stick->getStickName().isEmpty())
            {
                xml->writeStartElement("controlstickname");
                xml->writeAttribute("index", QString::number(stick->getRealJoyIndex()));
                xml->writeCharacters(stick->getStickName());
                xml->writeEndElement();
            }

            QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton*> *buttons = stick->getButtons();
            QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> iter(*buttons);
            while (iter.hasNext())
            {
                JoyControlStickButton *button = iter.next().value();
                if (button && !button->getButtonName().isEmpty())
                {
                    xml->writeStartElement("controlstickbuttonname");
                    xml->writeAttribute("index", QString::number(stick->getRealJoyIndex()));
                    xml->writeAttribute("button", QString::number(button->getRealJoyNumber()));
                    xml->writeCharacters(button->getButtonName());
                    xml->writeEndElement();
                }
            }
        }
    }

    for (int i=0; i < getNumberHats(); i++)
    {
        JoyDPad *dpad = tempSet->getJoyDPad(i);
        if (dpad)
        {
            if (!dpad->getDpadName().isEmpty())
            {
                xml->writeStartElement("dpadname");
                xml->writeAttribute("index", QString::number(dpad->getRealJoyNumber()));
                xml->writeCharacters(dpad->getDpadName());
                xml->writeEndElement();
            }

            QHash<int, JoyDPadButton*> *temp = dpad->getButtons();
            QHashIterator<int, JoyDPadButton*> iter(*temp);
            while (iter.hasNext())
            {
                JoyDPadButton *button = iter.next().value();
                if (button && !button->getButtonName().isEmpty())
                {
                    xml->writeStartElement("dpadbuttonname");
                    xml->writeAttribute("index", QString::number(dpad->getRealJoyNumber()));
                    xml->writeAttribute("button", QString::number(button->getRealJoyNumber()));
                    xml->writeCharacters(button->getButtonName());
                    xml->writeEndElement();
                }
            }
        }
    }

    for (int i=0; i < getNumberVDPads(); i++)
    {
        VDPad *vdpad = getActiveSetJoystick()->getVDPad(i);
        if (vdpad)
        {
            if (!vdpad->getDpadName().isEmpty())
            {
                xml->writeStartElement("vdpadname");
                xml->writeAttribute("index", QString::number(vdpad->getRealJoyNumber()));
                xml->writeCharacters(vdpad->getDpadName());
                xml->writeEndElement();
            }

            QHash<int, JoyDPadButton*> *temp = vdpad->getButtons();
            QHashIterator<int, JoyDPadButton*> iter(*temp);
            while (iter.hasNext())
            {
                JoyDPadButton *button = iter.next().value();
                if (button && !button->getButtonName().isEmpty())
                {
                    xml->writeStartElement("vdpadbutton");
                    xml->writeAttribute("index", QString::number(vdpad->getRealJoyNumber()));
                    xml->writeAttribute("button", QString::number(button->getRealJoyNumber()));
                    xml->writeCharacters(button->getButtonName());
                    xml->writeEndElement();
                }
            }
        }
    }
    xml->writeEndElement(); // </names>

    xml->writeStartElement("sets");
    for (int i=0; i < joystick_sets.size(); i++)
    {
        joystick_sets.value(i)->writeConfig(xml);
    }
    xml->writeEndElement();

    xml->writeEndElement();
}

void InputDevice::changeSetAxisButtonAssociation(int button_index, int axis_index, int originset, int newset, int mode)
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

void InputDevice::changeSetStickButtonAssociation(int button_index, int stick_index, int originset, int newset, int mode)
{
    JoyControlStickButton *button = joystick_sets.value(newset)->getJoyStick(stick_index)->getDirectionButton((JoyControlStick::JoyStickDirections)button_index);

    JoyButton::SetChangeCondition tempmode = (JoyButton::SetChangeCondition)mode;
    button->setChangeSetSelection(originset);
    button->setChangeSetCondition(tempmode, true);
}

void InputDevice::changeSetDPadButtonAssociation(int button_index, int dpad_index, int originset, int newset, int mode)
{
    JoyDPadButton *button = joystick_sets.value(newset)->getJoyDPad(dpad_index)->getJoyButton(button_index);

    JoyButton::SetChangeCondition tempmode = (JoyButton::SetChangeCondition)mode;
    button->setChangeSetSelection(originset);
    button->setChangeSetCondition(tempmode, true);
}

void InputDevice::propogateSetAxisThrottleChange(int index, int originset)
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

void InputDevice::removeControlStick(int index)
{
    for (int i=0; i < NUMBER_JOYSETS; i++)
    {
        SetJoystick *currentset = getSetJoystick(i);
        if (currentset->getJoyStick(index))
        {
            currentset->removeControlStick(index);
        }
    }
}

bool InputDevice::isActive()
{
    return buttonDownCount != 0;
}

void InputDevice::buttonDownEvent(int setindex, int buttonindex)
{
    Q_UNUSED(setindex);
    Q_UNUSED(buttonindex);

    bool old = isActive();
    buttonDownCount += 1;
    if (isActive() != old)
    {
        emit clicked(joyNumber);
    }
}

void InputDevice::buttonUpEvent(int setindex, int buttonindex)
{
    Q_UNUSED(setindex);
    Q_UNUSED(buttonindex);

    bool old = isActive();
    buttonDownCount -= 1;
    if (buttonDownCount < 0)
    {
        buttonDownCount = 0;
    }

    if (isActive() != old)
    {
        emit released(joyNumber);
    }
}

void InputDevice::buttonClickEvent(int buttonindex)
{
    emit rawButtonClick(buttonindex);
}

void InputDevice::buttonReleaseEvent(int buttonindex)
{
    emit rawButtonRelease(buttonindex);
}

void InputDevice::axisButtonDownEvent(int setindex, int axisindex, int buttonindex)
{
    Q_UNUSED(axisindex);

    buttonDownEvent(setindex, buttonindex);
}

void InputDevice::axisButtonUpEvent(int setindex, int axisindex, int buttonindex)
{
    Q_UNUSED(axisindex);

    buttonUpEvent(setindex, buttonindex);
}

void InputDevice::dpadButtonClickEvent(int buttonindex)
{
    JoyDPadButton *dpadbutton = static_cast<JoyDPadButton*>(sender());

    emit rawDPadButtonClick(dpadbutton->getDPad()->getIndex(), buttonindex);
}

void InputDevice::dpadButtonReleaseEvent(int buttonindex)
{
    JoyDPadButton *dpadbutton = static_cast<JoyDPadButton*>(sender());

    emit rawDPadButtonRelease(dpadbutton->getDPad()->getIndex(), buttonindex);
}

void InputDevice::dpadButtonDownEvent(int setindex, int dpadindex, int buttonindex)
{
    Q_UNUSED(dpadindex);

    buttonDownEvent(setindex, buttonindex);
}

void InputDevice::dpadButtonUpEvent(int setindex, int dpadindex, int buttonindex)
{
    Q_UNUSED(dpadindex);

    buttonUpEvent(setindex, buttonindex);
}

void InputDevice::stickButtonDownEvent(int setindex, int stickindex, int buttonindex)
{
    Q_UNUSED(stickindex);

    buttonDownEvent(setindex, buttonindex);
}

void InputDevice::stickButtonUpEvent(int setindex, int stickindex, int buttonindex)
{
    Q_UNUSED(stickindex);

    buttonUpEvent(setindex, buttonindex);
}

void InputDevice::setButtonName(int index, QString tempName)
{
    QHashIterator<int, SetJoystick*> iter(joystick_sets);
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, SIGNAL(setButtonNameChange(int)), this, SLOT(updateSetButtonNames(int)));
        JoyButton *button = tempSet->getJoyButton(index);
        if (button)
        {
            button->setButtonName(tempName);
        }
        connect(tempSet, SIGNAL(setButtonNameChange(int)), this, SLOT(updateSetButtonNames(int)));
    }
}

void InputDevice::setAxisButtonName(int axisIndex, int buttonIndex, QString tempName)
{
    QHashIterator<int, SetJoystick*> iter(joystick_sets);
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, SIGNAL(setAxisButtonNameChange(int,int)), this, SLOT(updateSetAxisButtonNames(int,int)));
        JoyAxis *axis = tempSet->getJoyAxis(axisIndex);
        if (axis)
        {
            JoyAxisButton *button = 0;
            if (buttonIndex == 0)
            {
                button = axis->getNAxisButton();
            }
            else if (buttonIndex == 1)
            {
                button = axis->getPAxisButton();
            }

            if (button)
            {
                button->setButtonName(tempName);
            }
        }
        connect(tempSet, SIGNAL(setAxisButtonNameChange(int,int)), this, SLOT(updateSetAxisButtonNames(int,int)));
    }
}

void InputDevice::setStickButtonName(int stickIndex, int buttonIndex, QString tempName)
{
    QHashIterator<int, SetJoystick*> iter(joystick_sets);
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, SIGNAL(setStickButtonNameChange(int,int)), this, SLOT(updateSetStickButtonNames(int,int)));
        JoyControlStick *stick = tempSet->getJoyStick(stickIndex);
        if (stick)
        {
            JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::JoyStickDirections(buttonIndex));
            if (button)
            {
                button->setButtonName(tempName);
            }
        }
        connect(tempSet, SIGNAL(setStickButtonNameChange(int,int)), this, SLOT(updateSetStickButtonNames(int,int)));
    }
}

void InputDevice::setDPadButtonName(int dpadIndex, int buttonIndex, QString tempName)
{
    QHashIterator<int, SetJoystick*> iter(joystick_sets);
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, SIGNAL(setDPadButtonNameChange(int,int)), this, SLOT(updateSetDPadButtonNames(int,int)));
        JoyDPad *dpad = tempSet->getJoyDPad(dpadIndex);
        if (dpad)
        {
            JoyDPadButton *button = dpad->getJoyButton(buttonIndex);
            if (button)
            {
                button->setButtonName(tempName);
            }
        }
        connect(tempSet, SIGNAL(setDPadButtonNameChange(int,int)), this, SLOT(updateSetDPadButtonNames(int,int)));
    }
}

void InputDevice::setVDPadButtonName(int vdpadIndex, int buttonIndex, QString tempName)
{
    QHashIterator<int, SetJoystick*> iter(joystick_sets);
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, SIGNAL(setVDPadButtonNameChange(int,int)), this, SLOT(updateSetVDPadButtonNames(int,int)));
        VDPad *vdpad = tempSet->getVDPad(vdpadIndex);
        if (vdpad)
        {
            JoyDPadButton *button = vdpad->getJoyButton(buttonIndex);
            if (button)
            {
                button->setButtonName(tempName);
            }
        }
        connect(tempSet, SIGNAL(setVDPadButtonNameChange(int,int)), this, SLOT(updateSetVDPadButtonNames(int,int)));
    }
}

void InputDevice::setAxisName(int axisIndex, QString tempName)
{
    QHashIterator<int, SetJoystick*> iter(joystick_sets);
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, SIGNAL(setAxisNameChange(int)), this, SLOT(updateSetAxisNames(int)));
        JoyAxis *axis = tempSet->getJoyAxis(axisIndex);
        if (axis)
        {
            axis->setAxisName(tempName);
        }
        connect(tempSet, SIGNAL(setAxisNameChange(int)), this, SLOT(updateSetAxisNames(int)));
    }
}

void InputDevice::setStickName(int stickIndex, QString tempName)
{
    QHashIterator<int, SetJoystick*> iter(joystick_sets);
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, SIGNAL(setStickNameChange(int)), this, SLOT(updateSetStickNames(int)));
        JoyControlStick *stick = tempSet->getJoyStick(stickIndex);
        if (stick)
        {
            stick->setStickName(tempName);
        }
        connect(tempSet, SIGNAL(setStickNameChange(int)), this, SLOT(updateSetStickNames(int)));
    }
}

void InputDevice::setDPadName(int dpadIndex, QString tempName)
{
    QHashIterator<int, SetJoystick*> iter(joystick_sets);
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, SIGNAL(setDPadNameChange(int)), this, SLOT(updateSetDPadNames(int)));
        JoyDPad *dpad = tempSet->getJoyDPad(dpadIndex);
        if (dpad)
        {
            dpad->setDPadName(tempName);
        }
        connect(tempSet, SIGNAL(setDPadNameChange(int)), this, SLOT(updateSetDPadNames(int)));
    }
}

void InputDevice::setVDPadName(int vdpadIndex, QString tempName)
{
    QHashIterator<int, SetJoystick*> iter(joystick_sets);
    while (iter.hasNext())
    {
        SetJoystick *tempSet = iter.next().value();
        disconnect(tempSet, SIGNAL(setVDPadNameChange(int)), this, SLOT(updateSetVDPadNames(int)));
        VDPad *vdpad = tempSet->getVDPad(vdpadIndex);
        if (vdpad)
        {
            vdpad->setDPadName(tempName);
        }
        connect(tempSet, SIGNAL(setVDPadNameChange(int)), this, SLOT(updateSetVDPadNames(int)));
    }
}


void InputDevice::updateSetButtonNames(int index)
{
    JoyButton *button = getActiveSetJoystick()->getJoyButton(index);
    if (button)
    {
        setButtonName(index, button->getButtonName());
    }
}

void InputDevice::updateSetAxisButtonNames(int axisIndex, int buttonIndex)
{
    JoyAxis *axis = getActiveSetJoystick()->getJoyAxis(axisIndex);
    if (axis)
    {
        JoyAxisButton *button = 0;
        if (buttonIndex == 0)
        {
            button = axis->getNAxisButton();
        }
        else if (buttonIndex == 1)
        {
            button = axis->getPAxisButton();
        }

        if (button)
        {
            setAxisButtonName(axisIndex, buttonIndex, button->getButtonName());
        }
    }
}

void InputDevice::updateSetStickButtonNames(int stickIndex, int buttonIndex)
{
    JoyControlStick *stick = getActiveSetJoystick()->getJoyStick(stickIndex);
    if (stick)
    {
        JoyControlStickButton *button = stick->getDirectionButton(JoyControlStick::JoyStickDirections(buttonIndex));
        if (button)
        {
            setStickButtonName(stickIndex, buttonIndex, button->getButtonName());
        }
    }
}

void InputDevice::updateSetDPadButtonNames(int dpadIndex, int buttonIndex)
{
    JoyDPad *dpad = getActiveSetJoystick()->getJoyDPad(dpadIndex);
    if (dpad)
    {
        JoyDPadButton *button = dpad->getJoyButton(buttonIndex);
        if (button)
        {
            setDPadButtonName(dpadIndex, buttonIndex, button->getButtonName());
        }
    }
}

void InputDevice::updateSetVDPadButtonNames(int vdpadIndex, int buttonIndex)
{
    VDPad *vdpad = getActiveSetJoystick()->getVDPad(vdpadIndex);
    if (vdpad)
    {
        JoyDPadButton *button = vdpad->getJoyButton(buttonIndex);
        if (button)
        {
            setVDPadButtonName(vdpadIndex, buttonIndex, button->getButtonName());
        }
    }
}

void InputDevice::updateSetAxisNames(int axisIndex)
{
    JoyAxis *axis = getActiveSetJoystick()->getJoyAxis(axisIndex);
    if (axis)
    {
        setAxisName(axisIndex, axis->getAxisName());
    }
}

void InputDevice::updateSetStickNames(int stickIndex)
{
    JoyControlStick *stick = getActiveSetJoystick()->getJoyStick(stickIndex);
    if (stick)
    {
        setStickName(stickIndex, stick->getStickName());
    }
}

void InputDevice::updateSetDPadNames(int dpadIndex)
{
    JoyDPad *dpad = getActiveSetJoystick()->getJoyDPad(dpadIndex);
    if (dpad)
    {
        setDPadName(dpadIndex, dpad->getDpadName());
    }
}

void InputDevice::updateSetVDPadNames(int vdpadIndex)
{
    VDPad *vdpad = getActiveSetJoystick()->getVDPad(vdpadIndex);
    if (vdpad)
    {
        setVDPadName(vdpadIndex, vdpad->getDpadName());
    }
}

void InputDevice::resetButtonDownCount()
{
    buttonDownCount = 0;
    released(0);
}

void InputDevice::enableSetConnections(SetJoystick *setstick)
{
    connect(setstick, SIGNAL(setChangeActivated(int)), this, SLOT(setActiveSetNumber(int)));
    connect(setstick, SIGNAL(setChangeActivated(int)), this, SLOT(propogateSetChange(int)));
    connect(setstick, SIGNAL(setAssignmentButtonChanged(int,int,int,int)), this, SLOT(changeSetButtonAssociation(int,int,int,int)));

    connect(setstick, SIGNAL(setAssignmentAxisChanged(int,int,int,int,int)), this, SLOT(changeSetAxisButtonAssociation(int,int,int,int,int)));
    connect(setstick, SIGNAL(setAssignmentDPadChanged(int,int,int,int,int)), this, SLOT(changeSetDPadButtonAssociation(int,int,int,int,int)));
    connect(setstick, SIGNAL(setAssignmentStickChanged(int,int,int,int,int)), this, SLOT(changeSetStickButtonAssociation(int,int,int,int,int)));
    connect(setstick, SIGNAL(setAssignmentAxisThrottleChanged(int,int)), this, SLOT(propogateSetAxisThrottleChange(int, int)));

    connect(setstick, SIGNAL(setButtonClick(int,int)), this, SLOT(buttonDownEvent(int,int)));

    connect(setstick, SIGNAL(setButtonRelease(int,int)), this, SLOT(buttonUpEvent(int,int)));

    connect(setstick, SIGNAL(setAxisButtonClick(int,int,int)), this, SLOT(axisButtonDownEvent(int,int,int)));
    connect(setstick, SIGNAL(setAxisButtonRelease(int,int,int)), this, SLOT(axisButtonUpEvent(int,int,int)));
    connect(setstick, SIGNAL(setAxisActivated(int,int, int)), this, SLOT(axisActivatedEvent(int,int,int)));

    connect(setstick, SIGNAL(setDPadButtonClick(int,int,int)), this, SLOT(dpadButtonDownEvent(int,int,int)));
    connect(setstick, SIGNAL(setDPadButtonRelease(int,int,int)), this, SLOT(dpadButtonUpEvent(int,int,int)));

    connect(setstick, SIGNAL(setStickButtonClick(int,int,int)), this, SLOT(stickButtonDownEvent(int,int,int)));
    connect(setstick, SIGNAL(setStickButtonRelease(int,int,int)), this, SLOT(stickButtonUpEvent(int,int,int)));

    connect(setstick, SIGNAL(setButtonNameChange(int)), this, SLOT(updateSetButtonNames(int)));
    connect(setstick, SIGNAL(setAxisButtonNameChange(int,int)), this, SLOT(updateSetAxisButtonNames(int,int)));
    connect(setstick, SIGNAL(setStickButtonNameChange(int,int)), this, SLOT(updateSetStickButtonNames(int,int)));
    connect(setstick, SIGNAL(setDPadButtonNameChange(int,int)), this, SLOT(updateSetDPadButtonNames(int,int)));
    connect(setstick, SIGNAL(setVDPadButtonNameChange(int,int)), this, SLOT(updateSetVDPadButtonNames(int,int)));

    connect(setstick, SIGNAL(setAxisNameChange(int)), this, SLOT(updateSetAxisNames(int)));
    connect(setstick, SIGNAL(setStickNameChange(int)), this, SLOT(updateSetStickNames(int)));
    connect(setstick, SIGNAL(setDPadNameChange(int)), this, SLOT(updateSetDPadNames(int)));
    connect(setstick, SIGNAL(setVDPadNameChange(int)), this, SLOT(updateSetVDPadNames(int)));
}

void InputDevice::axisActivatedEvent(int setindex, int axisindex, int value)
{
    Q_UNUSED(setindex);

    emit rawAxisActivated(axisindex, value);
}

void InputDevice::setIndex(int index)
{
    if (index >= 0)
    {
        joyNumber = index;
    }
    else
    {
        joyNumber = 0;
    }
}

#ifdef USE_SDL_2
QString InputDevice::getSDLPlatform()
{
    QString temp = SDL_GetPlatform();
    return temp;
}

#endif
