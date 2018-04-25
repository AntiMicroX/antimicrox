/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
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

#include "gamecontroller.h"
#include "gamecontrollerdpad.h"
#include "gamecontrollerset.h"
#include "antimicrosettings.h"
#include "joycontrolstick.h"
#include "joybuttontypes/joycontrolstickbutton.h"
#include "common.h"
//#include "logger.h"

#include <cmath>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>



const QString GameController::xmlName = "gamecontroller";

GameController::GameController(SDL_GameController *controller, int deviceIndex,
                               AntiMicroSettings *settings, QObject *parent) :
    InputDevice(deviceIndex, settings, parent)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    this->controller = controller;
    SDL_Joystick *joyhandle = SDL_GameControllerGetJoystick(controller);
    joystickID = SDL_JoystickInstanceID(joyhandle);

    for (int i=0; i < NUMBER_JOYSETS; i++)
    {
        GameControllerSet *controllerset = new GameControllerSet(this, i, this);
        joystick_sets.insert(i, controllerset);
        enableSetConnections(controllerset);
    }
}

QString GameController::getName()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    return QString(trUtf8("Game Controller")).append(" ").append(QString::number(getRealJoyNumber()));
}

QString GameController::getSDLName()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    QString temp = QString();
    if (controller != nullptr)
    {
        temp = SDL_GameControllerName(controller);
    }

    return temp;
}

QString GameController::getGUIDString()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    QString temp = getRawGUIDString();

    return temp;
}

QString GameController::getRawGUIDString()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    QString temp = QString();
    if (controller != nullptr)
    {
        SDL_Joystick *joyhandle = SDL_GameControllerGetJoystick(controller);
        if (joyhandle != nullptr)
        {
            SDL_JoystickGUID tempGUID = SDL_JoystickGetGUID(joyhandle);
            char guidString[65] = {'0'};
            SDL_JoystickGetGUIDString(tempGUID, guidString, sizeof(guidString));
            temp = QString(guidString);
        }
    }

    return temp;
}

QString GameController::getXmlName()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    return this->xmlName;
}

void GameController::closeSDLDevice()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    if ((controller != nullptr) && SDL_GameControllerGetAttached(controller))
    {
        SDL_GameControllerClose(controller);
        controller = nullptr;
    }
}

int GameController::getNumberRawButtons()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    return SDL_CONTROLLER_BUTTON_MAX;
}

int GameController::getNumberRawAxes()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    qDebug() << "Controller has " << SDL_CONTROLLER_AXIS_MAX << " raw axes";
    return SDL_CONTROLLER_AXIS_MAX;
}

int GameController::getNumberRawHats()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    return 0;
}

