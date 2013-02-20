#include <QDebug>

#include "xmlconfigmigration.h"

XMLConfigMigration::XMLConfigMigration(QXmlStreamReader *reader, QObject *parent) :
    QObject(parent)
{
    this->reader = reader;
    this->fileVersion = reader->attributes().value("configversion").toString().toInt();
    //fileName = QString(PadderCommon::configPath + "/brony.xml");
    //configFile = new QFile(fileName);
    //configFile->open(QFile::WriteOnly | QFile::Text);

    //this->writer = new QXmlStreamWriter();
    //writer->setAutoFormatting(true);
}

XMLConfigMigration::~XMLConfigMigration()
{
    /*if (writer)
    {
        delete writer;
        writer = 0;
    }*/
}

bool XMLConfigMigration::requiresMigration()
{
    bool toMigrate = false;
    if (fileVersion < PadderCommon::LATESTCONFIGFILEVERSION)
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
        if (tempFileVersion == 0)
        {
            tempXmlString = initialMigration();
            reader->clear();
            reader->addData(tempXmlString);
            tempFileVersion = 1;
        }
        /*if (tempFileVersion == 1)
        {
            tempXmlString = version0001Migration();
            tempFileVersion = 2;
        }*/
    }
    return tempXmlString;
}

QString XMLConfigMigration::initialMigration()
{
    QString tempXmlString;
    QXmlStreamWriter writer(&tempXmlString);
    writer.setAutoFormatting(true);
    reader->readNextStartElement();

    writer.writeStartDocument();
    writer.writeStartElement("joystick");
    writer.writeAttribute("configversion", QString::number(PadderCommon::LATESTCONFIGFILEVERSION));

    while (!reader->atEnd())
    {
        if (reader->name() == "button" && reader->isStartElement())
        {
            initialMigrationReadButton(writer);
        }

        else if (reader->name() == "axis" && reader->isStartElement())
        {
            int axismode = 0;
            int mousemode = 0;
            int pkeycode = 0;
            int nkeycode = 0;
            int mousespeed = 30;

            writer.writeCurrentToken(*reader);
            reader->readNextStartElement();
            while (!reader->atEnd() && (!reader->isEndElement() && reader->name() != "axis"))
            {
                if (reader->name() == "axismode" && reader->isStartElement())
                {
                    QString temptext = reader->readElementText();
                    axismode = temptext.toInt();
                }
                else if (reader->name() == "mousemode" && reader->isStartElement())
                {
                    QString temptext = reader->readElementText();
                    mousemode = temptext.toInt();
                }
                else if (reader->name() == "pkeycode" && reader->isStartElement())
                {
                    QString temptext = reader->readElementText();
                    pkeycode = temptext.toInt();
                }
                else if (reader->name() == "nkeycode" && reader->isStartElement())
                {
                    QString temptext = reader->readElementText();
                    nkeycode = temptext.toInt();
                }
                else if (reader->name() == "mousespeed" && reader->isStartElement())
                {
                    QString temptext = reader->readElementText();
                    mousespeed = temptext.toInt();
                }
                else if (reader->name() == "deadZone" && reader->isStartElement())
                {
                    writer.writeCurrentToken(*reader);
                    writer.writeCharacters(reader->readElementText());
                    writer.writeEndElement();
                }
                else if (reader->name() == "maxZone" && reader->isStartElement())
                {
                    writer.writeCurrentToken(*reader);
                    writer.writeCharacters(reader->readElementText());
                    writer.writeEndElement();
                }
                else if (reader->name() == "throttle" && reader->isStartElement())
                {
                    writer.writeCurrentToken(*reader);
                    writer.writeCharacters(reader->readElementText());
                    writer.writeEndElement();
                }

                reader->readNextStartElement();
            }

            writer.writeStartElement("axisbutton");
            writer.writeAttribute("index", QString::number(1));

            writer.writeStartElement("slots");
            if (axismode == 0)
            {
                bool usingMouse = false;
                if (nkeycode > 400)
                {
                    nkeycode -= 400;
                    usingMouse = true;
                }

                writer.writeStartElement("slot");
                writer.writeTextElement("code", QString::number(nkeycode));
                if (!usingMouse)
                {
                    writer.writeTextElement("mode", "keyboard");
                }
                else
                {
                    writer.writeTextElement("mode", "mousebutton");
                }
                writer.writeEndElement();
            }
            else
            {
                if (mousemode == 0)
                {
                    writer.writeStartElement("slot");
                    writer.writeTextElement("code", QString::number(3));
                    writer.writeTextElement("mode", "mousemovement");
                    writer.writeEndElement();
                }
                else if (mousemode == 1)
                {
                    writer.writeStartElement("slot");
                    writer.writeTextElement("code", QString::number(4));
                    writer.writeTextElement("mode", "mousemovement");
                    writer.writeEndElement();
                }
                else if (mousemode == 2)
                {
                    writer.writeStartElement("slot");
                    writer.writeTextElement("code", QString::number(1));
                    writer.writeTextElement("mode", "mousemovement");
                    writer.writeEndElement();
                }
                else if (mousemode == 3)
                {
                    writer.writeStartElement("slot");
                    writer.writeTextElement("code", QString::number(2));
                    writer.writeTextElement("mode", "mousemovement");
                    writer.writeEndElement();
                }
            }
            writer.writeEndElement();

            writer.writeTextElement("toggle", "0");
            writer.writeTextElement("useturbo", "false");
            writer.writeTextElement("turboInterval", "0");
            writer.writeTextElement("mousespeedx", QString::number(mousespeed));
            writer.writeTextElement("mousespeedy", QString::number(mousespeed));

            writer.writeEndElement();

            writer.writeStartElement("axisbutton");
            writer.writeAttribute("index", QString::number(2));

            writer.writeStartElement("slots");
            if (axismode == 0)
            {
                bool usingMouse = false;
                if (pkeycode > 400)
                {
                    pkeycode -= 400;
                    usingMouse = true;
                }

                writer.writeStartElement("slot");
                writer.writeTextElement("code", QString::number(pkeycode));
                if (!usingMouse)
                {
                    writer.writeTextElement("mode", "keyboard");
                }
                else
                {
                    writer.writeTextElement("mode", "mousebutton");
                }
                writer.writeEndElement();
            }
            else
            {
                if (mousemode == 0)
                {
                    writer.writeStartElement("slot");
                    writer.writeTextElement("code", QString::number(4));
                    writer.writeTextElement("mode", "mousemovement");
                    writer.writeEndElement();
                }
                else if (mousemode == 1)
                {
                    writer.writeStartElement("slot");
                    writer.writeTextElement("code", QString::number(3));
                    writer.writeTextElement("mode", "mousemovement");
                    writer.writeEndElement();
                }
                else if (mousemode == 2)
                {
                    writer.writeStartElement("slot");
                    writer.writeTextElement("code", QString::number(2));
                    writer.writeTextElement("mode", "mousemovement");
                    writer.writeEndElement();
                }
                else if (mousemode == 3)
                {
                    writer.writeStartElement("slot");
                    writer.writeTextElement("code", QString::number(1));
                    writer.writeTextElement("mode", "mousemovement");
                    writer.writeEndElement();
                }
            }

            writer.writeEndElement();

            writer.writeTextElement("toggle", "0");
            writer.writeTextElement("useturbo", "false");
            writer.writeTextElement("turboInterval", "0");

            writer.writeEndElement();

            writer.writeEndElement();
        }

        else if (reader->name() == "dpad" && reader->isStartElement())
        {
            writer.writeCurrentToken(*reader);
            reader->readNextStartElement();

            while (!reader->atEnd() && (!reader->isEndElement() && reader->name() != "dpad"))
            {
                initialMigrationReadButton(writer);
                reader->readNextStartElement();
            }

            writer.writeEndElement();
        }

        reader->readNextStartElement();
    }

    writer.writeEndElement();
    writer.writeEndDocument();

    //qDebug() << tempXmlString << endl;
    return tempXmlString;
}

