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

#include "globalvariables.h"
#include "gamecontrollerdpad.h"
#include "gamecontrollerset.h"
#include "antimicrosettings.h"
#include "joycontrolstick.h"
#include "joybuttontypes/joycontrolstickbutton.h"
#include "common.h"
#include "messagehandler.h"
//#include "logger.h"

#include <cmath>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>


GameController::GameController(SDL_GameController *controller, int deviceIndex,
                               AntiMicroSettings *settings, QObject *parent) :
    InputDevice(deviceIndex, settings, parent)
{

    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->controller = controller;

    SDL_Joystick *joyhandle = SDL_GameControllerGetJoystick(controller);
    joystickID = SDL_JoystickInstanceID(joyhandle);

    for (int i = 0; i < GlobalVariables::InputDevice::NUMBER_JOYSETS; i++)
    {
        GameControllerSet *controllerset = new GameControllerSet(this, i, this);
        getJoystick_sets().insert(i, controllerset);
        enableSetConnections(controllerset);
    }
}


QString GameController::getName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return QString(trUtf8("Game Controller")).append(" ").append(QString::number(getRealJoyNumber()));
}


QString GameController::getSDLName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();

    if (controller != nullptr)
    {
        temp = SDL_GameControllerName(controller);
    }

    return temp;
}


QString GameController::getGUIDString()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getRawGUIDString();
}


QString GameController::getRawGUIDString()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return GlobalVariables::GameController::xmlName;
}


void GameController::closeSDLDevice()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if ((controller != nullptr) && SDL_GameControllerGetAttached(controller))
    {
        SDL_GameControllerClose(controller);
        controller = nullptr;
    }
}


int GameController::getNumberRawButtons()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return SDL_CONTROLLER_BUTTON_MAX;
}


int GameController::getNumberRawAxes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    #ifndef QT_DEBUG_NO_OUTPUT
    qDebug() << "Controller has " << SDL_CONTROLLER_AXIS_MAX << " raw axes";
    #endif

    return SDL_CONTROLLER_AXIS_MAX;
}


int GameController::getNumberRawHats()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return 0;
}


