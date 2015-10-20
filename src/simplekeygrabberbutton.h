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

#ifndef SIMPLEKEYGRABBERBUTTON_H
#define SIMPLEKEYGRABBERBUTTON_H

#include <QPushButton>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QMetaType>

#include "joybuttonslot.h"

class SimpleKeyGrabberButton : public QPushButton
{
    Q_OBJECT
public:
    explicit SimpleKeyGrabberButton(QWidget *parent = 0);

    void setValue(int value, unsigned int alias, JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard);
    void setValue(int value, JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard);
    void setValue(QString value, JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyLoadProfile);
    JoyButtonSlot* getValue();
    bool isEdited();
    bool isGrabbing();

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual bool eventFilter(QObject *obj, QEvent *event);

    bool grabNextAction;
    bool grabbingWheel;
    bool edited;
    JoyButtonSlot buttonslot;

signals:
    void buttonCodeChanged(int value);
    
public slots:
    void refreshButtonLabel();

protected slots:

};

Q_DECLARE_METATYPE(SimpleKeyGrabberButton*)

#endif // SIMPLEKEYGRABBERBUTTON_H
