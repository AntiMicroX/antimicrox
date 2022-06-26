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

#include "gamecontrollerxml.h"

#include "gamecontroller/gamecontroller.h"

#include "common.h"
#include "globalvariables.h"
//#include "logger.h"
#include "gamecontroller/gamecontrollerdpad.h"
#include "gamecontroller/gamecontrollerset.h"
#include "joybuttontypes/joycontrolstickbutton.h"
#include "joybuttontypes/joysensorbutton.h"
#include "joycontrolstick.h"
#include "joysensor.h"

#include <cmath>

#include <QDebug>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

GameControllerXml::GameControllerXml(GameController *gameController, QObject *parent)
    : InputDeviceXml(gameController, parent)
{
    m_gameController = gameController;
}

/**
 * @brief Reads the XML tree from a "joystick" tag
 * @param xml XML stream at a "joystick" tag
 */
void GameControllerXml::readJoystickConfig(QXmlStreamReader *xml)
{
    int index = 0;
    int buttonIndex = 0;
    QString temp = QString();

    if (xml->isStartElement() && (xml->name() == "joystick"))
    {
        m_gameController->transferReset();

        QHash<int, SDL_GameControllerButton> buttons;
        QHash<int, SDL_GameControllerAxis> axes;
        QList<SDL_GameControllerButtonBind> hatButtons;

        m_gameController->fillContainers(buttons, axes, hatButtons);

        xml->readNextStartElement();

        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != "joystick")))
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

                        if ((index >= 0) && (index < m_gameController->getJoystick_sets().size()))
                        {
                            GameControllerSet *currentSet = qobject_cast<GameControllerSet *>(
                                m_gameController->getJoystick_sets().value(index)); // static_cast
                            currentSet->readJoystickConfig(xml, buttons, axes, hatButtons);
                        }
                    } else
                    {
                        // If none of the above, skip the element
                        xml->skipCurrentElement();
                    }

                    xml->readNextStartElement();
                }
            } else if ((xml->name() == "names") && xml->isStartElement())
            {
                bool dpadNameExists = false;
                bool vdpadNameExists = false;

                xml->readNextStartElement();

                while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != "names")))
                {
                    if ((xml->name() == "buttonname") && xml->isStartElement())
                    {
                        assignVariablesShort(xml, index, temp);

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            SDL_GameControllerButton current = buttons.value(index);

                            if (static_cast<int>(current) != -1)
                            {
                                m_gameController->setButtonName(current, temp);
                            }
                        }
                    } else if ((xml->name() == "axisbuttonname") && xml->isStartElement())
                    {
                        assignVariables(xml, index, buttonIndex, temp, true);

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            SDL_GameControllerAxis current = axes.value(index);
                            int currentInt = static_cast<int>(current);

                            switch (currentInt)
                            {
                            case SDL_CONTROLLER_AXIS_LEFTX:
                            case SDL_CONTROLLER_AXIS_LEFTY: {
                                m_gameController->setStickButtonName(0, buttonIndex, temp);
                                break;
                            }
                            case SDL_CONTROLLER_AXIS_RIGHTX:
                            case SDL_CONTROLLER_AXIS_RIGHTY: {
                                m_gameController->setStickButtonName(1, buttonIndex, temp);
                                break;
                            }
                            case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
                            case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: {
                                m_gameController->setAxisName(current, temp);
                            }
                            }
                        }
                    } else if ((xml->name() == "controlstickbuttonname") && xml->isStartElement())
                    {
                        assignVariables(xml, index, buttonIndex, temp, false);

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            m_gameController->setStickButtonName(index, buttonIndex, temp);
                        }
                    } else if ((xml->name() == "sensorbuttonname") && xml->isStartElement())
                    {
                        assignVariables(xml, index, buttonIndex, temp, false);

                        if ((index >= 0) && !temp.isEmpty())
                            m_gameController->setSensorButtonName(static_cast<JoySensorType>(index),
                                                                  static_cast<JoySensorDirection>(buttonIndex), temp);
                    } else if ((xml->name() == "dpadbuttonname") && xml->isStartElement())
                    {
                        assignVariables(xml, index, buttonIndex, temp, false);

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            bool found = false;
                            QListIterator<SDL_GameControllerButtonBind> iter(hatButtons);
                            SDL_GameControllerButtonBind current;

                            while (iter.hasNext())
                            {
                                current = iter.next();

                                if (current.value.hat.hat == index)
                                {
                                    found = true;
                                    iter.toBack();
                                }
                            }

                            if (found)
                            {
                                VDPad *dpad = m_gameController->getActiveSetJoystick()->getVDPad(0);

                                if (dpad != nullptr)
                                {
                                    JoyDPadButton *dpadbutton = dpad->getJoyButton(buttonIndex);

                                    if ((dpad != nullptr) && (dpadbutton != nullptr) &&
                                        dpadbutton->getActionName().isEmpty())
                                    {
                                        m_gameController->setVDPadButtonName(index, buttonIndex, temp);
                                    }
                                }
                            }
                        }
                    } else if ((xml->name() == "vdpadbuttonname") && xml->isStartElement())
                    {
                        assignVariables(xml, index, buttonIndex, temp, false);

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            bool found = false;
                            QListIterator<SDL_GameControllerButtonBind> iter(hatButtons);
                            SDL_GameControllerButtonBind current;

                            while (iter.hasNext())
                            {
                                current = iter.next();
                                if (current.value.hat.hat == index)
                                {
                                    found = true;
                                    iter.toBack();
                                }
                            }

                            if (found)
                            {
                                VDPad *dpad = m_gameController->getActiveSetJoystick()->getVDPad(0);

                                if (dpad != nullptr)
                                {
                                    JoyDPadButton *dpadbutton = dpad->getJoyButton(buttonIndex);

                                    if ((dpad != nullptr) && (dpadbutton != nullptr) &&
                                        dpadbutton->getActionName().isEmpty())
                                    {
                                        m_gameController->setVDPadButtonName(index, buttonIndex, temp);
                                    }
                                }
                            }
                        }
                    } else if ((xml->name() == "axisname") && xml->isStartElement())
                    {
                        assignVariablesShort(xml, index, temp);

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            if (axes.contains(index))
                            {
                                SDL_GameControllerAxis current = axes.value(index);
                                m_gameController->setAxisName(static_cast<int>(current), temp);
                            }
                        }
                    } else if ((xml->name() == "controlstickname") && xml->isStartElement())
                    {
                        assignVariablesShort(xml, index, temp);

                        if ((index >= 0) && !temp.isEmpty())
                        {
                            m_gameController->setStickName(index, temp);
                        }
                    } else if ((xml->name() == "sensorname") && xml->isStartElement())
                    {
                        assignVariablesShort(xml, index, temp);

                        if ((index >= 0) && !temp.isEmpty())
                            m_gameController->setSensorName(static_cast<JoySensorType>(index), temp);
                    } else if ((xml->name() == "dpadname") && xml->isStartElement())
                    {
                        readJoystickConfigXmlLong(hatButtons, dpadNameExists, vdpadNameExists, xml);
                    } else if ((xml->name() == "vdpadname") && xml->isStartElement())
                    {
                        readJoystickConfigXmlLong(hatButtons, dpadNameExists, vdpadNameExists, xml);
                    } else
                    {
                        // If none of the above, skip the element
                        xml->skipCurrentElement();
                    }

                    xml->readNextStartElement();
                }
            } else if ((xml->name() == "keyPressTime") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();

                if (tempchoice >= 10)
                    m_gameController->setDeviceKeyPressTime(tempchoice);

            } else if ((xml->name() == "profilename") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                m_gameController->setProfileName(temptext);
            } else
            {
                // If none of the above, skip the element
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }

        m_gameController->reInitButtons();
    }
}

