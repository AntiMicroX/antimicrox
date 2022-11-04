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
#include "haptictriggerps5.h"

#include <SDL2/SDL.h>

#pragma pack(push, 1)
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

constexpr u16 u16tole(u16 x)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return x;
#elif __BYTE_ORDER == __BIG_ENDIAN
    return ((x << 8) & 0xFF00) | ((x >> 8) & 0x00FF);
#else
    #error "Target machine has unknown endianness!"
#endif
}

constexpr u32 u32tole(u32 x)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return x;
#elif __BYTE_ORDER == __BIG_ENDIAN
    return ((x << 24) & 0xFF000000) | ((x << 8) & 0x00FF0000) | ((x >> 8) & 0x0000FF00) | ((x >> 24) & 0x000000FF);
#else
    #error "Target machine has unknown endianness!"
#endif
}

/**
 * @brief Effect message for no trigger effect.
 */
struct EffectNonePs5
{
    u8 _padding[10];

    inline void build() { memset(_padding, 0, 10); }
};

/**
 * @brief Effect message for a rigid trigger effect.
 */
struct EffectRigidPs5
{
    /// Defines if force feedback is active in the corresponding tenth of the trigger.
    /// Bitfield of 10 right aligned bits.
    u16 active_zones;
    /// Defines the force feedback strength in the corresponding tenth of the trigger.
    /// Bitfield of 30 right aligned bits. Every zone has three bits.
    u32 force_zones;
    u32 _padding;

    /**
     * @brief Builds a rigid effect message.
     * @param[in] strength Strength of the feedback force. Value between 0 and 7.
     */
    inline void build(int strength)
    {
        active_zones = u16tole(0x03FF);
        force_zones = 0;
        for (int i = 0; i < 30; i += 3)
            force_zones |= strength << i;
        force_zones = u32tole(force_zones);
        _padding = 0;
    }

    /**
     * @brief Builds a rigid gradient effect message.
     *   This effect starts with a strength of zero and slowly increases
     *   its strength until it reaches its given maximum strength at the end.
     * @param[in] strength Strength of the feedback force at the end.
     *   Value between 0 and 7.
     */
    inline void build_gradient(int strength)
    {
        active_zones = u16tole(0x03FE);
        force_zones = 0;
        double grad = 0.0;

        for (int i = 3; i < 30; i += 3)
        {
            int x = static_cast<int>((strength + 0.1) * grad);
            force_zones |= x << i;
            grad += 0.125;
        }

        force_zones = u32tole(force_zones);
        _padding = 0;
    }
};

/**
 * @brief Effect message for a click trigger effect.
 */
struct EffectClickPs5
{
    /// Bitfield with two set bits which define the start and stop zone of the effect.
    /// The trigger is devided into 9 zones.
    u16 start_stop_zone;
    /// Strength of the feedback force. Value between 0 and 7.
    u8 force;
    u8 _padding[7];

    /**
     * @brief Builds a click effect message.
     * @param[in] start Start point of the effect. Value between 2 and 7.
     *   2 is the middle of the trigger range, 7 the end.
     *   This defines the position where the resistance starts.
     * @param[in] end End point of the effect.
     *   Value between 2 and 8 which must be at least grater then start by two.
     *   2 is the middle of the trigger range, 8 the end.
     *   This defines the position where the trigger "clicks".
     * @param[in] strength Strength of the feedback force. Value between 0 and 7.
     */
    inline void build(int start, int end, int strength)
    {
        start = qBound(2, start, 6);
        end = qBound(start + 2, end, 8);

        start_stop_zone = u16tole((1 << start) | (1 << end));
        force = strength;
        memset(_padding, 0, 7);
    }
};

/**
 * @brief Effect message for a vibration trigger effect.
 */
