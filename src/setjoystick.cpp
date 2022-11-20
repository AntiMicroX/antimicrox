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

#include "setjoystick.h"

#include "globalvariables.h"
#include "inputdevice.h"
#include "joybuttontypes/joybutton.h"
#include "joybuttontypes/joycontrolstickbutton.h"
#include "joybuttontypes/joysensorbutton.h"
#include "joycontrolstick.h"
#include "joydpad.h"
#include "joysensor.h"
#include "joysensorfactory.h"
#include "vdpad.h"

#include <QDebug>
#include <QHashIterator>
#include <QtAlgorithms>

SetJoystick::SetJoystick(InputDevice *device, int index, QObject *parent)
    : SetJoystickXml(this, parent)
{
    m_device = device;
    m_index = index;

    reset();
}

SetJoystick::SetJoystick(InputDevice *device, int index, bool runreset, QObject *parent)
    : SetJoystickXml(this, parent)
{
    m_device = device;
    m_index = index;

    if (runreset)
        reset();
}

SetJoystick::~SetJoystick() { removeAllBtnFromQueue(); }

JoyButton *SetJoystick::getJoyButton(int index) const { return getButtons().value(index); }

JoyAxis *SetJoystick::getJoyAxis(int index) const
{
    Q_ASSERT(!axes.isEmpty());
    return axes.value(index);
}

JoyDPad *SetJoystick::getJoyDPad(int index) const { return getHats().value(index); }

VDPad *SetJoystick::getVDPad(int index) const { return getVdpads().value(index); }

JoyControlStick *SetJoystick::getJoyStick(int index) const { return getSticks().value(index); }

JoySensor *SetJoystick::getSensor(JoySensorType type) const { return m_sensors.value(type); }

void SetJoystick::refreshButtons()
{
    deleteButtons();

    for (int i = 0; i < m_device->getNumberRawButtons(); i++)
    {
        JoyButton *button = new JoyButton(i, m_index, this, this);
        m_buttons.insert(i, button);
        enableButtonConnections(button);
    }
}

void SetJoystick::refreshAxes()
{
    deleteAxes();

    InputDevice *device = getInputDevice();
    for (int i = 0; i < device->getNumberRawAxes(); i++)
    {
        JoyAxis *axis = new JoyAxis(i, m_index, this, this);
        axes.insert(i, axis);

        if (device->hasCalibrationThrottle(i))
        {
            JoyAxis::ThrottleTypes throttle = device->getCalibrationThrottle(i);
            axis->setInitialThrottle(throttle);
        }

        enableAxisConnections(axis);
    }
}

void SetJoystick::refreshHats()
{
    deleteHats();

    for (int i = 0; i < m_device->getNumberRawHats(); i++)
    {
        JoyDPad *dpad = new JoyDPad(i, m_index, this, this);
        hats.insert(i, dpad);
        enableHatConnections(dpad);
    }
}

/**
 * @brief Setup sensor objects for all available hardware sensors.
 */
void SetJoystick::refreshSensors()
{
    deleteSensors();

    for (size_t i = 0; i < SENSOR_COUNT; ++i)
    {
        JoySensorType type = static_cast<JoySensorType>(i);

        if (!getInputDevice()->hasRawSensor(type))
            continue;

        JoySensor *sensor = JoySensorFactory::build(type, getInputDevice()->getRawSensorRate(type), m_index, this, this);
        m_sensors.insert(type, sensor);
        enableSensorConnections(sensor);
    }
}

void SetJoystick::deleteButtons()
{
    QHashIterator<int, JoyButton *> iter(getButtons());

    while (iter.hasNext())
    {
        JoyButton *button = iter.next().value();

        if (button != nullptr)
        {
            button->deleteLater();
            button = nullptr;
        }
    }

    m_buttons.clear();
}

void SetJoystick::deleteAxes()
{
    QHashIterator<int, JoyAxis *> iter(axes);

    while (iter.hasNext())
    {
        JoyAxis *axis = iter.next().value();

        if (axis != nullptr)
        {
            axes.remove(iter.key());
            axis->deleteLater();
        }
    }

    axes.clear();
}

void SetJoystick::deleteSticks()
{
    QHashIterator<int, JoyControlStick *> iter(getSticks());

    while (iter.hasNext())
    {
        JoyControlStick *stick = iter.next().value();

        if (stick != nullptr)
        {
            stick->deleteLater();
            stick = nullptr;
        }
    }

    sticks.clear();
}

void SetJoystick::deleteVDpads()
{
    QHashIterator<int, VDPad *> iter(getVdpads());

    while (iter.hasNext())
    {
        VDPad *dpad = iter.next().value();

        if (dpad != nullptr)
        {
            dpad->deleteLater();
            dpad = nullptr;
        }
    }

    vdpads.clear();
}