void GameController::readJoystickConfig(QXmlStreamReader *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int index = 0;
    int buttonIndex = 0;
    QString temp = QString();

    if (xml->isStartElement() && (xml->name() == "joystick"))
    {
        transferReset();

        QHash<int, SDL_GameControllerButton> buttons;
        QHash<int, SDL_GameControllerAxis> axes;
        QList<SDL_GameControllerButtonBind> hatButtons;

        fillContainers(buttons, axes, hatButtons);

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

                        if ((index >= 0) && (index < getJoystick_sets().size()))
                        {
                            GameControllerSet *currentSet = qobject_cast<GameControllerSet*>(getJoystick_sets().value(index)); // static_cast
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
                                setButtonName(current, temp);
                            }
                        }
                    }
                    else if ((xml->name() == "axisbuttonname") && xml->isStartElement())
                    {
                        assignVariables(xml, index, buttonIndex, temp, true);


                        if ((index >= 0) && !temp.isEmpty())
                        {
                            SDL_GameControllerAxis current = axes.value(index);
                            int currentInt = static_cast<int>(current);

                            switch (currentInt)
                            {
                                case SDL_CONTROLLER_AXIS_LEFTX:
                                case SDL_CONTROLLER_AXIS_LEFTY:
                                {
                                    setStickButtonName(0, buttonIndex, temp);
                                    break;
                                }
                                case SDL_CONTROLLER_AXIS_RIGHTX:
                                case SDL_CONTROLLER_AXIS_RIGHTY:
                                {
                                    setStickButtonName(1, buttonIndex, temp);
                                    break;
                                }
                                case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
                                case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
                                {
                                    setAxisName(current, temp);
                                }
                            }
                        }
                    }
                    else if ((xml->name() == "controlstickbuttonname") && xml->isStartElement())
                    {
                        assignVariables(xml, index, buttonIndex, temp, false);


                        if ((index >= 0) && !temp.isEmpty())
                        {
                            setStickButtonName(index, buttonIndex, temp);
                        }
                    }
                    else if ((xml->name() == "dpadbuttonname") && xml->isStartElement())
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
                                VDPad *dpad = getActiveSetJoystick()->getVDPad(0);

                                if (dpad != nullptr)
                                {
                                    JoyDPadButton *dpadbutton = dpad->getJoyButton(buttonIndex);

                                    if ((dpad != nullptr) && (dpadbutton != nullptr) && dpadbutton->getActionName().isEmpty())
                                    {
                                        setVDPadButtonName(index, buttonIndex, temp);
                                    }
                                }
                            }
                        }
                    }
                    else if ((xml->name() == "vdpadbuttonname") && xml->isStartElement())
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
                                VDPad *dpad = getActiveSetJoystick()->getVDPad(0);

                                if (dpad != nullptr)
                                {
                                    JoyDPadButton *dpadbutton = dpad->getJoyButton(buttonIndex);

                                    if ((dpad != nullptr) && (dpadbutton != nullptr) && dpadbutton->getActionName().isEmpty())
                                    {
                                        setVDPadButtonName(index, buttonIndex, temp);
                                    }
                                }
                            }
                        }
                    }
                    else if ((xml->name() == "axisname") && xml->isStartElement())
                    {
                        assignVariablesShort(xml, index, temp);


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
                        assignVariablesShort(xml, index, temp);


                        if ((index >= 0) && !temp.isEmpty())
                        {
                            setStickName(index, temp);
                        }
                    }
                    else if ((xml->name() == "dpadname") && xml->isStartElement())
                    {
                        readJoystickConfigXmlLong(hatButtons, dpadNameExists, vdpadNameExists, xml);
                    }
                    else if ((xml->name() == "vdpadname") && xml->isStartElement())
                    {
                        readJoystickConfigXmlLong(hatButtons, dpadNameExists, vdpadNameExists, xml);
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

                if (tempchoice >= 10) this->setDeviceKeyPressTime(tempchoice);

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


inline void GameController::assignVariables(QXmlStreamReader *xml, int& index, int& buttonIndex, QString& temp, bool buttonDecreased)
{
    index = xml->attributes().value("index").toString().toInt();
    buttonIndex = xml->attributes().value("button").toString().toInt();
    temp = xml->readElementText();
    index = index - 1;

    if (buttonDecreased) buttonIndex = buttonIndex - 1;
}


inline void GameController::assignVariablesShort(QXmlStreamReader *xml, int& index, QString& temp)
{
    index = xml->attributes().value("index").toString().toInt();
    temp = xml->readElementText();
    index = index - 1;
}


void GameController::fillContainers(QHash<int, SDL_GameControllerButton> &buttons, QHash<int, SDL_GameControllerAxis> &axes, QList<SDL_GameControllerButtonBind> &hatButtons)
{

    for (int i = 0; i < SDL_JoystickNumHats(getJoyHandle()); i++)
    {
        SDL_GameControllerButton currentButton = static_cast<SDL_GameControllerButton>(i);
        SDL_GameControllerButtonBind bound = SDL_GameControllerGetBindForButton(this->controller, currentButton);

        #ifndef QT_DEBUG_NO_OUTPUT
        qDebug() << "Hat " << (i + 1);
        #endif

        if (bound.bindType == SDL_CONTROLLER_BINDTYPE_HAT)
        {
            hatButtons.append(bound);
        }
    }

    for (int i = 0; i < SDL_JoystickNumButtons(getJoyHandle()); i++)
    {
        #ifndef QT_DEBUG_NO_OUTPUT
        qDebug() << "Button " << (i + 1);
        #endif

        SDL_GameControllerButton currentButton = static_cast<SDL_GameControllerButton>(i);
        SDL_GameControllerButtonBind bound = SDL_GameControllerGetBindForButton(this->controller, currentButton);

        if (bound.bindType == SDL_CONTROLLER_BINDTYPE_BUTTON)
        {
            buttons.insert(bound.value.button, currentButton);
        }
    }

    for (int i = 0; i < SDL_JoystickNumAxes(getJoyHandle()); i++)
    {
        #ifndef QT_DEBUG_NO_OUTPUT
        qDebug() << "Axis " << (i + 1);
        #endif

        SDL_GameControllerAxis currentAxis = static_cast<SDL_GameControllerAxis>(i);
        SDL_GameControllerButtonBind bound = SDL_GameControllerGetBindForAxis(this->controller, currentAxis);

        if (bound.bindType == SDL_CONTROLLER_BINDTYPE_AXIS)
        {
            axes.insert(bound.value.axis, currentAxis);
        }
    }
}


void GameController::readJoystickConfigXmlLong(QList<SDL_GameControllerButtonBind>& hatButtons, bool& dpadNameExists, bool& vdpadNameExists, QXmlStreamReader *xml)
{
    int index = -1;
    bool first = false;
    bool second = false;
    QString temp = QString();

    assignVariablesShort(xml, index, temp);


    if (xml->name() == "vdpadname")
    {
        first = dpadNameExists;
        second = vdpadNameExists;
    }
    else if (xml->name() == "dpadname")
    {
        first = vdpadNameExists;
        second = dpadNameExists;
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
            second = true;
            VDPad *dpad = getActiveSetJoystick()->getVDPad(0);

            if (dpad != nullptr)
            {
                if (dpad->getDpadName().isEmpty())
                    setVDPadName(index, temp);
            }
        }
    }
}


