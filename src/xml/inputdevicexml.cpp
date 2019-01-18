#include "inputdevicexml.h"
#include "inputdevice.h"
#include "vdpad.h"
#include "joycontrolstick.h"
#include "joybuttontypes/joycontrolstickbutton.h"

#include "globalvariables.h"
#include "messagehandler.h"
#include "common.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>


InputDeviceXml::InputDeviceXml(InputDevice *inputDevice, QObject *parent) : m_inputDevice(inputDevice), QObject(parent)
{

}


InputDeviceXml::InputDeviceXml(int deviceIndex, AntiMicroSettings *setting, QObject *parent) : QObject(parent)
{
    m_inputDevice = nullptr;
}


void InputDeviceXml::readConfig(QXmlStreamReader *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (xml->isStartElement() && (xml->name() == m_inputDevice->getXmlName()))
    {
        m_inputDevice->transferReset();
        xml->readNextStartElement();

        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != m_inputDevice->getXmlName())))
        {
            if ((xml->name() == "sets") && xml->isStartElement())
            {
                xml->readNextStartElement();

                while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != "sets")))
                {
                    if ((xml->name() == "set") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        index = index - 1;

                        if ((index >= 0) && (index < m_inputDevice->getJoystick_sets().size()))
                            m_inputDevice->getJoystick_sets().value(index)->readConfig(xml);
                    }
                    else
                    {
                        // If none of the above, skip the element
                        xml->skipCurrentElement();
                    }

                    xml->readNextStartElement();
                }
            }
            else if ((xml->name() == "stickAxisAssociation") && xml->isStartElement())
            {
                int stickIndex = xml->attributes().value("index").toString().toInt();
                int xAxis = xml->attributes().value("xAxis").toString().toInt();
                int yAxis = xml->attributes().value("yAxis").toString().toInt();

                if ((stickIndex > 0) && (xAxis > 0) && (yAxis > 0))
                {
                    xAxis -= 1;
                    yAxis -= 1;
                    stickIndex -= 1;

                    for (int i = 0; i < m_inputDevice->getJoystick_sets().size(); i++)
                    {
                        SetJoystick *currentset = m_inputDevice->getJoystick_sets().value(i);
                        JoyAxis *axis1 = currentset->getJoyAxis(xAxis);
                        JoyAxis *axis2 = currentset->getJoyAxis(yAxis);

                        if ((axis1 != nullptr) && (axis2 != nullptr))
                        {
                            JoyControlStick *stick = new JoyControlStick(axis1, axis2, stickIndex, i, m_inputDevice);
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
            else if ((xml->name() == "vdpadButtonAssociations") && xml->isStartElement())
            {
                int vdpadIndex = xml->attributes().value("index").toString().toInt();
                if (vdpadIndex > 0)
                {
                    for (int i = 0; i < m_inputDevice->getJoystick_sets().size(); i++)
                    {
                        SetJoystick *currentset = m_inputDevice->getJoystick_sets().value(i);
                        VDPad *vdpad = currentset->getVDPad(vdpadIndex - 1);

                        if (vdpad == nullptr)
                        {
                            vdpad = new VDPad(vdpadIndex-1, i, currentset, currentset);
                            currentset->addVDPad(vdpadIndex-1, vdpad);
                        }
                    }

                    xml->readNextStartElement();

                    while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != "vdpadButtonAssociations")))
                    {
                        if ((xml->name() == "vdpadButtonAssociation") && xml->isStartElement())
                        {
                            int vdpadAxisIndex = xml->attributes().value("axis").toString().toInt();
                            int vdpadButtonIndex = xml->attributes().value("button").toString().toInt();
                            int vdpadDirection = xml->attributes().value("direction").toString().toInt();

                            if ((vdpadAxisIndex > 0) && (vdpadDirection > 0))
                            {
                                vdpadAxisIndex -= 1;

                                for (int i = 0; i < m_inputDevice->getJoystick_sets().size(); i++)
                                {
                                    SetJoystick *currentset = m_inputDevice->getJoystick_sets().value(i);
                                    VDPad *vdpad = currentset->getVDPad(vdpadIndex - 1);

                                    if (vdpad != nullptr)
                                    {
                                        JoyAxis *axis = currentset->getJoyAxis(vdpadAxisIndex);

                                        if (axis != nullptr)
                                        {
                                            JoyButton *button = nullptr;

                                            if (vdpadButtonIndex == 0) button = axis->getNAxisButton();
                                            else if (vdpadButtonIndex == 1) button = axis->getPAxisButton();

                                            if (button != nullptr)
                                                vdpad->addVButton(static_cast<JoyDPadButton::JoyDPadDirections>(vdpadDirection), button);
                                        }
                                    }
                                }
                            }
                            else if ((vdpadButtonIndex > 0) && (vdpadDirection > 0))
                            {
                                vdpadButtonIndex -= 1;

                                for (int i = 0; i < m_inputDevice->getJoystick_sets().size(); i++)
                                {
                                    SetJoystick *currentset = m_inputDevice->getJoystick_sets().value(i);
                                    VDPad *vdpad = currentset->getVDPad(vdpadIndex - 1);

                                    if (vdpad != nullptr)
                                    {
                                        JoyButton *button = currentset->getJoyButton(vdpadButtonIndex);

                                        if (button != nullptr)
                                            vdpad->addVButton(static_cast<JoyDPadButton::JoyDPadDirections>(vdpadDirection), button);
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

                for (int i = 0; i < m_inputDevice->getJoystick_sets().size(); i++)
                {
                    SetJoystick *currentset = m_inputDevice->getJoystick_sets().value(i);

                    for (int j = 0; j < currentset->getNumberVDPads(); j++)
                    {
                        VDPad *vdpad = currentset->getVDPad(j);

                        if ((vdpad != nullptr) && vdpad->isEmpty())
                            currentset->removeVDPad(j);
                    }
                }
            }
            else if ((xml->name() == "names") && xml->isStartElement())
            {
                xml->readNextStartElement();

                while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != "names")))
                {
                    if ((xml->name() == "buttonname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            m_inputDevice->setButtonName(index, temp);
                        }
                    }
                    else if ((xml->name() == "axisbuttonname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        int buttonIndex = xml->attributes().value("button").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        buttonIndex = buttonIndex - 1;

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            m_inputDevice->setAxisButtonName(index, buttonIndex, temp);
                        }
                    }
                    else if ((xml->name() == "controlstickbuttonname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        int buttonIndex = xml->attributes().value("button").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            m_inputDevice->setStickButtonName(index, buttonIndex, temp);
                        }
                    }
                    else if ((xml->name() == "dpadbuttonname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        int buttonIndex = xml->attributes().value("button").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            m_inputDevice->setDPadButtonName(index, buttonIndex, temp);
                        }
                    }
                    else if ((xml->name() == "vdpadbuttonname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        int buttonIndex = xml->attributes().value("button").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            m_inputDevice->setVDPadButtonName(index, buttonIndex, temp);
                        }
                    }
                    else if ((xml->name() == "axisname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            m_inputDevice->setAxisName(index, temp);
                        }
                    }
                    else if ((xml->name() == "controlstickname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            m_inputDevice->setStickName(index, temp);
                        }
                    }
                    else if ((xml->name() == "dpadname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            m_inputDevice->setDPadName(index, temp);
                        }
                    }
                    else if ((xml->name() == "vdpadname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            m_inputDevice->setVDPadName(index, temp);
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
            else if ((xml->name() == "keyPressTime") && xml->isStartElement())
            {
                int tempchoice = xml->readElementText().toInt();

                if (tempchoice >= 10)
                {
                    m_inputDevice->setDeviceKeyPressTime(tempchoice);
                }
            }
            else if ((xml->name() == "profilename") && xml->isStartElement())
            {
                m_inputDevice->setProfileName(xml->readElementText());
            }
            else
            {
                // If none of the above, skip the element
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }

        m_inputDevice->reInitButtons();
    }
}

void InputDeviceXml::writeConfig(QXmlStreamWriter *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    xml->writeStartElement(m_inputDevice->getXmlName());
    xml->writeAttribute("configversion", QString::number(PadderCommon::LATESTCONFIGFILEVERSION));
    xml->writeAttribute("appversion", PadderCommon::programVersion);

    xml->writeComment("The SDL name for a joystick is included for informational purposes only.");
    xml->writeTextElement("sdlname", m_inputDevice->getSDLName());
    xml->writeComment("The GUID for a joystick is included for informational purposes only.");
    xml->writeTextElement("guid", m_inputDevice->getGUIDString());

    if (!m_inputDevice->getProfileName().isEmpty())
        xml->writeTextElement("profilename", m_inputDevice->getProfileName());

    for (int i = 0; i < m_inputDevice->getNumberSticks(); i++)
    {
        JoyControlStick *stick = m_inputDevice->getActiveSetJoystick()->getJoyStick(i);

        xml->writeStartElement("stickAxisAssociation");
        xml->writeAttribute("index", QString::number(stick->getRealJoyIndex()));
        xml->writeAttribute("xAxis", QString::number(stick->getAxisX()->getRealJoyIndex()));
        xml->writeAttribute("yAxis", QString::number(stick->getAxisY()->getRealJoyIndex()));
        xml->writeEndElement();
    }

    for (int i = 0; i < m_inputDevice->getNumberVDPads(); i++)
    {
        VDPad *vdpad = m_inputDevice->getActiveSetJoystick()->getVDPad(i);
        xml->writeStartElement("vdpadButtonAssociations");
        xml->writeAttribute("index", QString::number(vdpad->getRealJoyNumber()));
        JoyButton *button = vdpad->getVButton(JoyDPadButton::DpadUp);

        if (button != nullptr)
        {
            xml->writeStartElement("vdpadButtonAssociation");

            if (typeid(*button) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = qobject_cast<JoyAxisButton*>(button);
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

        if (button != nullptr)
        {
            xml->writeStartElement("vdpadButtonAssociation");

            if (typeid(*button) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = qobject_cast<JoyAxisButton*>(button);
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

        if (button != nullptr)
        {
            xml->writeStartElement("vdpadButtonAssociation");

            if (typeid(*button) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = qobject_cast<JoyAxisButton*>(button);
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

        if (button != nullptr)
        {
            xml->writeStartElement("vdpadButtonAssociation");

            if (typeid(*button) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = qobject_cast<JoyAxisButton*>(button);
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

    bool tempHasNames = m_inputDevice->elementsHaveNames();

    if (tempHasNames)
    {
        xml->writeStartElement("names"); // <name>
        SetJoystick *tempSet = m_inputDevice->getActiveSetJoystick();

        for (int i = 0; i < m_inputDevice->getNumberButtons(); i++)
        {
            JoyButton *button = tempSet->getJoyButton(i);

            if ((button != nullptr) && !button->getButtonName().isEmpty())
            {
                xml->writeStartElement("buttonname");
                xml->writeAttribute("index", QString::number(button->getRealJoyNumber()));
                xml->writeCharacters(button->getButtonName());
                xml->writeEndElement();
            }
        }

        for (int i = 0; i < m_inputDevice->getNumberAxes(); i++)
        {
            JoyAxis *axis = tempSet->getJoyAxis(i);

            if (axis != nullptr)
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

        for (int i = 0; i < m_inputDevice->getNumberSticks(); i++)
        {
            JoyControlStick *stick = tempSet->getJoyStick(i);

            if (stick != nullptr)
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

        for (int i = 0; i < m_inputDevice->getNumberHats(); i++)
        {
            JoyDPad *dpad = tempSet->getJoyDPad(i);

            if (dpad != nullptr)
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

        for (int i = 0; i < m_inputDevice->getNumberVDPads(); i++)
        {
            VDPad *vdpad = m_inputDevice->getActiveSetJoystick()->getVDPad(i);

            if (vdpad != nullptr)
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

                    if ((button != nullptr) && !button->getButtonName().isEmpty())
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
    }

    if ((m_inputDevice->getDeviceKeyPressTime() > 0) && (m_inputDevice->getDeviceKeyPressTime() != GlobalVariables::InputDevice::DEFAULTKEYPRESSTIME))
        xml->writeTextElement("keyPressTime", QString::number(m_inputDevice->getDeviceKeyPressTime()));

    xml->writeStartElement("sets");

    for (int i = 0; i < m_inputDevice->getJoystick_sets().size(); i++)
        m_inputDevice->getJoystick_sets().value(i)->writeConfig(xml);

    xml->writeEndElement();
    xml->writeEndElement();
}

