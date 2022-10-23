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

#ifndef XMLCONFIGREADER_H
#define XMLCONFIGREADER_H

#include <QObject>
#include <QStringList>

class InputDevice;
class QXmlStreamReader;
class InputDeviceXml;
class QFile;

/**
 * @brief Main XML config reader class
 */
class XMLConfigReader : public QObject
{
    Q_OBJECT

  public:
    explicit XMLConfigReader(QObject *parent = nullptr);
    ~XMLConfigReader();
    void setJoystick(InputDevice *joystick);
    void setFileName(QString filename);
    const QString getErrorString();
    bool hasError();
    bool read();

    const QXmlStreamReader *getXml();
    QString const &getFileName();
    const QFile *getConfigFile();
    const InputDevice *getJoystick();
    QStringList const &getDeviceTypes();

  protected:
    void initDeviceTypes();

  public slots:
    void configJoystick(InputDevice *joystick);

  private:
    QXmlStreamReader *xml;
    QString fileName;
    QFile *configFile;
    InputDevice *m_joystick;
    QStringList deviceTypes;
};

#endif // XMLCONFIGREADER_H
