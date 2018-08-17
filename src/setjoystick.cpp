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

#include "setjoystick.h"

#include "messagehandler.h"
#include "inputdevice.h"
#include "joycontrolstick.h"
#include "joydpad.h"
#include "joybutton.h"
#include "vdpad.h"
#include "joybuttontypes/joycontrolstickbutton.h"

#include <QDebug>
#include <QHashIterator>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QtAlgorithms>


const int SetJoystick::MAXNAMELENGTH = 30;
const int SetJoystick::RAISEDDEADZONE = 20000;

SetJoystick::SetJoystick(InputDevice *device, int index, QObject *parent) :
    QObject(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    m_device = device;
    m_index = index;

    reset();
}

SetJoystick::SetJoystick(InputDevice *device, int index, bool runreset, QObject *parent) :
    QObject(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    m_device = device;
    m_index = index;

    if (runreset)
    {
        reset();
    }
}

SetJoystick::~SetJoystick()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    deleteSticks();
    deleteVDpads();
    deleteButtons();
    deleteAxes();
    deleteHats();

    removeAllBtnFromQueue();
}

JoyButton* SetJoystick::getJoyButton(int index) const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getButtons().value(index);
}

JoyAxis* SetJoystick::getJoyAxis(int index) const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return axes.value(index);
}

JoyDPad* SetJoystick::getJoyDPad(int index) const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getHats().value(index);
}

VDPad* SetJoystick::getVDPad(int index) const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getVdpads().value(index);
}

JoyControlStick* SetJoystick::getJoyStick(int index) const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getSticks().value(index);
}

void SetJoystick::refreshButtons()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    deleteButtons();

    for (int i=0; i < m_device->getNumberRawButtons(); i++)
    {
        JoyButton *button = new JoyButton (i, m_index, this, this);
        m_buttons.insert(i, button);
        enableButtonConnections(button);
    }
}

void SetJoystick::refreshAxes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    deleteAxes();

    InputDevice *device = getInputDevice();
    for (int i=0; i < device->getNumberRawAxes(); i++)
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    deleteHats();

    for (int i=0; i < m_device->getNumberRawHats(); i++)
    {
        JoyDPad *dpad = new JoyDPad(i, m_index, this, this);
        hats.insert(i, dpad);
        enableHatConnections(dpad);
    }
}

void SetJoystick::deleteButtons()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<int, JoyButton*> iter(getButtons());
    while (iter.hasNext())
    {
        JoyButton *button = iter.next().value();
        if (button != nullptr)
        {
            delete button;
            button = nullptr;
        }
    }

    m_buttons.clear();
}


void SetJoystick::deleteAxes()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<int, JoyAxis*> iter(axes);
    while (iter.hasNext())
    {
        JoyAxis *axis = iter.next().value();
        if (axis != nullptr)
        {
            delete axis;
            axis = nullptr;
        }
    }

    axes.clear();
}

void SetJoystick::deleteSticks()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<int, JoyControlStick*> iter(getSticks());
    while (iter.hasNext())
    {
        JoyControlStick *stick = iter.next().value();
        if (stick != nullptr)
        {
            delete stick;
            stick = nullptr;
        }
    }

    sticks.clear();
}

void SetJoystick::deleteVDpads()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<int, VDPad*> iter(getVdpads());
    while (iter.hasNext())
    {
        VDPad *dpad = iter.next().value();
        if (dpad != nullptr)
        {
            delete dpad;
            dpad = nullptr;
        }
    }

    vdpads.clear();
}

void SetJoystick::deleteHats()
{

    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<int, JoyDPad*> iter(getHats());
    while (iter.hasNext())
    {
        JoyDPad *dpad = iter.next().value();
        if (dpad != nullptr)
        {
            delete dpad;
            dpad = nullptr;
        }
    }

    hats.clear();
}

int SetJoystick::getNumberButtons() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getButtons().count();
}

int SetJoystick::getNumberAxes() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return axes.count();
}

int SetJoystick::getNumberHats() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getHats().count();
}

int SetJoystick::getNumberSticks() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getSticks().size();
}

int SetJoystick::getNumberVDPads() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return getVdpads().size();
}

