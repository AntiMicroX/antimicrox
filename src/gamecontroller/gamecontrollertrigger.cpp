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

void GameControllerTrigger::writeConfig(QXmlStreamWriter *xml)
{
    if (!isDefault())
    {
        xml->writeStartElement(getXmlName());
        xml->writeAttribute("index", QString::number((index+1)-SDL_CONTROLLER_AXIS_TRIGGERLEFT));

        xml->writeTextElement("deadZone", QString::number(deadZone));
        xml->writeTextElement("maxZone", QString::number(maxZoneValue));

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

        naxisbutton->writeConfig(xml);
        paxisbutton->writeConfig(xml);

        xml->writeEndElement();
    }
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
