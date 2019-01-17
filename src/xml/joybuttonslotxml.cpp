#include "joybuttonslotxml.h"
#include "joybuttonslot.h"
#include "messagehandler.h"
#include "antkeymapper.h"
#include "globalvariables.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFileInfo>
#include <QDebug>


JoyButtonSlotXml::JoyButtonSlotXml(JoyButtonSlot *joyBtnSlot, QObject *parent) : QObject(parent), m_joyBtnSlot(joyBtnSlot)
{

}


void JoyButtonSlotXml::readConfig(QXmlStreamReader *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (xml->isStartElement() && (xml->name() == "slot"))
    {
        QString profile = QString();
        QString tempStringData = QString();
        QString extraStringData = QString();

        xml->readNextStartElement();

        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != "slot")))
        {
            if ((xml->name() == "code") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                bool ok = false;
                int tempchoice = temptext.toInt(&ok, 0);

                if (ok) m_joyBtnSlot->setSlotCode(tempchoice);
            }
            else if ((xml->name() == "profile") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                profile = temptext;
            }
            else if ((xml->name() == "text") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                tempStringData = temptext;
            }
            else if ((xml->name() == "path") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                tempStringData = temptext;
            }
            else if ((xml->name() == "arguments") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                extraStringData = temptext;
            }
            else if ((xml->name() == "mode") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();

                if (temptext == "keyboard")
                {
                    m_joyBtnSlot->setSlotMode(JoyButtonSlot::JoyKeyboard);
                }
                else if (temptext == "mousebutton")
                {
                    m_joyBtnSlot->setSlotMode(JoyButtonSlot::JoyMouseButton);
                }
                else if (temptext == "mousemovement")
                {
                    m_joyBtnSlot->setSlotMode(JoyButtonSlot::JoyMouseMovement);
                }
                else if (temptext == "pause")
                {
                    m_joyBtnSlot->setSlotMode(JoyButtonSlot::JoyPause);
                }
                else if (temptext == "hold")
                {
                    m_joyBtnSlot->setSlotMode(JoyButtonSlot::JoyHold);
                }
                else if (temptext == "cycle")
                {
                    m_joyBtnSlot->setSlotMode(JoyButtonSlot::JoyCycle);
                }
                else if (temptext == "distance")
                {
                    m_joyBtnSlot->setSlotMode(JoyButtonSlot::JoyDistance);
                }
                else if (temptext == "release")
                {
                    m_joyBtnSlot->setSlotMode(JoyButtonSlot::JoyRelease);
                }
                else if (temptext == "mousespeedmod")
                {
                    m_joyBtnSlot->setSlotMode(JoyButtonSlot::JoyMouseSpeedMod);
                }
                else if (temptext == "keypress")
                {
                    m_joyBtnSlot->setSlotMode(JoyButtonSlot::JoyKeyPress);
                }
                else if (temptext == "delay")
                {
                    m_joyBtnSlot->setSlotMode(JoyButtonSlot::JoyDelay);
                }
                else if (temptext == "loadprofile")
                {
                    m_joyBtnSlot->setSlotMode(JoyButtonSlot::JoyLoadProfile);
                }
                else if (temptext == "setchange")
                {
                    m_joyBtnSlot->setSlotMode(JoyButtonSlot::JoySetChange);
                }
                else if (temptext == "textentry")
                {
                    m_joyBtnSlot->setSlotMode(JoyButtonSlot::JoyTextEntry);
                }
                else if (temptext == "execute")
                {
                    m_joyBtnSlot->setSlotMode(JoyButtonSlot::JoyExecute);
                }
            }
            else
            {
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }

        if (m_joyBtnSlot->getSlotMode() == JoyButtonSlot::JoyKeyboard)
        {
            int virtualkey = AntKeyMapper::getInstance()->returnVirtualKey(m_joyBtnSlot->getSlotCode());
            int tempkey = m_joyBtnSlot->getSlotCode();

            if (virtualkey)
            {
                // Mapping found a valid native keysym.
                m_joyBtnSlot->setSlotCode(virtualkey, tempkey);
            }
            else if (m_joyBtnSlot->getSlotCode() > QtKeyMapperBase::nativeKeyPrefix)
            {
                // Value is in the native key range. Remove prefix and use
                // new value as a native keysym.
                int temp = m_joyBtnSlot->getSlotCode() - QtKeyMapperBase::nativeKeyPrefix;
                m_joyBtnSlot->setSlotCode(temp);
            }
        }
        else if ((m_joyBtnSlot->getSlotMode() == JoyButtonSlot::JoyLoadProfile) && !profile.isEmpty())
        {
            QFileInfo profileInfo(profile);

            if (!profileInfo.exists() || !((profileInfo.suffix() == "amgp") || (profileInfo.suffix() == "xml")))
            {
                m_joyBtnSlot->setTextData("");
            }
            else
            {
                m_joyBtnSlot->setTextData(profile);
            }
        }
        else if (m_joyBtnSlot->getSlotMode() == JoyButtonSlot::JoySetChange && !(m_joyBtnSlot->getSlotCode() >= 0) && !(m_joyBtnSlot->getSlotCode() < GlobalVariables::InputDevice::NUMBER_JOYSETS))
        {
            m_joyBtnSlot->setSlotCode(-1);
        }
        else if ((m_joyBtnSlot->getSlotMode() == JoyButtonSlot::JoyTextEntry) && !tempStringData.isEmpty())
        {
            m_joyBtnSlot->setTextData(tempStringData);
        }
        else if ((m_joyBtnSlot->getSlotMode() == JoyButtonSlot::JoyExecute) && !tempStringData.isEmpty())
        {
            QFileInfo tempFile(tempStringData);

            if (tempFile.exists() && tempFile.isExecutable())
            {
                m_joyBtnSlot->setTextData(tempStringData);

                if (!extraStringData.isEmpty())
                    m_joyBtnSlot->setExtraData(QVariant(extraStringData));
            }
        }
    }
}

