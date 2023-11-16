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

#ifndef JOYBUTTONCONTEXTMENU_H
#define JOYBUTTONCONTEXTMENU_H

#include <QActionGroup>
#include <QMenu>

class JoyButton;
class QWidget;

class JoyButtonContextMenu : public QMenu
{
    Q_OBJECT

  public:
    explicit JoyButtonContextMenu(JoyButton *button, QWidget *parent = nullptr);
    void buildMenu();

  protected:
    JoyButton *button;

  private slots:
    void switchToggle();
    void switchTurbo();
    void switchSetMode(QAction *action);
    void disableSetMode();
    void clearButton();
    void createActionForGroup(QActionGroup *tempGroup, QString actionText, QAction *action, QMenu *tempSetMenu,
                              int setSelection, int currentSelection, int setDataInc, int setCondition);
};

#endif // JOYBUTTONCONTEXTMENU_H