void GameController::readJoystickConfig(QXmlStreamReader *xml)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    if (xml->isStartElement() && (xml->name() == "joystick"))
    {
        transferReset();

        QHash<int, SDL_GameControllerButton> buttons;
        QHash<int, SDL_GameControllerAxis> axes;
        QList<SDL_GameControllerButtonBind> hatButtons;

        for (int i = 0; i < SDL_JoystickNumHats(this->joyhandle); i++)
        {

            SDL_GameControllerButton currentButton = static_cast<SDL_GameControllerButton>(i);
            SDL_GameControllerButtonBind bound = SDL_GameControllerGetBindForButton(this->controller, currentButton);
            qDebug() << "Hat " << (i + 1);
            if (bound.bindType == SDL_CONTROLLER_BINDTYPE_HAT)
            {
                hatButtons.append(bound);
            }
        }

        for (int i = 0; i < SDL_JoystickNumButtons(this->joyhandle); i++)
        {
            qDebug() << "Button " << (i + 1);
            SDL_GameControllerButton currentButton = static_cast<SDL_GameControllerButton>(i);
            SDL_GameControllerButtonBind bound = SDL_GameControllerGetBindForButton(this->controller, currentButton);
            if (bound.bindType == SDL_CONTROLLER_BINDTYPE_BUTTON)
            {
                buttons.insert(bound.value.button, currentButton);
            }
        }

        for (int i = 0; i < SDL_JoystickNumAxes(this->joyhandle); i++)
        {
            qDebug() << "Axis " << (i + 1);
            SDL_GameControllerAxis currentAxis = static_cast<SDL_GameControllerAxis>(i);
            SDL_GameControllerButtonBind bound = SDL_GameControllerGetBindForAxis(this->controller, currentAxis);
            if (bound.bindType == SDL_CONTROLLER_BINDTYPE_AXIS)
            {
                axes.insert(bound.value.axis, currentAxis);
            }
        }

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
                        if ((index >= 0) && (index < joystick_sets.size()))
                        {
                            GameControllerSet *currentSet = qobject_cast<GameControllerSet*>(joystick_sets.value(index)); // static_cast
                            currentSet->readJoystickConfig(xml, buttons, axes, hatButtons);
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
            else if ((xml->name() == "names") && xml->isStartElement())
            {
                bool dpadNameExists = false;
                bool vdpadNameExists = false;

                xml->readNextStartElement();
                while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != "names"))
                {
                    if ((xml->name() == "buttonname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if ((index >= 0) && !temp.isEmpty())
                        {
                            SDL_GameControllerButton current = buttons.value(index);
                            if (current)
                            {
                                setButtonName(current, temp);
                            }
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
                            SDL_GameControllerAxis current = axes.value(index);
                            if (current)
                            {
                                if (current == SDL_CONTROLLER_AXIS_LEFTX)
                                {
                                    setStickButtonName(0, buttonIndex, temp);
                                }
                                else if (current == SDL_CONTROLLER_AXIS_LEFTY)
                                {
                                    setStickButtonName(0, buttonIndex, temp);
                                }
                                else if (current == SDL_CONTROLLER_AXIS_RIGHTX)
                                {
                                    setStickButtonName(1, buttonIndex, temp);
                                }
                                else if (current == SDL_CONTROLLER_AXIS_RIGHTY)
                                {
                                    setStickButtonName(1, buttonIndex, temp);
                                }
                                else if (current == SDL_CONTROLLER_AXIS_TRIGGERLEFT)
                                {
                                    setAxisName(current, temp);
                                }
                                else if (current == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
                                {
                                    setAxisName(current, temp);
                                }
                            }
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
                            setStickButtonName(index, buttonIndex, temp);
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
                                VDPad *dpad = getActiveSetJoystick()->getVDPad(0);
                                if (dpad != nullptr)
                                {
                                    JoyDPadButton *dpadbutton = dpad->getJoyButton(buttonIndex);
                                    if ((dpad != nullptr) && dpadbutton->getActionName().isEmpty())
                                    {
                                        setVDPadButtonName(index, buttonIndex, temp);
                                    }
                                }
                            }
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
                                VDPad *dpad = getActiveSetJoystick()->getVDPad(0);
                                if (dpad != nullptr)
                                {
                                    JoyDPadButton *dpadbutton = dpad->getJoyButton(buttonIndex);
                                    if ((dpad != nullptr) && dpadbutton->getActionName().isEmpty())
                                    {
                                        setVDPadButtonName(index, buttonIndex, temp);
                                    }
                                }
                            }
                        }
                    }
                    else if ((xml->name() == "axisname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if ((index >= 0) && !temp.isEmpty())
                        {
                            if (axes.contains(index))
                            {
                                SDL_GameControllerAxis current = axes.value(index);
                                setAxisName(static_cast<int>(current), temp);
                            }
                        }
                    }
                    else if ((xml->name() == "controlstickname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if ((index >= 0) && !temp.isEmpty())
                        {
                            setStickName(index, temp);
                        }
                    }
                    else if ((xml->name() == "dpadname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if ((index >= 0) && !temp.isEmpty() && !vdpadNameExists)
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
                                dpadNameExists = true;

                                VDPad *dpad = getActiveSetJoystick()->getVDPad(0);
                                if (dpad != nullptr)
                                {
                                    if (dpad->getDpadName().isEmpty())
                                    {
                                        setVDPadName(index, temp);
                                    }
                                }
                            }
                        }
                    }
                    else if ((xml->name() == "vdpadname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if ((index >= 0) && !temp.isEmpty() && !dpadNameExists)
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
                                vdpadNameExists = true;

                                VDPad *dpad = getActiveSetJoystick()->getVDPad(0);
                                if (dpad != nullptr)
                                {
                                    if (dpad->getDpadName().isEmpty())
                                    {
                                        setVDPadName(index, temp);
                                    }
                                }
                            }
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
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                if (tempchoice >= 10)
                {
                    this->setDeviceKeyPressTime(tempchoice);
                }
            }
            else if ((xml->name() == "profilename") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                this->setProfileName(temptext);
            }
            else
            {
                // If none of the above, skip the element
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }

        reInitButtons();
    }
}


void GameController::readConfig(QXmlStreamReader *xml)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    if (xml->isStartElement() && (xml->name() == getXmlName()))
    {
        transferReset();

        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != getXmlName())))
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
                        if ((index >= 0) && (index < joystick_sets.size()))
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
                            setButtonName(index, temp);
                        }
                    }
                    else if ((xml->name() == "triggerbuttonname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        int buttonIndex = xml->attributes().value("button").toString().toInt();
                        QString temp = xml->readElementText();
                        index = (index - 1) + SDL_CONTROLLER_AXIS_TRIGGERLEFT;
                        buttonIndex = buttonIndex - 1;
                        if (((index == SDL_CONTROLLER_AXIS_TRIGGERLEFT) ||
                             (index == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)) && !temp.isEmpty())
                        {
                            setAxisButtonName(index, buttonIndex, temp);
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
                            setStickButtonName(index, buttonIndex, temp);
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
                            setVDPadButtonName(index, buttonIndex, temp);
                        }
                    }
                    else if ((xml->name() == "triggername") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = (index - 1) + SDL_CONTROLLER_AXIS_TRIGGERLEFT;
                        if ((index == SDL_CONTROLLER_AXIS_TRIGGERLEFT ||
                             index == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) && !temp.isEmpty())
                        {
                            setAxisName(index, temp);
                        }
                    }
                    else if ((xml->name() == "controlstickname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if ((index >= 0) && !temp.isEmpty())
                        {
                            setStickName(index, temp);
                        }
                    }
                    else if ((xml->name() == "dpadname") && xml->isStartElement())
                    {
                        int index = xml->attributes().value("index").toString().toInt();
                        QString temp = xml->readElementText();
                        index = index - 1;
                        if ((index >= 0) && !temp.isEmpty())
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
            else if ((xml->name() == "keyPressTime") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                if (tempchoice >= 10)
                {
                    this->setDeviceKeyPressTime(tempchoice);
                }
            }
            else if ((xml->name() == "profilename") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                this->setProfileName(temptext);
            }
            else
            {
                // If none of the above, skip the element
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }

        reInitButtons();
    }
    else if (xml->isStartElement() && (xml->name() == "joystick"))
    {
        this->readJoystickConfig(xml);
    }
}

void GameController::writeConfig(QXmlStreamWriter *xml)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    xml->writeStartElement(getXmlName());
    xml->writeAttribute("configversion", QString::number(PadderCommon::LATESTCONFIGFILEVERSION));
    xml->writeAttribute("appversion", PadderCommon::programVersion);

    xml->writeComment("The SDL name for a joystick is included for informational purposes only.");
    xml->writeTextElement("sdlname", getSDLName());
    xml->writeComment("The GUID for a joystick is included for informational purposes only.");
    xml->writeTextElement("guid", getGUIDString());

    if (!profileName.isEmpty())
    {
        xml->writeTextElement("profilename", profileName);
    }

    xml->writeStartElement("names"); // <names>

    SetJoystick *tempSet = getActiveSetJoystick();
    for (int i=0; i < getNumberButtons(); i++)
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

    for (int i=0; i < getNumberAxes(); i++)
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

    for (int i=0; i < getNumberSticks(); i++)
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

    for (int i=0; i < getNumberVDPads(); i++)
    {
        VDPad *vdpad = getActiveSetJoystick()->getVDPad(i);
        if (vdpad != nullptr)
        {
            if (!vdpad->getDpadName().isEmpty())
            {
                xml->writeStartElement("dpadname");
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
                    xml->writeStartElement("dpadbutton");
                    xml->writeAttribute("index", QString::number(vdpad->getRealJoyNumber()));
                    xml->writeAttribute("button", QString::number(button->getRealJoyNumber()));
                    xml->writeCharacters(button->getButtonName());
                    xml->writeEndElement();
                }
            }
        }
    }
    xml->writeEndElement(); // </names>

    if ((keyPressTime > 0) && (keyPressTime != DEFAULTKEYPRESSTIME))
    {
        xml->writeTextElement("keyPressTime", QString::number(keyPressTime));
    }

    xml->writeStartElement("sets");
    for (int i=0; i < joystick_sets.size(); i++)
    {
        joystick_sets.value(i)->writeConfig(xml);
    }
    xml->writeEndElement();

    xml->writeEndElement();
}

