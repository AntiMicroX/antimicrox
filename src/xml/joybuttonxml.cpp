/* antimicrox Gamepad to KB+M event mapper
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

#include "joybuttonxml.h"
#include "joybuttonslotxml.h"
#include "joybuttontypes/joybutton.h"

#include <QDebug>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

JoyButtonXml::JoyButtonXml(JoyButton *joyButton, QObject *parent)
    : QObject(parent)
{
    m_joyButton = joyButton;
}

bool JoyButtonXml::readButtonConfig(QXmlStreamReader *xml)
{
    bool found = false;

    if ((xml->name().toString() == "toggle") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();

        if (temptext == "true")
            m_joyButton->setToggle(true);
    } else if ((xml->name().toString() == "turbointerval") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        m_joyButton->setTurboInterval(tempchoice);
    } else if ((xml->name().toString() == "turbomode") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();

        if (temptext == "normal")
            m_joyButton->setTurboMode(JoyButton::NormalTurbo);
        else if (temptext == "gradient")
            m_joyButton->setTurboMode(JoyButton::GradientTurbo);
        else if (temptext == "pulse")
            m_joyButton->setTurboMode(JoyButton::PulseTurbo);
    } else if ((xml->name().toString() == "useturbo") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();

        if (temptext == "true")
            m_joyButton->setUseTurbo(true);
    } else if ((xml->name().toString() == "mousespeedx") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        m_joyButton->setMouseSpeedX(tempchoice);
    } else if ((xml->name().toString() == "mousespeedy") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        m_joyButton->setMouseSpeedY(tempchoice);
    } else if ((xml->name().toString() == "cycleresetactive") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();

        if (temptext == "true")
            m_joyButton->setCycleResetStatus(true);
    } else if ((xml->name().toString() == "cycleresetinterval") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();

        if (tempchoice >= GlobalVariables::JoyButton::MINCYCLERESETTIME)
            m_joyButton->setCycleResetTime(tempchoice);
    } else if ((xml->name().toString() == "slots") && xml->isStartElement())
    {
        found = true;
        xml->readNextStartElement();

        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name().toString() != "slots")))
        {
            if ((xml->name().toString() == "slot") && xml->isStartElement())
            {
                JoyButtonSlot *buttonslot = new JoyButtonSlot(m_joyButton);
                JoyButtonSlotXml *buttonSlotXml = new JoyButtonSlotXml(buttonslot);
                buttonSlotXml->readConfig(xml);
                delete buttonSlotXml;
                buttonSlotXml = nullptr;

                if (buttonslot->isValidSlot())
                {
                    bool inserted = m_joyButton->insertAssignedSlot(buttonslot, false);

                    if (!inserted)
                    {
                        if (buttonslot->getSlotMode() == JoyButtonSlot::JoyMix && buttonslot->getMixSlots() != nullptr)
                        {
                            qDeleteAll(*buttonslot->getMixSlots());
                            buttonslot->getMixSlots()->clear();
                            delete buttonslot->getMixSlots();
                            buttonslot->assignMixSlotsToNull();
                        }

                        buttonslot->deleteLater();
                        buttonslot = nullptr;
                    }
                } else
                {
                    if (buttonslot->getSlotMode() == JoyButtonSlot::JoyMix && buttonslot->getMixSlots() != nullptr)
                    {
                        qDeleteAll(*buttonslot->getMixSlots());
                        buttonslot->getMixSlots()->clear();
                        delete buttonslot->getMixSlots();
                        buttonslot->assignMixSlotsToNull();
                    }

                    buttonslot->deleteLater();
                    buttonslot = nullptr;
                }
            } else
            {
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }
    } else if ((xml->name().toString() == "setselect") && xml->isStartElement())
    {
        if (!m_joyButton->isModifierButton())
        {
            found = true;
            QString temptext = xml->readElementText();
            int tempchoice = temptext.toInt();

            if ((tempchoice >= 0) && (tempchoice <= GlobalVariables::InputDevice::NUMBER_JOYSETS))
                m_joyButton->setChangeSetSelection(tempchoice - 1, false);
        }
    } else if ((xml->name().toString() == "setselectcondition") && xml->isStartElement())
    {
        if (!m_joyButton->isModifierButton())
        {
            found = true;
            QString temptext = xml->readElementText();
            JoyButton::SetChangeCondition tempcondition = JoyButton::SetChangeDisabled;

            if (temptext == "one-way")
                tempcondition = JoyButton::SetChangeOneWay;
            else if (temptext == "two-way")
                tempcondition = JoyButton::SetChangeTwoWay;
            else if (temptext == "while-held")
                tempcondition = JoyButton::SetChangeWhileHeld;

            if (tempcondition != JoyButton::SetChangeDisabled)
                m_joyButton->setChangeSetCondition(tempcondition, false, false);
        }
    } else if ((xml->name().toString() == "mousemode") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();

        if (temptext == "cursor")
            m_joyButton->setMouseMode(JoyButton::MouseCursor);
        else if (temptext == "spring")
            m_joyButton->setMouseMode(JoyButton::MouseSpring);
    } else if ((xml->name().toString() == "mouseacceleration") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();

        if (temptext == "linear")
            m_joyButton->setMouseCurve(JoyButton::LinearCurve);
        else if (temptext == "quadratic")
            m_joyButton->setMouseCurve(JoyButton::QuadraticCurve);
        else if (temptext == "cubic")
            m_joyButton->setMouseCurve(JoyButton::CubicCurve);
        else if (temptext == "quadratic-extreme")
            m_joyButton->setMouseCurve(JoyButton::QuadraticExtremeCurve);
        else if (temptext == "power")
            m_joyButton->setMouseCurve(JoyButton::PowerCurve);
        else if (temptext == "precision")
            m_joyButton->setMouseCurve(JoyButton::EnhancedPrecisionCurve);
        else if (temptext == "easing-quadratic")
            m_joyButton->setMouseCurve(JoyButton::EasingQuadraticCurve);
        else if (temptext == "easing-cubic")
            m_joyButton->setMouseCurve(JoyButton::EasingCubicCurve);
    } else if ((xml->name().toString() == "mousespringwidth") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        m_joyButton->setSpringWidth(tempchoice);
    } else if ((xml->name().toString() == "mousespringheight") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        m_joyButton->setSpringHeight(tempchoice);
    } else if ((xml->name().toString() == "mousesensitivity") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        double tempchoice = temptext.toDouble();
        m_joyButton->setSensitivity(tempchoice);
    } else if ((xml->name().toString() == "actionname") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();

        if (!temptext.isEmpty())
            m_joyButton->setActionName(temptext);
    } else if ((xml->name().toString() == "wheelspeedx") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        m_joyButton->setWheelSpeed(tempchoice, 'X');
    } else if ((xml->name().toString() == "wheelspeedy") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();
        m_joyButton->setWheelSpeed(tempchoice, 'Y');
    } else if ((xml->name().toString() == "relativespring") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();

        if (temptext == "true")
            m_joyButton->setSpringRelativeStatus(true);
    } else if ((xml->name().toString() == "easingduration") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        double tempchoice = temptext.toDouble();
        m_joyButton->setEasingDuration(tempchoice);
    } else if ((xml->name().toString() == "extraacceleration") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();

        if (temptext == "true")
            m_joyButton->setExtraAccelerationStatus(true);
    } else if ((xml->name().toString() == "accelerationmultiplier") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        double tempchoice = temptext.toDouble();
        m_joyButton->setExtraAccelerationMultiplier(tempchoice);
    } else if ((xml->name().toString() == "startaccelmultiplier") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        double tempchoice = temptext.toDouble();
        m_joyButton->setStartAccelMultiplier(tempchoice);
    } else if ((xml->name().toString() == "minaccelthreshold") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        double tempchoice = temptext.toDouble();
        m_joyButton->setMinAccelThreshold(tempchoice);
    } else if ((xml->name().toString() == "maxaccelthreshold") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        double tempchoice = temptext.toDouble();
        m_joyButton->setMaxAccelThreshold(tempchoice);
    } else if ((xml->name().toString() == "accelextraduration") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        double tempchoice = temptext.toDouble();
        m_joyButton->setAccelExtraDuration(tempchoice);
    } else if ((xml->name().toString() == "extraaccelerationcurve") && xml->isStartElement())
    {
        found = true;

        QString temptext = xml->readElementText();
        JoyButton::JoyExtraAccelerationCurve tempcurve = JoyButton::DEFAULTEXTRAACCELCURVE;

        if (temptext == "linear")
            tempcurve = JoyButton::LinearAccelCurve;
        else if (temptext == "easeoutsine")
            tempcurve = JoyButton::EaseOutSineCurve;
        else if (temptext == "easeoutquad")
            tempcurve = JoyButton::EaseOutQuadAccelCurve;
        else if (temptext == "easeoutcubic")
            tempcurve = JoyButton::EaseOutCubicAccelCurve;

        m_joyButton->setExtraAccelerationCurve(tempcurve);
    } else if ((xml->name().toString() == "springreleaseradius") && xml->isStartElement())
    {
        found = true;
        QString temptext = xml->readElementText();
        int tempchoice = temptext.toInt();

        if (!m_joyButton->isRelativeSpring())
            m_joyButton->setSpringDeadCircleMultiplier(tempchoice);
    }

    return found;
}

void JoyButtonXml::readConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && (xml->name().toString() == m_joyButton->getXmlName()))
    {
        xml->readNextStartElement();

        while (!xml->atEnd() && (!xml->isEndElement() && (xml->name().toString() != m_joyButton->getXmlName())))
        {
            bool found = readButtonConfig(xml);

            if (!found)
                xml->skipCurrentElement();
            else
                m_joyButton->buildActiveZoneSummaryString();

            xml->readNextStartElement();
        }
    }
}

void JoyButtonXml::writeConfig(QXmlStreamWriter *xml)
{
    if (!m_joyButton->isDefault())
    {
        xml->writeStartElement(m_joyButton->getXmlName());
        xml->writeAttribute("index", QString::number(m_joyButton->getRealJoyNumber()));

        if (m_joyButton->getToggleState() != GlobalVariables::JoyButton::DEFAULTTOGGLE)
            xml->writeTextElement("toggle", m_joyButton->getToggleState() ? "true" : "false");

        if (m_joyButton->getTurboInterval() != GlobalVariables::JoyButton::DEFAULTTURBOINTERVAL)
            xml->writeTextElement("turbointerval", QString::number(m_joyButton->getTurboInterval()));

        if ((m_joyButton->getTurboMode() != JoyButton::DEFAULTTURBOMODE) &&
            (m_joyButton->getTurboMode() == JoyButton::GradientTurbo))
            xml->writeTextElement("turbomode", "gradient");
        else if ((m_joyButton->getTurboMode() != JoyButton::DEFAULTTURBOMODE) &&
                 (m_joyButton->getTurboMode() == JoyButton::PulseTurbo))
            xml->writeTextElement("turbomode", "pulse");

        if (m_joyButton->isUsingTurbo() != GlobalVariables::JoyButton::DEFAULTUSETURBO)
            xml->writeTextElement("useturbo", m_joyButton->isUsingTurbo() ? "true" : "false");

        if (m_joyButton->getMouseSpeedX() != GlobalVariables::JoyButton::DEFAULTMOUSESPEEDX)
            xml->writeTextElement("mousespeedx", QString::number(m_joyButton->getMouseSpeedX()));

        if (m_joyButton->getMouseSpeedY() != GlobalVariables::JoyButton::DEFAULTMOUSESPEEDY)
            xml->writeTextElement("mousespeedy", QString::number(m_joyButton->getMouseSpeedY()));

        if ((m_joyButton->getMouseMode() != JoyButton::DEFAULTMOUSEMODE) &&
            (m_joyButton->getMouseMode() == JoyButton::MouseCursor))
        {
            xml->writeTextElement("mousemode", "cursor");
        } else if ((m_joyButton->getMouseMode() != JoyButton::DEFAULTMOUSEMODE) &&
                   (m_joyButton->getMouseMode() == JoyButton::MouseSpring))
        {
            xml->writeTextElement("mousemode", "spring");
            xml->writeTextElement("mousespringwidth", QString::number(m_joyButton->getSpringWidth()));
            xml->writeTextElement("mousespringheight", QString::number(m_joyButton->getSpringHeight()));
        }

        if (m_joyButton->getMouseCurve() != m_joyButton->getDefaultMouseCurve())
        {
            switch (m_joyButton->getMouseCurve())
            {
            case JoyButton::LinearCurve:
                xml->writeTextElement("mouseacceleration", "linear");
                break;

            case JoyButton::QuadraticCurve:
                xml->writeTextElement("mouseacceleration", "quadratic");
                break;

            case JoyButton::CubicCurve:
                xml->writeTextElement("mouseacceleration", "cubic");
                break;

            case JoyButton::QuadraticExtremeCurve:
                xml->writeTextElement("mouseacceleration", "quadratic-extreme");
                break;

            case JoyButton::PowerCurve:
                xml->writeTextElement("mouseacceleration", "power");
                xml->writeTextElement("mousesensitivity", QString::number(m_joyButton->getSensitivity()));
                break;

            case JoyButton::EnhancedPrecisionCurve:
                xml->writeTextElement("mouseacceleration", "precision");
                break;

            case JoyButton::EasingQuadraticCurve:
                xml->writeTextElement("mouseacceleration", "easing-quadratic");
                break;

            case JoyButton::EasingCubicCurve:
                xml->writeTextElement("mouseacceleration", "easing-cubic");
                break;
            }
        }

        if (m_joyButton->getWheelSpeedX() != GlobalVariables::JoyButton::DEFAULTWHEELX)
            xml->writeTextElement("wheelspeedx", QString::number(m_joyButton->getWheelSpeedX()));

        if (m_joyButton->getWheelSpeedY() != GlobalVariables::JoyButton::DEFAULTWHEELY)
            xml->writeTextElement("wheelspeedy", QString::number(m_joyButton->getWheelSpeedY()));

        if (!m_joyButton->isModifierButton())
        {
            if (m_joyButton->getChangeSetCondition() != JoyButton::SetChangeDisabled)
            {
                xml->writeTextElement("setselect", QString::number(m_joyButton->getSetSelection() + 1));
                QString temptext = QString();

                switch (m_joyButton->getChangeSetCondition())
                {
                case JoyButton::SetChangeOneWay:
                    temptext = "one-way";
                    break;

                case JoyButton::SetChangeTwoWay:
                    temptext = "two-way";
                    break;

                case JoyButton::SetChangeWhileHeld:
                    temptext = "while-held";
                    break;

                default:
                    break;
                }

                xml->writeTextElement("setselectcondition", temptext);
            }
        }

        if (!m_joyButton->getActionName().isEmpty())
            xml->writeTextElement("actionname", m_joyButton->getActionName());

        if (m_joyButton->isCycleResetActive())
            xml->writeTextElement("cycleresetactive", "true");

        if (m_joyButton->getCycleResetTime() >= GlobalVariables::JoyButton::MINCYCLERESETTIME)
            xml->writeTextElement("cycleresetinterval", QString::number(m_joyButton->getCycleResetTime()));

        if (m_joyButton->isRelativeSpring())
            xml->writeTextElement("relativespring", "true");

        if (!qFuzzyCompare(m_joyButton->getEasingDuration(), GlobalVariables::JoyButton::DEFAULTEASINGDURATION))
            xml->writeTextElement("easingduration", QString::number(m_joyButton->getEasingDuration()));

        if (m_joyButton->isExtraAccelerationEnabled())
            xml->writeTextElement("extraacceleration", "true");

        if (!qFuzzyCompare(m_joyButton->getExtraAccelerationMultiplier(), GlobalVariables::JoyButton::DEFAULTEXTRACCELVALUE))
            xml->writeTextElement("accelerationmultiplier", QString::number(m_joyButton->getExtraAccelerationMultiplier()));

        if (!qFuzzyCompare(m_joyButton->getStartAccelMultiplier(), GlobalVariables::JoyButton::DEFAULTSTARTACCELMULTIPLIER))
            xml->writeTextElement("startaccelmultiplier", QString::number(m_joyButton->getStartAccelMultiplier()));

        if (!qFuzzyCompare(m_joyButton->getMinAccelThreshold(), GlobalVariables::JoyButton::DEFAULTMINACCELTHRESHOLD))
            xml->writeTextElement("minaccelthreshold", QString::number(m_joyButton->getMinAccelThreshold()));

        if (!qFuzzyCompare(m_joyButton->getMaxAccelThreshold(), GlobalVariables::JoyButton::DEFAULTMAXACCELTHRESHOLD))
            xml->writeTextElement("maxaccelthreshold", QString::number(m_joyButton->getMaxAccelThreshold()));

        if (!qFuzzyCompare(m_joyButton->getAccelExtraDuration(), GlobalVariables::JoyButton::DEFAULTACCELEASINGDURATION))
            xml->writeTextElement("accelextraduration", QString::number(m_joyButton->getAccelExtraDuration()));

        if (m_joyButton->getSpringDeadCircleMultiplier() != GlobalVariables::JoyButton::DEFAULTSPRINGRELEASERADIUS)
            xml->writeTextElement("springreleaseradius", QString::number(m_joyButton->getSpringDeadCircleMultiplier()));

        if (m_joyButton->getExtraAccelerationCurve() != JoyButton::DEFAULTEXTRAACCELCURVE)
        {
            QString temp = QString();

            switch (m_joyButton->getExtraAccelerationCurve())
            {
            case JoyButton::LinearAccelCurve:
                temp = "linear";
                break;

            case JoyButton::EaseOutSineCurve:
                temp = "easeoutsine";
                break;

            case JoyButton::EaseOutQuadAccelCurve:
                temp = "easeoutquad";
                break;

            case JoyButton::EaseOutCubicAccelCurve:
                temp = "easeoutcubic";
                break;
            }

            if (!temp.isEmpty())
                xml->writeTextElement("extraaccelerationcurve", temp);
        }

        // Write information about assigned slots.
        if (!m_joyButton->getAssignedSlots()->isEmpty())
        {
            xml->writeStartElement("slots");
            QListIterator<JoyButtonSlot *> iter(*m_joyButton->getAssignedSlots());

            while (iter.hasNext())
            {
                JoyButtonSlot *buttonslot = iter.next();
                JoyButtonSlotXml *buttonslotxml = new JoyButtonSlotXml(buttonslot);
                buttonslotxml->writeConfig(xml);
                delete buttonslotxml;
                buttonslotxml = nullptr;
            }

            xml->writeEndElement();
        }

        xml->writeEndElement();
    }
}