void SetJoystick::deleteHats()
{
    QHashIterator<int, JoyDPad *> iter(getHats());

    while (iter.hasNext())
    {
        JoyDPad *dpad = iter.next().value();

        if (dpad != nullptr)
        {
            hats.remove(iter.key());
            dpad->deleteLater();
        }
    }

    hats.clear();
}

/**
 * @brief Destroy all sensor objects in this set
 */
void SetJoystick::deleteSensors()
{
    for (const auto &sensor : m_sensors)
    {
        if (sensor != nullptr)
            sensor->deleteLater();
    }

    m_sensors.clear();
}

int SetJoystick::getNumberButtons() const { return getButtons().count(); }

int SetJoystick::getNumberAxes() const { return axes.count(); }

int SetJoystick::getNumberHats() const { return getHats().count(); }

int SetJoystick::getNumberSticks() const { return getSticks().size(); }

/**
 * @brief Checks if this set has a sensor
 * @returns True if sensor type is present, false otherwise.
 */
bool SetJoystick::hasSensor(JoySensorType type) const { return m_sensors.contains(type); }

int SetJoystick::getNumberVDPads() const { return getVdpads().size(); }

/**
 * @brief Re-enumerates inputs from the associated device and
 *  resets all mappings in this set.
 */
void SetJoystick::reset()
{
    deleteSticks();
    deleteSensors();
    deleteVDpads();
    refreshAxes();
    refreshSensors();
    refreshButtons();
    refreshHats();
    m_name = QString();
}

void SetJoystick::propogateSetChange(int index) { emit setChangeActivated(index); }

void SetJoystick::propogateSetButtonAssociation(int button, int newset, int mode)
{
    if (newset != m_index)
        emit setAssignmentButtonChanged(button, m_index, newset, mode);
}

void SetJoystick::propogateSetAxisButtonAssociation(int button, int axis, int newset, int mode)
{
    if (newset != m_index)
        emit setAssignmentAxisChanged(button, axis, m_index, newset, mode);
}

void SetJoystick::propogateSetStickButtonAssociation(int button, int stick, int newset, int mode)
{
    if (newset != m_index)
        emit setAssignmentStickChanged(button, stick, m_index, newset, mode);
}

/**
 * @brief Forwards set change slot mapping event to InputDevice
 */
void SetJoystick::propagateSetSensorButtonAssociation(JoySensorDirection direction, JoySensorType sensor, int newset,
                                                      int mode)
{
    if (newset != m_index)
        emit setAssignmentSensorChanged(direction, sensor, m_index, newset, mode);
}

void SetJoystick::propogateSetDPadButtonAssociation(int button, int dpad, int newset, int mode)
{
    if (newset != m_index)
        emit setAssignmentDPadChanged(button, dpad, m_index, newset, mode);
}

void SetJoystick::propogateSetVDPadButtonAssociation(int button, int dpad, int newset, int mode)
{
    if (newset != m_index)
        emit setAssignmentVDPadChanged(button, dpad, m_index, newset, mode);
}

/**
 * @brief Perform a release of all elements of a set. Stick and vdpad
 *     releases will be handled by the associated button or axis.
 */
void SetJoystick::release()
{
    QHashIterator<int, JoyAxis *> iterAxes(axes);

    while (iterAxes.hasNext())
    {
        JoyAxis *axis = iterAxes.next().value();
        axis->clearPendingEvent();
        axis->joyEvent(axis->getCurrentThrottledDeadValue(), true);
        axis->eventReset();
    }

    QHashIterator<int, JoyDPad *> iterDPads(getHats());

    while (iterDPads.hasNext())
    {
        JoyDPad *dpad = iterDPads.next().value();
        dpad->clearPendingEvent();
        dpad->joyEvent(0, true);
        dpad->eventReset();
    }

    for (auto &sensor : m_sensors)
    {
        float values[3] = {0};
        sensor->clearPendingEvent();
        sensor->joyEvent(values, true);
    }

    QHashIterator<int, JoyButton *> iterButtons(getButtons());

    while (iterButtons.hasNext())
    {
        JoyButton *button = iterButtons.next().value();
        button->clearPendingEvent();
        button->joyEvent(false, true);
        button->eventReset();
    }
}

/**
 * @brief Check if this set has any mapped event.
 * @returns True if any event is mapped to a keyboard or mouse event, false otherwise.
 */