void GameController::readConfig(QXmlStreamReader *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);


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

                        if ((index >= 0) && (index < getJoystick_sets().size()))
                        {
                            getJoystick_sets().value(index)->readConfig(xml);
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
                        readXmlNamesShort("buttonname", xml);
                    }
                    else if ((xml->name() == "triggerbuttonname") && xml->isStartElement())
                    {
                        readXmlNamesLong("triggerbuttonname", xml);
                    }
                    else if ((xml->name() == "controlstickbuttonname") && xml->isStartElement())
                    {
                        readXmlNamesMiddle("controlstickbuttonname", xml);
                    }
                    else if ((xml->name() == "dpadbuttonname") && xml->isStartElement())
                    {
                        readXmlNamesMiddle("dpadbuttonname", xml);
                    }
                    else if ((xml->name() == "triggername") && xml->isStartElement())
                    {
                        readXmlNamesLong("triggername", xml);
                    }
                    else if ((xml->name() == "controlstickname") && xml->isStartElement())
                    {
                        readXmlNamesShort("controlstickname", xml);
                    }
                    else if ((xml->name() == "dpadname") && xml->isStartElement())
                    {
                        readXmlNamesShort("dpadname", xml);
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


void GameController::readXmlNamesShort(QString name, QXmlStreamReader *xml)
{
    int index = -1;
    QString temp = QString();

    assignVariablesShort(xml, index, temp);

    if ((index >= 0) && !temp.isEmpty())
    {
        if (name == "buttonname") setButtonName(index, temp);
        else if (name == "controlstickname") setStickName(index, temp);
        else if (name == "dpadname") setVDPadName(index, temp);
    }
}


void GameController::readXmlNamesMiddle(QString name, QXmlStreamReader *xml)
{
    int index = -1;
    int buttonIndex = -1;
    QString temp = QString();

    assignVariables(xml, index, buttonIndex, temp, false);

    if ((name == "dpadbuttonname") && (index >= 0) && !temp.isEmpty()) setVDPadButtonName(index, buttonIndex, temp);
    else if ((name == "controlstickbuttonname") && (index >= 0) && !temp.isEmpty()) setStickButtonName(index, buttonIndex, temp);
}


void GameController::readXmlNamesLong(QString name, QXmlStreamReader *xml)
{
    int index = xml->attributes().value("index").toString().toInt();
    QString temp = xml->readElementText();
    index = (index - 1) + SDL_CONTROLLER_AXIS_TRIGGERLEFT;

    if ((index == SDL_CONTROLLER_AXIS_TRIGGERLEFT ||
         index == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) && !temp.isEmpty())
    {
        if (name == "triggername")
        {
            setAxisName(index, temp);
        }
        else if (name == "triggerbuttonname")
        {
            int buttonIndex = xml->attributes().value("button").toString().toInt();
            buttonIndex = buttonIndex - 1;
            setAxisButtonName(index, buttonIndex, temp);
        }
    }
}


void GameController::writeConfig(QXmlStreamWriter *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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

    writeXmlForButtons(tempSet, xml);
    writeXmlForAxes(tempSet, xml);
    writeXmlForSticks(tempSet, xml);
    writeXmlForVDpad(xml);

    xml->writeEndElement(); // </names>

    if ((keyPressTime > 0) && (keyPressTime != GlobalVariables::InputDevice::DEFAULTKEYPRESSTIME))
    {
        xml->writeTextElement("keyPressTime", QString::number(keyPressTime));
    }

    xml->writeStartElement("sets");

    for (int i = 0; i < getJoystick_sets().size(); i++)
    {
        getJoystick_sets().value(i)->writeConfig(xml);
    }

    xml->writeEndElement();

    xml->writeEndElement();
}


void GameController::writeXmlForButtons(SetJoystick *tempSet, QXmlStreamWriter *xml)
{
    for (int i = 0; i < getNumberButtons(); i++)
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
}


void GameController::writeXmlForAxes(SetJoystick *tempSet, QXmlStreamWriter *xml)
{
    for (int i = 0; i < getNumberAxes(); i++)
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

            writeXmlAxBtn(axis, axis->getNAxisButton(), xml);
            writeXmlAxBtn(axis, axis->getPAxisButton(), xml);

        }
    }
}


