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

#ifndef QTKEYMAPPERBASE_H
#define QTKEYMAPPERBASE_H

#include <QObject>
#include <QHash>

class QtKeyMapperBase : public QObject
{
    Q_OBJECT
public:
    explicit QtKeyMapperBase(QObject *parent = 0);

    typedef struct _charKeyInformation
    {
        Qt::KeyboardModifiers modifiers;
        unsigned int virtualkey;
    } charKeyInformation;

    virtual unsigned int returnVirtualKey(unsigned int qkey);
    virtual unsigned int returnQtKey(unsigned int key, unsigned int scancode=0);
    virtual bool isModifier(unsigned int qkey);
    charKeyInformation getCharKeyInformation(QChar value);
    QString getIdentifier();

    static const unsigned int customQtKeyPrefix = 0x10000000;
    static const unsigned int customKeyPrefix = 0x20000000;
    static const unsigned int nativeKeyPrefix = 0x60000000;

    enum {
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
        AntKey_KP_Delete = Qt::Key_Delete | customQtKeyPrefix,
        AntKey_KP_End = Qt::Key_End | customQtKeyPrefix,
        AntKey_KP_Down = Qt::Key_Down | customQtKeyPrefix,
        AntKey_KP_Prior = Qt::Key_PageDown | customQtKeyPrefix,
        AntKey_KP_Left = Qt::Key_Left | customQtKeyPrefix,
        AntKey_KP_Begin = Qt::Key_Clear | customQtKeyPrefix,
        AntKey_KP_Right = Qt::Key_Right | customQtKeyPrefix,
        AntKey_KP_Home = Qt::Key_Home | customQtKeyPrefix,
        AntKey_KP_Up = Qt::Key_Up | customQtKeyPrefix,
        AntKey_KP_Next = Qt::Key_PageUp | customQtKeyPrefix,
        AntKey_KP_0 = Qt::Key_0 | customQtKeyPrefix,
        AntKey_KP_1 = Qt::Key_1 | customQtKeyPrefix,
        AntKey_KP_2 = Qt::Key_2 | customQtKeyPrefix,
        AntKey_KP_3 = Qt::Key_3 | customQtKeyPrefix,
        AntKey_KP_4 = Qt::Key_4 | customQtKeyPrefix,
        AntKey_KP_5 = Qt::Key_5 | customQtKeyPrefix,
        AntKey_KP_6 = Qt::Key_6 | customQtKeyPrefix,
        AntKey_KP_7 = Qt::Key_7 | customQtKeyPrefix,
        AntKey_KP_8 = Qt::Key_8 | customQtKeyPrefix,
        AntKey_KP_9 = Qt::Key_9 | customQtKeyPrefix
    };

protected:
    virtual void populateMappingHashes() = 0;
    virtual void populateCharKeyInformation() = 0;

    QHash<unsigned int, unsigned int> qtKeyToVirtualKey;
    QHash<unsigned int, unsigned int> virtualKeyToQtKey;
    // Unicode representation -> VK+Modifier information
    QHash<unsigned int, charKeyInformation> virtualkeyToCharKeyInformation;
    QString identifier;

signals:

public slots:

};

#endif // QTKEYMAPPERBASE_H