bool SetJoystick::isSetEmpty()
{
    bool result = true;
    QHashIterator<int, JoyButton *> iter(getButtons());

    while (iter.hasNext() && result)
    {
        JoyButton *button = iter.next().value();

        if (!button->isDefault())
            result = false;
    }

    QHashIterator<int, JoyAxis *> iter2(axes);

    while (iter2.hasNext() && result)
    {
        JoyAxis *axis = iter2.next().value();

        if (!axis->isDefault())
            result = false;
    }

    QHashIterator<int, JoyDPad *> iter3(getHats());

    while (iter3.hasNext() && result)
    {
        JoyDPad *dpad = iter3.next().value();

        if (!dpad->isDefault())
            result = false;
    }

    QHashIterator<int, JoyControlStick *> iter4(getSticks());

    while (iter4.hasNext() && result)
    {
        JoyControlStick *stick = iter4.next().value();

        if (!stick->isDefault())
            result = false;
    }

    for (const auto &sensor : m_sensors)
    {
        if (!result)
            break;

        if (!sensor->isDefault())
            result = false;
    }

    QHashIterator<int, VDPad *> iter5(getVdpads());

    while (iter5.hasNext() && result)
    {
        VDPad *vdpad = iter5.next().value();

        if (!vdpad->isDefault())
            result = false;
    }

    return result;
}

void SetJoystick::propogateSetAxisThrottleSetting(int index)
{
    JoyAxis *axis = axes.value(index);

    if (axis != nullptr)
        emit setAssignmentAxisThrottleChanged(index, axis->getCurrentlyAssignedSet());
}

void SetJoystick::addControlStick(int index, JoyControlStick *stick)
{
    sticks.insert(index, stick);
    connect(stick, &JoyControlStick::stickNameChanged, this, &SetJoystick::propogateSetStickNameChange);

    QHashIterator<JoyStickDirectionsType::JoyStickDirections, JoyControlStickButton *> iter(*stick->getButtons());

    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();

        if (button != nullptr)
        {
            connect(button, &JoyControlStickButton::setChangeActivated, this, &SetJoystick::propogateSetChange);
            connect(button, &JoyControlStickButton::setAssignmentChanged, this,
                    &SetJoystick::propogateSetStickButtonAssociation);
            connect(button, &JoyControlStickButton::clicked, this, &SetJoystick::propogateSetStickButtonClick,
                    Qt::QueuedConnection);
            connect(button, &JoyControlStickButton::released, this, &SetJoystick::propogateSetStickButtonRelease,
                    Qt::QueuedConnection);
            connect(button, &JoyControlStickButton::buttonNameChanged, this,
                    &SetJoystick::propogateSetStickButtonNameChange);
        }
    }
}

void SetJoystick::removeControlStick(int index)
{
    if (sticks.contains(index))
    {
        JoyControlStick *stick = getSticks().value(index);
        sticks.remove(index);
        stick->deleteLater();
        stick = nullptr;
    }
}

void SetJoystick::addVDPad(int index, VDPad *vdpad)
{
    vdpads.insert(index, vdpad);
    connect(vdpad, &VDPad::dpadNameChanged, this, &SetJoystick::propogateSetVDPadNameChange);

    QHashIterator<int, JoyDPadButton *> iter(*vdpad->getButtons());

    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();

        if (button != nullptr)
        {
            connect(button, &JoyDPadButton::setChangeActivated, this, &SetJoystick::propogateSetChange);
            connect(button, &JoyDPadButton::setAssignmentChanged, this, &SetJoystick::propogateSetVDPadButtonAssociation);
            connect(button, &JoyDPadButton::clicked, this, &SetJoystick::propogateSetDPadButtonClick, Qt::QueuedConnection);
            connect(button, &JoyDPadButton::released, this, &SetJoystick::propogateSetDPadButtonRelease,
                    Qt::QueuedConnection);
            connect(button, &JoyDPadButton::buttonNameChanged, this, &SetJoystick::propogateSetVDPadButtonNameChange);
        }
    }
}

void SetJoystick::removeVDPad(int index)
{
    if (vdpads.contains(index))
    {
        VDPad *vdpad = vdpads.value(index);
        vdpads.remove(index);
        vdpad->deleteLater();
        vdpad = nullptr;
    }
}

int SetJoystick::getIndex() const { return m_index; }

int SetJoystick::getRealIndex() const { return m_index + 1; }

void SetJoystick::propogateSetButtonClick(int button)
{
    JoyButton *jButton = qobject_cast<JoyButton *>(sender());

    if (jButton != nullptr && !jButton->getIgnoreEventState())
    {
        emit setButtonClick(m_index, button);
        lastClickedButtons.append(jButton);

        qDebug() << "Added button " << jButton->getPartialName(false, true) << " to list";
        qDebug() << "List has " << getLastClickedButtons().count() << " buttons";
    }
}

QList<JoyButton *> const &SetJoystick::getLastClickedButtons() const { return lastClickedButtons; }

