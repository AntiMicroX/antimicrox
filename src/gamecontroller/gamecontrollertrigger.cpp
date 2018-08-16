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

//#include <QDebug>

#include "gamecontrollertrigger.h"

#include "messagehandler.h"
#include "gamecontrollertriggerbutton.h"

#include <SDL2/SDL_gamecontroller.h>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>


const int GameControllerTrigger::AXISDEADZONE = 2000;
const int GameControllerTrigger::AXISMAXZONE = 32000;
const GameControllerTrigger::ThrottleTypes GameControllerTrigger::DEFAULTTHROTTLE = GameControllerTrigger::PositiveHalfThrottle;

const QString GameControllerTrigger::xmlName = "trigger";

GameControllerTrigger::GameControllerTrigger(int index, int originset, SetJoystick *parentSet, QObject *parent) :
    JoyAxis(index, originset, parentSet, parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    naxisbutton = new GameControllerTriggerButton(this, 0, originset, parentSet, this);
    paxisbutton = new GameControllerTriggerButton(this, 1, originset, parentSet, this);
    reset(index);
}

QString GameControllerTrigger::getXmlName()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return this->xmlName;
}

QString GameControllerTrigger::getPartialName(bool forceFullFormat, bool displayNames)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString label = QString();

    if (!axisName.isEmpty() && displayNames)
    {
        label.append(axisName);

        if (forceFullFormat)
        {
            label.append(" ").append(trUtf8("Trigger"));
        }
    }
    else if (!defaultAxisName.isEmpty())
    {
        label.append(defaultAxisName);

        if (forceFullFormat)
        {
            label.append(" ").append(trUtf8("Trigger"));
        }
    }
    else
    {
        label.append(trUtf8("Trigger")).append(" ");
        label.append(QString::number(getRealJoyIndex() - SDL_CONTROLLER_AXIS_TRIGGERLEFT));
    }

    return label;
}

void GameControllerTrigger::readJoystickConfig(QXmlStreamReader *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (xml->isStartElement() && (xml->name() == JoyAxis::xmlName))
    {
        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != JoyAxis::xmlName)))
        {
            bool found = readMainConfig(xml);
            if (!found && (xml->name() == JoyAxisButton::xmlName) && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();

                #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << "Index for axis in readJoystickConfig is: " << index;
                #endif

                if (index == 1)
                {
                    found = true;
                    GameControllerTriggerButton *triggerButton =
                            qobject_cast<GameControllerTriggerButton*>(naxisbutton); // static_cast
                    triggerButton->readJoystickConfig(xml);
                }
                else if (index == 2)
                {
                    found = true;
                    GameControllerTriggerButton *triggerButton =
                            qobject_cast<GameControllerTriggerButton*>(paxisbutton); // static_cast
                    triggerButton->readJoystickConfig(xml);
                }
            }

            if (!found)
            {
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }
    }

    if (this->throttle != static_cast<int>(PositiveHalfThrottle))
    {
        this->setThrottle(static_cast<int>(PositiveHalfThrottle));

        setCurrentRawValue(currentThrottledDeadValue);
        currentThrottledValue = calculateThrottledValue(currentRawValue);
    }
}

void GameControllerTrigger::correctJoystickThrottle()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (this->throttle != static_cast<int>(PositiveHalfThrottle))
    {
        this->setThrottle(static_cast<int>(PositiveHalfThrottle));

        setCurrentRawValue(currentThrottledDeadValue);
        currentThrottledValue = calculateThrottledValue(currentRawValue);
    }
}

void GameControllerTrigger::writeConfig(QXmlStreamWriter *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool currentlyDefault = isDefault();

    xml->writeStartElement(getXmlName());
    xml->writeAttribute("index", QString::number((m_index+1)-SDL_CONTROLLER_AXIS_TRIGGERLEFT));

    if (!currentlyDefault)
    {
        if (deadZone != AXISDEADZONE)
        {
            xml->writeTextElement("deadZone", QString::number(deadZone));
        }

        if (maxZoneValue != AXISMAXZONE)
        {
            xml->writeTextElement("maxZone", QString::number(maxZoneValue));
        }
    }

        xml->writeStartElement("throttle");

        if (throttle == static_cast<int>(JoyAxis::NegativeHalfThrottle))
        {
            xml->writeCharacters("negativehalf");
        }
        else if (throttle == static_cast<int>(JoyAxis::NegativeThrottle))
        {
            xml->writeCharacters("negative");
        }
        else if (throttle == static_cast<int>(JoyAxis::NormalThrottle))
        {
            xml->writeCharacters("normal");
        }
        else if (throttle == static_cast<int>(JoyAxis::PositiveThrottle))
        {
            xml->writeCharacters("positive");
        }
        else if (throttle == static_cast<int>(JoyAxis::PositiveHalfThrottle))
        {
            xml->writeCharacters("positivehalf");
        }

        xml->writeEndElement();

    if (!currentlyDefault)
    {
        naxisbutton->writeConfig(xml);
        paxisbutton->writeConfig(xml);
    }


    xml->writeEndElement();
}

int GameControllerTrigger::getDefaultDeadZone()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return this->AXISDEADZONE;
}

int GameControllerTrigger::getDefaultMaxZone()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return this->AXISMAXZONE;
}

JoyAxis::ThrottleTypes GameControllerTrigger::getDefaultThrottle()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return static_cast<ThrottleTypes>(this->DEFAULTTHROTTLE);
}