void SetJoystick::reset()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    deleteSticks();
    deleteVDpads();
    refreshAxes();
    refreshButtons();
    refreshHats();
    m_name = QString();
}

void SetJoystick::propogateSetChange(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    emit setChangeActivated(index);
}

void SetJoystick::propogateSetButtonAssociation(int button, int newset, int mode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (newset != m_index)
    {
        emit setAssignmentButtonChanged(button, m_index, newset, mode);
    }
}

void SetJoystick::propogateSetAxisButtonAssociation(int button, int axis, int newset, int mode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (newset != m_index)
    {
        emit setAssignmentAxisChanged(button, axis, m_index, newset, mode);
    }
}

void SetJoystick::propogateSetStickButtonAssociation(int button, int stick, int newset, int mode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (newset != m_index)
    {
        emit setAssignmentStickChanged(button, stick, m_index, newset, mode);
    }
}

void SetJoystick::propogateSetDPadButtonAssociation(int button, int dpad, int newset, int mode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (newset != m_index)
    {
        emit setAssignmentDPadChanged(button, dpad, m_index, newset, mode);
    }
}

void SetJoystick::propogateSetVDPadButtonAssociation(int button, int dpad, int newset, int mode)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (newset != m_index)
    {
        emit setAssignmentVDPadChanged(button, dpad, m_index, newset, mode);
    }
}

/**
 * @brief Perform a release of all elements of a set. Stick and vdpad
 *     releases will be handled by the associated button or axis.
 */
void SetJoystick::release()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<int, JoyAxis*> iterAxes(axes);
    while (iterAxes.hasNext())
    {
        JoyAxis *axis = iterAxes.next().value();
        axis->clearPendingEvent();
        axis->joyEvent(axis->getCurrentThrottledDeadValue(), true);
        axis->eventReset();

    }

    QHashIterator<int, JoyDPad*> iterDPads(getHats());
    while (iterDPads.hasNext())
    {
        JoyDPad *dpad = iterDPads.next().value();
        dpad->clearPendingEvent();
        dpad->joyEvent(0, true);
        dpad->eventReset();
    }

    QHashIterator<int, JoyButton*> iterButtons(getButtons());
    while (iterButtons.hasNext())
    {
        JoyButton *button = iterButtons.next().value();
        button->clearPendingEvent();
        button->joyEvent(false, true);
        button->eventReset();
    }
}

