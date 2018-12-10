#include "joybuttonslotxml.h"
#include "joybuttonslot.h"

#include "antkeymapper.h"
#include "messagehandler.h"
#include "globalvariables.h"

#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QFileInfo>
#include <QDebug>


JoyButtonSlotXml::JoyButtonSlotXml(JoyButtonSlot* btnSlot, QObject *parent) : QObject(parent)
{
    m_btnSlot = btnSlot;
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

                if (ok) m_btnSlot->setSlotCode(tempchoice);
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
                    m_btnSlot->setSlotMode(JoyButtonSlot::JoyKeyboard);
                }
                else if (temptext == "mousebutton")
                {
                    m_btnSlot->setSlotMode(JoyButtonSlot::JoyMouseButton);
                }
                else if (temptext == "mousemovement")
                {
                    m_btnSlot->setSlotMode(JoyButtonSlot::JoyMouseMovement);
                }
                else if (temptext == "pause")
                {
                    m_btnSlot->setSlotMode(JoyButtonSlot::JoyPause);
                }
                else if (temptext == "hold")
                {
                    m_btnSlot->setSlotMode(JoyButtonSlot::JoyHold);
                }
                else if (temptext == "cycle")
                {
                    m_btnSlot->setSlotMode(JoyButtonSlot::JoyCycle);
                }
                else if (temptext == "distance")
                {
                    m_btnSlot->setSlotMode(JoyButtonSlot::JoyDistance);
                }
                else if (temptext == "release")
                {
                    m_btnSlot->setSlotMode(JoyButtonSlot::JoyRelease);
                }
                else if (temptext == "mousespeedmod")
                {
                    m_btnSlot->setSlotMode(JoyButtonSlot::JoyMouseSpeedMod);
                }
                else if (temptext == "keypress")
                {
                    m_btnSlot->setSlotMode(JoyButtonSlot::JoyKeyPress);
                }
                else if (temptext == "delay")
                {
                    m_btnSlot->setSlotMode(JoyButtonSlot::JoyDelay);
                }
                else if (temptext == "loadprofile")
                {
                    m_btnSlot->setSlotMode(JoyButtonSlot::JoyLoadProfile);
                }
                else if (temptext == "setchange")
                {
                    m_btnSlot->setSlotMode(JoyButtonSlot::JoySetChange);
                }
                else if (temptext == "textentry")
                {
                    m_btnSlot->setSlotMode(JoyButtonSlot::JoyTextEntry);
                }
                else if (temptext == "execute")
                {
                    m_btnSlot->setSlotMode(JoyButtonSlot::JoyExecute);
                }
            }
            else
            {
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }

        if (m_btnSlot->getSlotMode() == JoyButtonSlot::JoyKeyboard)
        {
            int virtualkey = AntKeyMapper::getInstance()->returnVirtualKey(m_btnSlot->getSlotCode());
            int tempkey = m_btnSlot->getSlotCode();

            if (virtualkey)
            {
                // Mapping found a valid native keysym.
                m_btnSlot->setSlotCode(virtualkey, tempkey);
            }
            else if (m_btnSlot->getSlotCode() > QtKeyMapperBase::nativeKeyPrefix)
            {
                // Value is in the native key range. Remove prefix and use
                // new value as a native keysym.
                int temp = m_btnSlot->getSlotCode() - QtKeyMapperBase::nativeKeyPrefix;
                m_btnSlot->setSlotCode(temp);
            }
        }
        else if ((m_btnSlot->getSlotMode() == JoyButtonSlot::JoyLoadProfile) && !profile.isEmpty())
        {
            QFileInfo profileInfo(profile);

            if (!profileInfo.exists() || !((profileInfo.suffix() == "amgp") || (profileInfo.suffix() == "xml")))
            {
                m_btnSlot->setTextData("");
            }
            else
            {
                m_btnSlot->setTextData(profile);
            }
        }
        else if (m_btnSlot->getSlotMode() == JoyButtonSlot::JoySetChange && !(m_btnSlot->getSlotCode() >= 0) && !(m_btnSlot->getSlotCode() < GlobalVariables::InputDevice::NUMBER_JOYSETS))
        {
            m_btnSlot->setSlotCode(-1);
        }
        else if ((m_btnSlot->getSlotMode() == JoyButtonSlot::JoyTextEntry) && !tempStringData.isEmpty())
        {
            m_btnSlot->setTextData(tempStringData);
        }
        else if ((m_btnSlot->getSlotMode() == JoyButtonSlot::JoyExecute) && !tempStringData.isEmpty())
        {
            QFileInfo tempFile(tempStringData);

            if (tempFile.exists() && tempFile.isExecutable())
            {
                m_btnSlot->setTextData(tempStringData);

                if (!extraStringData.isEmpty())
                    m_btnSlot->setExtraData(QVariant(extraStringData));
            }
        }
    }
}

void JoyButtonSlotXml::writeConfig(QXmlStreamWriter *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    xml->writeStartElement(m_btnSlot->getXmlName());

    if (m_btnSlot->getSlotMode() == JoyButtonSlot::JoyKeyboard)
    {
        int basekey = AntKeyMapper::getInstance()->returnQtKey(m_btnSlot->getSlotCode());
        int qtkey = m_btnSlot->getSlotCodeAlias();

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
        else if (m_btnSlot->getSlotCode() > 0)
        {
            // No abstraction provided for key. Add prefix to native keysym.
            int tempkey = m_btnSlot->getSlotCode() | QtKeyMapperBase::nativeKeyPrefix;

            #ifndef QT_DEBUG_NO_OUTPUT
                qDebug() << "ANT KEY: " << QString::number(tempkey, 16);
            #endif

            xml->writeTextElement("code", QString("0x%1").arg(tempkey, 0, 16));
        }
    }
    else if ((m_btnSlot->getSlotMode() == JoyButtonSlot::JoyLoadProfile) && !m_btnSlot->getTextData().isEmpty())
    {
        xml->writeTextElement("profile", m_btnSlot->getTextData());
    }
    else if ((m_btnSlot->getSlotMode() == JoyButtonSlot::JoyTextEntry) && !m_btnSlot->getTextData().isEmpty())
    {
        xml->writeTextElement("text", m_btnSlot->getTextData());
    }
    else if ((m_btnSlot->getSlotMode() == JoyButtonSlot::JoyExecute) && !m_btnSlot->getTextData().isEmpty())
    {
        xml->writeTextElement("path", m_btnSlot->getTextData());

        if (!m_btnSlot->getExtraData().isNull() && m_btnSlot->getExtraData().canConvert<QString>())
        {
            xml->writeTextElement("arguments", m_btnSlot->getExtraData().toString());
        }
    }
    else
    {
        xml->writeTextElement("code", QString::number(m_btnSlot->getSlotCode()));
    }

    xml->writeStartElement("mode");

    switch(m_btnSlot->getSlotMode())
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

