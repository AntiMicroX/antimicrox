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

#include "joybuttonslot.h"

#include "antkeymapper.h"
#include "event.h"
#include "globalvariables.h"
#include "inputdevice.h"

#include <QDebug>
#include <QFileInfo>

JoyButtonSlot::JoyButtonSlot(QObject *parent)
    : QObject(parent)
    , extraData()
{
    deviceCode = 0;
    qkeyaliasCode = 0;
    m_mode = JoyKeyboard;
    m_distance = 0.0;
    previousDistance = 0.0;
    easingActive = false;
    mix_slots = nullptr;
}

JoyButtonSlot::JoyButtonSlot(int code, JoySlotInputAction mode, QObject *parent)
    : JoyButtonSlot(code, 0, mode, parent)
{
}

JoyButtonSlot::JoyButtonSlot(int code, int alias, JoySlotInputAction mode, QObject *parent)
    : JoyButtonSlot(parent)
{
    if (code > 0)
        deviceCode = code;

    if (alias > 0)
        qkeyaliasCode = alias;

    m_mode = mode;
}

JoyButtonSlot::JoyButtonSlot(JoyButtonSlot *slot, QObject *parent)
    : QObject(parent)
    , mix_slots(nullptr)
    , extraData()
{
    copyAssignments(*slot);
}

JoyButtonSlot::JoyButtonSlot(QString text, JoySlotInputAction mode, QObject *parent)
    : QObject(parent)
    , extraData()
{
    deviceCode = 0;
    qkeyaliasCode = 0;
    m_mode = mode;
    m_distance = 0.0;
    easingActive = false;
    mix_slots = nullptr;

    if ((mode == JoyLoadProfile) || (mode == JoyTextEntry) || (mode == JoyExecute))
    {
        m_textData = text;
    }
}

JoyButtonSlot::~JoyButtonSlot() {}

void JoyButtonSlot::setSlotCode(int code)
{
    if (code >= 0)
    {
        deviceCode = code;
        qkeyaliasCode = 0;
    }
}

void JoyButtonSlot::setSlotCode(int code, int alias)
{
    if ((m_mode == JoyButtonSlot::JoyKeyboard) && (code > 0))
    {
        deviceCode = code;
        qkeyaliasCode = alias;
    } else if (code >= 0)
    {
        deviceCode = code;
        qkeyaliasCode = 0;
    }
}

int JoyButtonSlot::getSlotCodeAlias() const { return qkeyaliasCode; }

int JoyButtonSlot::getSlotCode() const { return deviceCode; }

void JoyButtonSlot::setSlotMode(JoySlotInputAction selectedMode) { m_mode = selectedMode; }

JoyButtonSlot::JoySlotInputAction JoyButtonSlot::getSlotMode() const { return m_mode; }

QString JoyButtonSlot::movementString()
{
    QString newlabel = QString();

    if (m_mode == JoyMouseMovement)
    {
        newlabel.append(tr("Mouse")).append(" ");

        switch (deviceCode)
        {
        case 1:
            newlabel.append(tr("Up"));
            break;

        case 2:
            newlabel.append(tr("Down"));
            break;

        case 3:
            newlabel.append(tr("Left"));
            break;

        case 4:
            newlabel.append(tr("Right"));
            break;
        }
    }

    return newlabel;
}

void JoyButtonSlot::setDistance(double distance) { m_distance = distance; }

double JoyButtonSlot::getMouseDistance() { return m_distance; }

QElapsedTimer *JoyButtonSlot::getMouseInterval() { return &mouseInterval; }

void JoyButtonSlot::restartMouseInterval() { mouseInterval.restart(); }

QString JoyButtonSlot::getXmlName() { return GlobalVariables::JoyButtonSlot::xmlName; }

