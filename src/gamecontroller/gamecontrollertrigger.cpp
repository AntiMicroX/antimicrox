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

const int GameControllerTrigger::AXISDEADZONE = 2000;
const int GameControllerTrigger::AXISMAXZONE = 32000;
const GameControllerTrigger::ThrottleTypes GameControllerTrigger::DEFAULTTHROTTLE = GameControllerTrigger::PositiveHalfThrottle;

const QString GameControllerTrigger::xmlName = "trigger";

GameControllerTrigger::GameControllerTrigger(int index, int originset, SetJoystick *parentSet, QObject *parent) :
    JoyAxis(index, originset, parentSet, parent)
{
    naxisbutton = new GameControllerTriggerButton(this, 0, originset, parentSet, this);
    paxisbutton = new GameControllerTriggerButton(this, 1, originset, parentSet, this);
    reset(index);
}

QString GameControllerTrigger::getXmlName()
{
    return this->xmlName;
}

QString GameControllerTrigger::getPartialName(bool forceFullFormat, bool displayNames)
{
    QString label;

    if (!axisName.isEmpty() && displayNames)
    {
        label.append(axisName);

        if (forceFullFormat)
        {
            label.append(" ").append(tr("Trigger"));
        }
    }
    else if (!defaultAxisName.isEmpty())
    {
        label.append(defaultAxisName);

        if (forceFullFormat)
        {
            label.append(" ").append(tr("Trigger"));
        }
    }
    else
    {
        label.append(tr("Trigger")).append(" ");
        label.append(QString::number(getRealJoyIndex() - SDL_CONTROLLER_AXIS_TRIGGERLEFT));
    }

    return label;
}

void GameControllerTrigger::readJoystickConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && xml->name() == JoyAxis::xmlName)
    {
        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != JoyAxis::xmlName))
        {
            bool found = readMainConfig(xml);
            if (!found && xml->name() == JoyAxisButton::xmlName && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                if (index == 1)
                {
                    found = true;
                    GameControllerTriggerButton *triggerButton =
                            static_cast<GameControllerTriggerButton*>(naxisbutton);
                    triggerButton->readJoystickConfig(xml);
                }
                else if (index == 2)
                {
                    found = true;
                    GameControllerTriggerButton *triggerButton =
                            static_cast<GameControllerTriggerButton*>(paxisbutton);
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

    if (this->throttle != PositiveHalfThrottle)
    {
        this->setThrottle(PositiveHalfThrottle);

        setCurrentRawValue(currentThrottledDeadValue);
        currentThrottledValue = calculateThrottledValue(currentRawValue);
    }
}

void GameControllerTrigger::correctJoystickThrottle()
{
    if (this->throttle != PositiveHalfThrottle)
    {
        this->setThrottle(PositiveHalfThrottle);

        setCurrentRawValue(currentThrottledDeadValue);
        currentThrottledValue = calculateThrottledValue(currentRawValue);
    }
}

void GameControllerTrigger::writeConfig(QXmlStreamWriter *xml)
{
    bool currentlyDefault = isDefault();

    xml->writeStartElement(getXmlName());
    xml->writeAttribute("index", QString::number((index+1)-SDL_CONTROLLER_AXIS_TRIGGERLEFT));

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

    //if (throttle != DEFAULTTHROTTLE)
    //{
        xml->writeStartElement("throttle");

        if (throttle == JoyAxis::NegativeHalfThrottle)
        {
            xml->writeCharacters("negativehalf");
        }
        else if (throttle == JoyAxis::NegativeThrottle)
        {
            xml->writeCharacters("negative");
        }
        else if (throttle == JoyAxis::NormalThrottle)
        {
            xml->writeCharacters("normal");
        }
        else if (throttle == JoyAxis::PositiveThrottle)
        {
            xml->writeCharacters("positive");
        }
        else if (throttle == JoyAxis::PositiveHalfThrottle)
        {
            xml->writeCharacters("positivehalf");
        }

        xml->writeEndElement();
    //}

    if (!currentlyDefault)
    {
        naxisbutton->writeConfig(xml);
        paxisbutton->writeConfig(xml);
    }


    xml->writeEndElement();
}

int GameControllerTrigger::getDefaultDeadZone()
{
    return this->AXISDEADZONE;
}

int GameControllerTrigger::getDefaultMaxZone()
{
    return this->AXISMAXZONE;
}

JoyAxis::ThrottleTypes GameControllerTrigger::getDefaultThrottle()
{
    return (ThrottleTypes)this->DEFAULTTHROTTLE;
}
