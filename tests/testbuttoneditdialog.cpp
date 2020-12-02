/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
 * Copyright (C) 2020 Paweł Kotiuk <kotiuk@zohomail.eu>
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

#include "buttoneditdialog.h"

#include <QtTest/QtTest>

/*
if slotSummaryLabel->text() == tr("[NO KEY]") and lastJoyButton is nullptr lastJoyButton->getSlotsString().replace("&", "&&") == tr("[NO KEY]") then advancedPushButton should be disabled

if slotSummaryLabel->text() != tr("[NO KEY]") and lastJoyButton is not nullptr and lastJoyButton->getSlotsString().replace("&", "&&") != tr("[NO KEY]") then
advancedPushButton should be enabled

toggleCheckBox should be checked if lastJoyButton->getToggleState()

turboCheckBox should be checked if lastJoyButton->isUsingTurbo()

buttonNameLineEdit cannot be disabled

actionNameLineEdit cannot be disabled

buttonBox->Cancel cannot be disabled

virtualKeyMouseTabWidget->tab(0)->->findChildren<VirtualKeyPushButton*> ()
check if each
QString getXcodestring() const; != ""
QString getDisplayString() const; != tr("[NO KEY]")
int getKeycode() const; != 0
int getQkeyalias() const; != 0
only if it's not a none button and otherKeysPushbutton

QList<VirtualMousePushButton*> newlist = mouseTab->findChildren<VirtualMousePushButton*>();
QListIterator<VirtualMousePushButton*> iter(newlist);

while (iter.hasNext())
{
   VirtualMousePushButton *mousebutton = iter.next();
   // test if getMouseCode() is not null or less
}

getOtherKeysMenu()-actions()
if AntKeyMapper::getInstance()->returnQtKey(oneFromListAction) != 0
*/


class TestButtonEditDialog: public QObject
{
    Q_OBJECT

public:
    TestButtonEditDialog(QObject* parent = 0);

private slots:

private:
    ButtonEditDialog buttonEditDialog;
};

TestButtonEditDialog::TestButtonEditDialog(QObject* parent) :
    QObject(parent),
    buttonEditDialog()
{
    QTestEventLoop::instance().enterLoop(1);
}

// QTEST_MAIN(TestButtonEditDialog)
#include "testbuttoneditdialog.moc"