QString JoyButtonSlot::getSlotString()
{
    QString newlabel = QString();

    qDebug() << "deviceCode in getSlotString() is: " << deviceCode << " for mode: " << m_mode;

    if (deviceCode >= 0 || m_mode == JoyButtonSlot::JoyMix)
    {
        switch (m_mode)
        {
        case JoyButtonSlot::JoyKeyboard: {
            int tempDeviceCode = deviceCode;

            newlabel = newlabel.append(keysymToKeyString(tempDeviceCode, qkeyaliasCode).toUpper());
            break;
        }
        case JoyButtonSlot::JoyMouseButton: {
            newlabel.append(tr("Mouse")).append(" ");

            switch (deviceCode)
            {
            case 1:
                newlabel.append(tr("LB"));
                break;
            case 2:
                newlabel.append(tr("MB"));
                break;
            case 3:
                newlabel.append(tr("RB"));
                break;
            default:
                newlabel.append(QString::number(deviceCode));
                break;
            }

            break;
        }
        case JoyMouseMovement: {
            newlabel.append(movementString());
            break;
        }
        case JoyPause: {
            int minutes = deviceCode / 1000 / 60;
            int seconds = (deviceCode / 1000 % 60);
            int hundredths = deviceCode % 1000 / 10;

            newlabel.append(tr("Pause")).append(" ");

            if (minutes > 0)
                newlabel.append(QString("%1:").arg(minutes, 2, 10, QChar('0')));

            newlabel.append(QString("%1.%2").arg(seconds, 2, 10, QChar('0')).arg(hundredths, 2, 10, QChar('0')));

            break;
        }
        case JoyHold: {
            int minutes = deviceCode / 1000 / 60;
            int seconds = (deviceCode / 1000 % 60);
            int hundredths = deviceCode % 1000 / 10;

            newlabel.append(tr("Hold")).append(" ");

            if (minutes > 0)
                newlabel.append(QString("%1:").arg(minutes, 2, 10, QChar('0')));

            newlabel.append(QString("%1.%2").arg(seconds, 2, 10, QChar('0')).arg(hundredths, 2, 10, QChar('0')));

            break;
        }
        case JoyButtonSlot::JoyCycle: {
            newlabel.append(tr("Cycle"));
            break;
        }
        case JoyDistance: {
            QString temp(tr("Distance"));
            temp.append(" ").append(QString::number(deviceCode).append("%"));
            newlabel.append(temp);

            break;
        }
        case JoyRelease: {
            int minutes = deviceCode / 1000 / 60;
            int seconds = (deviceCode / 1000 % 60);
            int hundredths = deviceCode % 1000 / 10;

            newlabel.append(tr("Release")).append(" ");

            if (minutes > 0)
                newlabel.append(QString("%1:").arg(minutes, 2, 10, QChar('0')));

            newlabel.append(QString("%1.%2").arg(seconds, 2, 10, QChar('0')).arg(hundredths, 2, 10, QChar('0')));

            break;
        }
        case JoyMouseSpeedMod: {
            QString temp = QString();
            temp.append(tr("Mouse Mod")).append(" ");
            temp.append(QString::number(deviceCode).append("%"));
            newlabel.append(temp);

            break;
        }
        case JoyKeyPress: {
            int minutes = deviceCode / 1000 / 60;
            int seconds = (deviceCode / 1000 % 60);
            int hundredths = deviceCode % 1000 / 10;
            QString temp(tr("Press Time").append(" "));

            if (minutes > 0)
                temp.append(QString("%1:").arg(minutes, 2, 10, QChar('0')));

            temp.append(QString("%1.%2").arg(seconds, 2, 10, QChar('0')).arg(hundredths, 2, 10, QChar('0')));

            newlabel.append(temp);

            break;
        }
        case JoyDelay: {
            int minutes = deviceCode / 1000 / 60;
            int seconds = (deviceCode / 1000 % 60);
            int hundredths = deviceCode % 1000 / 10;
            QString temp(tr("Delay").append(" "));

            if (minutes > 0)
                temp.append(QString("%1:").arg(minutes, 2, 10, QChar('0')));

            temp.append(QString("%1.%2").arg(seconds, 2, 10, QChar('0')).arg(hundredths, 2, 10, QChar('0')));

            newlabel.append(temp);

            break;
        }
        case JoyLoadProfile: {
            if (!m_textData.isEmpty())
            {
                QFileInfo profileInfo(m_textData);
                QString temp(tr("Load %1").arg(PadderCommon::getProfileName(profileInfo)));
                newlabel.append(temp);
            }

            break;
        }
        case JoySetChange: {
            newlabel.append(tr("Set Change %1").arg(deviceCode + 1));

            break;
        }
        case JoyTextEntry: {
            QString temp = m_textData;

            if (temp.length() > GlobalVariables::JoyButtonSlot::MAXTEXTENTRYDISPLAYLENGTH)
            {
                temp.truncate(GlobalVariables::JoyButtonSlot::MAXTEXTENTRYDISPLAYLENGTH - 3);
                temp.append("...");
            }

            newlabel.append(tr("[Text] %1").arg(temp));

            break;
        }
        case JoyExecute: {
            QString temp = QString();

            if (!m_textData.isEmpty())
            {
                QFileInfo tempFileInfo(m_textData);
                temp.append(tempFileInfo.fileName());
            }

            newlabel.append(tr("[Exec] %1").arg(temp));

            break;
        }
        case JoyButtonSlot::JoyMix: {
            bool firstTime = true;

            for (auto minislot : *getMixSlots())
            {
                if (!firstTime)
                    newlabel.append("+");
                firstTime = false;

                newlabel =
                    newlabel.append(keysymToKeyString(minislot->getSlotCode(), minislot->getSlotCodeAlias()).toUpper());
            }

            break;
        }
        }
    } else
    {
        newlabel = newlabel.append(tr("[NO KEY]"));
    }

    if (newlabel == tr("[NO KEY]"))
    {
        qDebug() << "EMPTY JOYBUTTONSLOT";
    }

    return newlabel;
}

