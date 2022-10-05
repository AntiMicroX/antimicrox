/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2022 Max Maisel <max.maisel@posteo.de>
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
#pragma once

#include <QObject>
#include <QString>
#include <SDL2/SDL_gamecontroller.h>
#include <stdint.h>

#include "haptictriggermodeps5.h"

struct TriggerEffectMsgPs5;

/**
 * @brief Represents a single haptic trigger effect on a PS5 controller.
 *   See https://gist.github.com/Nielk1/6d54cc2c00d2201ccb8c2720ad7538db
 *   and https://github.com/Electronicks/JoyShockMapper/blob/master/JoyShockMapper/src/SDL2Wrapper.cpp
 */
class HapticTriggerPs5 : QObject
{
  public:
    explicit HapticTriggerPs5(QObject *parent, HapticTriggerModePs5 mode = HAPTIC_TRIGGER_NONE, int strength = 0,
                              int start = 0, int end = 0, int frequency = 0);

    HapticTriggerModePs5 get_mode() const;
    bool set_effect_mode(HapticTriggerModePs5 mode);
    bool set_effect(int strength, int start, int end, int frequency = 0);

    static void send(SDL_GameController *controller, const HapticTriggerPs5 &left, const HapticTriggerPs5 &right);
    static HapticTriggerModePs5 from_string(const QString &mode);
    static QString to_string(HapticTriggerModePs5 mode);

  private:
    enum
    {
        EFFECT_LEFT_EN = 0x04,
        EFFECT_RIGHT_EN = 0x08,
        LEGACY_RUMBLE_LEFT_EN = 0x0001,
        LEGACY_RUMBLE_RIGHT_EN = 0x0002,
        MICROPHONE_LIGHT_EN = 0x0100,

        MICROPHONE_LIGHT_OFF = 0x00,
        MICROPHONE_LIGHT_SOLID = 0x01,
        MICROPHONE_LIGHT_PULSE = 0x02
    };

    /**
     * @brief Enum of the supported low level haptic trigger effect modes of
     *   the PS5 controller.
     *   See https://gist.github.com/Nielk1/6d54cc2c00d2201ccb8c2720ad7538db
     */
    enum
    {
        EFFECT_MODE_NONE = 0x05,
        EFFECT_MODE_CLICK = 0x25,
        EFFECT_MODE_RIGID = 0x21,
        EFFECT_MODE_VIBRATION = 0x26
    };

    HapticTriggerModePs5 m_mode;
    int m_strength;
    int m_start;
    int m_end;
    int m_frequency;

    void to_message(TriggerEffectMsgPs5 &effect) const;
};