void SetJoystick::readConfig(QXmlStreamReader *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (xml->isStartElement() && (xml->name() == "set"))
    {

        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name() != "set")))
        {
            if ((xml->name() == "button") && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyButton *button = getJoyButton(index-1);
                if (button != nullptr)
                {
                    button->readConfig(xml);
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if ((xml->name() == "axis") && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyAxis *axis = getJoyAxis(index-1);
                if (axis != nullptr)
                {
                    axis->readConfig(xml);
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if ((xml->name() == "dpad") && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyDPad *dpad = getJoyDPad(index-1);
                if (dpad)
                {
                    dpad->readConfig(xml);
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if ((xml->name() == "stick") && xml->isStartElement())
            {
                int stickIndex = xml->attributes().value("index").toString().toInt();

                if (stickIndex > 0)
                {
                    stickIndex -= 1;
                    JoyControlStick *stick = getJoyStick(stickIndex);
                    if (stick)
                    {
                        stick->readConfig(xml);
                    }
                    else
                    {
                        xml->skipCurrentElement();
                    }
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if ((xml->name() == "vdpad") && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                VDPad *vdpad = getVDPad(index-1);
                if (vdpad)
                {
                    vdpad->readConfig(xml);
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if ((xml->name() == "name") && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                if (!temptext.isEmpty())
                {
                    setName(temptext);
                }
            }
            else
            {
                // If none of the above, skip the element
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }
    }
}

void SetJoystick::writeConfig(QXmlStreamWriter *xml)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (!isSetEmpty())
    {
        xml->writeStartElement("set");

        xml->writeAttribute("index", QString::number(m_index+1));

        if (!m_name.isEmpty())
        {
            xml->writeTextElement("name", m_name);
        }

        for (int i=0; i < getNumberSticks(); i++)
        {
            JoyControlStick *stick = getJoyStick(i);
            stick->writeConfig(xml);
        }

        for (int i=0; i < getNumberVDPads(); i++)
        {
            VDPad *vdpad = getVDPad(i);
            if (vdpad != nullptr)
            {
                vdpad->writeConfig(xml);
            }
        }

        for (int i=0; i < getNumberAxes(); i++)
        {
            JoyAxis *axis = getJoyAxis(i);
            if (!axis->isPartControlStick() && axis->hasControlOfButtons())
            {
                axis->writeConfig(xml);
            }
        }

        for (int i=0; i < getNumberHats(); i++)
        {
            JoyDPad *dpad = getJoyDPad(i);
            dpad->writeConfig(xml);
        }

        for (int i=0; i < getNumberButtons(); i++)
        {
            JoyButton *button = getJoyButton(i);
            if ((button != nullptr) && !button->isPartVDPad())
            {
                button->writeConfig(xml);
            }
        }

        xml->writeEndElement();
    }
}

bool SetJoystick::isSetEmpty()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool result = true;
    QHashIterator<int, JoyButton*> iter(getButtons());
    while (iter.hasNext() && result)
    {
        JoyButton *button = iter.next().value();
        if (!button->isDefault())
        {
            result = false;
        }
    }

    QHashIterator<int, JoyAxis*> iter2(axes);
    while (iter2.hasNext() && result)
    {
        JoyAxis *axis = iter2.next().value();
        if (!axis->isDefault())
        {
            result = false;
        }
    }

    QHashIterator<int, JoyDPad*> iter3(getHats());
    while (iter3.hasNext() && result)
    {
        JoyDPad *dpad = iter3.next().value();
        if (!dpad->isDefault())
        {
            result = false;
        }
    }

    QHashIterator<int, JoyControlStick*> iter4(getSticks());
    while (iter4.hasNext() && result)
    {
        JoyControlStick *stick = iter4.next().value();
        if (!stick->isDefault())
        {
            result = false;
        }
    }

    QHashIterator<int, VDPad*> iter5(getVdpads());
    while (iter5.hasNext() && result)
    {
        VDPad *vdpad = iter5.next().value();
        if (!vdpad->isDefault())
        {
            result = false;
        }
    }

    return result;
}

void SetJoystick::propogateSetAxisThrottleSetting(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyAxis *axis = axes.value(index);
    if (axis != nullptr)
    {
        emit setAssignmentAxisThrottleChanged(index, axis->getCurrentlyAssignedSet());
    }
}

void SetJoystick::addControlStick(int index, JoyControlStick *stick)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    sticks.insert(index, stick);
    connect(stick, &JoyControlStick::stickNameChanged, this, &SetJoystick::propogateSetStickNameChange);

    QHashIterator<JoyStickDirectionsType::JoyStickDirections, JoyControlStickButton*> iter(*stick->getButtons());
    while (iter.hasNext())
    {
        JoyControlStickButton *button = iter.next().value();
        if (button != nullptr)
        {
            connect(button, &JoyControlStickButton::setChangeActivated, this, &SetJoystick::propogateSetChange);
            connect(button, &JoyControlStickButton::setAssignmentChanged, this, &SetJoystick::propogateSetStickButtonAssociation);
            connect(button, &JoyControlStickButton::clicked, this, &SetJoystick::propogateSetStickButtonClick, Qt::QueuedConnection);
            connect(button, &JoyControlStickButton::released, this, &SetJoystick::propogateSetStickButtonRelease, Qt::QueuedConnection);
            connect(button, &JoyControlStickButton::buttonNameChanged, this, &SetJoystick::propogateSetStickButtonNameChange);
        }
    }
}

void SetJoystick::removeControlStick(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (getSticks().contains(index))
    {
        JoyControlStick *stick = getSticks().value(index);
        sticks.remove(index);
        delete stick;
        stick = nullptr;
    }
}

void SetJoystick::addVDPad(int index, VDPad *vdpad)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    vdpads.insert(index, vdpad);
    connect(vdpad, &VDPad::dpadNameChanged, this, &SetJoystick::propogateSetVDPadNameChange);

    QHashIterator<int, JoyDPadButton*> iter(*vdpad->getButtons());
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        if (button)
        {
            connect(button, &JoyDPadButton::setChangeActivated, this, &SetJoystick::propogateSetChange);
            connect(button, &JoyDPadButton::setAssignmentChanged, this, &SetJoystick::propogateSetVDPadButtonAssociation);
            connect(button, &JoyDPadButton::clicked, this, &SetJoystick::propogateSetDPadButtonClick, Qt::QueuedConnection);
            connect(button, &JoyDPadButton::released, this, &SetJoystick::propogateSetDPadButtonRelease, Qt::QueuedConnection);
            connect(button, &JoyDPadButton::buttonNameChanged, this, &SetJoystick::propogateSetVDPadButtonNameChange);
        }
    }
}

void SetJoystick::removeVDPad(int index)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (getVdpads().contains(index))
    {
        VDPad *vdpad = getVdpads().value(index);
        vdpads.remove(index);
        delete vdpad;
        vdpad = nullptr;
    }
}

int SetJoystick::getIndex() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_index;
}

int SetJoystick::getRealIndex() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_index + 1;
}

void SetJoystick::propogateSetButtonClick(int button)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButton* jButton = qobject_cast<JoyButton*>(sender());
    if (jButton != nullptr)
    {
        if (!jButton->getIgnoreEventState())
        {
            emit setButtonClick(m_index, button);
            lastClickedButtons.append(jButton);

            #ifndef QT_DEBUG_NO_OUTPUT
            qDebug() << "Added button " << jButton->getPartialName(false,true) << " to list";
            qDebug() << "List has " << getLastClickedButtons().count() << " buttons";
            #endif
        }
    }
}

