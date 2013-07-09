#include <QDebug>
#include <QDir>

#include "xmlconfigreader.h"
#include "xmlconfigmigration.h"
#include "xmlconfigwriter.h"

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

void XMLConfigReader::setJoystick(Joystick *joystick)
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

void XMLConfigReader::configJoystick(Joystick *joystick)
{
    this->joystick = joystick;
    read();
}

bool XMLConfigReader::read()
{
    //bool requiredMigration = false;
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
        if (xml->name() != "joystick")
        {
            xml->raiseError("Root node is not a joystick");
        }
        else
        {
            /*XMLConfigMigration *migration = new XMLConfigMigration(xml);
            if (migration->requiresMigration())
            {
                QString migrationString = migration->migrate();
                if (migrationString.length() > 0)
                {
                    xml->clear();
                    xml->addData(migrationString);
                    xml->readNextStartElement();
                    requiredMigration = true;
                }
            }*/
            //xml->readNextStartElement();
        }

        while (!xml->atEnd())
        {
            if (xml->name() == "joystick" && xml->isStartElement())
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

        configFile->close();

        /*if (requiredMigration && (!xml->hasError() || xml->error() == QXmlStreamReader::PrematureEndOfDocumentError))
        {
            XMLConfigWriter *newsave = new XMLConfigWriter();
            newsave->setFileName(configFile->fileName());
            newsave->write(joystick);
            delete newsave;
            newsave = 0;
        }*/

        if (xml->hasError() && xml->error() != QXmlStreamReader::PrematureEndOfDocumentError)
        {
            error = true;
        }
    }

    return error;
}