void SetJoystick::removeAllBtnFromQueue()
{
    if (!getLastClickedButtons().isEmpty())
        lastClickedButtons.clear();
}

int SetJoystick::getCountBtnInList(QString partialName)
{
    int count = 0;

    foreach (const JoyButton *joyBtn, getLastClickedButtons())
    {
        if (joyBtn->getPartialName(false, true) == partialName)
            count++;
    }

    return count;
}

void SetJoystick::propogateSetButtonRelease(int button)
{
    JoyButton *jButton = qobject_cast<JoyButton *>(sender());

    if (jButton != nullptr)
    {
        if (!jButton->getIgnoreEventState())
            emit setButtonRelease(m_index, button);
    }
}

void SetJoystick::propogateSetAxisButtonClick(int button)
{
    JoyAxisButton *axisButton = qobject_cast<JoyAxisButton *>(sender());

    if (axisButton != nullptr)
    {
        JoyAxis *axis = axisButton->getAxis();

        if (!axisButton->getIgnoreEventState())
            emit setAxisButtonClick(m_index, axis->getIndex(), button);
    }
}

void SetJoystick::propogateSetAxisButtonRelease(int button)
{
    JoyAxisButton *axisButton = qobject_cast<JoyAxisButton *>(sender());

    if (axisButton != nullptr)
    {
        JoyAxis *axis = axisButton->getAxis();

        if (!axisButton->getIgnoreEventState())
            emit setAxisButtonRelease(m_index, axis->getIndex(), button);
    }
}

void SetJoystick::propogateSetStickButtonClick(int button)
{
    JoyControlStickButton *stickButton = qobject_cast<JoyControlStickButton *>(sender());

    if (stickButton != nullptr)
    {
        JoyControlStick *stick = stickButton->getStick();

        if (stick && !stickButton->getIgnoreEventState())
            emit setStickButtonClick(m_index, stick->getIndex(), button);
    }
}

void SetJoystick::propogateSetStickButtonRelease(int button)
{
    JoyControlStickButton *stickButton = qobject_cast<JoyControlStickButton *>(sender());

    if (stickButton != nullptr)
    {
        JoyControlStick *stick = stickButton->getStick();

        if (!stickButton->getIgnoreEventState())
            emit setStickButtonRelease(m_index, stick->getIndex(), button);
    }
}

void SetJoystick::propagateSetSensorButtonClick(int button)
{
    JoySensorButton *sensorButton = qobject_cast<JoySensorButton *>(sender());

    if (sensorButton != nullptr)
    {
        JoySensor *sensor = sensorButton->getSensor();

        if (sensor && !sensorButton->getIgnoreEventState())
            emit setSensorButtonClick(m_index, sensor->getType(), static_cast<JoySensorDirection>(button));
    }
}

void SetJoystick::propagateSetSensorButtonRelease(int button)
{
    JoySensorButton *sensorButton = qobject_cast<JoySensorButton *>(sender());

    if (sensorButton != nullptr)
    {
        JoySensor *sensor = sensorButton->getSensor();

        if (!sensorButton->getIgnoreEventState())
            emit setSensorButtonRelease(m_index, sensor->getType(), static_cast<JoySensorDirection>(button));
    }
}

void SetJoystick::propogateSetDPadButtonClick(int button)
{
    JoyDPadButton *dpadButton = qobject_cast<JoyDPadButton *>(sender());

    if (dpadButton)
    {
        JoyDPad *dpad = dpadButton->getDPad();

        if (dpad && dpadButton->getButtonState() && !dpadButton->getIgnoreEventState())
        {
            emit setDPadButtonClick(m_index, dpad->getIndex(), button);
        }
    }
}

void SetJoystick::propogateSetDPadButtonRelease(int button)
{
    JoyDPadButton *dpadButton = qobject_cast<JoyDPadButton *>(sender());

    if (dpadButton != nullptr)
    {
        JoyDPad *dpad = dpadButton->getDPad();

        if (dpad && !dpadButton->getButtonState() && !dpadButton->getIgnoreEventState())
        {
            emit setDPadButtonRelease(m_index, dpad->getIndex(), button);
        }
    }
}

void SetJoystick::propogateSetButtonNameChange()
{
    JoyButton *button = qobject_cast<JoyButton *>(sender());
    disconnect(button, &JoyButton::buttonNameChanged, this, &SetJoystick::propogateSetButtonNameChange);
    emit setButtonNameChange(button->getJoyNumber());
    connect(button, &JoyButton::buttonNameChanged, this, &SetJoystick::propogateSetButtonNameChange);
}

