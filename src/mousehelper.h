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

#ifndef MOUSEHELPER_H
#define MOUSEHELPER_H

#include <QObject>
#include <QTimer>
#include <QDesktopWidget>

class MouseHelper : public QObject
{
    Q_OBJECT
public:
    explicit MouseHelper(QObject *parent = 0);
    QDesktopWidget* getDesktopWidget();

    bool springMouseMoving;
    int previousCursorLocation[2];
    int pivotPoint[2];
    QTimer mouseTimer;
    QDesktopWidget *deskWid;

signals:

public slots:
    void deleteDeskWid();
    void initDeskWid();
    
private slots:
    void resetSpringMouseMoving();
};

#endif // MOUSEHELPER_H
