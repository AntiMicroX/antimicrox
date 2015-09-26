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

//#include <QDebug>
#include <QDir>
#include <QStringList>

#include "xmlconfigreader.h"
#include "xmlconfigmigration.h"
#include "xmlconfigwriter.h"
#include "common.h"


XMLConfigReader::XMLConfigReader(QObject *parent) :
    QObject(parent)
{
    xml = new QXmlStreamReader();
    configFile = 0;
    joystick = 0;
    initDeviceTypes();
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
        if (!deviceTypes.contains(xml->name().toString()))
        {
            xml->raiseError("Root node is not a joystick or controller");
        }
        else if (xml->name() == Joystick::xmlName)
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
                    if (configFile->isOpen())
                    {
                        configFile->write(migrationString.toLocal8Bit());
                        configFile->close();
                    }
                    else
                    {
                        xml->raiseError(tr("Could not write updated profile XML to file %1.").arg(configFile->fileName()));
                    }
                }
            }
        }

        while (!xml->atEnd())
        {
            if (xml->isStartElement() && deviceTypes.contains(xml->name().toString()))
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
        else if (xml->hasError() && xml->error() == QXmlStreamReader::PrematureEndOfDocumentError)
        {
            xml->clear();
        }
    }

    return error;
}

QString XMLConfigReader::getErrorString()
{
    QString temp;
    if (xml->hasError())
    {
        temp = xml->errorString();
    }

    return temp;
}

bool XMLConfigReader::hasError()
{
    return xml->hasError();
}

void XMLConfigReader::initDeviceTypes()
{
    deviceTypes.clear();

    deviceTypes.append(Joystick::xmlName);
#ifdef USE_SDL_2
    deviceTypes.append(GameController::xmlName);
#endif
}