void XMLConfigMigration::initialMigrationReadButton(QXmlStreamWriter &writer)
{
    QString tagname = reader->name().toString();
    int keycode = 0;
    bool usemouse = false;
    int mousecode = 0;

    writer.writeCurrentToken(*reader);
    reader->readNextStartElement();

    while (!reader->atEnd() && (!reader->isEndElement() && reader->name() != tagname))
    {
        if (reader->name() == "keycode" && reader->isStartElement())
        {
            QString tempcode = reader->readElementText();
            keycode = tempcode.toInt();
        }
        else if (reader->name() == "usemouse" && reader->isStartElement())
        {
            QString tempchoice = reader->readElementText();
            usemouse = (tempchoice == "true") ? true : false;
        }
        else if (reader->name() == "mousecode" && reader->isStartElement())
        {
            QString tempchoice = reader->readElementText();
            mousecode = tempchoice.toInt();
        }
        else if (reader->name() == "toggle" && reader->isStartElement())
        {
            writer.writeCurrentToken(*reader);
            writer.writeCharacters(reader->readElementText());
            writer.writeEndElement();
        }
        else if (reader->name() == "turboInterval" && reader->isStartElement())
        {
            writer.writeCurrentToken(*reader);
            writer.writeCharacters(reader->readElementText());
            writer.writeEndElement();
        }
        reader->readNextStartElement();
    }

    writer.writeTextElement("mousespeedx", QString::number(30));
    writer.writeTextElement("mousespeedy", QString::number(30));

    writer.writeStartElement("slots");
    if (usemouse)
    {
        writer.writeStartElement("slot");
        writer.writeTextElement("code", QString::number(mousecode));
        writer.writeTextElement("mode", "mousebutton");
        writer.writeEndElement();
    }
    else if (keycode > 0)
    {
        writer.writeStartElement("slot");
        writer.writeTextElement("code", QString::number(keycode));
        writer.writeTextElement("mode", "keyboard");
        writer.writeEndElement();
    }
    writer.writeEndElement();
    writer.writeEndElement();
}
