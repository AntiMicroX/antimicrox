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

#ifndef COCOAHELPER_H
#define COCOAHELPER_H

#include <QObject>
#include <QString>
#include <QHash>

class CocoaHelper : public QObject
{
    Q_OBJECT
public:
    static CocoaHelper* getInstance();
    void deleteInstance();

    QString getDisplayString(unsigned int virtualkey);
    unsigned int getVirtualKey(QString codestring);
    unsigned int getCocoaVirtualKey(int qtcode);

protected:
    explicit CocoaHelper(QObject *parent = 0);
    ~CocoaHelper();

    void populateKnownAliases();

    static CocoaHelper *_instance;
    QHash<QString, unsigned int> knownAliasesX11SymVK;
    QHash<unsigned int, QString> knownAliasesVKStrings;
    QHash<unsigned int, unsigned int> knownCocoaCode;
};

#endif // COCOAHELPER_H