QList<JoyButton*> const& SetJoystick::getLastClickedButtons() const {

    return lastClickedButtons;
}

void SetJoystick::removeAllBtnFromQueue() {

    if (!getLastClickedButtons().isEmpty())
        lastClickedButtons.clear();

}

int SetJoystick::getCountBtnInList(QString partialName) {

    int count = 0;

    foreach(const JoyButton* joyBtn, getLastClickedButtons()) {
        if (joyBtn->getPartialName(false, true) == partialName) count++;
    }

    return count;
}

void SetJoystick::propogateSetButtonRelease(int button)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButton *jButton = qobject_cast<JoyButton*>(sender()); // static_cast
    if (jButton)
    {
        if (!jButton->getIgnoreEventState())
        {
            emit setButtonRelease(m_index, button);
        }
    }
}

void SetJoystick::propogateSetAxisButtonClick(int button)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyAxisButton *axisButton = qobject_cast<JoyAxisButton*>(sender()); // static_cast
    if (axisButton)
    {
        JoyAxis *axis = axisButton->getAxis();
        if (!axisButton->getIgnoreEventState())
        {
            emit setAxisButtonClick(m_index, axis->getIndex(), button);
        }
    }
}

void SetJoystick::propogateSetAxisButtonRelease(int button)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyAxisButton *axisButton = qobject_cast<JoyAxisButton*>(sender()); // static_cast
    if (axisButton)
    {
        JoyAxis *axis = axisButton->getAxis();
        if (!axisButton->getIgnoreEventState())
        {
            emit setAxisButtonRelease(m_index, axis->getIndex(), button);
        }
    }
}

void SetJoystick::propogateSetStickButtonClick(int button)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyControlStickButton *stickButton = qobject_cast<JoyControlStickButton*>(sender()); // static_cast
    if (stickButton)
    {
        JoyControlStick *stick = stickButton->getStick();
        if (stick && !stickButton->getIgnoreEventState())
        {
            emit setStickButtonClick(m_index, stick->getIndex(), button);
        }
    }
}

void SetJoystick::propogateSetStickButtonRelease(int button)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyControlStickButton *stickButton = qobject_cast<JoyControlStickButton*>(sender()); // static_cast
    if (stickButton)
    {
        JoyControlStick *stick = stickButton->getStick();
        if (!stickButton->getIgnoreEventState())
        {
            emit setStickButtonRelease(m_index, stick->getIndex(), button);
        }
    }
}

void SetJoystick::propogateSetDPadButtonClick(int button)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyDPadButton *dpadButton = qobject_cast<JoyDPadButton*>(sender()); // static_cast
    if (dpadButton)
    {
        JoyDPad *dpad = dpadButton->getDPad();
        if (dpad && dpadButton->getButtonState() &&
            !dpadButton->getIgnoreEventState())
        {
            emit setDPadButtonClick(m_index, dpad->getIndex(), button);
        }
    }
}

