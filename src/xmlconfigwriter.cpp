/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "xmlconfigwriter.h"

#include "common.h"
#include "inputdevice.h"
#include "xml/inputdevicexml.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QXmlStreamWriter>

XMLConfigWriter::XMLConfigWriter(QObject *parent)
    : QObject(parent)
{
    xml = new QXmlStreamWriter();
    xml->setAutoFormatting(true);
    configFile = nullptr;
    m_joystick = nullptr;
    m_joystickXml = nullptr;
    writerError = false;
}

XMLConfigWriter::~XMLConfigWriter()
{
    if (configFile != nullptr)
    {
        if (configFile->isOpen())
            configFile->close();

        delete configFile;
        configFile = nullptr;
    }

    if (xml != nullptr)
    {
        delete xml;
        xml = nullptr;
    }
}

/**
 * @brief Write input device config from the current object into XML file
 * @param[in] joystickXml InputDeviceXml which gets serialized
 */
void XMLConfigWriter::write(InputDeviceXml *joystickXml)
{
    writerError = false;

    if (!configFile->isOpen())
    {
        configFile->open(QFile::WriteOnly | QFile::Text);
        xml->setDevice(configFile);
    } else
    {
        writerError = true;
        writerErrorString = tr("Could not write to profile at %1.").arg(configFile->fileName());
    }

    if (!writerError)
    {
        xml->writeStartDocument();
        joystickXml->writeConfig(xml);
        xml->writeEndDocument();
    }

    if (configFile->isOpen())
        configFile->close();
}

/**
 * @brief Sets the filename of the to be written XML file
 */
void XMLConfigWriter::setFileName(QString filename)
{
    QFile *temp = new QFile(filename);
    fileName = filename;
    configFile = temp;
}

bool XMLConfigWriter::hasError() { return writerError; }

const QString XMLConfigWriter::getErrorString() { return writerErrorString; }

const QXmlStreamWriter *XMLConfigWriter::getXml() { return xml; }

QString const &XMLConfigWriter::getFileName() { return fileName; }

const QFile *XMLConfigWriter::getConfigFile() { return configFile; }

const InputDevice *XMLConfigWriter::getJoystick() { return m_joystick; }
