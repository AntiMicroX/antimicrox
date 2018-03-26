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

#ifndef UINPUTHELPER_H
#define UINPUTHELPER_H

#include <QObject>
#include <QString>
#include <QHash>

class UInputHelper : public QObject
{


public:
    static UInputHelper* getInstance();
    void deleteInstance();

    QString getDisplayString(int virtualkey); // unsigned
    int getVirtualKey(QString codestring); // unsigned

protected:
    explicit UInputHelper(QObject *parent = nullptr);
    ~UInputHelper();

    void populateKnownAliases();

    static UInputHelper *_instance;
    QHash<QString, int> knownAliasesX11SymVK; // <.., unsigned>
    QHash<int, QString> knownAliasesVKStrings; // <unsigned, ..>

};

#endif // UINPUTHELPER_H