void SetJoystick::propogateSetDPadButtonRelease(int button)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyDPadButton *dpadButton = qobject_cast<JoyDPadButton*>(sender()); // static_cast
    if (dpadButton)
    {
        JoyDPad *dpad = dpadButton->getDPad();
        if (dpad && !dpadButton->getButtonState() &&
            !dpadButton->getIgnoreEventState())
        {
            emit setDPadButtonRelease(m_index, dpad->getIndex(), button);
        }
    }
}

void SetJoystick::propogateSetButtonNameChange()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButton *button = qobject_cast<JoyButton*>(sender()); // static_cast
    disconnect(button, &JoyButton::buttonNameChanged, this, &SetJoystick::propogateSetButtonNameChange);
    emit setButtonNameChange(button->getJoyNumber());
    connect(button, &JoyButton::buttonNameChanged, this, &SetJoystick::propogateSetButtonNameChange);
}

void SetJoystick::propogateSetAxisButtonNameChange()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyAxisButton *button = qobject_cast<JoyAxisButton*>(sender()); // static_cast
    disconnect(button, &JoyAxisButton::buttonNameChanged, this, &SetJoystick::propogateSetAxisButtonNameChange);
    emit setAxisButtonNameChange(button->getAxis()->getIndex(), button->getJoyNumber());
    connect(button, &JoyAxisButton::buttonNameChanged, this, &SetJoystick::propogateSetAxisButtonNameChange);
}

void SetJoystick::propogateSetStickButtonNameChange()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyControlStickButton *button = qobject_cast<JoyControlStickButton*>(sender()); // static_cast
    disconnect(button, &JoyControlStickButton::buttonNameChanged, this, &SetJoystick::propogateSetStickButtonNameChange);
    emit setStickButtonNameChange(button->getStick()->getIndex(), button->getJoyNumber());
    connect(button, &JoyControlStickButton::buttonNameChanged, this, &SetJoystick::propogateSetStickButtonNameChange);
}

void SetJoystick::propogateSetDPadButtonNameChange()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyDPadButton *button = qobject_cast<JoyDPadButton*>(sender()); // static_cast
    disconnect(button, &JoyDPadButton::buttonNameChanged, this, &SetJoystick::propogateSetDPadButtonNameChange);
    emit setDPadButtonNameChange(button->getDPad()->getIndex(), button->getJoyNumber());
    connect(button, &JoyDPadButton::buttonNameChanged, this, &SetJoystick::propogateSetDPadButtonNameChange);
}

void SetJoystick::propogateSetVDPadButtonNameChange()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyDPadButton *button = qobject_cast<JoyDPadButton*>(sender()); // static_cast
    disconnect(button, &JoyDPadButton::buttonNameChanged, this, &SetJoystick::propogateSetVDPadButtonNameChange);
    emit setVDPadButtonNameChange(button->getDPad()->getIndex(), button->getJoyNumber());
    connect(button, &JoyDPadButton::buttonNameChanged, this, &SetJoystick::propogateSetVDPadButtonNameChange);
}

void SetJoystick::propogateSetAxisNameChange()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyAxis *axis = qobject_cast<JoyAxis*>(sender()); // static_cast
    disconnect(axis, &JoyAxis::axisNameChanged, this, &SetJoystick::propogateSetAxisNameChange);
    emit setAxisNameChange(axis->getIndex());
    connect(axis, &JoyAxis::axisNameChanged, this, &SetJoystick::propogateSetAxisNameChange);
}

void SetJoystick::propogateSetStickNameChange()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyControlStick *stick = qobject_cast<JoyControlStick*>(sender()); // static_cast
    disconnect(stick, &JoyControlStick::stickNameChanged, this, &SetJoystick::propogateSetStickNameChange);
    emit setStickNameChange(stick->getIndex());
    connect(stick, &JoyControlStick::stickNameChanged, this, &SetJoystick::propogateSetStickNameChange);
}

