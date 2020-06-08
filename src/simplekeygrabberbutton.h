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


#ifndef SIMPLEKEYGRABBERBUTTON_H
#define SIMPLEKEYGRABBERBUTTON_H

#include "joybuttonslot.h"

#include <QPushButton>


class QKeyEvent;
class QEvent;
class QWidget;

class SimpleKeyGrabberButton : public QPushButton
{
    Q_OBJECT

public:
    explicit SimpleKeyGrabberButton(QWidget *parent = nullptr);

    void setValue(int value, int alias, JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard); // (.., unsigned)
    void setValue(int value, JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyKeyboard);
    void setValue(QString value, JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyLoadProfile);
    void setValues(QString value, JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyMix);
    void setValues(QString value, QList<JoyButtonSlot *> *jbtn, JoyButtonSlot::JoySlotInputAction mode=JoyButtonSlot::JoyMix);
    JoyButtonSlot* getValue();
    JoyButtonSlot& getValueNonPointer();
    void setValue(JoyButtonSlot* jbS);
    bool isEdited();
    bool isGrabbing();


protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual bool eventFilter(QObject *obj, QEvent *event);

signals:
    void buttonCodeChanged(int value);
    
public slots:
    void refreshButtonLabel();

private:
    bool grabNextAction;
    bool grabbingWheel;
    bool edited;
    JoyButtonSlot buttonslot;

};

Q_DECLARE_METATYPE(SimpleKeyGrabberButton*)

#endif // SIMPLEKEYGRABBERBUTTON_H
