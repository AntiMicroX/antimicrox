#include "event.h"
#include "antkeymapper.h"

#include "xmlconfigmigration.h"

XMLConfigMigration::XMLConfigMigration(QXmlStreamReader *reader, QObject *parent) :
    QObject(parent)
{
    this->reader = reader;
    if (reader->device() && reader->device()->isOpen())
    {
        this->fileVersion = reader->attributes().value("configversion").toString().toInt();
    }
    else
    {
        this->fileVersion = 0;
    }
}

bool XMLConfigMigration::requiresMigration()
{
    bool toMigrate = false;
    if (fileVersion == 0)
    {
        toMigrate = false;
    }
    else if (fileVersion >= 2 && fileVersion <= PadderCommon::LATESTCONFIGMIGRATIONVERSION)
    {
        toMigrate = true;
    }

    return toMigrate;
}

QString XMLConfigMigration::migrate()
{
    QString tempXmlString;
    if (requiresMigration())
    {
        int tempFileVersion = fileVersion;
        QString initialData = readConfigToString();
        reader->clear();
        reader->addData(initialData);

        if (tempFileVersion >= 2 && tempFileVersion <= 5)
        {
            tempXmlString = version0006Migration();
            tempFileVersion = PadderCommon::LATESTCONFIGFILEVERSION;
        }
    }

    return tempXmlString;
}

QString XMLConfigMigration::readConfigToString()
{
    QString tempXmlString;
    QXmlStreamWriter writer(&tempXmlString);
    writer.setAutoFormatting(true);
    while (!reader->atEnd())
    {
        writer.writeCurrentToken(*reader);
        reader->readNext();
    }

    return tempXmlString;
}

QString XMLConfigMigration::version0006Migration()
{
    QString tempXmlString;
    QXmlStreamWriter writer(&tempXmlString);
    writer.setAutoFormatting(true);
    reader->readNextStartElement();
    reader->readNextStartElement();

    writer.writeStartDocument();
    writer.writeStartElement("joystick");
    writer.writeAttribute("configversion", QString::number(6));
    writer.writeAttribute("appversion", PadderCommon::programVersion);

    while (!reader->atEnd())
    {
        if (reader->name() == "slot" && reader->isStartElement())
        {
            unsigned int slotcode = 0;
            QString slotmode;
            writer.writeCurrentToken(*reader);
            reader->readNext();

            // Grab current slot code and slot mode
            while (!reader->atEnd() && (!reader->isEndElement() && reader->name() != "slot"))
            {
                if (reader->name() == "code" && reader->isStartElement())
                {
                    QString tempcode = reader->readElementText();
                    slotcode = tempcode.toInt();
                }
                else if (reader->name() == "mode" && reader->isStartElement())
                {
                    slotmode = reader->readElementText();
                }
                else
                {
                    writer.writeCurrentToken(*reader);
                }

                reader->readNext();
            }

            // Reformat slot code if associated with the keyboard
            if (slotcode && !slotmode.isEmpty())
            {
                if (slotmode == "keyboard")
                {
                    unsigned int tempcode = slotcode;
#ifdef Q_OS_WIN
                    slotcode = AntKeyMapper::returnQtKey(slotcode);
#else
                    slotcode = AntKeyMapper::returnQtKey(X11KeyCodeToX11KeySym(slotcode));
#endif
                    if (slotcode > 0)
                    {
                        writer.writeTextElement("code", QString("0x%1").arg(slotcode, 0, 16));
                    }
                    else if (tempcode > 0)
                    {
                        writer.writeTextElement("code", QString("0x%1").arg(tempcode | QtKeyMapperBase::nativeKeyPrefix, 0, 16));
                    }
                }
                else
                {
                    writer.writeTextElement("code", QString::number(slotcode));
                }

                writer.writeTextElement("mode", slotmode);
            }
            writer.writeCurrentToken(*reader);
        }
        else
        {
            writer.writeCurrentToken(*reader);
        }
        reader->readNext();
    }

    return tempXmlString;
}