/**
 * @brief Deserializes the given XML stream into a GameController object
 * @param[in] xml The XML stream to read from
 */
void GameControllerXml::readConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && (xml->name() == m_gameController->getXmlName()))
    {
        m_gameController->transferReset();

        xml->readNextStartElement();

        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != m_gameController->getXmlName())))
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

                        if ((index >= 0) && (index < m_gameController->getJoystick_sets().size()))
                        {
                            m_gameController->getJoystick_sets().value(index)->readConfig(xml);
                        }
                    } else
                    {
                        // If none of the above, skip the element
                        xml->skipCurrentElement();
                    }

                    xml->readNextStartElement();
                }
            } else if ((xml->name() == "names") && xml->isStartElement())
            {
                xml->readNextStartElement();

                while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != "names")))
                {
                    if ((xml->name() == "buttonname") && xml->isStartElement())
                    {
                        readXmlNamesShort("buttonname", xml);
                    } else if ((xml->name() == "triggerbuttonname") && xml->isStartElement())
                    {
                        readXmlNamesLong("triggerbuttonname", xml);
                    } else if ((xml->name() == "controlstickbuttonname") && xml->isStartElement())
                    {
                        readXmlNamesMiddle("controlstickbuttonname", xml);
                    } else if ((xml->name() == "sensorbuttonname") && xml->isStartElement())
                    {
                        readXmlNamesMiddle("sensorbuttonname", xml);
                    } else if ((xml->name() == "dpadbuttonname") && xml->isStartElement())
                    {
                        readXmlNamesMiddle("dpadbuttonname", xml);
                    } else if ((xml->name() == "triggername") && xml->isStartElement())
                    {
                        readXmlNamesLong("triggername", xml);
                    } else if ((xml->name() == "controlstickname") && xml->isStartElement())
                    {
                        readXmlNamesShort("controlstickname", xml);
                    } else if ((xml->name() == "sensorname") && xml->isStartElement())
                    {
                        readXmlNamesShort("sensorname", xml);
                    } else if ((xml->name() == "dpadname") && xml->isStartElement())
                    {
                        readXmlNamesShort("dpadname", xml);
                    } else
                    {
                        // If none of the above, skip the element
                        xml->skipCurrentElement();
                    }

                    xml->readNextStartElement();
                }
            } else if ((xml->name() == "keyPressTime") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();

                if (tempchoice >= 10)
                {
                    m_gameController->setDeviceKeyPressTime(tempchoice);
                }
            } else if ((xml->name() == "profilename") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                m_gameController->setProfileName(temptext);
            } else
            {
                // If none of the above, skip the element
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }

        m_gameController->reInitButtons();

    } else if (xml->isStartElement() && (xml->name() == "joystick"))
    {
        this->readJoystickConfig(xml);
    }
}

