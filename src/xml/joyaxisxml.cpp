#include "joyaxisxml.h"
#include "joyaxis.h"
#include "messagehandler.h"
#include "inputdevice.h"
#include "xml/joybuttonxml.h"
#include "joybuttontypes/joyaxisbutton.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>


JoyAxisXml::JoyAxisXml(JoyAxis* axis, QObject *parent) : QObject(parent)
{
    m_joyAxis = axis;
    joyButtonXmlNAxis = new JoyButtonXml(axis->getNAxisButton());
    joyButtonXmlPAxis = new JoyButtonXml(axis->getPAxisButton());
}


void JoyAxisXml::readConfig(QXmlStreamReader *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (xml->isStartElement() && (xml->name() == m_joyAxis->getXmlName()))
    {
        xml->readNextStartElement();

        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != m_joyAxis->getXmlName())))
        {
            bool found = readMainConfig(xml);

            if (!found && (xml->name() == m_joyAxis->getNAxisButton()->getXmlName()) && xml->isStartElement())
            {
                found = true;
                readButtonConfig(xml);
            }

            if (!found) xml->skipCurrentElement();

            xml->readNextStartElement();
        }
    }
}

void JoyAxisXml::writeConfig(QXmlStreamWriter *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool currentlyDefault = m_joyAxis->isDefault();

    xml->writeStartElement(m_joyAxis->getXmlName());
    xml->writeAttribute("index", QString::number(m_joyAxis->getRealJoyIndex()));

    if (!currentlyDefault)
    {
        if (m_joyAxis->getDeadZone() != GlobalVariables::JoyAxis::AXISDEADZONE)
            xml->writeTextElement("deadZone", QString::number(m_joyAxis->getDeadZone()));

        if (m_joyAxis->getMaxZoneValue() != GlobalVariables::JoyAxis::AXISMAXZONE)
            xml->writeTextElement("maxZone", QString::number(m_joyAxis->getMaxZoneValue()));
    }

    xml->writeTextElement("center_value", QString::number(m_joyAxis->getAxisCenterCal()));
    xml->writeTextElement("min_value", QString::number(m_joyAxis->getAxisMinCal()));
    xml->writeTextElement("max_value", QString::number(m_joyAxis->getAxisMaxCal()));
    xml->writeStartElement("throttle");

        switch(m_joyAxis->getThrottle())
        {
            case -2:
                xml->writeCharacters("negativehalf");
            break;

            case -1:
                xml->writeCharacters("negative");
            break;

            case 0:
                xml->writeCharacters("normal");
            break;

            case 1:
                xml->writeCharacters("positive");
            break;

            case 2:
                xml->writeCharacters("positivehalf");
            break;
        }

    xml->writeEndElement();

    if (!currentlyDefault)
    {
        joyButtonXmlNAxis->writeConfig(xml);
        joyButtonXmlPAxis->writeConfig(xml);
    }

    xml->writeEndElement();
}


bool JoyAxisXml::readMainConfig(QXmlStreamReader *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool found = false;

    if ((xml->name() == "deadZone") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();

        #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "From xml config dead zone is: " << tempchoice;
        #endif

        m_joyAxis->setDeadZone(tempchoice);
    }
    else if ((xml->name() == "maxZone") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();

        #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "From xml config max zone is: " << tempchoice;
        #endif

        m_joyAxis->setMaxZoneValue(tempchoice);
    }
    else if ((xml->name() == "center_value") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();

        #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "From xml config center value is: " << tempchoice;
        #endif

        m_joyAxis->setAxisCenterCal(tempchoice);
    }
    else if ((xml->name() == "min_value") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();

        #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "From xml config min value is: " << tempchoice;
        #endif

        m_joyAxis->setAxisMinCal(tempchoice);
    }
    else if ((xml->name() == "max_value") && xml->isStartElement())
    {

        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();

        #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "From xml config max value is: " << tempchoice;
        #endif

        m_joyAxis->setAxisMaxCal(tempchoice);
    }
    else if ((xml->name() == "throttle") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();

        #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "From xml config throttle name is: " << temptext;
        #endif

        if (temptext == "negativehalf")
        {
            m_joyAxis->setThrottle(static_cast<int>(JoyAxis::NegativeHalfThrottle));
        }
        else if (temptext == "negative")
        {
            m_joyAxis->setThrottle(static_cast<int>(JoyAxis::NegativeThrottle));
        }
        else if (temptext == "normal")
        {
            m_joyAxis->setThrottle(static_cast<int>(JoyAxis::NormalThrottle));
        }
        else if (temptext == "positive")
        {
            m_joyAxis->setThrottle(static_cast<int>(JoyAxis::PositiveThrottle));
        }
        else if (temptext == "positivehalf")
        {
            m_joyAxis->setThrottle(static_cast<int>(JoyAxis::PositiveHalfThrottle));
        }

        InputDevice *device = m_joyAxis->getParentSet()->getInputDevice();

        if (!device->hasCalibrationThrottle(m_joyAxis->getIndex()))
        {
            device->setCalibrationStatus(m_joyAxis->getIndex(),
                                         static_cast<JoyAxis::ThrottleTypes>(m_joyAxis->getThrottle()));
        }

        m_joyAxis->setCurrentRawValue(m_joyAxis->getCurrentThrottledDeadValue());
        m_joyAxis->updateCurrentThrottledValue(m_joyAxis->calculateThrottledValue(m_joyAxis->getCurrentRawValue()));
    }

    return found;
}

bool JoyAxisXml::readButtonConfig(QXmlStreamReader *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool found = false;

    int index_local = xml->attributes().value("index").toString().toInt();

    if (index_local == 1)
    {
        found = true;
        joyButtonXmlNAxis->readConfig(xml);
    }
    else if (index_local == 2)
    {
        found = true;
        joyButtonXmlPAxis->readConfig(xml);
    }

    return found;
}
