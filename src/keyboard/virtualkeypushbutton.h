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

#ifndef VIRTUALKEYPUSHBUTTON_H
#define VIRTUALKEYPUSHBUTTON_H

#include <QPushButton>
#include <QString>
#include <QHash>

#include <joybutton.h>

class VirtualKeyPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit VirtualKeyPushButton(JoyButton *button, QString xcodestring, QWidget *parent = 0);
    int calculateFontSize();
    
protected:
    int keycode;
    unsigned int qkeyalias;
    QString xcodestring;
    QString displayString;
    bool currentlyActive;
    bool onCurrentButton;
    JoyButton *button;
    static QHash<QString, QString> knownAliases;

    QString setDisplayString(QString xcodestring);
    void populateKnownAliases();

signals:
    void keycodeObtained(int code, unsigned int alias);

public slots:

private slots:
    void processSingleSelection();
};

#endif // VIRTUALKEYPUSHBUTTON_H