/**
 * @brief Serializes a GameController object into the the given XML stream
 * @param[in,out] xml The XML stream to write to
 */
void GameControllerXml::writeConfig(QXmlStreamWriter *xml)
{
    xml->writeStartElement(m_gameController->getXmlName());
    xml->writeAttribute("configversion", QString::number(PadderCommon::LATESTCONFIGFILEVERSION));
    xml->writeAttribute("appversion", PadderCommon::programVersion);

    xml->writeComment("The SDL name for a joystick is included for informational purposes only.");
    xml->writeTextElement("sdlname", m_gameController->getSDLName());
    xml->writeComment("The Unique ID for a joystick is included for informational purposes only.");
    xml->writeTextElement("uniqueID", m_gameController->getUniqueIDString());
    //    xml->writeComment("The GUID for a joystick is included for informational purposes only.");
    //    xml->writeTextElement("guid", m_gameController->getGUIDString());

    if (!m_gameController->getProfileName().isEmpty())
    {
        xml->writeTextElement("profilename", m_gameController->getProfileName());
    }

    xml->writeStartElement("names"); // <names>

    SetJoystick *tempSet = m_gameController->getActiveSetJoystick();

    writeXmlForButtons(tempSet, xml);
    writeXmlForAxes(tempSet, xml);
    writeXmlForSticks(tempSet, xml);
    writeXmlForSensors(tempSet, xml);
    writeXmlForVDpad(xml);

    xml->writeEndElement(); // </names>

    if ((m_gameController->getDeviceKeyPressTime() > 0) &&
        (m_gameController->getDeviceKeyPressTime() != GlobalVariables::InputDevice::DEFAULTKEYPRESSTIME))
    {
        xml->writeTextElement("keyPressTime", QString::number(m_gameController->getDeviceKeyPressTime()));
    }

    xml->writeStartElement("sets");

    QHashIterator<int, SetJoystick *> currHash(m_gameController->getJoystick_sets());
    while (currHash.hasNext())
    {
        currHash.next();
        currHash.value()->writeConfig(xml);
    }

    xml->writeEndElement();

    xml->writeEndElement();
}

void GameControllerXml::writeXmlForButtons(SetJoystick *tempSet, QXmlStreamWriter *xml)
{
    QHashIterator<int, JoyButton *> currBtn(tempSet->getButtons());

    while (currBtn.hasNext())
    {
        currBtn.next();

        if ((currBtn.value() != nullptr) && !currBtn.value()->getButtonName().isEmpty())
        {
            xml->writeStartElement("buttonname");
            xml->writeAttribute("index", QString::number(currBtn.value()->getRealJoyNumber()));
            xml->writeCharacters(currBtn.value()->getButtonName());
            xml->writeEndElement();
        }
    }
}