QString GameController::getBindStringForAxis(int index, bool)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    QString temp = QString();
    SDL_GameControllerButtonBind bind =
            SDL_GameControllerGetBindForAxis(controller,
                                             static_cast<SDL_GameControllerAxis>(index));
    if (bind.bindType != SDL_CONTROLLER_BINDTYPE_NONE)
    {

        if (bind.bindType == SDL_CONTROLLER_BINDTYPE_BUTTON)
        {
            temp.append(QString("Button %1").arg(bind.value.button)); // bind.value.button + offset
        }
        else if (bind.bindType == SDL_CONTROLLER_BINDTYPE_AXIS)
        {
            temp.append(QString("Axis %1").arg(bind.value.axis + 1));
        }
    }
    return temp;
}

QString GameController::getBindStringForButton(int index, bool trueIndex)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    QString temp = QString();
    SDL_GameControllerButtonBind bind =
            SDL_GameControllerGetBindForButton(controller,
                                               static_cast<SDL_GameControllerButton>(index));

    if (bind.bindType != SDL_CONTROLLER_BINDTYPE_NONE)
    {
        int offset = trueIndex ? 0 : 1;

        if (bind.bindType == SDL_CONTROLLER_BINDTYPE_BUTTON)
        {
            temp.append(QString("Button %1").arg(bind.value.button + offset));
        }
        else if (bind.bindType == SDL_CONTROLLER_BINDTYPE_AXIS)
        {
            temp.append(QString("Axis %1").arg(bind.value.axis + offset));
        }
        else if (bind.bindType == SDL_CONTROLLER_BINDTYPE_HAT)
        {
            temp.append(QString("Hat %1.%2").arg(bind.value.hat.hat + offset)
                    .arg(bind.value.hat.hat_mask));
        }
    }
    return temp;
}

