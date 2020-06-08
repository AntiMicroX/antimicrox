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

#ifndef QTX11KEYMAPPER_H
#define QTX11KEYMAPPER_H

#include "qtkeymapperbase.h"


class QtX11KeyMapper : public QtKeyMapperBase
{
    Q_OBJECT

public:
    explicit QtX11KeyMapper(QObject *parent = nullptr);

protected:
    void populateMappingHashes() override;
    void populateCharKeyInformation() override;

private:
    void mapMiscKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash);
    void mapCursorMovesKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash);
    void mapModifiersKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash);
    void mapAdditionalKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash);
    void mapKeypadKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash);
    void mapInternationKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash);
    void mapJapanKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash);
    void mapKoreanKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash);
    void mapDeadKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash);
    void mapBrowseKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash);
    void mapMediaKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash);
    void mapLaunchKeysQtX11(QHash<int, int>& qtKeyToVirtKeyHash);

};

#endif // QTX11KEYMAPPER_H