void SetJoystick::propogateSetAxisButtonNameChange()
{
    JoyAxisButton *button = qobject_cast<JoyAxisButton *>(sender());
    disconnect(button, &JoyAxisButton::buttonNameChanged, this, &SetJoystick::propogateSetAxisButtonNameChange);
    emit setAxisButtonNameChange(button->getAxis()->getIndex(), button->getJoyNumber());
    connect(button, &JoyAxisButton::buttonNameChanged, this, &SetJoystick::propogateSetAxisButtonNameChange);
}

void SetJoystick::propogateSetStickButtonNameChange()
{
    JoyControlStickButton *button = qobject_cast<JoyControlStickButton *>(sender());
    disconnect(button, &JoyControlStickButton::buttonNameChanged, this, &SetJoystick::propogateSetStickButtonNameChange);
    emit setStickButtonNameChange(button->getStick()->getIndex(), button->getJoyNumber());
    connect(button, &JoyControlStickButton::buttonNameChanged, this, &SetJoystick::propogateSetStickButtonNameChange);
}

/**
 * @brief Propagate button rename event to InputDevice
 */
void SetJoystick::propagateSetSensorButtonNameChange()
{
    JoySensorButton *button = qobject_cast<JoySensorButton *>(sender());
    disconnect(button, &JoySensorButton::buttonNameChanged, this, &SetJoystick::propagateSetSensorButtonNameChange);
    emit setSensorButtonNameChange(button->getSensor()->getType(), static_cast<JoySensorDirection>(button->getJoyNumber()));
    connect(button, &JoySensorButton::buttonNameChanged, this, &SetJoystick::propagateSetSensorButtonNameChange);
}

void SetJoystick::propogateSetDPadButtonNameChange()
{
    JoyDPadButton *button = qobject_cast<JoyDPadButton *>(sender());
    disconnect(button, &JoyDPadButton::buttonNameChanged, this, &SetJoystick::propogateSetDPadButtonNameChange);
    emit setDPadButtonNameChange(button->getDPad()->getIndex(), button->getJoyNumber());
    connect(button, &JoyDPadButton::buttonNameChanged, this, &SetJoystick::propogateSetDPadButtonNameChange);
}

void SetJoystick::propogateSetVDPadButtonNameChange()
{
    JoyDPadButton *button = qobject_cast<JoyDPadButton *>(sender());
    disconnect(button, &JoyDPadButton::buttonNameChanged, this, &SetJoystick::propogateSetVDPadButtonNameChange);
    emit setVDPadButtonNameChange(button->getDPad()->getIndex(), button->getJoyNumber());
    connect(button, &JoyDPadButton::buttonNameChanged, this, &SetJoystick::propogateSetVDPadButtonNameChange);
}

void SetJoystick::propogateSetAxisNameChange()
{
    JoyAxis *axis = qobject_cast<JoyAxis *>(sender());
    disconnect(axis, &JoyAxis::axisNameChanged, this, &SetJoystick::propogateSetAxisNameChange);
    emit setAxisNameChange(axis->getIndex());
    connect(axis, &JoyAxis::axisNameChanged, this, &SetJoystick::propogateSetAxisNameChange);
}

void SetJoystick::propogateSetStickNameChange()
{
    JoyControlStick *stick = qobject_cast<JoyControlStick *>(sender());
    disconnect(stick, &JoyControlStick::stickNameChanged, this, &SetJoystick::propogateSetStickNameChange);
    emit setStickNameChange(stick->getIndex());
    connect(stick, &JoyControlStick::stickNameChanged, this, &SetJoystick::propogateSetStickNameChange);
}

void SetJoystick::propagateSetSensorNameChange()
{
    JoySensor *sensor = qobject_cast<JoySensor *>(sender());
    disconnect(sensor, &JoySensor::sensorNameChanged, this, &SetJoystick::propagateSetSensorNameChange);
    emit setSensorNameChange(sensor->getType());
    connect(sensor, &JoySensor::sensorNameChanged, this, &SetJoystick::propagateSetSensorNameChange);
}

void SetJoystick::propogateSetDPadNameChange()
{
    JoyDPad *dpad = qobject_cast<JoyDPad *>(sender());
    disconnect(dpad, &JoyDPad::dpadNameChanged, this, &SetJoystick::propogateSetDPadButtonNameChange);
    emit setDPadNameChange(dpad->getIndex());
    connect(dpad, &JoyDPad::dpadNameChanged, this, &SetJoystick::propogateSetDPadButtonNameChange);
}

void SetJoystick::propogateSetVDPadNameChange()
{
    VDPad *vdpad = qobject_cast<VDPad *>(sender());
    disconnect(vdpad, &VDPad::dpadNameChanged, this, &SetJoystick::propogateSetVDPadNameChange);
    emit setVDPadNameChange(vdpad->getIndex());
    connect(vdpad, &VDPad::dpadNameChanged, this, &SetJoystick::propogateSetVDPadNameChange);
}