void JoyButtonSlot::setPreviousDistance(double distance) { previousDistance = distance; }

double JoyButtonSlot::getPreviousDistance() const { return previousDistance; }

double JoyButtonSlot::getDistance() const { return m_distance; }

bool JoyButtonSlot::isModifierKey()
{
    bool modifier = false;

    if ((m_mode == JoyKeyboard) && AntKeyMapper::getInstance()->isModifierKey(qkeyaliasCode))
    {
        modifier = true;
    }

    return modifier;
}

bool JoyButtonSlot::isEasingActive() const { return easingActive; }

void JoyButtonSlot::setEasingStatus(bool isActive) { easingActive = isActive; }

QElapsedTimer *JoyButtonSlot::getEasingTime() { return &easingTime; }

void JoyButtonSlot::setTextData(QString textData) { m_textData = textData; }

QString JoyButtonSlot::getTextData() const
{
    if (m_textData.isNull() || m_textData.isEmpty())
        return "";
    return m_textData;
}

void JoyButtonSlot::setExtraData(QVariant data) { this->extraData = data; }

QVariant JoyButtonSlot::getExtraData() const { return extraData; }

/**
 * @brief Deep-copies member variables from another JoyButtonSlot object
 *   into this object.
 * @param[in] slot Slot from which data gets copied
 */
void JoyButtonSlot::copyAssignments(const JoyButtonSlot &slot)
{
    deviceCode = slot.deviceCode;
    qkeyaliasCode = slot.qkeyaliasCode;
    m_mode = slot.m_mode;

    if (slot.mix_slots != nullptr)
    {
        mix_slots = new QList<JoyButtonSlot *>();
        for (const auto minislot : *slot.mix_slots)
            mix_slots->append(
                new JoyButtonSlot(minislot->getSlotCode(), minislot->getSlotCodeAlias(), minislot->getSlotMode()));
    }

    m_distance = slot.m_distance;
    previousDistance = slot.previousDistance;

    easingTime = QElapsedTimer();
    if (slot.easingTime.isValid())
        easingTime.start();
    easingActive = slot.easingActive;

    if (!slot.getTextData().isNull() && (slot.getTextData() != ""))
        m_textData = slot.getTextData();

    extraData = slot.extraData;
}

void JoyButtonSlot::secureMixSlotsInit()
{
    if (mix_slots == nullptr)
    {
        mix_slots = new QList<JoyButtonSlot *>();
        // connect(qApp, &QApplication::aboutToQuit, this, &JoyButtonSlot::cleanMixSlots);
    }
}

QList<JoyButtonSlot *> *JoyButtonSlot::getMixSlots()
{
    secureMixSlotsInit();
    return mix_slots;
}

void JoyButtonSlot::assignMixSlotsToNull() { mix_slots = nullptr; }

void JoyButtonSlot::cleanMixSlots()
{
    if (mix_slots != nullptr)
    {
        // disconnect(qApp, &QApplication::aboutToQuit, this, &JoyButtonSlot::cleanMixSlots);

        if (mix_slots->size() != 0)
        {
            qDeleteAll(*mix_slots);
            mix_slots->clear();
        }

        delete mix_slots;
        mix_slots = nullptr;
    }
}

void JoyButtonSlot::setMixSlots(QList<JoyButtonSlot *> *mixSlots) { mix_slots = mixSlots; }

bool JoyButtonSlot::isValidSlot()
{
    bool result = true;

    switch (m_mode)
    {
    case JoyLoadProfile:
    case JoyTextEntry:
    case JoyExecute: {
        if (m_textData.isEmpty())
            result = false;

        break;
    }
    case JoySetChange: {
        if (deviceCode < 0)
            result = false;

        break;
    }
    case JoyMix: {
        if (mix_slots->count() == 0)
            return false;

        break;
    }
    default: {
        break;
    }
    }

    return result;
}

JoyButtonSlot &JoyButtonSlot::operator=(JoyButtonSlot *slot)
{
    copyAssignments(*slot);
    return *this;
}
