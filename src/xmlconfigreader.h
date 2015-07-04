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

#ifndef XMLCONFIGREADER_H
#define XMLCONFIGREADER_H

#include <QObject>
#include <QXmlStreamReader>
#include <QFile>

#include "inputdevice.h"
#include "joystick.h"

#ifdef USE_SDL_2
#include "gamecontroller/gamecontroller.h"
#endif

#include "common.h"

class XMLConfigReader : public QObject
{
    Q_OBJECT
public:
    explicit XMLConfigReader(QObject *parent = 0);
    ~XMLConfigReader();
    void setJoystick(InputDevice *joystick);
    void setFileName(QString filename);
    QString getErrorString();
    bool hasError();
    bool read();

protected:
    void initDeviceTypes();

    QXmlStreamReader *xml;
    QString fileName;
    QFile *configFile;
    InputDevice* joystick;
    QStringList deviceTypes;

signals:
    
public slots:
    void configJoystick(InputDevice *joystick);

};

#endif // XMLCONFIGREADER_H