void GameControllerXml::writeXmlForAxes(SetJoystick *tempSet, QXmlStreamWriter *xml)
{
    QHashIterator<int, JoyAxis *> currAxis(*tempSet->getAxes());

    while (currAxis.hasNext())
    {
        currAxis.next();

        if (currAxis.value() != nullptr)
        {
            if (!currAxis.value()->getAxisName().isEmpty())
            {
                xml->writeStartElement("axisname");
                xml->writeAttribute("index", QString::number(currAxis.value()->getRealJoyIndex()));
                xml->writeCharacters(currAxis.value()->getAxisName());
                xml->writeEndElement();
            }

            writeXmlAxBtn(currAxis.value(), currAxis.value()->getNAxisButton(), xml);
            writeXmlAxBtn(currAxis.value(), currAxis.value()->getPAxisButton(), xml);
        }
    }
}

void GameControllerXml::writeXmlAxBtn(JoyAxis *axis, JoyAxisButton *axisbutton, QXmlStreamWriter *xml)
{
    if (!axisbutton->getButtonName().isEmpty())
    {
        xml->writeStartElement("axisbuttonname");
        xml->writeAttribute("index", QString::number(axis->getRealJoyIndex()));
        xml->writeAttribute("button", QString::number(axisbutton->getRealJoyNumber()));
        xml->writeCharacters(axisbutton->getButtonName());
        xml->writeEndElement();
    }
}

void GameControllerXml::writeXmlForSticks(SetJoystick *tempSet, QXmlStreamWriter *xml)
{
    QHashIterator<int, JoyControlStick *> currStick(tempSet->getSticks());

    while (currStick.hasNext())
    {
        currStick.next();

        if (currStick.value() != nullptr)
        {
            if (!currStick.value()->getStickName().isEmpty())
            {
                xml->writeStartElement("controlstickname");
                xml->writeAttribute("index", QString::number(currStick.value()->getRealJoyIndex()));
                xml->writeCharacters(currStick.value()->getStickName());
                xml->writeEndElement();
            }

            QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton *> *buttons = currStick.value()->getButtons();
            QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton *> iter(*buttons);

            while (iter.hasNext())
            {
                JoyControlStickButton *button = iter.next().value();

                if ((button != nullptr) && !button->getButtonName().isEmpty())
                {
                    xml->writeStartElement("controlstickbuttonname");
                    xml->writeAttribute("index", QString::number(currStick.value()->getRealJoyIndex()));
                    xml->writeAttribute("button", QString::number(button->getRealJoyNumber()));
                    xml->writeCharacters(button->getButtonName());
                    xml->writeEndElement();
                }
            }
        }
    }
}

/**
 * @brief Writes all sensor button mappings in the current set to XML
 * @param[in] tempSet To be deserialized SetJoystick
 * @param[in,out] xml XML stream to which is written
 */
