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


#ifndef UINPUTHELPER_H
#define UINPUTHELPER_H

#include <QObject>
#include <QString>
#include <QHash>

class UInputHelper : public QObject
{
    Q_OBJECT

public:
    static UInputHelper* getInstance();
    void deleteInstance();

    QString getDisplayString(int virtualkey);
    int getVirtualKey(QString codestring);
    QHash<QString, int> const& getKnownAliasesX11SymVK();
    QHash<int, QString> const& getKnownAliasesVKStrings();

protected:
    explicit UInputHelper(QObject *parent = nullptr);
    ~UInputHelper();

    void populateKnownAliases();

    static UInputHelper *_instance;
    QHash<QString, int> knownAliasesX11SymVK;
    QHash<int, QString> knownAliasesVKStrings;

private:
    void populateXVkStrings(QHash<int, QString>& knownAliasesVKStrings);
    void populateX11SymVk(QHash<QString, int>& knownAliasesX11SymVK);

};

#endif // UINPUTHELPER_H
