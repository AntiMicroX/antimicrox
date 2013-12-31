#include <QDir>

#include "xmlconfigwriter.h"

XMLConfigWriter::XMLConfigWriter(QObject *parent) :
    QObject(parent)
{
    xml = new QXmlStreamWriter();
    xml->setAutoFormatting(true);
    configFile = 0;
    joystick = 0;
}

XMLConfigWriter::~XMLConfigWriter()
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

void XMLConfigWriter::write(InputDevice *joystick)
{
    if (!configFile->isOpen())
    {
        configFile->open(QFile::WriteOnly | QFile::Text);
        xml->setDevice(configFile);
    }

    xml->writeStartDocument();

    joystick->writeConfig(xml);

    xml->writeEndDocument();

    configFile->close();
}

void XMLConfigWriter::setFileName(QString filename)
{
    QFile *temp = new QFile(filename);
    fileName = filename;
    configFile = temp;
}