void GameControllerXml::writeXmlForSensors(SetJoystick *tempSet, QXmlStreamWriter *xml)
{
    auto sensors = tempSet->getSensors();
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

            auto buttons = sensor->getButtons();
            for (auto iter = buttons->cbegin(); iter != buttons->cend(); ++iter)
            {
                JoySensorButton *button = iter.value();

                if ((button != nullptr) && !button->getButtonName().isEmpty())
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
}

void GameControllerXml::writeXmlForVDpad(QXmlStreamWriter *xml)
{
    QHashIterator<int, VDPad *> currVDPad(m_gameController->getActiveSetJoystick()->getVdpads());

    while (currVDPad.hasNext())
    {
        currVDPad.next();

        if (currVDPad.value() != nullptr)
        {
            if (!currVDPad.value()->getDpadName().isEmpty())
            {
                xml->writeStartElement("dpadname");
                xml->writeAttribute("index", QString::number(currVDPad.value()->getRealJoyNumber()));
                xml->writeCharacters(currVDPad.value()->getDpadName());
                xml->writeEndElement();
            }

            QHash<int, JoyDPadButton *> *temp = currVDPad.value()->getButtons();
            QHashIterator<int, JoyDPadButton *> iter(*temp);

            while (iter.hasNext())
            {
                JoyDPadButton *button = iter.next().value();

                if ((button != nullptr) && !button->getButtonName().isEmpty())
                {
                    xml->writeStartElement("dpadbutton");
                    xml->writeAttribute("index", QString::number(currVDPad.value()->getRealJoyNumber()));
                    xml->writeAttribute("button", QString::number(button->getRealJoyNumber()));
                    xml->writeCharacters(button->getButtonName());
                    xml->writeEndElement();
                }
            }
        }
    }
}

void GameControllerXml::readXmlNamesShort(QString name, QXmlStreamReader *xml)
{
    int index = -1;
    QString temp = QString();

    assignVariablesShort(xml, index, temp);

    if ((index >= 0) && !temp.isEmpty())
    {
        if (name == "buttonname")
            m_gameController->setButtonName(index, temp);
        else if (name == "controlstickname")
            m_gameController->setStickName(index, temp);
        else if (name == "dpadname")
            m_gameController->setVDPadName(index, temp);
    }
}

void GameControllerXml::readXmlNamesMiddle(QString name, QXmlStreamReader *xml)
{
    int index = -1;
    int buttonIndex = -1;
    QString temp = QString();

    assignVariables(xml, index, buttonIndex, temp, false);

    if ((name == "dpadbuttonname") && (index >= 0) && !temp.isEmpty())
        m_gameController->setVDPadButtonName(index, buttonIndex, temp);
    else if ((name == "controlstickbuttonname") && (index >= 0) && !temp.isEmpty())
        m_gameController->setStickButtonName(index, buttonIndex, temp);
}

void GameControllerXml::readXmlNamesLong(QString name, QXmlStreamReader *xml)
{
    int index = xml->attributes().value("index").toString().toInt();
    QString temp = xml->readElementText();
    index = (index - 1) + SDL_CONTROLLER_AXIS_TRIGGERLEFT;

    if ((index == SDL_CONTROLLER_AXIS_TRIGGERLEFT || index == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) && !temp.isEmpty())
    {
        if (name == "triggername")
        {
            m_gameController->setAxisName(index, temp);
        } else if (name == "triggerbuttonname")
        {
            int buttonIndex = xml->attributes().value("button").toString().toInt();
            buttonIndex = buttonIndex - 1;
            m_gameController->setAxisButtonName(index, buttonIndex, temp);
        }
    }
}

void GameControllerXml::readJoystickConfigXmlLong(QList<SDL_GameControllerButtonBind> &hatButtons, bool &dpadNameExists,
                                                  bool &vdpadNameExists, QXmlStreamReader *xml)
{
    int index = -1;
    bool first = false;
    QString temp = QString();

    assignVariablesShort(xml, index, temp);

    if (xml->name() == "vdpadname")
    {
        first = dpadNameExists;
    } else if (xml->name() == "dpadname")
    {
        first = vdpadNameExists;
    }

    if ((index >= 0) && !temp.isEmpty() && !first)
    {
        bool found = false;
        QListIterator<SDL_GameControllerButtonBind> iter(hatButtons);
        SDL_GameControllerButtonBind current;

        while (iter.hasNext())
        {
            current = iter.next();

            if (current.value.hat.hat == index)
            {
                found = true;
                iter.toBack();
            }
        }

        if (found)
        {
            VDPad *dpad = m_gameController->getActiveSetJoystick()->getVDPad(0);

            if (dpad != nullptr)
            {
                if (dpad->getDpadName().isEmpty())
                    m_gameController->setVDPadName(index, temp);
            }
        }
    }
}

/**
 * @brief Reads index, buttonIndex and tag content from xml into C++ variables
 * @param[in] xml XML stream at a button element
 * @param[out] index Value of the "index" XML attribute
 * @param[out] buttonIndex Value of the "button" XML attribute
 * @param[out] temp Content of the XML element
 */
inline void GameControllerXml::assignVariables(QXmlStreamReader *xml, int &index, int &buttonIndex, QString &temp,
                                               bool buttonDecreased)
{
    index = xml->attributes().value("index").toString().toInt();
    buttonIndex = xml->attributes().value("button").toString().toInt();
    temp = xml->readElementText();
    index = index - 1;

    if (buttonDecreased)
        buttonIndex = buttonIndex - 1;
}

inline void GameControllerXml::assignVariablesShort(QXmlStreamReader *xml, int &index, QString &temp)
{
    index = xml->attributes().value("index").toString().toInt();
    temp = xml->readElementText();
    index = index - 1;
}