void SetJoystick::propogateSetDPadNameChange()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyDPad *dpad = qobject_cast<JoyDPad*>(sender()); // static_cast
    disconnect(dpad, &JoyDPad::dpadNameChanged, this, &SetJoystick::propogateSetDPadButtonNameChange);
    emit setDPadNameChange(dpad->getIndex());
    connect(dpad, &JoyDPad::dpadNameChanged, this, &SetJoystick::propogateSetDPadButtonNameChange);
}

void SetJoystick::propogateSetVDPadNameChange()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    VDPad *vdpad = qobject_cast<VDPad*>(sender()); // static_cast
    disconnect(vdpad, &VDPad::dpadNameChanged, this, &SetJoystick::propogateSetVDPadNameChange);
    emit setVDPadNameChange(vdpad->getIndex());
    connect(vdpad, &VDPad::dpadNameChanged, this, &SetJoystick::propogateSetVDPadNameChange);
}

void SetJoystick::setIgnoreEventState(bool ignore)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<int, JoyButton*> iter(getButtons());
    while (iter.hasNext())
    {
        JoyButton *button = iter.next().value();
        if (button)
        {
            button->setIgnoreEventState(ignore);
        }
    }

    QHashIterator<int, JoyAxis*> iter2(axes);
    while (iter2.hasNext())
    {
        JoyAxis *axis = iter2.next().value();
        if (axis)
        {
            JoyAxisButton *naxisbutton = axis->getNAxisButton();
            naxisbutton->setIgnoreEventState(ignore);

            JoyAxisButton *paxisbutton = axis->getPAxisButton();
            paxisbutton->setIgnoreEventState(ignore);
        }
    }

    QHashIterator<int, JoyDPad*> iter3(getHats());
    while (iter3.hasNext())
    {
        JoyDPad *dpad = iter3.next().value();

        if (dpad)
        {
            QHash<int, JoyDPadButton*>* dpadbuttons = dpad->getButtons();
            QHashIterator<int, JoyDPadButton*> iterdpadbuttons(*dpadbuttons);
            while (iterdpadbuttons.hasNext())
            {
                JoyDPadButton *dpadbutton = iterdpadbuttons.next().value();
                if (dpadbutton)
                {
                    dpadbutton->setIgnoreEventState(ignore);
                }
            }
        }
    }

    QHashIterator<int, JoyControlStick*> iter4(getSticks());
    while (iter4.hasNext())
    {
        JoyControlStick *stick = iter4.next().value();
        if (stick)
        {
            QHash<JoyControlStick::JoyStickDirections, JoyControlStickButton*> *stickButtons = stick->getButtons();
            QHashIterator<JoyControlStick::JoyStickDirections, JoyControlStickButton*> iterstickbuttons(*stickButtons);
            while (iterstickbuttons.hasNext())
            {
                JoyControlStickButton *stickbutton = iterstickbuttons.next().value();
                stickbutton->setIgnoreEventState(ignore);
            }
        }
    }

    QHashIterator<int, VDPad*> iter5(getVdpads());
    while (iter5.hasNext())
    {
        VDPad *vdpad = iter5.next().value();
        if (vdpad)
        {
            QHash<int, JoyDPadButton*>* dpadbuttons = vdpad->getButtons();
            QHashIterator<int, JoyDPadButton*> itervdpadbuttons(*dpadbuttons);
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyAxis *axis = qobject_cast<JoyAxis*>(sender()); // static_cast
    emit setAxisActivated(m_index, axis->getIndex(), value);
}

void SetJoystick::propogateSetAxisReleased(int value)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyAxis *axis = qobject_cast<JoyAxis*>(sender()); // static_cast
    emit setAxisReleased(m_index, axis->getIndex(), value);
}

void SetJoystick::enableButtonConnections(JoyButton *button)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    connect(dpad, &JoyDPad::dpadNameChanged, this, &SetJoystick::propogateSetDPadNameChange);

    QHash<int, JoyDPadButton*> *buttons = dpad->getJoyButtons();
    QHashIterator<int, JoyDPadButton*> iter(*buttons);
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

InputDevice* SetJoystick::getInputDevice() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_device;
}

