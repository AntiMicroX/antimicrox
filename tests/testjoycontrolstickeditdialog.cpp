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

#include "joycontrolstickeditdialog.h"

#include <QtTest/QtTest>

/*
presetsComboBox->count should be 12
joyModeComboBox->count should be 4
deadZoneSlider->value == stick->getDeadZone()
deadZoneSpinBox->value == stick->getDeadZone()
maxZoneSlider->value == stick->getMaxZone()
maxZoneSpinBox->value == stick->getMaxZone()
diagonalRangeSlider->value == stick->getDiagonalRange()
diagonalRangeSpinBox->value == stick->getDiagonalRange()
squareStickSlider->value == stick->getCircleAdjust() * 100
squareStickSpinBox->value == stick->getCircleAdjust() * 100
stickDelaySlider->value == stick->getStickDelay() * .1
stickDelayDoubleSpinBox->value == stick->getStickDelay() * .001
modifierPushButton->text == stick->getModifierButton()->getSlotsSummary()
stickNameLineEdit->text == stick->getStickName()

QString xCoorString = QString::number(stick->getXCoordinate());
    if (stick->getCircleAdjust() > 0.0)
    {
        xCoorString.append(QString(" (%1)").arg(stick->getCircleXCoordinate()));
    }

xCoordinateLabel->text == xCoorString

QString yCoorString = QString::number(stick->getYCoordinate());
    if (stick->getCircleAdjust() > 0.0)
    {
        yCoorString.append(QString(" (%1)").arg(stick->getCircleYCoordinate()));
    }

yCoordinateLabel->text() == yCoorString

distanceLabel->text() == QString::number(stick->getAbsoluteRawDistance())
diagonalLabel->text() == QString::number(stick->calculateBearing())
fromSafeZoneValueLabel->text() == QString::number(stick->getDistanceFromDeadZone() * 100.0)
stickStatusBoxWidget->getStick() == stick
stickStatusBoxWidget has to be visible
mouseSettingsPushButton runs new window
mouseSettingsPushButton is never disabled
presetsComboBox is not disabled
joyModeComboBox is not disabled
deadZoneSlider is not disabled
maxZoneSlider is not disabled
diagonalRangeSlider is not disabled
squareStickSlider is not disabled
stickDelaySlider is not disabled
stickNameLineEdit is not disabled

deadZoneSlider->setValue(int)
deadZoneSlider->value() == deadZoneSpinBox->value()
stickStatusBoxWidget->getStick()->getDeadZone()

maxZoneSlider->setValue(int)
maxZoneSlider->value() == maxZoneSlider->value()
stickStatusBoxWidget->getStick()->getMaxZone()

diagonalRangeSlider->setValue(int)
diagonalRangeSlider->value() == diagonalRangeSlider->value()
stickStatusBoxWidget->getStick()->getDiagonalRange()

squareStickSlider->setValue(int)
squareStickSlider->value() == squareStickSpinBox->value()
stickStatusBoxWidget->getStick()->getCircleAdjust() == squareStickSpinBox->value() / 100

stickDelaySlider->setValue(int)
stickDelaySlider->value() == stickDelayDoubleSpinBox->value()
stickStatusBoxWidget->getStick()->getStickDelay() == stickDelayDoubleSpinBox->value() / .001

window->title() == Set + stick->getStickName()

*/


class TestJoyControlStickEditDialog: public QObject
{
    Q_OBJECT

public:
    TestJoyControlStickEditDialog(QObject* parent = 0);

private slots:

private:
    JoyControlStickEditDialog axisEditDialog;
};

TestJoyControlStickEditDialog::TestJoyControlStickEditDialog(QObject* parent) :
    QObject(parent),
    JoyControlStickEditDialog()
{
    QTestEventLoop::instance().enterLoop(1);
}

// QTEST_MAIN(TestJoyControlStickEditDialog)
#include "testjoycontrolstickeditdialog.moc"


