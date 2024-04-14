/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "inputdevicexml.h"
#include "inputdevice.h"
#include "joybuttontypes/joycontrolstickbutton.h"
#include "joybuttontypes/joysensorbutton.h"
#include "joycontrolstick.h"
#include "joysensor.h"
#include "vdpad.h"

#include "common.h"
#include "globalvariables.h"
#include "logger.h"

#include <QDebug>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

InputDeviceXml::InputDeviceXml(InputDevice *inputDevice, QObject *parent)
    : QObject(parent)
    , m_inputDevice(inputDevice)
{
    QThread *thread = m_inputDevice->thread();
    this->moveToThread(thread);
    connect(this, &InputDeviceXml::readConfigSig, this, &InputDeviceXml::readConfig);
}

/**
 * @brief Deserializes the given XML stream into an InputDevice object
 * @param[in] xml The XML stream to read from
 */
void InputDeviceXml::readConfig(QXmlStreamReader *xml)
{
    m_mutex_read_config.try_lock();
    // reading of config should be handled in inputEventThread
    if (this->thread() != QThread::currentThread())
    {
        emit readConfigSig(xml);
        DEBUG() << "Redirecting readConfig call to antoher Thread, waiting on mutex here.";
        bool opened = m_mutex_read_config.tryLock(1000);
        if (opened)
            DEBUG() << "readConfig mutex opened";
        else
            WARN() << "Could not open mutex_read_config";
        m_mutex_read_config.unlock();
        return;
    }
    if (xml->isStartElement() && (xml->name().toString() == m_inputDevice->getXmlName()))
    {
        m_inputDevice->transferReset();
        xml->readNextStartElement();

        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name().toString() != m_inputDevice->getXmlName())))
        {
            if ((xml->name().toString() == "sets") && xml->isStartElement())
            {
                xml->readNextStartElement();

                while (!xml->atEnd() && (!xml->isEndElement() && (xml->name().toString() != "sets")))
                {
                    if ((xml->name().toString() == "set") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        index = index - 1;

                        if ((index >= 0) && (index < m_inputDevice->getJoystick_sets().size()))
                            m_inputDevice->getJoystick_sets().value(index)->readConfig(xml);
                    } else
                    {
                        // If none of the above, skip the element
                        xml->skipCurrentElement();
                    }

                    xml->readNextStartElement();
                }
            } else if ((xml->name().toString() == "stickAxisAssociation") && xml->isStartElement())
            {
                int stickIndex = xml->attributes().value("index").toString().toInt();
                int xAxis = xml->attributes().value("xAxis").toString().toInt();
                int yAxis = xml->attributes().value("yAxis").toString().toInt();

                if ((stickIndex > 0) && (xAxis > 0) && (yAxis > 0))
                {
                    xAxis -= 1;
                    yAxis -= 1;
                    stickIndex -= 1;

                    QList<SetJoystick *> setsList = m_inputDevice->getJoystick_sets().values();

                    for (QList<SetJoystick *>::iterator setJoy = setsList.begin(); setJoy != setsList.end(); setJoy++)
                    {
                        int i = setJoy - setsList.begin();
                        JoyAxis *axis1 = (*setJoy)->getJoyAxis(xAxis);
                        JoyAxis *axis2 = (*setJoy)->getJoyAxis(yAxis);

                        if ((axis1 != nullptr) && (axis2 != nullptr))
                        {
                            JoyControlStick *stick = new JoyControlStick(axis1, axis2, stickIndex, i, m_inputDevice);
                            (*setJoy)->addControlStick(stickIndex, stick);
                        }
                    }

                    xml->readNext();
                } else
                {
                    xml->skipCurrentElement();
                }
            } else if ((xml->name().toString() == "vdpadButtonAssociations") && xml->isStartElement())
            {
                int vdpadIndex = xml->attributes().value("index").toString().toInt();

                if (vdpadIndex > 0)
                {
                    QList<SetJoystick *> setsList = m_inputDevice->getJoystick_sets().values();

                    for (QList<SetJoystick *>::iterator setJoy = setsList.begin(); setJoy != setsList.end(); setJoy++)
                    {
                        int i = setJoy - setsList.begin();
                        VDPad *vdpad = (*setJoy)->getVDPad(vdpadIndex - 1);

                        if (vdpad == nullptr)
                        {
                            vdpad = new VDPad(vdpadIndex - 1, i, *setJoy, *setJoy);
                            (*setJoy)->addVDPad(vdpadIndex - 1, vdpad);
                        }
                    }

                    xml->readNextStartElement();

                    while (!xml->atEnd() && (!xml->isEndElement() && (xml->name().toString() != "vdpadButtonAssociations")))
                    {
                        if ((xml->name().toString() == "vdpadButtonAssociation") && xml->isStartElement())
                        {
                            int vdpadAxisIndex = xml->attributes().value("axis").toString().toInt();
                            int vdpadButtonIndex = xml->attributes().value("button").toString().toInt();
                            int vdpadDirection = xml->attributes().value("direction").toString().toInt();

                            if ((vdpadAxisIndex > 0) && (vdpadDirection > 0))
                            {
                                vdpadAxisIndex -= 1;
                                QList<SetJoystick *> setsListJoy = m_inputDevice->getJoystick_sets().values();

                                for (QList<SetJoystick *>::iterator setJoyCur = setsListJoy.begin();
                                     setJoyCur != setsListJoy.end(); setJoyCur++)
                                {
                                    VDPad *vdpad = (*setJoyCur)->getVDPad(vdpadIndex - 1);

                                    if (vdpad != nullptr)
                                    {
                                        JoyAxis *axis = (*setJoyCur)->getJoyAxis(vdpadAxisIndex);

                                        if (axis != nullptr)
                                        {
                                            JoyButton *button = nullptr;

                                            if (vdpadButtonIndex == 0)
                                                button = axis->getNAxisButton();
                                            else if (vdpadButtonIndex == 1)
                                                button = axis->getPAxisButton();

                                            if (button != nullptr)
                                                vdpad->addVButton(
                                                    static_cast<JoyDPadButton::JoyDPadDirections>(vdpadDirection), button);
                                        }
                                    }
                                }
                            } else if ((vdpadButtonIndex > 0) && (vdpadDirection > 0))
                            {
                                vdpadButtonIndex -= 1;
                                QList<SetJoystick *> setsListJoy = m_inputDevice->getJoystick_sets().values();

                                for (QList<SetJoystick *>::iterator setJoyCur = setsListJoy.begin();
                                     setJoyCur != setsListJoy.end(); setJoyCur++)
                                {
                                    VDPad *vdpad = (*setJoyCur)->getVDPad(vdpadIndex - 1);

                                    if (vdpad != nullptr)
                                    {
                                        JoyButton *button = (*setJoyCur)->getJoyButton(vdpadButtonIndex);

                                        if (button != nullptr)
                                            vdpad->addVButton(static_cast<JoyDPadButton::JoyDPadDirections>(vdpadDirection),
                                                              button);
                                    }
                                }
                            }

                            xml->readNext();
                        } else
                        {
                            xml->skipCurrentElement();
                        }

                        xml->readNextStartElement();
                    }
                }

                QList<SetJoystick *> setJoys = m_inputDevice->getJoystick_sets().values();

                for (QList<SetJoystick *>::iterator currJoy = setJoys.begin(); currJoy != setJoys.end(); currJoy++)
                {
                    QList<VDPad *> VDPadLists = (*currJoy)->getVdpads().values();

                    for (QList<VDPad *>::iterator currVDPad = VDPadLists.begin(); currVDPad != VDPadLists.end(); currVDPad++)
                    {
                        if (((*currVDPad) != nullptr) && (*currVDPad)->isEmpty())
                            (*currJoy)->removeVDPad(currVDPad - VDPadLists.begin());
                    }
                }
            } else if ((xml->name().toString() == "names") && xml->isStartElement())
            {
                xml->readNextStartElement();

                while (!xml->atEnd() && (!xml->isEndElement() && (xml->name().toString() != "names")))
                {
                    if ((xml->name().toString() == "buttonname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            m_inputDevice->setButtonName(index, temp);
                        }
                    } else if ((xml->name().toString() == "axisbuttonname") && xml->isStartElement())
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
                    } else if ((xml->name().toString() == "controlstickbuttonname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        int buttonIndex = xml->attributes().value("button").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            m_inputDevice->setStickButtonName(index, buttonIndex, temp);
                        }
                    } else if ((xml->name().toString() == "sensorbuttonname") && xml->isStartElement())
                    {
                        int type = xml->attributes().value("type").toString().toInt();
                        int direction = xml->attributes().value("button").toString().toInt();
                        QString temp = xml->readElementText();
                        if (!temp.isEmpty())
                            m_inputDevice->setSensorButtonName(static_cast<JoySensorType>(type),
                                                               static_cast<JoySensorDirection>(direction), temp);
                    } else if ((xml->name().toString() == "dpadbuttonname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        int buttonIndex = xml->attributes().value("button").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            m_inputDevice->setDPadButtonName(index, buttonIndex, temp);
                        }
                    } else if ((xml->name().toString() == "vdpadbuttonname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        int buttonIndex = xml->attributes().value("button").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            m_inputDevice->setVDPadButtonName(index, buttonIndex, temp);
                        }
                    } else if ((xml->name().toString() == "axisname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            m_inputDevice->setAxisName(index, temp);
                        }
                    } else if ((xml->name().toString() == "controlstickname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            m_inputDevice->setStickName(index, temp);
                        }
                    } else if ((xml->name().toString() == "sensorname") && xml->isStartElement())
                    {
                        int type = xml->attributes().value("type").toString().toInt();
                        QString temp = xml->readElementText();
                        if (!temp.isEmpty())
                            m_inputDevice->setSensorName(static_cast<JoySensorType>(type), temp);
                    } else if ((xml->name().toString() == "dpadname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            m_inputDevice->setDPadName(index, temp);
                        }
                    } else if ((xml->name().toString() == "vdpadname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            m_inputDevice->setVDPadName(index, temp);
                        }
                    } else
                    {
                        // If none of the above, skip the element
                        xml->skipCurrentElement();
                    }

                    xml->readNextStartElement();
                }
            } else if ((xml->name().toString() == "calibration") && xml->isStartElement())
            {
                m_inputDevice->getCalibrationBackend()->readConfig(xml);
                m_inputDevice->getCalibrationBackend()->applyCalibrations();
            } else if ((xml->name().toString() == "keyPressTime") && xml->isStartElement())
            {
                int tempchoice = xml->readElementText().toInt();

                if (tempchoice >= 10)
                {
                    m_inputDevice->setDeviceKeyPressTime(tempchoice);
                }
            } else if ((xml->name().toString() == "profilename") && xml->isStartElement())
            {
                m_inputDevice->setProfileName(xml->readElementText());
            } else
            {
                // If none of the above, skip the element
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }

        m_inputDevice->reInitButtons();
    }
    m_mutex_read_config.unlock();
}