void SetJoystick::setIgnoreEventState(bool ignore)
{
    QHashIterator<int, JoyButton *> iter(getButtons());

    while (iter.hasNext())
    {
        JoyButton *button = iter.next().value();

        if (button != nullptr)
            button->setIgnoreEventState(ignore);
    }

    QHashIterator<int, JoyAxis *> iter2(axes);

    while (iter2.hasNext())
    {
        JoyAxis *axis = iter2.next().value();

        if (axis != nullptr)
        {
            JoyAxisButton *naxisbutton = axis->getNAxisButton();
            naxisbutton->setIgnoreEventState(ignore);

            JoyAxisButton *paxisbutton = axis->getPAxisButton();
            paxisbutton->setIgnoreEventState(ignore);
        }
    }

    QHashIterator<int, JoyDPad *> iter3(getHats());

    while (iter3.hasNext())
    {
        JoyDPad *dpad = iter3.next().value();

        if (dpad != nullptr)
        {
            QHash<int, JoyDPadButton *> *dpadbuttons = dpad->getButtons();
            QHashIterator<int, JoyDPadButton *> iterdpadbuttons(*dpadbuttons);

            while (iterdpadbuttons.hasNext())
            {
                JoyDPadButton *dpadbutton = iterdpadbuttons.next().value();

                if (dpadbutton)
                    dpadbutton->setIgnoreEventState(ignore);
            }
        }
    }

    QHashIterator<int, JoyControlStick *> iter4(getSticks());

    while (iter4.hasNext())
    {
        JoyControlStick *stick = iter4.next().value();

        if (stick != nullptr)
        {
            QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton *> *stickButtons = stick->getButtons();
            QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton *> iterstickbuttons(*stickButtons);

            while (iterstickbuttons.hasNext())
            {
                JoyControlStickButton *stickbutton = iterstickbuttons.next().value();
                stickbutton->setIgnoreEventState(ignore);
            }
        }
    }

    QHashIterator<int, VDPad *> iter5(getVdpads());

    while (iter5.hasNext())
    {
        VDPad *vdpad = iter5.next().value();

        if (vdpad != nullptr)
        {
            QHash<int, JoyDPadButton *> *dpadbuttons = vdpad->getButtons();
            QHashIterator<int, JoyDPadButton *> itervdpadbuttons(*dpadbuttons);

            while (itervdpadbuttons.hasNext())
            {
                JoyDPadButton *dpadbutton = itervdpadbuttons.next().value();
                dpadbutton->setIgnoreEventState(ignore);
            }
        }
    }
}

void SetJoystick::propogateSetAxisActivated(int value)
{
    JoyAxis *axis = qobject_cast<JoyAxis *>(sender()); // static_cast
    emit setAxisActivated(m_index, axis->getIndex(), value);
}

void SetJoystick::propogateSetAxisReleased(int value)
{
    JoyAxis *axis = qobject_cast<JoyAxis *>(sender()); // static_cast
    emit setAxisReleased(m_index, axis->getIndex(), value);
}

void SetJoystick::enableButtonConnections(JoyButton *button)
{
    connect(button, &JoyButton::setChangeActivated, this, &SetJoystick::propogateSetChange);
    connect(button, &JoyButton::setAssignmentChanged, this, &SetJoystick::propogateSetButtonAssociation);
    connect(button, &JoyButton::clicked, this, &SetJoystick::propogateSetButtonClick, Qt::QueuedConnection);
    connect(button, &JoyButton::clicked, m_device, &InputDevice::buttonClickEvent, Qt::QueuedConnection);
    connect(button, &JoyButton::released, this, &SetJoystick::propogateSetButtonRelease);
    connect(button, &JoyButton::released, m_device, &InputDevice::buttonReleaseEvent);
    connect(button, &JoyButton::buttonNameChanged, this, &SetJoystick::propogateSetButtonNameChange);
}

