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


#ifndef SPRINGMODEREGIONPREVIEW_H
#define SPRINGMODEREGIONPREVIEW_H

#include <QWidget>

class QPaintEvent;


class SpringModeRegionPreview : public QWidget
{
    Q_OBJECT

public:
    explicit SpringModeRegionPreview(int width = 0, int height = 0, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event);

    int adjustSpringSizeWidth(int width);
    int adjustSpringSizeHeight(int height);

public slots:
    void setSpringWidth(int width);
    void setSpringHeight(int height);
    void setSpringSize(int width, int height);
};

#endif // SPRINGMODEREGIONPREVIEW_H
