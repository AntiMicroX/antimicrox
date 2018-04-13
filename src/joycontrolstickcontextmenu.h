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

#ifndef JOYCONTROLSTICKCONTEXTMENU_H
#define JOYCONTROLSTICKCONTEXTMENU_H


#include "uihelpers/joycontrolstickcontextmenuhelper.h"

#include <QMenu>

class JoyControlStick;
class QWidget;

class JoyControlStickContextMenu : public QMenu
{
    Q_OBJECT
public:
    explicit JoyControlStickContextMenu(JoyControlStick *stick, QWidget *parent = nullptr);
    void buildMenu();

protected:
    int getPresetIndex();

    JoyControlStick *stick;
    JoyControlStickContextMenuHelper helper;

private slots:
    void setStickPreset();
    void setStickMode();
    void openMouseSettingsDialog();
};

#endif // JOYCONTROLSTICKCONTEXTMENU_H
