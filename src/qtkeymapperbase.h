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

#ifndef QTKEYMAPPERBASE_H
#define QTKEYMAPPERBASE_H

#include <QHash>
#include <QObject>

/**
 * @brief Base class for platform-specific Qt key mappers.
 *
 * QtKeyMapperBase provides an abstract API for mapping between Qt key
 * constants (Qt::Key_*) and platform-specific virtual key codes (for
 * example X11 keysyms). Subclasses implement the mapping population
 * and character information routines and use the provided hash tables
 * to perform lookups.
 *
 * Typical usage:
 *  - Subclasses populate mappings by implementing populateMappingHashes()
 *    and populateCharKeyInformation(), usually from the constructor.
 *  - Call returnVirtualKey() to obtain a platform virtual key for a
 *    Qt key.
 *  - Call returnQtKey() to obtain the Qt key for a platform virtual key.
 */
class QtKeyMapperBase : public QObject
{
    Q_OBJECT

  public:
    explicit QtKeyMapperBase(QObject *parent = nullptr);

    /**
     * @brief Information about a printable character key.
     *
     * Contains the required Qt modifiers to produce the character and the
     * platform virtual keycode that generates it.
     */
    typedef struct _charKeyInformation
    {
        Qt::KeyboardModifiers modifiers; /**< Required modifiers (Shift/AltGr/etc.) */
        int virtualkey;                  /**< Platform-specific virtual keycode */
    } charKeyInformation;

    /**
     * @brief Get platform virtual key for a given Qt key.
     * @param qkey Qt::Key_* value (may include custom prefixes)
     * @return Platform virtual key code, or 0 if unmapped.
     */
    virtual int returnVirtualKey(int qkey);

    /**
     * @brief Get Qt key for a given platform virtual key.
     * @param key Platform virtual key code
     * @param scancode Optional scancode (unused by default)
     * @return Qt::Key_* value, or 0 if unmapped.
     */
    virtual int returnQtKey(int key, int scancode = 0);

    /**
     * @brief Check whether the given Qt key is a modifier key.
     * @param qkey Qt key value
     * @return true if the key is a modifier (Shift, Control, Alt, Meta)
     */
    virtual bool isModifier(int qkey);

    /**
     * @brief Get key information (virtual key + modifiers) for a character.
     * @param value Unicode character to query
     * @return charKeyInformation with modifier flags and virtual key; if not
     *         found, a struct with virtualkey == 0 and NoModifier is returned.
     */
    charKeyInformation getCharKeyInformation(QChar value);

    /**
     * @brief Identifier string for this mapper implementation.
     * @return Identifier (e.g. "xtest").
     */
    QString getIdentifier();

    /** Prefix values for constructing custom key values. */
    static const int customQtKeyPrefix = 0x10000000;
    static const int customKeyPrefix = 0x20000000;
    static const int nativeKeyPrefix = 0x60000000;

    /** Synthetic/custom key values used within the codebase. */
    enum
    {
        AntKey_Shift_R = Qt::Key_Shift | customQtKeyPrefix,
        AntKey_Control_R = Qt::Key_Control | customQtKeyPrefix,
        AntKey_Shift_Lock = 0xffe6 | customKeyPrefix, // XK_Shift_Lock | 0x20000000
        AntKey_Meta_R = Qt::Key_Meta | customQtKeyPrefix,
        AntKey_Alt_R = Qt::Key_Alt | customQtKeyPrefix,
        AntKey_KP_Divide = Qt::Key_Slash | customQtKeyPrefix,
        AntKey_KP_Multiply = Qt::Key_Asterisk | customQtKeyPrefix,
        AntKey_KP_Subtract = Qt::Key_Minus | customQtKeyPrefix,
        AntKey_KP_Add = Qt::Key_Plus | customQtKeyPrefix,
        AntKey_KP_Decimal = Qt::Key_Period | customQtKeyPrefix,
        AntKey_KP_Insert = Qt::Key_Insert | customQtKeyPrefix,
        AntKey_Delete = Qt::Key_Delete | customQtKeyPrefix,
        AntKey_KP_Delete = 0xff9f | customKeyPrefix,
        AntKey_KP_End = Qt::Key_End | customQtKeyPrefix,
        AntKey_KP_Down = Qt::Key_Down | customQtKeyPrefix,
        AntKey_KP_Prior = Qt::Key_PageUp | customQtKeyPrefix,
        AntKey_KP_Left = Qt::Key_Left | customQtKeyPrefix,
        AntKey_KP_Begin = Qt::Key_Clear | customQtKeyPrefix,
        AntKey_KP_Right = Qt::Key_Right | customQtKeyPrefix,
        AntKey_KP_Home = Qt::Key_Home | customQtKeyPrefix,
        AntKey_KP_Up = Qt::Key_Up | customQtKeyPrefix,
        AntKey_KP_Next = Qt::Key_PageDown | customQtKeyPrefix,
        AntKey_KP_Enter = 0xff8d | customKeyPrefix,
        AntKey_KP_0 = 0xffb0 | customKeyPrefix,
        AntKey_KP_1 = 0xffb1 | customKeyPrefix,
        AntKey_KP_2 = 0xffb2 | customKeyPrefix,
        AntKey_KP_3 = 0xffb3 | customKeyPrefix,
        AntKey_KP_4 = 0xffb4 | customKeyPrefix,
        AntKey_KP_5 = 0xffb5 | customKeyPrefix,
        AntKey_KP_6 = 0xffb6 | customKeyPrefix,
        AntKey_KP_7 = 0xffb7 | customKeyPrefix,
        AntKey_KP_8 = 0xffb8 | customKeyPrefix,
        AntKey_KP_9 = 0xffb9 | customKeyPrefix
    };

  protected:
    /**
     * @brief Populate the Qt <-> platform virtual key mappings.
     * Implementations must fill `qtKeyToVirtKeyHash` (Qt -> VK) and
     * `virtKeyToQtKeyHash` (VK -> Qt).
     */
    virtual void populateMappingHashes() = 0;

    /**
     * @brief Populate `virtkeyToCharKeyInfo` with character -> VK+modifier info.
     */
    virtual void populateCharKeyInformation() = 0;

    /** Map from Qt key to platform virtual key */
    QHash<int, int> qtKeyToVirtKeyHash;
    /** Map from platform virtual key to Qt key */
    QHash<int, int> virtKeyToQtKeyHash;
    /** Unicode code point -> charKeyInformation (VK + modifiers) */
    QHash<int, charKeyInformation> virtkeyToCharKeyInfo; // Unicode representation -> VK+Modifier information
    /** Short identifier for this mapper (set by subclasses) */
    QString identifier;
};

#endif // QTKEYMAPPERBASE_H