void SetJoystick::enableAxisConnections(JoyAxis *axis)
{
    connect(axis, &JoyAxis::throttleChangePropogated, this, &SetJoystick::propogateSetAxisThrottleSetting);
    connect(axis, &JoyAxis::axisNameChanged, this, &SetJoystick::propogateSetAxisNameChange);
    connect(axis, &JoyAxis::active, this, &SetJoystick::propogateSetAxisActivated);
    connect(axis, &JoyAxis::released, this, &SetJoystick::propogateSetAxisReleased);

    JoyAxisButton *button = axis->getNAxisButton();
    connect(button, &JoyAxisButton::setChangeActivated, this, &SetJoystick::propogateSetChange);
    connect(button, &JoyAxisButton::setAssignmentChanged, this, &SetJoystick::propogateSetAxisButtonAssociation);
    connect(button, &JoyAxisButton::clicked, this, &SetJoystick::propogateSetAxisButtonClick, Qt::QueuedConnection);
    connect(button, &JoyAxisButton::released, this, &SetJoystick::propogateSetAxisButtonRelease, Qt::QueuedConnection);
    connect(button, &JoyAxisButton::buttonNameChanged, this, &SetJoystick::propogateSetAxisButtonNameChange);

    button = axis->getPAxisButton();
    connect(button, &JoyAxisButton::setChangeActivated, this, &SetJoystick::propogateSetChange);
    connect(button, &JoyAxisButton::setAssignmentChanged, this, &SetJoystick::propogateSetAxisButtonAssociation);
    connect(button, &JoyAxisButton::clicked, this, &SetJoystick::propogateSetAxisButtonClick, Qt::QueuedConnection);
    connect(button, &JoyAxisButton::released, this, &SetJoystick::propogateSetAxisButtonRelease, Qt::QueuedConnection);
    connect(button, &JoyAxisButton::buttonNameChanged, this, &SetJoystick::propogateSetAxisButtonNameChange);
}

void SetJoystick::enableHatConnections(JoyDPad *dpad)
{
    connect(dpad, &JoyDPad::dpadNameChanged, this, &SetJoystick::propogateSetDPadNameChange);

    QHash<int, JoyDPadButton *> *buttons = dpad->getJoyButtons();
    QHashIterator<int, JoyDPadButton *> iter(*buttons);

    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        connect(button, &JoyDPadButton::setChangeActivated, this, &SetJoystick::propogateSetChange);
        connect(button, &JoyDPadButton::setAssignmentChanged, this, &SetJoystick::propogateSetDPadButtonAssociation);

        connect(button, &JoyDPadButton::clicked, this, &SetJoystick::propogateSetDPadButtonClick, Qt::QueuedConnection);
        connect(button, &JoyDPadButton::clicked, m_device, &InputDevice::dpadButtonClickEvent, Qt::QueuedConnection);
        connect(button, &JoyDPadButton::released, this, &SetJoystick::propogateSetDPadButtonRelease, Qt::QueuedConnection);
        connect(button, &JoyDPadButton::released, m_device, &InputDevice::dpadButtonReleaseEvent, Qt::QueuedConnection);
        connect(button, &JoyDPadButton::buttonNameChanged, this, &SetJoystick::propogateSetDPadButtonNameChange);
    }
}

/**
 * @brief Establishes connections for event propagation between JoySensor and InputDevice
 */
void SetJoystick::enableSensorConnections(JoySensor *sensor)
{
    connect(sensor, &JoySensor::sensorNameChanged, this, &SetJoystick::propagateSetSensorNameChange);

    auto buttons = sensor->getButtons();
    for (auto iter = buttons->cbegin(); iter != buttons->cend(); ++iter)
    {
        connect(iter.value(), &JoySensorButton::setChangeActivated, this, &SetJoystick::propogateSetChange);
        connect(iter.value(), &JoySensorButton::setAssignmentChanged, this,
                &SetJoystick::propagateSetSensorButtonAssociation);
        connect(iter.value(), &JoySensorButton::clicked, this, &SetJoystick::propagateSetSensorButtonClick,
                Qt::QueuedConnection);
        connect(iter.value(), &JoySensorButton::released, this, &SetJoystick::propagateSetSensorButtonRelease,
                Qt::QueuedConnection);
        connect(iter.value(), &JoySensorButton::buttonNameChanged, this, &SetJoystick::propagateSetSensorButtonNameChange);
    }
}

InputDevice *SetJoystick::getInputDevice() const { return m_device; }

void SetJoystick::setName(QString name)
{
    if (name.length() <= GlobalVariables::SetJoystick::MAXNAMELENGTH)
    {
        m_name = name;
        emit propertyUpdated();
    } else
    {
        name.truncate(GlobalVariables::SetJoystick::MAXNAMELENGTH -
                      3); // Truncate name to 27 characters. Add ellipsis at the end.
        m_name = QString(name).append("...");
        emit propertyUpdated();
    }
}

QString SetJoystick::getName() const { return m_name; }

