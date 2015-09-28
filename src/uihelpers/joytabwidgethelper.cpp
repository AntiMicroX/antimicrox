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

#include "joytabwidgethelper.h"

JoyTabWidgetHelper::JoyTabWidgetHelper(InputDevice *device, QObject *parent) :
    QObject(parent)
{
    Q_ASSERT(device);

    this->device = device;
    this->reader = 0;
    this->writer = 0;
    this->errorOccurred = false;
}

JoyTabWidgetHelper::~JoyTabWidgetHelper()
{
    if (this->reader)
    {
        delete this->reader;
        this->reader = 0;
    }

    if (this->writer)
    {
        delete this->writer;
        this->writer = 0;
    }
}

bool JoyTabWidgetHelper::hasReader()
{
    return (this->reader != 0);
}

XMLConfigReader* JoyTabWidgetHelper::getReader()
{
    return this->reader;
}

bool JoyTabWidgetHelper::hasWriter()
{
    return (this->writer != 0);
}

XMLConfigWriter* JoyTabWidgetHelper::getWriter()
{
    return this->writer;
}

bool JoyTabWidgetHelper::hasError()
{
    return errorOccurred;
}

QString JoyTabWidgetHelper::getErrorString()
{
    return lastErrorString;
}

bool JoyTabWidgetHelper::readConfigFile(QString filepath)
{
    bool result = false;
    device->disconnectPropertyUpdatedConnection();

    if (device->getActiveSetNumber() != 0)
    {
        device->setActiveSetNumber(0);
    }

    device->resetButtonDownCount();
    if (this->reader)
    {
        this->reader->deleteLater();
        this->reader = 0;
    }

    this->reader = new XMLConfigReader;
    this->reader->setFileName(filepath);
    this->reader->configJoystick(device);

    device->establishPropertyUpdatedConnection();

    result = !this->reader->hasError();
    return result;
}

bool JoyTabWidgetHelper::readConfigFileWithRevert(QString filepath)
{
    bool result = false;
    device->revertProfileEdited();

    result = readConfigFile(filepath);

    return result;
}

bool JoyTabWidgetHelper::writeConfigFile(QString filepath)
{
    bool result = false;

    if (this->writer)
    {
        this->writer->deleteLater();
        this->writer = 0;
    }

    this->writer = new XMLConfigWriter;
    this->writer->setFileName(filepath);
    this->writer->write(device);

    result = !this->writer->hasError();
    return result;
}

void JoyTabWidgetHelper::reInitDevice()
{
    device->disconnectPropertyUpdatedConnection();

    if (device->getActiveSetNumber() != 0)
    {
        device->setActiveSetNumber(0);
    }

    device->transferReset();
    device->resetButtonDownCount();
    device->reInitButtons();


    device->establishPropertyUpdatedConnection();
}

void JoyTabWidgetHelper::reInitDeviceWithRevert()
{
    device->revertProfileEdited();
    reInitDevice();
}
