/* antimicrox Gamepad to KB+M event mapper
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

#include "joydpad.h"

#include "globalvariables.h"
#include "inputdevice.h"

#include <QDebug>
#include <QHashIterator>

JoyDPad::JoyDPad(int index, int originset, SetJoystick *parentSet, QObject *parent)
    : QObject(parent)
{
    m_index = index;
    buttons = QHash<int, JoyDPadButton *>();
    activeDiagonalButton = nullptr;
    prevDirection = JoyDPadButton::DpadCentered;
    pendingDirection = prevDirection;
    m_originset = originset;
    currentMode = StandardMode;
    m_parentSet = parentSet;
    this->dpadDelay = GlobalVariables::JoyDPad::DEFAULTDPADDELAY;

    populateButtons();

    pendingEvent = false;
    pendingEventDirection = prevDirection;
    pendingIgnoreSets = false;

    directionDelayTimer.setSingleShot(true);
    connect(&directionDelayTimer, &QTimer::timeout, this, &JoyDPad::dpadDirectionChangeEvent);
}

JoyDPadButton *JoyDPad::getJoyButton(int index_local) { return buttons.value(index_local); }

void JoyDPad::populateButtons()
{
    JoyDPadButton *button = new JoyDPadButton(JoyDPadButton::DpadUp, m_originset, this, m_parentSet, this);
    buttons.insert(JoyDPadButton::DpadUp, button);

    button = new JoyDPadButton(JoyDPadButton::DpadDown, m_originset, this, m_parentSet, this);
    buttons.insert(JoyDPadButton::DpadDown, button);

    button = new JoyDPadButton(JoyDPadButton::DpadRight, m_originset, this, m_parentSet, this);
    buttons.insert(JoyDPadButton::DpadRight, button);

    button = new JoyDPadButton(JoyDPadButton::DpadLeft, m_originset, this, m_parentSet, this);
    buttons.insert(JoyDPadButton::DpadLeft, button);

    button = new JoyDPadButton(JoyDPadButton::DpadLeftUp, m_originset, this, m_parentSet, this);
    buttons.insert(JoyDPadButton::DpadLeftUp, button);

    button = new JoyDPadButton(JoyDPadButton::DpadRightUp, m_originset, this, m_parentSet, this);
    buttons.insert(JoyDPadButton::DpadRightUp, button);

    button = new JoyDPadButton(JoyDPadButton::DpadRightDown, m_originset, this, m_parentSet, this);
    buttons.insert(JoyDPadButton::DpadRightDown, button);

    button = new JoyDPadButton(JoyDPadButton::DpadLeftDown, m_originset, this, m_parentSet, this);
    buttons.insert(JoyDPadButton::DpadLeftDown, button);
}

QString JoyDPad::getName(bool fullForceFormat, bool displayNames)
{
    QString label = QString();

    if (!dpadName.isEmpty() && displayNames)
    {
        if (fullForceFormat)
        {
            label.append(tr("DPad")).append(" ");
        }

        label.append(dpadName);
    } else if (!defaultDPadName.isEmpty())
    {
        if (fullForceFormat)
        {
            label.append(tr("DPad")).append(" ");
        }
        label.append(defaultDPadName);
    } else
    {
        label.append(tr("DPad")).append(" ");
        label.append(QString::number(getRealJoyNumber()));
    }

    return label;
}

int JoyDPad::getJoyNumber() { return m_index; }

int JoyDPad::getIndex() { return m_index; }

int JoyDPad::getRealJoyNumber() { return m_index + 1; }

QString JoyDPad::getXmlName() { return GlobalVariables::JoyDPad::xmlName; }

void JoyDPad::queuePendingEvent(int value, bool ignoresets)
{
    pendingEvent = true;
    pendingEventDirection = value;
    pendingIgnoreSets = ignoresets;
}

void JoyDPad::activatePendingEvent()
{
    if (pendingEvent)
    {
        joyEvent(pendingEventDirection, pendingIgnoreSets);

        pendingEvent = false;
        pendingEventDirection = static_cast<int>(JoyDPadButton::DpadCentered);
        pendingIgnoreSets = false;
    }
}

bool JoyDPad::hasPendingEvent() { return pendingEvent; }

void JoyDPad::clearPendingEvent()
{
    pendingEvent = false;
    pendingEventDirection = static_cast<int>(JoyDPadButton::DpadCentered);
    pendingIgnoreSets = false;
}

void JoyDPad::joyEvent(int value, bool ignoresets)
{
    if (value != static_cast<int>(pendingDirection))
    {
        if (value != static_cast<int>(JoyDPadButton::DpadCentered))
        {
            if (prevDirection == JoyDPadButton::DpadCentered)
            {
                emit active(value);
            }

            pendingDirection = static_cast<JoyDPadButton::JoyDPadDirections>(value);

            if (ignoresets || (dpadDelay == 0))
            {
                if (directionDelayTimer.isActive())
                {
                    directionDelayTimer.stop();
                }

                createDeskEvent(ignoresets);
            } else if (pendingDirection != prevDirection)
            {
                if (!directionDelayTimer.isActive())
                {
                    directionDelayTimer.start(dpadDelay);
                }
            } else
            {
                if (directionDelayTimer.isActive())
                {
                    directionDelayTimer.stop();
                }
            }
        } else
        {
            emit released(value);
            pendingDirection = JoyDPadButton::DpadCentered;
            if (ignoresets || dpadDelay == 0)
            {
                if (directionDelayTimer.isActive())
                {
                    directionDelayTimer.stop();
                }

                createDeskEvent(ignoresets);
            } else
            {
                if (!directionDelayTimer.isActive())
                {
                    directionDelayTimer.start(dpadDelay);
                }
            }
        }
    }
}

QHash<int, JoyDPadButton *> *JoyDPad::getJoyButtons() { return &buttons; }

int JoyDPad::getCurrentDirection() { return prevDirection; }

void JoyDPad::setJoyMode(JoyMode mode)
{
    currentMode = mode;
    emit joyModeChanged();
    emit propertyUpdated();
}

JoyDPad::JoyMode JoyDPad::getJoyMode() { return currentMode; }

void JoyDPad::releaseButtonEvents()
{
    QHashIterator<int, JoyDPadButton *> iter(buttons);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->joyEvent(false, true);
    }
}

QHash<int, JoyDPadButton *> *JoyDPad::getButtons() { return &buttons; }

bool JoyDPad::isDefault()
{
    bool value = true;
    value = value && (currentMode == StandardMode);
    value = value && (dpadDelay == GlobalVariables::JoyDPad::DEFAULTDPADDELAY);

    QHashIterator<int, JoyDPadButton *> iter(buttons);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        value = value && (button->isDefault());
    }
    return value;
}

void JoyDPad::setButtonsMouseMode(JoyButton::JoyMouseMovementMode mode)
{
    QHashIterator<int, JoyDPadButton *> iter(buttons);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->setMouseMode(mode);
    }
}

bool JoyDPad::hasSameButtonsMouseMode()
{
    bool result = true;

    JoyButton::JoyMouseMovementMode initialMode = JoyButton::MouseCursor;
    QHash<int, JoyDPadButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return result;

    JoyDPadButton *button = iter.value();
    initialMode = button->getMouseMode();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        JoyButton::JoyMouseMovementMode temp = button->getMouseMode();
        if (temp != initialMode)
        {
            result = false;
            break;
        }
    }

    return result;
}

JoyButton::JoyMouseMovementMode JoyDPad::getButtonsPresetMouseMode()
{
    JoyButton::JoyMouseMovementMode resultMode = JoyButton::MouseCursor;

    QHash<int, JoyDPadButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return resultMode;

    JoyDPadButton *button = iter.value();
    resultMode = button->getMouseMode();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        JoyButton::JoyMouseMovementMode temp = button->getMouseMode();
        if (temp != resultMode)
        {
            resultMode = JoyButton::MouseCursor;
            break;
        }
    }

    return resultMode;
}

void JoyDPad::setButtonsMouseCurve(JoyButton::JoyMouseCurve mouseCurve)
{
    QHashIterator<int, JoyDPadButton *> iter(buttons);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->setMouseCurve(mouseCurve);
    }
}

bool JoyDPad::hasSameButtonsMouseCurve()
{
    bool result = true;

    QHash<int, JoyDPadButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return result;

    JoyDPadButton *button = iter.value();
    JoyButton::JoyMouseCurve initialCurve = button->getMouseCurve();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        JoyButton::JoyMouseCurve temp = button->getMouseCurve();
        if (temp != initialCurve)
        {
            result = false;
            break;
        }
    }

    return result;
}

JoyButton::JoyMouseCurve JoyDPad::getButtonsPresetMouseCurve()
{
    JoyButton::JoyMouseCurve resultCurve = JoyButton::LinearCurve;

    QHash<int, JoyDPadButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return resultCurve;

    JoyDPadButton *button = iter.value();
    resultCurve = button->getMouseCurve();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        JoyButton::JoyMouseCurve temp = button->getMouseCurve();
        if (temp != resultCurve)
        {
            resultCurve = JoyButton::LinearCurve;
            break;
        }
    }

    return resultCurve;
}

void JoyDPad::setButtonsSpringWidth(int value)
{
    QHashIterator<int, JoyDPadButton *> iter(buttons);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->setSpringWidth(value);
    }
}

void JoyDPad::setButtonsSpringHeight(int value)
{
    QHashIterator<int, JoyDPadButton *> iter(buttons);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->setSpringHeight(value);
    }
}

int JoyDPad::getButtonsPresetSpringWidth()
{
    int presetSpringWidth = 0;

    QHash<int, JoyDPadButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return presetSpringWidth;

    JoyDPadButton *button = iter.value();
    presetSpringWidth = button->getSpringWidth();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        int temp = button->getSpringWidth();
        if (temp != presetSpringWidth)
        {
            presetSpringWidth = 0;
            break;
        }
    }

    return presetSpringWidth;
}

int JoyDPad::getButtonsPresetSpringHeight()
{
    int presetSpringHeight = 0;

    QHash<int, JoyDPadButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return presetSpringHeight;

    JoyDPadButton *button = iter.value();
    presetSpringHeight = button->getSpringHeight();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        int temp = button->getSpringHeight();
        if (temp != presetSpringHeight)
        {
            presetSpringHeight = 0;
            break;
        }
    }

    return presetSpringHeight;
}

void JoyDPad::setButtonsSensitivity(double value)
{
    QHashIterator<int, JoyDPadButton *> iter(buttons);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->setSensitivity(value);
    }
}

double JoyDPad::getButtonsPresetSensitivity()
{
    double presetSensitivity = 1.0;

    QHash<int, JoyDPadButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return presetSensitivity;

    JoyDPadButton *button = iter.value();
    presetSensitivity = button->getSensitivity();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        double temp = button->getSensitivity();
        if (!qFuzzyCompare(temp, presetSensitivity))
        {
            presetSensitivity = 1.0;
            break;
        }
    }

    return presetSensitivity;
}

QHash<int, JoyDPadButton *> JoyDPad::getApplicableButtons()
{
    QHash<int, JoyDPadButton *> temphash;

    if ((currentMode == StandardMode) || (currentMode == EightWayMode) || (currentMode == FourWayCardinal))
    {
        temphash.insert(JoyDPadButton::DpadUp, buttons.value(JoyDPadButton::DpadUp));
        temphash.insert(JoyDPadButton::DpadDown, buttons.value(JoyDPadButton::DpadDown));
        temphash.insert(JoyDPadButton::DpadLeft, buttons.value(JoyDPadButton::DpadLeft));
        temphash.insert(JoyDPadButton::DpadRight, buttons.value(JoyDPadButton::DpadRight));
    }

    if ((currentMode == EightWayMode) || (currentMode == FourWayDiagonal))
    {
        temphash.insert(JoyDPadButton::DpadLeftUp, buttons.value(JoyDPadButton::DpadLeftUp));
        temphash.insert(JoyDPadButton::DpadRightUp, buttons.value(JoyDPadButton::DpadRightUp));
        temphash.insert(JoyDPadButton::DpadRightDown, buttons.value(JoyDPadButton::DpadRightDown));
        temphash.insert(JoyDPadButton::DpadLeftDown, buttons.value(JoyDPadButton::DpadLeftDown));
    }

    return temphash;
}

void JoyDPad::setDPadName(QString tempName)
{
    if ((tempName.length() <= 20) && (tempName != dpadName))
    {
        dpadName = tempName;
        emit dpadNameChanged();
        emit propertyUpdated();
    }
}

const QString JoyDPad::getDpadName() { return dpadName; }

const QString JoyDPad::getDefaultDpadName() { return defaultDPadName; }

void JoyDPad::setButtonsWheelSpeedX(int value)
{
    QHashIterator<int, JoyDPadButton *> iter(buttons);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->setWheelSpeed(value, 'X');
    }
}

void JoyDPad::setButtonsWheelSpeedY(int value)
{
    QHashIterator<int, JoyDPadButton *> iter(buttons);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->setWheelSpeed(value, 'Y');
    }
}

void JoyDPad::setDefaultDPadName(QString tempname)
{
    defaultDPadName = tempname;
    emit dpadNameChanged();
}

QString JoyDPad::getDefaultDPadName() { return defaultDPadName; }

SetJoystick *JoyDPad::getParentSet() { return m_parentSet; }

void JoyDPad::establishPropertyUpdatedConnection()
{
    connect(this, &JoyDPad::propertyUpdated, getParentSet()->getInputDevice(), &InputDevice::profileEdited);
}

void JoyDPad::disconnectPropertyUpdatedConnection()
{
    disconnect(this, &JoyDPad::propertyUpdated, getParentSet()->getInputDevice(), &InputDevice::profileEdited);
}

bool JoyDPad::hasSlotsAssigned()
{
    bool hasSlots = false;

    QHash<int, JoyDPadButton *> temphash = getApplicableButtons();
    QHashIterator<int, JoyDPadButton *> iter(temphash);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        if (button != nullptr)
        {
            if (button->getAssignedSlots()->count() > 0)
            {
                hasSlots = true;
                iter.toBack();
            }
        }
    }

    return hasSlots;
}

void JoyDPad::setButtonsSpringRelativeStatus(bool value)
{
    QHashIterator<int, JoyDPadButton *> iter(buttons);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->setSpringRelativeStatus(value);
    }
}

bool JoyDPad::isRelativeSpring()
{
    bool relative = false;

    QHash<int, JoyDPadButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return relative;

    JoyDPadButton *button = iter.value();
    relative = button->isRelativeSpring();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        bool temp = button->isRelativeSpring();
        if (temp != relative)
        {
            relative = false;
            break;
        }
    }

    return relative;
}

void JoyDPad::copyAssignments(JoyDPad *destDPad)
{
    destDPad->activeDiagonalButton = activeDiagonalButton;
    destDPad->prevDirection = prevDirection;
    destDPad->currentMode = currentMode;
    destDPad->dpadDelay = dpadDelay;

    QHashIterator<int, JoyDPadButton *> iter(destDPad->buttons);
    while (iter.hasNext())
    {
        JoyDPadButton *destButton = iter.next().value();
        if (destButton != nullptr)
        {
            JoyDPadButton *sourceButton = buttons.value(destButton->getDirection());
            if (sourceButton != nullptr)
            {
                sourceButton->copyAssignments(destButton);
            }
        }
    }

    if (!destDPad->isDefault())
    {
        emit propertyUpdated();
    }
}

void JoyDPad::createDeskEvent(bool ignoresets)
{
    JoyDPadButton *curButton = nullptr;
    JoyDPadButton *prevButton = nullptr;
    JoyDPadButton::JoyDPadDirections value = pendingDirection;

    if (pendingDirection != prevDirection)
    {
        if (activeDiagonalButton != nullptr)
        {
            activeDiagonalButton->joyEvent(false, ignoresets);
            activeDiagonalButton = nullptr;
        } else
        {
            if (currentMode == StandardMode)
            {
                if ((prevDirection & JoyDPadButton::DpadUp) && (!(value & JoyDPadButton::DpadUp)))
                {
                    prevButton = buttons.value(JoyDPadButton::DpadUp);
                    prevButton->joyEvent(false, ignoresets);
                }

                if ((prevDirection & JoyDPadButton::DpadDown) && (!(value & JoyDPadButton::DpadDown)))
                {
                    prevButton = buttons.value(JoyDPadButton::DpadDown);
                    prevButton->joyEvent(false, ignoresets);
                }

                if ((prevDirection & JoyDPadButton::DpadLeft) && (!(value & JoyDPadButton::DpadLeft)))
                {
                    prevButton = buttons.value(JoyDPadButton::DpadLeft);
                    prevButton->joyEvent(false, ignoresets);
                }

                if ((prevDirection & JoyDPadButton::DpadRight) && (!(value & JoyDPadButton::DpadRight)))
                {
                    prevButton = buttons.value(JoyDPadButton::DpadRight);
                    prevButton->joyEvent(false, ignoresets);
                }
            } else if ((currentMode == EightWayMode) && prevDirection)
            {
                prevButton = buttons.value(prevDirection);
                prevButton->joyEvent(false, ignoresets);
            } else if ((currentMode == FourWayCardinal) && (static_cast<int>(prevDirection) != 0))
            {
                if (((prevDirection == JoyDPadButton::DpadUp) || (prevDirection == JoyDPadButton::DpadRightUp)) &&
                    ((value != JoyDPadButton::DpadUp) && (value != JoyDPadButton::DpadRightUp)))
                {
                    prevButton = buttons.value(JoyDPadButton::DpadUp);
                } else if (((prevDirection == JoyDPadButton::DpadDown) || (prevDirection == JoyDPadButton::DpadLeftDown)) &&
                           ((value != JoyDPadButton::DpadDown) && (value != JoyDPadButton::DpadLeftDown)))
                {
                    prevButton = buttons.value(JoyDPadButton::DpadDown);
                } else if (((prevDirection == JoyDPadButton::DpadLeft) || (prevDirection == JoyDPadButton::DpadLeftUp)) &&
                           ((value != JoyDPadButton::DpadLeft) && (value != JoyDPadButton::DpadLeftUp)))
                {
                    prevButton = buttons.value(JoyDPadButton::DpadLeft);
                } else if (((prevDirection == JoyDPadButton::DpadRight) ||
                            (prevDirection == JoyDPadButton::DpadRightDown)) &&
                           ((value != JoyDPadButton::DpadRight) && (value != JoyDPadButton::DpadRightDown)))
                {
                    prevButton = buttons.value(JoyDPadButton::DpadRight);
                }

                if (prevButton != nullptr)
                {
                    prevButton->joyEvent(false, ignoresets);
                }

            } else if ((currentMode == FourWayDiagonal) && (static_cast<int>(prevDirection) != 0))
            {
                prevButton = buttons.value(prevDirection);
                prevButton->joyEvent(false, ignoresets);
            }
        }

        switch (currentMode)
        {
        case StandardMode: {
            if ((value & JoyDPadButton::DpadUp) && (!(prevDirection & JoyDPadButton::DpadUp)))
            {
                curButton = buttons.value(JoyDPadButton::DpadUp);
                curButton->joyEvent(true, ignoresets);
            }

            if ((value & JoyDPadButton::DpadDown) && (!(prevDirection & JoyDPadButton::DpadDown)))
            {
                curButton = buttons.value(JoyDPadButton::DpadDown);
                curButton->joyEvent(true, ignoresets);
            }

            if ((value & JoyDPadButton::DpadLeft) && (!(prevDirection & JoyDPadButton::DpadLeft)))
            {
                curButton = buttons.value(JoyDPadButton::DpadLeft);
                curButton->joyEvent(true, ignoresets);
            }

            if ((value & JoyDPadButton::DpadRight) && (!(prevDirection & JoyDPadButton::DpadRight)))
            {
                curButton = buttons.value(JoyDPadButton::DpadRight);
                curButton->joyEvent(true, ignoresets);
            }

            break;
        }
        case EightWayMode: {
            switch (value)
            {
            case JoyDPadButton::DpadLeftUp: {
                activeDiagonalButton = buttons.value(JoyDPadButton::DpadLeftUp);
                activeDiagonalButton->joyEvent(true, ignoresets);

                break;
            }
            case JoyDPadButton::DpadRightUp: {
                activeDiagonalButton = buttons.value(JoyDPadButton::DpadRightUp);
                activeDiagonalButton->joyEvent(true, ignoresets);

                break;
            }
            case JoyDPadButton::DpadRightDown: {
                activeDiagonalButton = buttons.value(JoyDPadButton::DpadRightDown);
                activeDiagonalButton->joyEvent(true, ignoresets);

                break;
            }
            case JoyDPadButton::DpadLeftDown: {
                activeDiagonalButton = buttons.value(JoyDPadButton::DpadLeftDown);
                activeDiagonalButton->joyEvent(true, ignoresets);

                break;
            }
            case JoyDPadButton::DpadUp: {
                curButton = buttons.value(JoyDPadButton::DpadUp);
                curButton->joyEvent(true, ignoresets);

                break;
            }
            case JoyDPadButton::DpadDown: {
                curButton = buttons.value(JoyDPadButton::DpadDown);
                curButton->joyEvent(true, ignoresets);

                break;
            }
            case JoyDPadButton::DpadLeft: {
                curButton = buttons.value(JoyDPadButton::DpadLeft);
                curButton->joyEvent(true, ignoresets);

                break;
            }
            case JoyDPadButton::DpadRight: {
                curButton = buttons.value(JoyDPadButton::DpadRight);
                curButton->joyEvent(true, ignoresets);

                break;
            }
            default:

                break;
            }

            break;
        }
        case FourWayCardinal: {
            if ((value == JoyDPadButton::DpadUp) || (value == JoyDPadButton::DpadRightUp))
            {
                curButton = buttons.value(JoyDPadButton::DpadUp);
                curButton->joyEvent(true, ignoresets);
            } else if ((value == JoyDPadButton::DpadDown) || (value == JoyDPadButton::DpadLeftDown))
            {
                curButton = buttons.value(JoyDPadButton::DpadDown);
                curButton->joyEvent(true, ignoresets);
            } else if ((value == JoyDPadButton::DpadLeft) || (value == JoyDPadButton::DpadLeftUp))
            {
                curButton = buttons.value(JoyDPadButton::DpadLeft);
                curButton->joyEvent(true, ignoresets);
            } else if ((value == JoyDPadButton::DpadRight) || (value == JoyDPadButton::DpadRightDown))
            {
                curButton = buttons.value(JoyDPadButton::DpadRight);
                curButton->joyEvent(true, ignoresets);
            }

            break;
        }
        case FourWayDiagonal: {
            switch (value)
            {
            case JoyDPadButton::DpadLeftUp: {
                activeDiagonalButton = buttons.value(JoyDPadButton::DpadLeftUp);
                activeDiagonalButton->joyEvent(true, ignoresets);

                break;
            }
            case JoyDPadButton::DpadRightUp: {
                activeDiagonalButton = buttons.value(JoyDPadButton::DpadRightUp);
                activeDiagonalButton->joyEvent(true, ignoresets);

                break;
            }
            case JoyDPadButton::DpadRightDown: {
                activeDiagonalButton = buttons.value(JoyDPadButton::DpadRightDown);
                activeDiagonalButton->joyEvent(true, ignoresets);

                break;
            }
            case JoyDPadButton::DpadLeftDown: {
                activeDiagonalButton = buttons.value(JoyDPadButton::DpadLeftDown);
                activeDiagonalButton->joyEvent(true, ignoresets);

                break;
            }
            default:
                break;
            }

            break;
        }
        }

        prevDirection = pendingDirection;
    }
}

void JoyDPad::dpadDirectionChangeEvent() { createDeskEvent(); }

void JoyDPad::setDPadDelay(int value)
{
    if (((value >= 10) && (value <= 1000)) || (value == 0))
    {
        this->dpadDelay = value;
        emit dpadDelayChanged(value);
        emit propertyUpdated();
    }
}

int JoyDPad::getDPadDelay() { return dpadDelay; }

void JoyDPad::setButtonsEasingDuration(double value)
{
    QHash<int, JoyDPadButton *> temphash = getApplicableButtons();
    QHashIterator<int, JoyDPadButton *> iter(temphash);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->setEasingDuration(value);
    }
}

double JoyDPad::getButtonsEasingDuration()
{
    double result = GlobalVariables::JoyButton::DEFAULTEASINGDURATION;

    QHash<int, JoyDPadButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return result;

    JoyDPadButton *button = iter.value();
    result = button->getEasingDuration();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        double temp = button->getEasingDuration();
        if (!qFuzzyCompare(temp, result))
        {
            result = GlobalVariables::JoyButton::DEFAULTEASINGDURATION;
            break;
        }
    }

    return result;
}

void JoyDPad::setButtonsSpringDeadCircleMultiplier(int value)
{
    QHash<int, JoyDPadButton *> temphash = getApplicableButtons();
    QHashIterator<int, JoyDPadButton *> iter(temphash);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->setSpringDeadCircleMultiplier(value);
    }
}

int JoyDPad::getButtonsSpringDeadCircleMultiplier()
{
    int result = GlobalVariables::JoyButton::DEFAULTSPRINGRELEASERADIUS;

    QHash<int, JoyDPadButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return result;

    JoyDPadButton *button = iter.value();
    result = button->getSpringDeadCircleMultiplier();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        int temp = button->getSpringDeadCircleMultiplier();
        if (temp != result)
        {
            result = GlobalVariables::JoyButton::DEFAULTSPRINGRELEASERADIUS;
            break;
        }
    }

    return result;
}

void JoyDPad::setButtonsExtraAccelerationCurve(JoyButton::JoyExtraAccelerationCurve curve)
{
    QHash<int, JoyDPadButton *> temphash = getApplicableButtons();
    QHashIterator<int, JoyDPadButton *> iter(temphash);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->setExtraAccelerationCurve(curve);
    }
}

JoyButton::JoyExtraAccelerationCurve JoyDPad::getButtonsExtraAccelerationCurve()
{
    JoyButton::JoyExtraAccelerationCurve result = JoyButton::LinearAccelCurve;

    QHash<int, JoyDPadButton *> temphash = getApplicableButtons();
    auto iter = temphash.cbegin();

    if (iter == temphash.cend())
        return result;

    JoyDPadButton *button = iter.value();
    result = button->getExtraAccelerationCurve();

    while (++iter != temphash.cend())
    {
        button = iter.value();
        JoyButton::JoyExtraAccelerationCurve temp = button->getExtraAccelerationCurve();
        if (temp != result)
        {
            result = JoyButton::LinearAccelCurve;
            break;
        }
    }

    return result;
}

QHash<int, JoyDPadButton *> JoyDPad::getDirectionButtons(JoyDPadButton::JoyDPadDirections direction)
{
    QHash<int, JoyDPadButton *> temphash;

    switch (currentMode)
    {
    case StandardMode: {
        if (direction & JoyDPadButton::DpadUp)
        {
            temphash.insert(JoyDPadButton::DpadUp, buttons.value(JoyDPadButton::DpadUp));
        }

        if (direction & JoyDPadButton::DpadDown)
        {
            temphash.insert(JoyDPadButton::DpadDown, buttons.value(JoyDPadButton::DpadDown));
        }

        if (direction & JoyDPadButton::DpadLeft)
        {
            temphash.insert(JoyDPadButton::DpadLeft, buttons.value(JoyDPadButton::DpadLeft));
        }

        if (direction & JoyDPadButton::DpadRight)
        {
            temphash.insert(JoyDPadButton::DpadRight, buttons.value(JoyDPadButton::DpadRight));
        }

        break;
    }
    case EightWayMode: {
        if (direction != JoyDPadButton::DpadCentered)
        {
            temphash.insert(direction, buttons.value(direction));
        }

        break;
    }
    case FourWayCardinal: {
        if ((direction == JoyDPadButton::DpadUp) || (direction == JoyDPadButton::DpadDown) ||
            (direction == JoyDPadButton::DpadLeft) || (direction == JoyDPadButton::DpadRight))
        {
            temphash.insert(direction, buttons.value(direction));
        }

        break;
    }
    case FourWayDiagonal: {
        if ((direction == JoyDPadButton::DpadRightUp) || (direction == JoyDPadButton::DpadRightDown) ||
            (direction == JoyDPadButton::DpadLeftDown) || (direction == JoyDPadButton::DpadLeftUp))
        {
            temphash.insert(direction, buttons.value(direction));
        }

        break;
    }
    }

    return temphash;
}

void JoyDPad::setDirButtonsUpdateInitAccel(JoyDPadButton::JoyDPadDirections direction, bool state)
{
    QHash<int, JoyDPadButton *> apphash = getDirectionButtons(direction);
    QHashIterator<int, JoyDPadButton *> iter(apphash);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        if (button != nullptr)
        {
            button->setUpdateInitAccel(state);
        }
    }
}

void JoyDPad::copyLastDistanceValues(JoyDPad *srcDPad)
{
    QHash<int, JoyDPadButton *> apphash = srcDPad->getApplicableButtons();
    QHashIterator<int, JoyDPadButton *> iter(apphash);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        if ((button != nullptr) && button->getButtonState())
        {
            this->buttons.value(iter.key())->copyLastAccelerationDistance(button);
            this->buttons.value(iter.key())->copyLastMouseDistanceFromDeadZone(button);
        }
    }
}

void JoyDPad::eventReset()
{
    QHash<int, JoyDPadButton *> temphash = getApplicableButtons();
    QHashIterator<int, JoyDPadButton *> iter(temphash);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->eventReset();
    }
}
