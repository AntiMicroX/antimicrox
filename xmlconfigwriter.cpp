#include <QDir>

#include "xmlconfigwriter.h"

XMLConfigWriter::XMLConfigWriter(QObject *parent) :
    QObject(parent)
{

    //fileName = QString(PadderCommon::configPath + "/brony.xml");
    //configFile = new QFile(fileName);
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

void XMLConfigWriter::write(Joystick *joystick)
{
    if (!configFile->isOpen())
    {
        configFile->open(QFile::WriteOnly | QFile::Text);
        xml->setDevice(configFile);
    }

    xml->writeStartDocument();
    xml->writeStartElement("joystick");
    xml->writeAttribute("configversion", QString::number(PadderCommon::LATESTCONFIGFILEVERSION));

    for (int i=0; i < joystick->getNumberAxes(); i++)
    {
        JoyAxis *axis = joystick->getJoyAxis(i);
        axis->writeConfig(xml);
    }

    for (int i=0; i < joystick->getNumberHats(); i++)
    {
        JoyDPad *dpad = joystick->getJoyDPad(i);
        dpad->writeConfig(xml);
    }

    for (int i=0; i < joystick->getNumberButtons(); i++)
    {
        JoyButton *button = joystick->getJoyButton(i);
        button->writeConfig(xml);
    }

    xml->writeEndElement();
    xml->writeEndDocument();

    configFile->close();
}

void XMLConfigWriter::setFileName(QString filename)
{
    QFile *temp = new QFile(filename);
    fileName = filename;
    configFile = temp;
}
