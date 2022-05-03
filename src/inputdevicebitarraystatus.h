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

#ifndef INPUTDEVICESTATUSEVENT_H
#define INPUTDEVICESTATUSEVENT_H

#include <QBitArray>
#include <QList>
#include <QObject>

class InputDevice;

class InputDeviceBitArrayStatus : public QObject
{
    Q_OBJECT

  public:
    explicit InputDeviceBitArrayStatus(InputDevice *device, bool readCurrent, QObject *parent);

    void changeAxesStatus(int axisIndex, bool value);
    void changeButtonStatus(int buttonIndex, bool value);
    void changeHatStatus(int hatIndex, bool value);
    void changeSensorStatus(int sensorIndex, bool value);

    QBitArray generateFinalBitArray();
    void clearStatusValues();

  private:
    QBitArray &getButtonStatusLocal();

    QList<bool> axesStatus;
    QList<bool> hatButtonStatus;
    QBitArray buttonStatus;
    QBitArray m_sensor_status;
};

#endif // INPUTDEVICESTATUSEVENT_H