void GameController::writeXmlAxBtn(JoyAxis *axis, JoyAxisButton *axisbutton, QXmlStreamWriter *xml)
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


void GameController::writeXmlForSticks(SetJoystick *tempSet, QXmlStreamWriter *xml)
{
    for (int i = 0; i < getNumberSticks(); i++)
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

                if ((button != nullptr) && !button->getButtonName().isEmpty())
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
}


void GameController::writeXmlForVDpad(QXmlStreamWriter *xml)
{
    for (int i = 0; i < getNumberVDPads(); i++)
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
}


QString GameController::getBindStringForAxis(int index, bool)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();

    SDL_GameControllerButtonBind bind =
            SDL_GameControllerGetBindForAxis(controller,
                                             static_cast<SDL_GameControllerAxis>(index));


    if (bind.bindType == SDL_CONTROLLER_BINDTYPE_BUTTON)
    {
        temp.append(QString("Button %1").arg(bind.value.button));
    }
    else if (bind.bindType == SDL_CONTROLLER_BINDTYPE_AXIS)
    {
        temp.append(QString("Axis %1").arg(bind.value.axis + 1));
    }

    return temp;
}


QString GameController::getBindStringForButton(int index, bool trueIndex)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();

    SDL_GameControllerButtonBind bind =
            SDL_GameControllerGetBindForButton(controller,
                                               static_cast<SDL_GameControllerButton>(index));


    int offset = trueIndex ? 0 : 1;
    int bindInt = static_cast<int>(bind.bindType);

    switch(bindInt) {

            case SDL_CONTROLLER_BINDTYPE_BUTTON:
                temp.append(QString("Button %1").arg(bind.value.button + offset));
            break;


            case SDL_CONTROLLER_BINDTYPE_AXIS:
                temp.append(QString("Axis %1").arg(bind.value.axis + offset));
            break;


            case SDL_CONTROLLER_BINDTYPE_HAT:
                temp.append(QString("Hat %1.%2").arg(bind.value.hat.hat + offset)
                    .arg(bind.value.hat.hat_mask));
            break;
    }

    return temp;
}


SDL_GameControllerButtonBind GameController::getBindForAxis(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return SDL_GameControllerGetBindForAxis(controller, static_cast<SDL_GameControllerAxis>(index));
}


SDL_GameControllerButtonBind GameController::getBindForButton(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return SDL_GameControllerGetBindForButton(controller, static_cast<SDL_GameControllerButton>(index));
}


void GameController::buttonClickEvent(int)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);
}


void GameController::buttonReleaseEvent(int)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);
}


void GameController::axisActivatedEvent(int, int, int)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);
}


SDL_JoystickID GameController::getSDLJoystickID()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return joystickID;
}

/**
 * @brief Check if device is using the SDL Game Controller API
 * @return Status showing if device is using the Game Controller API
 */
bool GameController::isGameController()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return InputDevice::isRelevantGUID(tempGUID);
}


void GameController::rawButtonEvent(int index, bool pressed)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool knownbutton = getRawbuttons().contains(index);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool knownaxis = getAxisvalues().contains(index);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool knowndpad = getDpadvalues().contains(index);

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


QHash<int, bool> const& GameController::getRawbuttons()  {

    return rawbuttons;
}


QHash<int, int> const& GameController::getAxisvalues()  {

    return axisvalues;
}


QHash<int, int> const& GameController::getDpadvalues()  {

    return dpadvalues;
}


SDL_GameController* GameController::getController() const  {

    return controller;
}
