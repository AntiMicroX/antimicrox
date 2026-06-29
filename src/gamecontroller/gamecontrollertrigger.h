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

#ifndef GAMECONTROLLERTRIGGER_H
#define GAMECONTROLLERTRIGGER_H

#include "joyaxis.h"

class HapticTriggerPs5;
class QXmlStreamReader;
class QXmlStreamWriter;
class SetJoystick;

class GameControllerTrigger : public JoyAxis
{
    Q_OBJECT

  public:
    explicit GameControllerTrigger(int index, int originset, SetJoystick *parentSet, QObject *parent = nullptr);

    QString getXmlName() override; // GameControllerTriggerXml class
    QString getPartialName(bool forceFullFormat, bool displayNames) override;

    int getDefaultDeadZone() override;
    int getDefaultMaxZone() override;
    ThrottleTypes getDefaultThrottle() override;

    static const ThrottleTypes DEFAULTTHROTTLE;

    bool hasHapticTrigger() const override;
    HapticTriggerPs5 *getHapticTrigger() const override;
    void setHapticTriggerMode(HapticTriggerModePs5 mode) override;

  public slots:
    void reset() override;
    void reset(int index) override;

  protected:
    void correctJoystickThrottle();

    HapticTriggerPs5 *m_haptic_trigger;
};

#endif // GAMECONTROLLERTRIGGER_H
