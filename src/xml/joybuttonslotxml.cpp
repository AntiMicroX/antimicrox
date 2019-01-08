#include "joybuttonslotxml.h"
#include "messagehandler.h"
#include "antkeymapper.h"
#include "globalvariables.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFileInfo>

#include <QDebug>


JoyButtonSlotXml::JoyButtonSlotXml(JoyButtonSlot *joyBtnSlot, QObject *parent) : m_joyBtnSlot(joyBtnSlot)
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

                if (ok) this->setSlotCode(tempchoice);
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
                    this->setSlotMode(JoyKeyboard);
                }
                else if (temptext == "mousebutton")
                {
                    this->setSlotMode(JoyMouseButton);
                }
                else if (temptext == "mousemovement")
                {
                    this->setSlotMode(JoyMouseMovement);
                }
                else if (temptext == "pause")
                {
                    this->setSlotMode(JoyPause);
                }
                else if (temptext == "hold")
                {
                    this->setSlotMode(JoyHold);
                }
                else if (temptext == "cycle")
                {
                    this->setSlotMode(JoyCycle);
                }
                else if (temptext == "distance")
                {
                    this->setSlotMode(JoyDistance);
                }
                else if (temptext == "release")
                {
                    this->setSlotMode(JoyRelease);
                }
                else if (temptext == "mousespeedmod")
                {
                    this->setSlotMode(JoyMouseSpeedMod);
                }
                else if (temptext == "keypress")
                {
                    this->setSlotMode(JoyKeyPress);
                }
                else if (temptext == "delay")
                {
                    this->setSlotMode(JoyDelay);
                }
                else if (temptext == "loadprofile")
                {
                    this->setSlotMode(JoyLoadProfile);
                }
                else if (temptext == "setchange")
                {
                    this->setSlotMode(JoySetChange);
                }
                else if (temptext == "textentry")
                {
                    this->setSlotMode(JoyTextEntry);
                }
                else if (temptext == "execute")
                {
                    this->setSlotMode(JoyExecute);
                }
            }
            else
            {
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }

        if (this->getSlotMode() == JoyButtonSlot::JoyKeyboard)
        {
            int virtualkey = AntKeyMapper::getInstance()->returnVirtualKey(this->getSlotCode());
            int tempkey = this->getSlotCode();

            if (virtualkey)
            {
                // Mapping found a valid native keysym.
                this->setSlotCode(virtualkey, tempkey);
            }
            else if (this->getSlotCode() > QtKeyMapperBase::nativeKeyPrefix)
            {
                // Value is in the native key range. Remove prefix and use
                // new value as a native keysym.
                int temp = this->getSlotCode() - QtKeyMapperBase::nativeKeyPrefix;
                this->setSlotCode(temp);
            }
        }
        else if ((this->getSlotMode() == JoyButtonSlot::JoyLoadProfile) && !profile.isEmpty())
        {
            QFileInfo profileInfo(profile);

            if (!profileInfo.exists() || !((profileInfo.suffix() == "amgp") || (profileInfo.suffix() == "xml")))
            {
                this->setTextData("");
            }
            else
            {
                this->setTextData(profile);
            }
        }
        else if (this->getSlotMode() == JoySetChange && !(this->getSlotCode() >= 0) && !(this->getSlotCode() < GlobalVariables::InputDevice::NUMBER_JOYSETS))
        {
            this->setSlotCode(-1);
        }
        else if ((this->getSlotMode() == JoyTextEntry) && !tempStringData.isEmpty())
        {
            this->setTextData(tempStringData);
        }
        else if ((this->getSlotMode() == JoyExecute) && !tempStringData.isEmpty())
        {
            QFileInfo tempFile(tempStringData);

            if (tempFile.exists() && tempFile.isExecutable())
            {
                this->setTextData(tempStringData);

                if (!extraStringData.isEmpty())
                    this->setExtraData(QVariant(extraStringData));
            }
        }
    }
}

void JoyButtonSlotXml::writeConfig(QXmlStreamWriter *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    xml->writeStartElement(getXmlName());

    if (m_joyBtnSlot->getSlotMode() == JoyKeyboard)
    {
        int basekey = AntKeyMapper::getInstance()->returnQtKey(m_joyBtnSlot->getSlotCode());
        int qtkey = this->getSlotCodeAlias();

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
    else if ((m_joyBtnSlot->getSlotMode() == JoyLoadProfile) && !m_joyBtnSlot->getTextData().isEmpty())
    {
        xml->writeTextElement("profile", m_joyBtnSlot->getTextData());
    }
    else if ((m_joyBtnSlot->getSlotMode() == JoyTextEntry) && !m_joyBtnSlot->getTextData().isEmpty())
    {
        xml->writeTextElement("text", m_joyBtnSlot->getTextData());
    }
    else if ((m_joyBtnSlot->getSlotMode() == JoyExecute) && !m_joyBtnSlot->getTextData().isEmpty())
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
        case JoyKeyboard:
            xml->writeCharacters("keyboard");
            break;

        case JoyMouseButton:
            xml->writeCharacters("mousebutton");
            break;

        case JoyMouseMovement:
            xml->writeCharacters("mousemovement");
            break;

        case JoyPause:
            xml->writeCharacters("pause");
            break;

        case JoyHold:
            xml->writeCharacters("hold");
            break;

        case JoyCycle:
            xml->writeCharacters("cycle");
            break;

        case JoyDistance:
            xml->writeCharacters("distance");
            break;

        case JoyRelease:
            xml->writeCharacters("release");
            break;

        case JoyMouseSpeedMod:
            xml->writeCharacters("mousespeedmod");
            break;

        case JoyKeyPress:
            xml->writeCharacters("keypress");
            break;

        case JoyDelay:
            xml->writeCharacters("delay");
            break;

        case JoyLoadProfile:
            xml->writeCharacters("loadprofile");
            break;

        case JoySetChange:
            xml->writeCharacters("setchange");
            break;

        case JoyTextEntry:
            xml->writeCharacters("textentry");
            break;

        case JoyExecute:
            xml->writeCharacters("execute");
            break;
    }

    xml->writeEndElement();
    xml->writeEndElement();
}


