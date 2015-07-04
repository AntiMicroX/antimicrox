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

#ifndef JOYAXISCONTEXTMENU_H
#define JOYAXISCONTEXTMENU_H

#include <QMenu>

#include "joyaxis.h"

class JoyAxisContextMenu : public QMenu
{
    Q_OBJECT
public:
    explicit JoyAxisContextMenu(JoyAxis *axis, QWidget *parent = 0);
    void buildMenu();
    void buildAxisMenu();
    void buildTriggerMenu();

protected:
    int getPresetIndex();
    int getTriggerPresetIndex();

    JoyAxis *axis;

signals:

public slots:

private slots:
    void setAxisPreset();
    void setTriggerPreset();
    void openMouseSettingsDialog();
};

#endif // JOYAXISCONTEXTMENU_H