void JoyButtonSlotXml::writeConfig(QXmlStreamWriter *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    xml->writeStartElement(m_joyBtnSlot->getXmlName());

    if (m_joyBtnSlot->getSlotMode() == JoyButtonSlot::JoyKeyboard)
    {
        int basekey = AntKeyMapper::getInstance()->returnQtKey(m_joyBtnSlot->getSlotCode());
        int qtkey = m_joyBtnSlot->getSlotCodeAlias();

        if ((qtkey > 0) || (basekey > 0))
        {
            // Did not add an alias to slot. If a possible Qt key value
            // was found, use it.
            if ((qtkey == 0) && (basekey > 0)) qtkey = basekey;

            // Found a valid abstract keysym.
            #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << "ANT KEY: " << QString::number(qtkey, 16);
            #endif

            xml->writeTextElement("code", QString("0x%1").arg(qtkey, 0, 16));
        }
        else if (m_joyBtnSlot->getSlotCode() > 0)
        {
            // No abstraction provided for key. Add prefix to native keysym.
            int tempkey = m_joyBtnSlot->getSlotCode() | QtKeyMapperBase::nativeKeyPrefix;

            #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << "ANT KEY: " << QString::number(tempkey, 16);
            #endif

            xml->writeTextElement("code", QString("0x%1").arg(tempkey, 0, 16));
        }
    }
    else if ((m_joyBtnSlot->getSlotMode() == JoyButtonSlot::JoyLoadProfile) && !m_joyBtnSlot->getTextData().isEmpty())
    {
        xml->writeTextElement("profile", m_joyBtnSlot->getTextData());
    }
    else if ((m_joyBtnSlot->getSlotMode() == JoyButtonSlot::JoyTextEntry) && !m_joyBtnSlot->getTextData().isEmpty())
    {
        xml->writeTextElement("text", m_joyBtnSlot->getTextData());
    }
    else if ((m_joyBtnSlot->getSlotMode() == JoyButtonSlot::JoyExecute) && !m_joyBtnSlot->getTextData().isEmpty())
    {
        xml->writeTextElement("path", m_joyBtnSlot->getTextData());

        if (!m_joyBtnSlot->getExtraData().isNull() && m_joyBtnSlot->getExtraData().canConvert<QString>())
        {
            xml->writeTextElement("arguments", m_joyBtnSlot->getExtraData().toString());
        }
    }
    else
    {
        xml->writeTextElement("code", QString::number(m_joyBtnSlot->getSlotCode()));
    }

    xml->writeStartElement("mode");

    switch(m_joyBtnSlot->getSlotMode())
    {
        case JoyButtonSlot::JoyKeyboard:
            xml->writeCharacters("keyboard");
            break;

        case JoyButtonSlot::JoyMouseButton:
            xml->writeCharacters("mousebutton");
            break;

        case JoyButtonSlot::JoyMouseMovement:
            xml->writeCharacters("mousemovement");
            break;

        case JoyButtonSlot::JoyPause:
            xml->writeCharacters("pause");
            break;

        case JoyButtonSlot::JoyHold:
            xml->writeCharacters("hold");
            break;

        case JoyButtonSlot::JoyCycle:
            xml->writeCharacters("cycle");
            break;

        case JoyButtonSlot::JoyDistance:
            xml->writeCharacters("distance");
            break;

        case JoyButtonSlot::JoyRelease:
            xml->writeCharacters("release");
            break;

        case JoyButtonSlot::JoyMouseSpeedMod:
            xml->writeCharacters("mousespeedmod");
            break;

        case JoyButtonSlot::JoyKeyPress:
            xml->writeCharacters("keypress");
            break;

        case JoyButtonSlot::JoyDelay:
            xml->writeCharacters("delay");
            break;

        case JoyButtonSlot::JoyLoadProfile:
            xml->writeCharacters("loadprofile");
            break;

        case JoyButtonSlot::JoySetChange:
            xml->writeCharacters("setchange");
            break;

        case JoyButtonSlot::JoyTextEntry:
            xml->writeCharacters("textentry");
            break;

        case JoyButtonSlot::JoyExecute:
            xml->writeCharacters("execute");
            break;
    }

    xml->writeEndElement();
    xml->writeEndElement();
}


