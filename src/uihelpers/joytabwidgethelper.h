/* antimicroX Gamepad to KB+M event mapper
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


#ifndef JOYTABWIDGETHELPER_H
#define JOYTABWIDGETHELPER_H

#include <QObject>

class InputDevice;
class XMLConfigReader;
class XMLConfigWriter;

class JoyTabWidgetHelper : public QObject
{
    Q_OBJECT

public:
    explicit JoyTabWidgetHelper(InputDevice *device, QObject *parent = nullptr);
    ~JoyTabWidgetHelper();

    bool hasReader();
    XMLConfigReader* getReader();

    bool hasWriter();
    XMLConfigWriter* getWriter();

    bool hasError();
    QString getErrorString();

protected:
    InputDevice *device;
    XMLConfigReader *reader;
    XMLConfigWriter *writer;
    bool errorOccurred;
    QString lastErrorString;

public slots:
    bool readConfigFile(QString filepath);
    bool readConfigFileWithRevert(QString filepath);
    bool writeConfigFile(QString filepath);
    void reInitDevice();
    void reInitDeviceWithRevert();
};

#endif // JOYTABWIDGETHELPER_H
