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

#include "inputdevicebitarraystatus.h"

InputDeviceBitArrayStatus::InputDeviceBitArrayStatus(InputDevice *device, bool readCurrent, QObject *parent) :
    QObject(parent)
{
    for (int i=0; i < device->getNumberRawAxes(); i++)
    {
        SetJoystick *currentSet = device->getActiveSetJoystick();
        JoyAxis *axis = currentSet->getJoyAxis(i);

        if (axis && readCurrent)
        {
            axesStatus.append(!axis->inDeadZone(axis->getCurrentRawValue()) ? true : false);
        }
        else
        {
            axesStatus.append(false);
        }
    }

    for (int i=0; i < device->getNumberRawHats(); i++)
    {
        SetJoystick *currentSet = device->getActiveSetJoystick();
        JoyDPad *dpad = currentSet->getJoyDPad(i);
        if (dpad && readCurrent)
        {
            hatButtonStatus.append(dpad->getCurrentDirection() != JoyDPadButton::DpadCentered ? true : false);
        }
        else
        {
            hatButtonStatus.append(false);
        }
    }

    buttonStatus.resize(device->getNumberRawButtons());
    buttonStatus.fill(0);

    for (int i=0; i < device->getNumberRawButtons(); i++)
    {
        SetJoystick *currentSet = device->getActiveSetJoystick();
        JoyButton *button = currentSet->getJoyButton(i);
        if (button && readCurrent)
        {
            buttonStatus.setBit(i, button->getButtonState());
        }
    }
}

void InputDeviceBitArrayStatus::changeAxesStatus(int axisIndex, bool value)
{
    if (axisIndex >= 0 && axisIndex <= axesStatus.size())
    {
        axesStatus.replace(axisIndex, value);
    }
}

void InputDeviceBitArrayStatus::changeButtonStatus(int buttonIndex, bool value)
{
    if (buttonIndex >= 0 && buttonIndex <= buttonStatus.size())
    {
        buttonStatus.setBit(buttonIndex, value);
    }
}

void InputDeviceBitArrayStatus::changeHatStatus(int hatIndex, bool value)
{
    if (hatIndex >= 0 && hatIndex <= hatButtonStatus.size())
    {
        hatButtonStatus.replace(hatIndex, value);
    }
}

QBitArray InputDeviceBitArrayStatus::generateFinalBitArray()
{
    unsigned int totalArraySize = 0;
    totalArraySize = axesStatus.size() + hatButtonStatus.size() + buttonStatus.size();

    QBitArray aggregateBitArray(totalArraySize, false);
    unsigned int currentBit = 0;

    for (int i=0; i < axesStatus.size(); i++)
    {
        aggregateBitArray.setBit(currentBit, axesStatus.at(i));
        currentBit++;
    }

    for (int i=0; i < hatButtonStatus.size(); i++)
    {
        aggregateBitArray.setBit(currentBit, hatButtonStatus.at(i));
        currentBit++;
    }

    for (int i=0; i < buttonStatus.size(); i++)
    {
        aggregateBitArray.setBit(currentBit, buttonStatus.at(i));
        currentBit++;
    }

    return aggregateBitArray;
}

void InputDeviceBitArrayStatus::clearStatusValues()
{
    for (int i=0; i < axesStatus.size(); i++)
    {
        axesStatus.replace(i, false);
    }

    for (int i=0; i < hatButtonStatus.size(); i++)
    {
        hatButtonStatus.replace(i, false);
    }

    buttonStatus.fill(false);
}