/**
 * @brief Serializes an InputDevice object into the the given XML stream
 * @param[in,out] xml The XML stream to write to
 */
void InputDeviceXml::writeConfig(QXmlStreamWriter *xml)
{
    xml->writeStartElement(m_inputDevice->getXmlName());
    xml->writeAttribute("configversion", QString::number(PadderCommon::LATESTCONFIGFILEVERSION));
    xml->writeAttribute("appversion", PadderCommon::programVersion);

    xml->writeComment("The SDL name for a joystick is included for informational purposes only.");
    xml->writeTextElement("sdlname", m_inputDevice->getSDLName());
    xml->writeComment("The Unique ID for a joystick is included for informational purposes only.");
    xml->writeTextElement("uniqueID", m_inputDevice->getUniqueIDString());
    // xml->writeTextElement("guid", m_inputDevice->getGUIDString());

    if (!m_inputDevice->getProfileName().isEmpty())
        xml->writeTextElement("profilename", m_inputDevice->getProfileName());

    QListIterator<JoyControlStick *> currJoyStick(m_inputDevice->getActiveSetJoystick()->getSticks().values());
    while (currJoyStick.hasNext())
    {
        JoyControlStick *stick = currJoyStick.next();

        xml->writeStartElement("stickAxisAssociation");
        xml->writeAttribute("index", QString::number(stick->getRealJoyIndex()));
        xml->writeAttribute("xAxis", QString::number(stick->getAxisX()->getRealJoyIndex()));
        xml->writeAttribute("yAxis", QString::number(stick->getAxisY()->getRealJoyIndex()));
        xml->writeEndElement();
    }

    // write vdpadButtonAssociations
    QListIterator<VDPad *> currVDPad(m_inputDevice->getActiveSetJoystick()->getVdpads().values());
    while (currVDPad.hasNext())
    {
        VDPad *vdpad = currVDPad.next();
        xml->writeStartElement("vdpadButtonAssociations");
        xml->writeAttribute("index", QString::number(vdpad->getRealJoyNumber()));
        JoyButton *button = vdpad->getVButton(JoyDPadButton::DpadUp);

        if (button != nullptr)
        {
            xml->writeStartElement("vdpadButtonAssociation");

            if (typeid(*button) == typeid(JoyAxisButton))
            {
                JoyAxisButton *axisbutton = qobject_cast<JoyAxisButton *>(button);
                xml->writeAttribute("axis", QString::number(axisbutton->getAxis()->getRealJoyIndex()));
                xml->writeAttribute("button", QString::number(button->getJoyNumber()));
            } else
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
                JoyAxisButton *axisbutton = qobject_cast<JoyAxisButton *>(button);
                xml->writeAttribute("axis", QString::number(axisbutton->getAxis()->getRealJoyIndex()));
                xml->writeAttribute("button", QString::number(button->getJoyNumber()));
            } else
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
                JoyAxisButton *axisbutton = qobject_cast<JoyAxisButton *>(button);
                xml->writeAttribute("axis", QString::number(axisbutton->getAxis()->getRealJoyIndex()));
                xml->writeAttribute("button", QString::number(button->getJoyNumber()));
            } else
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
                JoyAxisButton *axisbutton = qobject_cast<JoyAxisButton *>(button);
                xml->writeAttribute("axis", QString::number(axisbutton->getAxis()->getRealJoyIndex()));
                xml->writeAttribute("button", QString::number(button->getJoyNumber()));
            } else
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

        // write buttons of joystick
        QListIterator<JoyButton *> currJoyBtn(m_inputDevice->getActiveSetJoystick()->getButtons().values());
        while (currJoyBtn.hasNext())
        {
            JoyButton *button = currJoyBtn.next();

            if ((button != nullptr) && !button->getButtonName().isEmpty())
            {
                xml->writeStartElement("buttonname");
                xml->writeAttribute("index", QString::number(button->getRealJoyNumber()));
                xml->writeCharacters(button->getButtonName());
                xml->writeEndElement();
            }
        }

        // write axes of joystick
        QListIterator<JoyAxis *> currentAxis(m_inputDevice->getActiveSetJoystick()->getAxes()->values());
        while (currentAxis.hasNext())
        {
            JoyAxis *axis = currentAxis.next();

            if (axis != nullptr)
            {
                if (!axis->getAxisName().isEmpty())
                {
                    xml->writeStartElement("axisname");
                    xml->writeAttribute("index", QString::number(axis->getRealJoyIndex()));
                    xml->writeCharacters(axis->getAxisName());
                    xml->writeEndElement();
                }

                // write button of axis (with negative values)
                JoyAxisButton *naxisbutton = axis->getNAxisButton();

                if (!naxisbutton->getButtonName().isEmpty())
                {
                    xml->writeStartElement("axisbuttonname");
                    xml->writeAttribute("index", QString::number(axis->getRealJoyIndex()));
                    xml->writeAttribute("button", QString::number(naxisbutton->getRealJoyNumber()));
                    xml->writeCharacters(naxisbutton->getButtonName());
                    xml->writeEndElement();
                }

                // write button of axis (with positive values)
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

        // write sticks
        QListIterator<JoyControlStick *> currStick(m_inputDevice->getActiveSetJoystick()->getSticks().values());

        while (currStick.hasNext())
        {
            JoyControlStick *stick = currStick.next();

            if (stick != nullptr)
            {
                if (!stick->getStickName().isEmpty())
                {
                    xml->writeStartElement("controlstickname");
                    xml->writeAttribute("index", QString::number(stick->getRealJoyIndex()));
                    xml->writeCharacters(stick->getStickName());
                    xml->writeEndElement();
                }

                // write button of each stick
                QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton *> *buttons = stick->getButtons();
                QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton *> iter(*buttons);

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

        // write sensors
        auto sensors = m_inputDevice->getActiveSetJoystick()->getSensors();
        for (const auto &sensor : sensors)
        {
            if (sensor != nullptr)
            {
                if (!sensor->getSensorName().isEmpty())
                {
                    xml->writeStartElement("sensorname");
                    xml->writeAttribute("type", QString::number(sensor->getType()));
                    xml->writeCharacters(sensor->getSensorName());
                    xml->writeEndElement();
                }

                // write button of each sensor
                auto buttons = sensor->getButtons();
                for (auto iter = buttons->cbegin(); iter != buttons->cend(); ++iter)
                {
                    JoySensorButton *button = iter.value();

                    if (button && !button->getButtonName().isEmpty())
                    {
                        xml->writeStartElement("sensorbuttonname");
                        xml->writeAttribute("type", QString::number(sensor->getType()));
                        xml->writeAttribute("button", QString::number(button->getRealJoyNumber()));
                        xml->writeCharacters(button->getButtonName());
                        xml->writeEndElement();
                    }
                }
            }
        }

        // write Hats
        QListIterator<JoyDPad *> currJoyDPad(m_inputDevice->getActiveSetJoystick()->getHats().values());

        while (currJoyDPad.hasNext())
        {
            JoyDPad *dpad = currJoyDPad.next();

            if (dpad != nullptr)
            {
                if (!dpad->getDpadName().isEmpty())
                {
                    xml->writeStartElement("dpadname");
                    xml->writeAttribute("index", QString::number(dpad->getRealJoyNumber()));
                    xml->writeCharacters(dpad->getDpadName());
                    xml->writeEndElement();
                }

                // write buttons of each Hat
                QHash<int, JoyDPadButton *> *temp = dpad->getButtons();
                QHashIterator<int, JoyDPadButton *> iter(*temp);

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

        // write VDPads
        QListIterator<VDPad *> currVDPad(m_inputDevice->getActiveSetJoystick()->getVdpads().values());

        while (currVDPad.hasNext())
        {
            VDPad *vdpad = currVDPad.next();

            if (vdpad != nullptr)
            {
                if (!vdpad->getDpadName().isEmpty())
                {
                    xml->writeStartElement("vdpadname");
                    xml->writeAttribute("index", QString::number(vdpad->getRealJoyNumber()));
                    xml->writeCharacters(vdpad->getDpadName());
                    xml->writeEndElement();
                }

                // write buttons of each VDPad
                QHash<int, JoyDPadButton *> *temp = vdpad->getButtons();
                QHashIterator<int, JoyDPadButton *> iter(*temp);

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

    if ((m_inputDevice->getDeviceKeyPressTime() > 0) &&
        (m_inputDevice->getDeviceKeyPressTime() != GlobalVariables::InputDevice::DEFAULTKEYPRESSTIME))
        xml->writeTextElement("keyPressTime", QString::number(m_inputDevice->getDeviceKeyPressTime()));

    m_inputDevice->getCalibrationBackend()->writeConfig(xml);

    xml->writeStartElement("sets");

    for (const auto &joyset : m_inputDevice->getJoystick_sets())
    {
        if (!joyset->isSetEmpty())
            joyset->writeConfig(xml);
        else
            qDebug() << "Set is empty";
    }

    xml->writeEndElement();
    xml->writeEndElement();
}