SDL_GameControllerButtonBind GameController::getBindForAxis(int index)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    SDL_GameControllerButtonBind bind = SDL_GameControllerGetBindForAxis(controller, (SDL_GameControllerAxis)index);
    return bind;
}

SDL_GameControllerButtonBind GameController::getBindForButton(int index)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    SDL_GameControllerButtonBind bind = SDL_GameControllerGetBindForButton(controller, (SDL_GameControllerButton)index);
    return bind;
}

void GameController::buttonClickEvent(int buttonindex)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    SDL_GameControllerButtonBind bind = getBindForButton(buttonindex); // static_cast<SDL_GameControllerButton>
    if (bind.bindType != SDL_CONTROLLER_BINDTYPE_NONE)
    {
        if (bind.bindType == SDL_CONTROLLER_BINDTYPE_AXIS)
        {
            //emit rawAxisButtonClick(bind.value.axis, 0);
            //emit rawAxisActivated(bind.value.axis, JoyAxis::AXISMAX);
        }
        else if (bind.bindType == SDL_CONTROLLER_BINDTYPE_BUTTON)
        {
            //emit rawButtonClick(bind.value.button);
        }
        else if (bind.bindType == SDL_CONTROLLER_BINDTYPE_HAT)
        {
            //emit rawDPadButtonClick(bind.value.hat.hat, bind.value.hat.hat_mask);
        }
    }
}