struct EffectVibrationPs5
{
    /// Defines if force feedback is active in the corresponding tenth of the trigger.
    /// Bitfield of 10 right aligned bits.
    u16 active_zones;
    /// Defines the force feedback strength in the corresponding tenth of the trigger.
    /// Bitfield of 30 right aligned bits. Every zone has three bits.
    u32 amplitude_zones;
    u16 _padding1;
    /// Frequeny in Hz
    u8 frequency;
    u8 _padding2;

    /**
     * @brief Builds a vibration effect message.
     * @param[in] start Start point of the effect. Value between 0 and 10.
     *   0 is the beginning of the trigger range, 10 the end.
     * @param[in] end End point of the effect.
     *   Value between 2 and 10 which must be at least grater then start by two.
     * @param[in] strength Strength of the feedback force. Value between 0 and 7.
     * @param[in] frequency Virbration frequency in Hz. Value between 1 and 255.
     */
    inline void build(int start, int end, int strength, int freq)
    {
        active_zones = 0;
        amplitude_zones = 0;
        for (int i = start; i <= end; ++i)
        {
            active_zones |= (1 << i);
            amplitude_zones |= (strength << (3 * i));
        }
        active_zones = u16tole(active_zones);
        amplitude_zones = u32tole(amplitude_zones);
        _padding1 = 0;
        frequency = freq;
        _padding2 = 0;
    }
};

/**
 * @brief Effect message for a single trigger.
 */
struct TriggerEffectMsgPs5
{
    u8 mode;
    union
    {
        EffectNonePs5 none;
        EffectRigidPs5 rigid;
        EffectClickPs5 click;
        EffectVibrationPs5 vibration;
    };
};

/**
 * @brief Binary representation of a PS5 controller haptic feedback message.
 */
struct EffectMessagePs5
{
    u16 enable_bits = 0;
    u8 rumble_right = 0;
    u8 rumble_left = 0;
    u8 headphone_volume = 0;
    u8 speaker_volume = 255;
    u8 microphone_volume = 0;
    u8 audio_enable_bits = 0;
    u8 mic_light_mode = 0;
    u8 audio_mute_bits = 0;
    TriggerEffectMsgPs5 right_trigger_effect;
    TriggerEffectMsgPs5 left_trigger_effect;
    u8 _reserved1[6] = {0};
    u8 led_flags = 0;
    u8 _reverved2[2] = {0};
    u8 led_animation = 0;
    u8 led_brightness = 0;
    u8 pad_lights = 0;
    u8 led_red = 0;
    u8 led_green = 0;
    u8 led_bBlue = 0;
};
#pragma pack(pop)

HapticTriggerPs5::HapticTriggerPs5(QObject *parent, HapticTriggerModePs5 mode, int strength, int start, int end,
                                   int frequency)
    : QObject(parent)
    , m_mode(mode)
    , m_strength(strength)
    , m_start(start)
    , m_end(end)
    , m_frequency(frequency)
{
}

/**
 * @brief Returns the current haptic feedback effect mode.
 */
HapticTriggerModePs5 HapticTriggerPs5::get_mode() const { return m_mode; }

/**
 * @brief Changes the haptic feedback mode to the given type.
 * @param[in] mode New haptic feedback mode.
 * @returns True when the mode was changed, false otherwise.
 */
bool HapticTriggerPs5::set_effect_mode(HapticTriggerModePs5 mode)
{
    bool changed = m_mode != mode;
    m_mode = mode;
    return changed;
}

/**
 * @brief Changes the haptic feedback effect.
 * @param[in] strength Strength of the feedback force between 0 and 255.
 * @param[in] start Start point of the effect. Value between 0 and 320.
 * @param[in] end End point of the effect. Value between 0 and 320.
 * @param[in] frequency Frequency of the effect in Hz. Value between 1 and 255.
 * @returns True when the effect was changed, false otherwise.
 */
