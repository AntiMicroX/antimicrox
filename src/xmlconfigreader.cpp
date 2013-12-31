#include <QDebug>
#include <QDir>

#include "xmlconfigmigration.h"
#include "xmlconfigwriter.h"
#include "xmlconfigreader.h"

XMLConfigReader::XMLConfigReader(QObject *parent) :
    QObject(parent)
{
    xml = new QXmlStreamReader();
    configFile = 0;
    joystick = 0;
}

XMLConfigReader::~XMLConfigReader()
{
    if (configFile)
    {
        if (configFile->isOpen())
        {
            configFile->close();
        }

        delete configFile;
        configFile = 0;
    }

    if (xml)
    {
        delete xml;
        xml = 0;
    }
}

void XMLConfigReader::setJoystick(InputDevice *joystick)
{
    this->joystick = joystick;
}

void XMLConfigReader::setFileName(QString filename)
{
    QFile *temp = new QFile(filename);
    if (temp->exists())
    {
        configFile = temp;
    }
    else
    {
        delete temp;
        temp = 0;
    }
}

void XMLConfigReader::configJoystick(InputDevice *joystick)
{
    this->joystick = joystick;
    read();
}

bool XMLConfigReader::read()
{
    bool error = false;

    if (configFile && configFile->exists() && joystick)
    {
        xml->clear();

        if (!configFile->isOpen())
        {
            configFile->open(QFile::ReadOnly | QFile::Text);
            xml->setDevice(configFile);
        }

        xml->readNextStartElement();
        if (xml->name() != joystick->getXmlName())
        {
            xml->raiseError("Root node is not a joystick");
        }
        else
        {
            XMLConfigMigration migration(xml);
            if (migration.requiresMigration())
            {
                QString migrationString = migration.migrate();
                if (migrationString.length() > 0)
                {
                    // Remove QFile from reader and clear state
                    xml->clear();
                    // Add converted XML string to reader
                    xml->addData(migrationString);
                    // Skip joystick root node
                    xml->readNextStartElement();
                    // Close current config file
                    configFile->close();

                    // Write converted XML to file
                    configFile->open(QFile::WriteOnly | QFile::Text);
                    configFile->write(migrationString.toLocal8Bit());
                    configFile->close();
                }
            }
        }

        while (!xml->atEnd())
        {
            if (xml->name() == joystick->getXmlName() && xml->isStartElement())
            {
                joystick->readConfig(xml);
            }
            else
            {
                // If none of the above, skip the element
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }

        if (configFile->isOpen())
        {
            configFile->close();
        }

        if (xml->hasError() && xml->error() != QXmlStreamReader::PrematureEndOfDocumentError)
        {
            error = true;
        }
    }

    return error;
}
