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


#ifndef JOYAXISCONTEXTMENU_H
#define JOYAXISCONTEXTMENU_H



#include "uihelpers/joyaxiscontextmenuhelper.h"

#include <QMenu>

class JoyAxis;
class QWidget;

class JoyAxisContextMenu : public QMenu
{
    Q_OBJECT

public:
    explicit JoyAxisContextMenu(JoyAxis *axis, QWidget *parent = nullptr);
    void buildMenu();
    void buildAxisMenu();
    void buildTriggerMenu();

protected:
    int getPresetIndex();
    int getTriggerPresetIndex();

private slots:
    void setAxisPreset(QAction* action);
    void setTriggerPreset(QAction* action);
    void openMouseSettingsDialog();

private:
    JoyAxisContextMenuHelper& getHelperLocal();

    JoyAxis *axis;
    JoyAxisContextMenuHelper helper;
};

#endif // JOYAXISCONTEXTMENU_H
