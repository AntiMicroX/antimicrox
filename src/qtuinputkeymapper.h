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


#ifndef QTUINPUTKEYMAPPER_H
#define QTUINPUTKEYMAPPER_H

#include "qtkeymapperbase.h"


class QtUInputKeyMapper : public QtKeyMapperBase
{
    Q_OBJECT

public:
    explicit QtUInputKeyMapper(QObject *parent = nullptr);

protected:
    void populateMappingHashes() override;
    void populateCharKeyInformation() override;

    void populateAlphaHashes();
    void populateFKeyHashes();
    void populateNumPadHashes();
    void populateSpecialCharHashes();

private:
    void addVirtualKeyToHash(int key, QChar character, charKeyInformation& charKeyInfo);
    void mapMiscKeysQtUinput(QHash<int, int>& qtKeyToVirtKeyHash);
    void mapCursorMovesKeysQtUinput(QHash<int, int>& qtKeyToVirtKeyHash);
    void mapModifiersKeysQtUinput(QHash<int, int>& qtKeyToVirtKeyHash);
    void mapKeypadKeysQtUinput(QHash<int, int>& qtKeyToVirtKeyHash);
    void mapJapanKeysQtUinput(QHash<int, int>& qtKeyToVirtKeyHash);
    void mapKoreanKeysQtUinput(QHash<int, int>& qtKeyToVirtKeyHash);
    void mapMediaKeysQtUinput(QHash<int, int>& qtKeyToVirtKeyHash);

};

#endif // QTUINPUTKEYMAPPER_H
