/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
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

#include <QDir>

#include "xmlconfigwriter.h"

XMLConfigWriter::XMLConfigWriter(QObject *parent) :
    QObject(parent)
{
    xml = new QXmlStreamWriter();
    xml->setAutoFormatting(true);
    configFile = 0;
    joystick = 0;
    writerError = false;
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
    writerError = false;

    if (!configFile->isOpen())
    {
        configFile->open(QFile::WriteOnly | QFile::Text);
        xml->setDevice(configFile);
    }
    else
    {
        writerError = true;
        writerErrorString = tr("Could not write to profile at %1.").arg(configFile->fileName());
    }

    if (!writerError)
    {
        xml->writeStartDocument();
        joystick->writeConfig(xml);
        xml->writeEndDocument();
    }

    if (configFile->isOpen())
    {
        configFile->close();
    }
}

void XMLConfigWriter::setFileName(QString filename)
{
    QFile *temp = new QFile(filename);
    fileName = filename;
    configFile = temp;
}

bool XMLConfigWriter::hasError()
{
    return writerError;
}

QString XMLConfigWriter::getErrorString()
{
    return writerErrorString;
}
