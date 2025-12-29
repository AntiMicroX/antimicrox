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

#ifndef ANTKEYMAPPER_H
#define ANTKEYMAPPER_H

#include <QObject>

#ifdef Q_OS_WIN
    #include "qtwinkeymapper.h"
#else
    #ifdef WITH_XTEST
        #include "qtx11keymapper.h"
    #endif

    #ifdef WITH_UINPUT
        #include "qtuinputkeymapper.h"
    #endif
#endif

/**
 * @brief Singleton facade for platform key mapping selection.
 *
 * AntKeyMapper chooses and exposes an appropriate platform-specific
 * key mapper implementation (a subclass of `QtKeyMapperBase`) based on
 * the selected event generator (e.g. "xtest", "uinput" or "sendinput").
 * It provides convenient wrappers to translate between Qt keys and
 * platform virtual keys and to query whether a native mapper is
 * available.
 */
class AntKeyMapper : public QObject
{
    Q_OBJECT

  public:
    /**
     * @brief Obtain the singleton instance.
     * @param handler Name of the backend handler to use (must be one of
     *                the supported event generators). If an instance does
     *                not yet exist it will be created using this handler.
     * @return Pointer to the global AntKeyMapper instance.
     */
    static AntKeyMapper *getInstance(QString handler = "");
    void deleteInstance();

    /**
     * @brief Translate a Qt key to a platform virtual key code.
     * @param qkey Qt::Key_* value
     * @return Platform virtual key (or 0 if no mapping exists)
     */
    int returnVirtualKey(int qkey);

    /**
     * @brief Translate a platform virtual key to a Qt key.
     * @param key Platform virtual key code
     * @param scancode Optional scancode used by some backends
     * @return Qt::Key_* value (or 0 if no mapping exists)
     */
    int returnQtKey(int key, int scancode = 0);

    /**
     * @brief Check if a given Qt key is treated as a modifier key.
     * @param qkey Qt key value
     * @return true if the key is a modifier (Shift, Control, Alt, Meta)
     */
    bool isModifierKey(int qkey);
    QtKeyMapperBase *getNativeKeyMapper() const;
    QtKeyMapperBase *getKeyMapper() const;
    bool hasNativeKeyMapper();

  protected:
    static AntKeyMapper *_instance;

  private:
    explicit AntKeyMapper(QString handler = "", QObject *parent = nullptr);

    QtKeyMapperBase *internalMapper;  /**< Active mapper used by the app */
    QtKeyMapperBase *nativeKeyMapper; /**< Optional native mapper for low-level queries */
};

#endif // ANTKEYMAPPER_H