void SetJoystick::copyAssignments(SetJoystick *destSet)
{
    for (int i = 0; i < m_device->getNumberAxes(); i++)
    {
        JoyAxis *sourceAxis = axes.value(i);
        JoyAxis *destAxis = destSet->axes.value(i);

        if (sourceAxis && destAxis)
            sourceAxis->copyAssignments(destAxis);
    }

    QHashIterator<int, JoyControlStick *> stickIter(getSticks());

    while (stickIter.hasNext())
    {
        stickIter.next();
        int index = stickIter.key();
        JoyControlStick *sourceStick = stickIter.value();
        JoyControlStick *destStick = destSet->getSticks().value(index);

        if (sourceStick && destStick)
            sourceStick->copyAssignments(destStick);
    }

    for (auto iter = m_sensors.cbegin(); iter != m_sensors.cend(); ++iter)
    {
        JoySensorType type = iter.key();
        JoySensor *sourceSensor = iter.value();
        JoySensor *destSensor = destSet->getSensor(type);

        if (sourceSensor && destSensor)
            sourceSensor->copyAssignments(destSensor);
    }

    for (int i = 0; i < m_device->getNumberHats(); i++)
    {
        JoyDPad *sourceDPad = getHats().value(i);
        JoyDPad *destDPad = destSet->getHats().value(i);

        if (sourceDPad && destDPad)
            sourceDPad->copyAssignments(destDPad);
    }

    QHashIterator<int, VDPad *> vdpadIter(getVdpads());

    while (vdpadIter.hasNext())
    {
        vdpadIter.next();
        int index = vdpadIter.key();
        VDPad *sourceVDpad = vdpadIter.value();
        VDPad *destVDPad = destSet->getVdpads().value(index);

        if (sourceVDpad && destVDPad)
            sourceVDpad->copyAssignments(destVDPad);
    }

    for (int i = 0; i < m_device->getNumberButtons(); i++)
    {
        JoyButton *sourceButton = getButtons().value(i);
        JoyButton *destButton = destSet->getButtons().value(i);

        if (sourceButton && destButton)
            sourceButton->copyAssignments(destButton);
    }
}

QString SetJoystick::getSetLabel()
{
    QString temp = QString();

    if (!m_name.isEmpty())
        temp = tr("Set %1: %2").arg(m_index + 1).arg(m_name);
    else
        temp = tr("Set %1").arg(m_index + 1);

    return temp;
}

void SetJoystick::establishPropertyUpdatedConnection()
{
    connect(this, &SetJoystick::propertyUpdated, getInputDevice(), &InputDevice::profileEdited);
}

void SetJoystick::disconnectPropertyUpdatedConnection()
{
    disconnect(this, &SetJoystick::propertyUpdated, getInputDevice(), &InputDevice::profileEdited);
}

/**
 * @brief Raise the dead zones for axes. Used when launching
 *     the controller mapping window.
 */
void SetJoystick::raiseAxesDeadZones(int deadZone)
{
    int tempDeadZone = deadZone;

    if ((deadZone <= 0) || (deadZone > 32767))
        tempDeadZone = GlobalVariables::SetJoystick::RAISEDDEADZONE;

    QHashIterator<int, JoyAxis *> axisIter(axes);

    while (axisIter.hasNext())
    {
        JoyAxis *temp = axisIter.next().value();
        temp->disconnectPropertyUpdatedConnection();
        temp->setDeadZone(tempDeadZone);
        temp->establishPropertyUpdatedConnection();
    }
}

void SetJoystick::currentAxesDeadZones(QList<int> *axesDeadZones)
{
    QHashIterator<int, JoyAxis *> axisIter(axes);

    while (axisIter.hasNext())
    {
        JoyAxis *temp = axisIter.next().value();
        axesDeadZones->append(temp->getDeadZone());
    }
}

void SetJoystick::setAxesDeadZones(QList<int> *axesDeadZones)
{
    QListIterator<int> iter(*axesDeadZones);
    int axisNum = 0;

    while (iter.hasNext())
    {
        int deadZoneValue = iter.next();

        if (axes.contains(axisNum))
        {
            JoyAxis *temp = getJoyAxis(axisNum);
            temp->disconnectPropertyUpdatedConnection();
            temp->setDeadZone(deadZoneValue);
            temp->establishPropertyUpdatedConnection();
        }

        axisNum++;
    }
}

void SetJoystick::setAxisThrottle(int axisNum, JoyAxis::ThrottleTypes throttle)
{
    if (axes.contains(axisNum))
    {
        JoyAxis *temp = axes.value(axisNum);
        temp->setInitialThrottle(throttle);
    }
}

QHash<int, JoyAxis *> *SetJoystick::getAxes() { return &axes; }

QHash<int, JoyButton *> const &SetJoystick::getButtons() const { return m_buttons; }

QHash<int, JoyDPad *> const &SetJoystick::getHats() const { return hats; }

QHash<int, JoyControlStick *> const &SetJoystick::getSticks() const { return sticks; }

/**
 * @brief Get all sensor objects in this set.
 * @returns Sensors in this set
 */
QHash<JoySensorType, JoySensor *> const &SetJoystick::getSensors() const { return m_sensors; }

QHash<int, VDPad *> const &SetJoystick::getVdpads() const { return vdpads; }