void SetJoystick::setName(QString name)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (name.length() <= MAXNAMELENGTH)
    {
        m_name = name;
        emit propertyUpdated();
    }
    else if (name.length() > MAXNAMELENGTH)
    {
        // Truncate name to 27 characters. Add ellipsis at the end.
        name.truncate(MAXNAMELENGTH-3);
        m_name = QString(name).append("...");
        emit propertyUpdated();
    }
}

QString SetJoystick::getName() const
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    return m_name;
}

void SetJoystick::copyAssignments(SetJoystick *destSet)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    for (int i=0; i < m_device->getNumberAxes(); i++)
    {
        JoyAxis *sourceAxis = axes.value(i);
        JoyAxis *destAxis = destSet->axes.value(i);
        if (sourceAxis && destAxis)
        {
            sourceAxis->copyAssignments(destAxis);
        }
    }

    QHashIterator<int, JoyControlStick*> stickIter(getSticks());
    while (stickIter.hasNext())
    {
        stickIter.next();
        int index = stickIter.key();
        JoyControlStick *sourceStick = stickIter.value();
        JoyControlStick *destStick = destSet->getSticks().value(index);
        if (sourceStick && destStick)
        {
            sourceStick->copyAssignments(destStick);
        }
    }

    for (int i=0; i < m_device->getNumberHats(); i++)
    {
        JoyDPad *sourceDPad = getHats().value(i);
        JoyDPad *destDPad = destSet->getHats().value(i);
        if (sourceDPad && destDPad)
        {
            sourceDPad->copyAssignments(destDPad);
        }
    }

    QHashIterator<int, VDPad*> vdpadIter(getVdpads());
    while (vdpadIter.hasNext())
    {
        vdpadIter.next();
        int index = vdpadIter.key();
        VDPad *sourceVDpad = vdpadIter.value();
        VDPad *destVDPad = destSet->getVdpads().value(index);
        if (sourceVDpad && destVDPad)
        {
            sourceVDpad->copyAssignments(destVDPad);
        }
    }

    for (int i=0; i < m_device->getNumberButtons(); i++)
    {
        JoyButton *sourceButton = getButtons().value(i);
        JoyButton *destButton = destSet->getButtons().value(i);
        if (sourceButton && destButton)
        {
            sourceButton->copyAssignments(destButton);
        }
    }
}

QString SetJoystick::getSetLabel()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QString temp = QString();
    if (!m_name.isEmpty())
    {
        temp = trUtf8("Set %1: %2").arg(m_index+1).arg(m_name);
    }
    else
    {
        temp = trUtf8("Set %1").arg(m_index+1);
    }

    return temp;
}

void SetJoystick::establishPropertyUpdatedConnection()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    connect(this, &SetJoystick::propertyUpdated, getInputDevice(), &InputDevice::profileEdited);
}

void SetJoystick::disconnectPropertyUpdatedConnection()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    disconnect(this, &SetJoystick::propertyUpdated, getInputDevice(), &InputDevice::profileEdited);
}

/**
 * @brief Raise the dead zones for axes. Used when launching
 *     the controller mapping window.
 */
void SetJoystick::raiseAxesDeadZones(int deadZone)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int tempDeadZone = deadZone;
    if ((deadZone <= 0) || (deadZone > 32767))
    {
        tempDeadZone = RAISEDDEADZONE;
    }

    QHashIterator<int, JoyAxis*> axisIter(axes);
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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QHashIterator<int, JoyAxis*> axisIter(axes);
    while (axisIter.hasNext())
    {
        JoyAxis *temp = axisIter.next().value();
        axesDeadZones->append(temp->getDeadZone());
    }
}

void SetJoystick::setAxesDeadZones(QList<int> *axesDeadZones)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

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
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (axes.contains(axisNum))
    {
        JoyAxis *temp = axes.value(axisNum);
        temp->setInitialThrottle(throttle);
    }
}

QHash<int, JoyAxis*>* SetJoystick::getAxes() {

    return &axes;
}

QHash<int, JoyButton*> const& SetJoystick::getButtons() const {

    return m_buttons;
}

QHash<int, JoyDPad*> const& SetJoystick::getHats() const {

    return hats;
}

QHash<int, JoyControlStick*> const& SetJoystick::getSticks() const {

    return sticks;
}

QHash<int, VDPad*> const& SetJoystick::getVdpads() const {

    return vdpads;
}