bool HapticTriggerPs5::set_effect(int strength, int start, int end, int frequency)
{
    strength = qBound(0, strength / 32, 7);
    start = qBound(0, start / 31, 8);
    end = qBound(0, end / 31, 10);
    frequency = qBound(1, frequency, 255);

    bool changed = m_strength != strength || m_start != start || m_end != end || m_frequency != frequency;
    m_strength = strength;
    m_start = start;
    m_end = end;
    m_frequency = frequency;
    return changed;
}

/**
 * @brief Creates an low level message from two HapticTriggerPs5 objects and
 *   send them to the controller.
 * @param[in] controller Controller to which the message is send.
 * @param[in] left HapticTriggerPs5 effect for the left trigger.
 * @param[in] right HapticTriggerPs5 effect for the right trigger.
 */
void HapticTriggerPs5::send(SDL_GameController *controller, const HapticTriggerPs5 &left, const HapticTriggerPs5 &right)
{
#if SDL_VERSION_ATLEAST(2, 0, 16)
    EffectMessagePs5 message;
    message.enable_bits |= EFFECT_LEFT_EN | EFFECT_RIGHT_EN;

    left.to_message(message.left_trigger_effect);
    right.to_message(message.right_trigger_effect);

    SDL_GameControllerSendEffect(controller, &message, sizeof(EffectMessagePs5));
#endif
}

/**
 * @brief Low level function to write one HapticTriggerPs5 effect into a PS5
 *   controller message.
 * @param[in] effect Pointer to the triggers effect data position
 *   in the effect message.
 */
void HapticTriggerPs5::to_message(TriggerEffectMsgPs5 &effect) const
{
    switch (m_mode)
    {
    case HAPTIC_TRIGGER_NONE:
        effect.mode = EFFECT_MODE_NONE;
        effect.none.build();
        return;
    case HAPTIC_TRIGGER_CLICK:
        effect.mode = EFFECT_MODE_CLICK;
        effect.click.build(m_start, m_end, m_strength);
        return;
    case HAPTIC_TRIGGER_RIGID:
        effect.mode = EFFECT_MODE_RIGID;
        effect.rigid.build(m_strength);
        return;
    case HAPTIC_TRIGGER_RIGID_GRADIENT:
        effect.mode = EFFECT_MODE_RIGID;
        effect.rigid.build_gradient(m_strength);
        return;
    case HAPTIC_TRIGGER_VIBRATION:
        effect.mode = EFFECT_MODE_VIBRATION;
        effect.vibration.build(m_start, m_end, m_strength, m_frequency);
        return;
    }
}

/**
 * @brief Converts a HapticTriggerModePs5 from string representation.
 */
HapticTriggerModePs5 HapticTriggerPs5::from_string(const QString &name)
{
    HapticTriggerModePs5 mode = HAPTIC_TRIGGER_NONE;
    if (name == "None")
    {
        mode = HAPTIC_TRIGGER_NONE;
    } else if (name == "Click")
    {
        mode = HAPTIC_TRIGGER_CLICK;
    } else if (name == "Rigid")
    {
        mode = HAPTIC_TRIGGER_RIGID;
    } else if (name == "RigidGradient")
    {
        mode = HAPTIC_TRIGGER_RIGID_GRADIENT;
    } else if (name == "Vibration")
    {
        mode = HAPTIC_TRIGGER_VIBRATION;
    }
    return mode;
}

/**
 * @brief Returns string representation of a HapticTriggerModePs5 object.
 */
QString HapticTriggerPs5::to_string(HapticTriggerModePs5 mode)
{
    switch (mode)
    {
    case HAPTIC_TRIGGER_NONE:
        return "None";
    case HAPTIC_TRIGGER_CLICK:
        return "Click";
    case HAPTIC_TRIGGER_RIGID:
        return "Rigid";
    case HAPTIC_TRIGGER_RIGID_GRADIENT:
        return "RigidGradient";
    case HAPTIC_TRIGGER_VIBRATION:
        return "Vibration";
    }
    return "None";
}