void GameController::buttonReleaseEvent(int buttonindex)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    SDL_GameControllerButtonBind bind = getBindForButton(buttonindex); // static_cast<SDL_GameControllerButton>
    if (bind.bindType != SDL_CONTROLLER_BINDTYPE_NONE)
    {
        if (bind.bindType == SDL_CONTROLLER_BINDTYPE_AXIS)
        {
            //emit rawAxisButtonRelease(bind.value.axis, 0);
        }
        else if (bind.bindType == SDL_CONTROLLER_BINDTYPE_BUTTON)
        {
            //emit rawButtonRelease(bind.value.button);
        }
        else if (bind.bindType == SDL_CONTROLLER_BINDTYPE_HAT)
        {
            //emit rawDPadButtonRelease(bind.value.hat.hat, bind.value.hat.hat_mask);
        }
    }
}

void GameController::axisActivatedEvent(int setindex, int axisindex, int value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    Q_UNUSED(setindex);

    SDL_GameControllerButtonBind bind = getBindForAxis(axisindex); // static_cast<SDL_GameControllerButton>
    if (bind.bindType != SDL_CONTROLLER_BINDTYPE_NONE)
    {
        if (bind.bindType == SDL_CONTROLLER_BINDTYPE_AXIS)
        {
            //emit rawAxisButtonClick(bind.value.axis, 0);
            //emit rawAxisActivated(bind.value.axis, value);
        }
        else if (bind.bindType == SDL_CONTROLLER_BINDTYPE_BUTTON)
        {
            //emit rawButtonClick(bind.value.button);
        }
        else if (bind.bindType == SDL_CONTROLLER_BINDTYPE_HAT)
        {
            //emit rawDPadButtonClick(bind.value.hat.hat, bind.value.hat.hat_mask);
        }
    }
}

SDL_JoystickID GameController::getSDLJoystickID()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    return joystickID;
}

/**
 * @brief Check if device is using the SDL Game Controller API
 * @return Status showing if device is using the Game Controller API
 */
bool GameController::isGameController()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    return true;
}

/**
 * @brief Check if GUID passed matches the expected GUID for a device.
 *     Needed for xinput GUID abstraction.
 * @param GUID string
 * @return if GUID is considered a match.
 */
bool GameController::isRelevantGUID(QString tempGUID)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    bool result = false;

    if (InputDevice::isRelevantGUID(tempGUID))// || isEmptyGUID(tempGUID))
    {
        result = true;
    }

    return result;
}

void GameController::rawButtonEvent(int index, bool pressed)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    bool knownbutton = rawbuttons.contains(index);
    if (!knownbutton && pressed)
    {
        rawbuttons.insert(index, pressed);
        emit rawButtonClick(index);
    }
    else if (knownbutton && !pressed)
    {
        rawbuttons.remove(index);
        emit rawButtonRelease(index);
    }
}

void GameController::rawAxisEvent(int index, int value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    bool knownaxis = axisvalues.contains(index);

    if (!knownaxis && (fabs(value) > rawAxisDeadZone))
    {
        axisvalues.insert(index, value);
        emit rawAxisActivated(index, value);
    }
    else if (knownaxis && (fabs(value) < rawAxisDeadZone))
    {
        axisvalues.remove(index);
        emit rawAxisReleased(index, value);
    }

    emit rawAxisMoved(index, value);
}

void GameController::rawDPadEvent(int index, int value)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
    bool knowndpad = dpadvalues.contains(index);
    if (!knowndpad && (value != 0))
    {
        dpadvalues.insert(index, value);
        emit rawDPadButtonClick(index, value);
    }
    else if (knowndpad && (value == 0))
    {
        dpadvalues.remove(index);
        emit rawDPadButtonRelease(index, value);
    }
}


QHash<int, bool> GameController::getRawbuttons() const  {

    return rawbuttons;
}

QHash<int, int> GameController::getAxisvalues() const  {

    return axisvalues;
}

QHash<int, int> GameController::getDpadvalues() const  {

    return dpadvalues;
}

SDL_GameController* GameController::getController() const  {

    return controller;
}
