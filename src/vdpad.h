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

#ifndef VDPAD_H
#define VDPAD_H

#include "joydpad.h"
#include "joybutton.h"

class VDPad : public JoyDPad
{
    Q_OBJECT
public:
    explicit VDPad(int index, int originset, SetJoystick *parentSet, QObject *parent = 0);
    explicit VDPad(JoyButton *upButton, JoyButton *downButton, JoyButton *leftButton, JoyButton *rightButton,
                   int index, int originset, SetJoystick *parentSet, QObject *parent = 0);
    ~VDPad();

    void joyEvent(bool pressed, bool ignoresets=false);
    void addVButton(JoyDPadButton::JoyDPadDirections direction, JoyButton *button);
    void removeVButton(JoyDPadButton::JoyDPadDirections direction);
    void removeVButton(JoyButton *button);
    JoyButton* getVButton(JoyDPadButton::JoyDPadDirections direction);
    bool isEmpty();
    virtual QString getName(bool forceFullFormat=false, bool displayName=false);
    virtual QString getXmlName();

    void queueJoyEvent(bool ignoresets=false);
    bool hasPendingEvent();
    void clearPendingEvent();

    static const QString xmlName;

protected:
    JoyButton *upButton;
    JoyButton *downButton;
    JoyButton *leftButton;
    JoyButton *rightButton;
    bool pendingVDPadEvent;

signals:

public slots:
    void activatePendingEvent();
};

#endif // VDPAD_H
