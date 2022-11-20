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

#include "joytabwidgethelper.h"

#include "inputdevice.h"
#include "joybuttonslot.h"
#include "joybuttontypes/joybutton.h"
#include "logger.h"
#include "xml/inputdevicexml.h"
#include "xmlconfigreader.h"
#include "xmlconfigwriter.h"

#include <QDebug>

JoyTabWidgetHelper::JoyTabWidgetHelper(InputDevice *device, QObject *parent)
    : QObject(parent)
{
    Q_ASSERT(device);

    this->device = device;
    this->reader = nullptr;
    this->writer = nullptr;
    this->errorOccurred = false;
}

JoyTabWidgetHelper::~JoyTabWidgetHelper()
{
    if (this->reader != nullptr)
    {
        delete this->reader;
        this->reader = nullptr;
    }

    if (this->writer != nullptr)
    {
        delete this->writer;
        this->writer = nullptr;
    }
}

bool JoyTabWidgetHelper::hasReader() { return (this->reader != nullptr); }

XMLConfigReader *JoyTabWidgetHelper::getReader() { return this->reader; }

bool JoyTabWidgetHelper::hasWriter() { return (this->writer != nullptr); }

XMLConfigWriter *JoyTabWidgetHelper::getWriter() { return this->writer; }

bool JoyTabWidgetHelper::hasError() { return errorOccurred; }

QString JoyTabWidgetHelper::getErrorString() { return lastErrorString; }

/**
 * @brief XML read entry point for the GUI
 */
bool JoyTabWidgetHelper::readConfigFile(QString filepath)
{
    bool result = false;
    device->disconnectPropertyUpdatedConnection();

    if (device->getActiveSetNumber() != 0)
    {
        device->setActiveSetNumber(0);
    }

    device->resetButtonDownCount();
    if (this->reader != nullptr)
    {
        this->reader->deleteLater();
        this->reader = nullptr;
    }

    this->reader = new XMLConfigReader;
    this->reader->setFileName(filepath);
    this->reader->configJoystick(device);

    device->establishPropertyUpdatedConnection();

    result = !this->reader->hasError();
    VERBOSE() << "Loading config file: " << filepath << (result ? " succeeded." : " failed.");
    return result;
}

bool JoyTabWidgetHelper::readConfigFileWithRevert(QString filepath)
{
    device->revertProfileEdited();

    return readConfigFile(filepath);
}

/**
 * @brief XML write entry point for the GUI
 */
bool JoyTabWidgetHelper::writeConfigFile(QString filepath)
{
    bool result = false;

    if (this->writer != nullptr)
    {
        this->writer->deleteLater();
        this->writer = nullptr;
    }

    this->writer = new XMLConfigWriter;
    this->writer->setFileName(filepath);
    InputDeviceXml *deviceXml = new InputDeviceXml(device);
    this->writer->write(deviceXml);
    delete deviceXml;

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
