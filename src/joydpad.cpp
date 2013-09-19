#include <QHashIterator>

#include "joydpad.h"

const QString JoyDPad::xmlName = "dpad";

JoyDPad::JoyDPad(QObject *parent) :
    QObject(parent)
{
    buttons = QHash<int, JoyDPadButton*> ();
    activeDiagonalButton = 0;
    prevDirection = JoyDPadButton::DpadCentered;
    originset = 0;
    currentMode = StandardMode;
    populateButtons();
}

JoyDPad::JoyDPad(int index, int originset, QObject *parent) :
    QObject(parent)
{
    this->index = index;
    buttons = QHash<int, JoyDPadButton*> ();
    activeDiagonalButton = 0;
    prevDirection = JoyDPadButton::DpadCentered;
    this->originset = originset;
    currentMode = StandardMode;

    populateButtons();
}

JoyDPad::~JoyDPad()
{
    QHashIterator<int, JoyDPadButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        delete button;
        button = 0;
    }

    buttons.clear();
}

JoyDPadButton *JoyDPad::getJoyButton(int index)
{
    return buttons.value(index);
}

void JoyDPad::populateButtons()
{
    JoyDPadButton* button = new JoyDPadButton (JoyDPadButton::DpadUp, originset, this, this);
    buttons.insert(JoyDPadButton::DpadUp, button);

    button = new JoyDPadButton (JoyDPadButton::DpadDown, originset, this, this);
    buttons.insert(JoyDPadButton::DpadDown, button);

    button = new JoyDPadButton(JoyDPadButton::DpadRight, originset, this, this);
    buttons.insert(JoyDPadButton::DpadRight, button);

    button = new JoyDPadButton(JoyDPadButton::DpadLeft, originset, this, this);
    buttons.insert(JoyDPadButton::DpadLeft, button);

    button = new JoyDPadButton(JoyDPadButton::DpadLeftUp, originset, this, this);
    buttons.insert(JoyDPadButton::DpadLeftUp, button);

    button = new JoyDPadButton(JoyDPadButton::DpadRightUp, originset, this, this);
    buttons.insert(JoyDPadButton::DpadRightUp, button);

    button = new JoyDPadButton(JoyDPadButton::DpadRightDown, originset, this, this);
    buttons.insert(JoyDPadButton::DpadRightDown, button);

    button = new JoyDPadButton(JoyDPadButton::DpadLeftDown, originset, this, this);
    buttons.insert(JoyDPadButton::DpadLeftDown, button);
}

QString JoyDPad::getName()
{
    QString label = QString("DPad ");
    label = label.append(QString::number(getRealJoyNumber()));
    return label;
}

int JoyDPad::getJoyNumber()
{
    return index;
}

int JoyDPad::getIndex()
{
    return index;
}

int JoyDPad::getRealJoyNumber()
{
    return index + 1;
}

QString JoyDPad::getXmlName()
{
    return this->xmlName;
}

void JoyDPad::readConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && xml->name() == getXmlName())
    {
        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != getXmlName()))
        {
            if (xml->name() == "dpadbutton" && xml->isStartElement())
            {
                int index = xml->attributes().value("index").toString().toInt();
                JoyDPadButton* button = this->getJoyButton(index);
                if (button)
                {
                    button->readConfig(xml);
                }
                else
                {
                    xml->skipCurrentElement();
                }
            }
            else if (xml->name() == "mode" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                if (temptext == "eight-way")
                {
                    this->setJoyMode(EightWayMode);
                }
            }
            else
            {
                xml->skipCurrentElement();
            }

            xml->readNextStartElement();
        }
    }
}

void JoyDPad::writeConfig(QXmlStreamWriter *xml)
{
    if (!isDefault())
    {
        xml->writeStartElement(getXmlName());
        xml->writeAttribute("index", QString::number(index+1));
        if (currentMode == EightWayMode)
        {
            xml->writeTextElement("mode", "eight-way");
        }

        QHashIterator<int, JoyDPadButton*> iter(buttons);
        while (iter.hasNext())
        {
            JoyDPadButton *button = iter.next().value();
            button->writeConfig(xml);
        }

        xml->writeEndElement();
    }
}

void JoyDPad::joyEvent(int value, bool ignoresets)
{
    JoyDPadButton *curButton = 0;
    JoyDPadButton *prevButton = 0;

    if (value != (int)prevDirection)
    {
        if (value != JoyDPadButton::DpadCentered)
        {
            emit active(index);
        }
        else
        {
            emit released(index);
        }

        if (activeDiagonalButton)
        {
            activeDiagonalButton->joyEvent(false, ignoresets);
            activeDiagonalButton = 0;
        }
        else {
            if ((prevDirection & JoyDPadButton::DpadUp) && (!(value & JoyDPadButton::DpadUp) || currentMode == EightWayMode))
            {
                prevButton = buttons.value(JoyDPadButton::DpadUp);
                prevButton->joyEvent(false, ignoresets);
            }

            if ((prevDirection & JoyDPadButton::DpadDown) && (!(value & JoyDPadButton::DpadDown) || currentMode == EightWayMode))
            {
                prevButton = buttons.value(JoyDPadButton::DpadDown);
                prevButton->joyEvent(false, ignoresets);
            }

            if ((prevDirection & JoyDPadButton::DpadLeft) && (!(value & JoyDPadButton::DpadLeft) || currentMode == EightWayMode))
            {
                prevButton = buttons.value(JoyDPadButton::DpadLeft);
                prevButton->joyEvent(false, ignoresets);
            }

            if ((prevDirection & JoyDPadButton::DpadRight) && (!(value & JoyDPadButton::DpadRight) || currentMode == EightWayMode))
            {
                prevButton = buttons.value(JoyDPadButton::DpadRight);
                prevButton->joyEvent(false, ignoresets);
            }
        }

        if (currentMode == EightWayMode && value == JoyDPadButton::DpadLeftUp)
        {
            activeDiagonalButton = buttons.value(JoyDPadButton::DpadLeftUp);
            activeDiagonalButton->joyEvent(true, ignoresets);
        }
        else if (currentMode == EightWayMode && value == JoyDPadButton::DpadRightUp)
        {
            activeDiagonalButton = buttons.value(JoyDPadButton::DpadRightUp);
            activeDiagonalButton->joyEvent(true, ignoresets);
        }
        else if (currentMode == EightWayMode && value == JoyDPadButton::DpadRightDown)
        {
            activeDiagonalButton = buttons.value(JoyDPadButton::DpadRightDown);
            activeDiagonalButton->joyEvent(true, ignoresets);
        }
        else if (currentMode == EightWayMode && value == JoyDPadButton::DpadLeftDown)
        {
            activeDiagonalButton = buttons.value(JoyDPadButton::DpadLeftDown);
            activeDiagonalButton->joyEvent(true, ignoresets);
        }
        else
        {
            if ((value & JoyDPadButton::DpadUp) && (!(prevDirection & JoyDPadButton::DpadUp) || currentMode == EightWayMode))
            {
                curButton = buttons.value(JoyDPadButton::DpadUp);
                curButton->joyEvent(true, ignoresets);
            }

            if ((value & JoyDPadButton::DpadDown) && (!(prevDirection & JoyDPadButton::DpadDown) || currentMode == EightWayMode))
            {
                curButton = buttons.value(JoyDPadButton::DpadDown);
                curButton->joyEvent(true, ignoresets);
            }

            if ((value & JoyDPadButton::DpadLeft) && (!(prevDirection & JoyDPadButton::DpadLeft) || currentMode == EightWayMode))
            {
                curButton = buttons.value(JoyDPadButton::DpadLeft);
                curButton->joyEvent(true, ignoresets);
            }

            if ((value & JoyDPadButton::DpadRight) && (!(prevDirection & JoyDPadButton::DpadRight) || currentMode == EightWayMode))
            {
                curButton = buttons.value(JoyDPadButton::DpadRight);
                curButton->joyEvent(true, ignoresets);
            }
        }
    }

    prevDirection = (JoyDPadButton::JoyDPadDirections)value;
}

QHash<int, JoyDPadButton*>* JoyDPad::getJoyButtons()
{
    return &buttons;
}

int JoyDPad::getCurrentDirection()
{
    return prevDirection;
}

void JoyDPad::setJoyMode(JoyMode mode)
{
    currentMode = mode;
}

JoyDPad::JoyMode JoyDPad::getJoyMode()
{
    return currentMode;
}

void JoyDPad::releaseButtonEvents()
{
    QHashIterator<int, JoyDPadButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->joyEvent(false, true);
    }
}

QHash<int, JoyDPadButton*>* JoyDPad::getButtons()
{
    return &buttons;
}

bool JoyDPad::isDefault()
{
    bool value = true;
    value = value && (currentMode == StandardMode);
    QHashIterator<int, JoyDPadButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        value = value && (button->isDefault());
    }
    return value;
}

void JoyDPad::setButtonsMouseMode(JoyButton::JoyMouseMovementMode mode)
{
    QHashIterator<int, JoyDPadButton*> iter(buttons);
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
    QHash<int, JoyDPadButton*> temphash;
    temphash.insert(JoyDPadButton::DpadUp, buttons.value(JoyDPadButton::DpadUp));
    temphash.insert(JoyDPadButton::DpadDown, buttons.value(JoyDPadButton::DpadDown));
    temphash.insert(JoyDPadButton::DpadLeft, buttons.value(JoyDPadButton::DpadLeft));
    temphash.insert(JoyDPadButton::DpadRight, buttons.value(JoyDPadButton::DpadRight));
    if (currentMode == EightWayMode)
    {
        temphash.insert(JoyDPadButton::DpadLeftUp, buttons.value(JoyDPadButton::DpadLeftUp));
        temphash.insert(JoyDPadButton::DpadRightUp, buttons.value(JoyDPadButton::DpadRightUp));
        temphash.insert(JoyDPadButton::DpadRightDown, buttons.value(JoyDPadButton::DpadRightDown));
        temphash.insert(JoyDPadButton::DpadLeftDown, buttons.value(JoyDPadButton::DpadLeftDown));
    }

    QHashIterator<int, JoyDPadButton*> iter(temphash);
    while (iter.hasNext())
    {
        if (!iter.hasPrevious())
        {
            JoyDPadButton *button = iter.next().value();
            initialMode = button->getMouseMode();
        }
        else
        {
            JoyDPadButton *button = iter.next().value();
            JoyButton::JoyMouseMovementMode temp = button->getMouseMode();
            if (temp != initialMode)
            {
                result = false;
                iter.toBack();
            }
        }
    }

    return result;
}

JoyButton::JoyMouseMovementMode JoyDPad::getButtonsPresetMouseMode()
{
    JoyButton::JoyMouseMovementMode resultMode = JoyButton::MouseCursor;

    QHash<int, JoyDPadButton*> temphash;
    temphash.insert(JoyDPadButton::DpadUp, buttons.value(JoyDPadButton::DpadUp));
    temphash.insert(JoyDPadButton::DpadDown, buttons.value(JoyDPadButton::DpadDown));
    temphash.insert(JoyDPadButton::DpadLeft, buttons.value(JoyDPadButton::DpadLeft));
    temphash.insert(JoyDPadButton::DpadRight, buttons.value(JoyDPadButton::DpadRight));
    if (currentMode == EightWayMode)
    {
        temphash.insert(JoyDPadButton::DpadLeftUp, buttons.value(JoyDPadButton::DpadLeftUp));
        temphash.insert(JoyDPadButton::DpadRightUp, buttons.value(JoyDPadButton::DpadRightUp));
        temphash.insert(JoyDPadButton::DpadRightDown, buttons.value(JoyDPadButton::DpadRightDown));
        temphash.insert(JoyDPadButton::DpadLeftDown, buttons.value(JoyDPadButton::DpadLeftDown));
    }

    QHashIterator<int, JoyDPadButton*> iter(temphash);
    while (iter.hasNext())
    {
        if (!iter.hasPrevious())
        {
            JoyDPadButton *button = iter.next().value();
            resultMode = button->getMouseMode();
        }
        else
        {
            JoyDPadButton *button = iter.next().value();
            JoyButton::JoyMouseMovementMode temp = button->getMouseMode();
            if (temp != resultMode)
            {
                resultMode = JoyButton::MouseCursor;
                iter.toBack();
            }
        }
    }

    return resultMode;
}

void JoyDPad::setButtonsMouseCurve(JoyButton::JoyMouseCurve mouseCurve)
{
    QHashIterator<int, JoyDPadButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->setMouseCurve(mouseCurve);
    }
}

bool JoyDPad::hasSameButtonsMouseCurve()
{
    bool result = true;

    JoyButton::JoyMouseCurve initialCurve = JoyButton::LinearCurve;
    QHash<int, JoyDPadButton*> temphash;
    temphash.insert(JoyDPadButton::DpadUp, buttons.value(JoyDPadButton::DpadUp));
    temphash.insert(JoyDPadButton::DpadDown, buttons.value(JoyDPadButton::DpadDown));
    temphash.insert(JoyDPadButton::DpadLeft, buttons.value(JoyDPadButton::DpadLeft));
    temphash.insert(JoyDPadButton::DpadRight, buttons.value(JoyDPadButton::DpadRight));
    if (currentMode == EightWayMode)
    {
        temphash.insert(JoyDPadButton::DpadLeftUp, buttons.value(JoyDPadButton::DpadLeftUp));
        temphash.insert(JoyDPadButton::DpadRightUp, buttons.value(JoyDPadButton::DpadRightUp));
        temphash.insert(JoyDPadButton::DpadRightDown, buttons.value(JoyDPadButton::DpadRightDown));
        temphash.insert(JoyDPadButton::DpadLeftDown, buttons.value(JoyDPadButton::DpadLeftDown));
    }

    QHashIterator<int, JoyDPadButton*> iter(temphash);
    while (iter.hasNext())
    {
        if (!iter.hasPrevious())
        {
            JoyDPadButton *button = iter.next().value();
            initialCurve = button->getMouseCurve();
        }
        else
        {
            JoyDPadButton *button = iter.next().value();
            JoyButton::JoyMouseCurve temp = button->getMouseCurve();
            if (temp != initialCurve)
            {
                result = false;
                iter.toBack();
            }
        }
    }

    return result;
}

JoyButton::JoyMouseCurve JoyDPad::getButtonsPresetMouseCurve()
{
    JoyButton::JoyMouseCurve resultCurve = JoyButton::LinearCurve;

    QHash<int, JoyDPadButton*> temphash;
    temphash.insert(JoyDPadButton::DpadUp, buttons.value(JoyDPadButton::DpadUp));
    temphash.insert(JoyDPadButton::DpadDown, buttons.value(JoyDPadButton::DpadDown));
    temphash.insert(JoyDPadButton::DpadLeft, buttons.value(JoyDPadButton::DpadLeft));
    temphash.insert(JoyDPadButton::DpadRight, buttons.value(JoyDPadButton::DpadRight));
    if (currentMode == EightWayMode)
    {
        temphash.insert(JoyDPadButton::DpadLeftUp, buttons.value(JoyDPadButton::DpadLeftUp));
        temphash.insert(JoyDPadButton::DpadRightUp, buttons.value(JoyDPadButton::DpadRightUp));
        temphash.insert(JoyDPadButton::DpadRightDown, buttons.value(JoyDPadButton::DpadRightDown));
        temphash.insert(JoyDPadButton::DpadLeftDown, buttons.value(JoyDPadButton::DpadLeftDown));
    }

    QHashIterator<int, JoyDPadButton*> iter(temphash);
    while (iter.hasNext())
    {
        if (!iter.hasPrevious())
        {
            JoyDPadButton *button = iter.next().value();
            resultCurve = button->getMouseCurve();
        }
        else
        {
            JoyDPadButton *button = iter.next().value();
            JoyButton::JoyMouseCurve temp = button->getMouseCurve();
            if (temp != resultCurve)
            {
                resultCurve = JoyButton::LinearCurve;
                iter.toBack();
            }
        }
    }

    return resultCurve;
}

void JoyDPad::setButtonsSpringWidth(int value)
{
    QHashIterator<int, JoyDPadButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->setSpringWidth(value);
    }
}

void JoyDPad::setButtonsSpringHeight(int value)
{
    QHashIterator<int, JoyDPadButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->setSpringHeight(value);
    }
}

int JoyDPad::getButtonsPresetSpringWidth()
{
    int presetSpringWidth = 0;

    QHash<int, JoyDPadButton*> temphash;
    temphash.insert(JoyDPadButton::DpadUp, buttons.value(JoyDPadButton::DpadUp));
    temphash.insert(JoyDPadButton::DpadDown, buttons.value(JoyDPadButton::DpadDown));
    temphash.insert(JoyDPadButton::DpadLeft, buttons.value(JoyDPadButton::DpadLeft));
    temphash.insert(JoyDPadButton::DpadRight, buttons.value(JoyDPadButton::DpadRight));
    if (currentMode == EightWayMode)
    {
        temphash.insert(JoyDPadButton::DpadLeftUp, buttons.value(JoyDPadButton::DpadLeftUp));
        temphash.insert(JoyDPadButton::DpadRightUp, buttons.value(JoyDPadButton::DpadRightUp));
        temphash.insert(JoyDPadButton::DpadRightDown, buttons.value(JoyDPadButton::DpadRightDown));
        temphash.insert(JoyDPadButton::DpadLeftDown, buttons.value(JoyDPadButton::DpadLeftDown));
    }

    QHashIterator<int, JoyDPadButton*> iter(temphash);
    while (iter.hasNext())
    {
        if (!iter.hasPrevious())
        {
            JoyDPadButton *button = iter.next().value();
            presetSpringWidth = button->getSpringWidth();
        }
        else
        {
            JoyDPadButton *button = iter.next().value();
            int temp = button->getSpringWidth();
            if (temp != presetSpringWidth)
            {
                presetSpringWidth = 0;
                iter.toBack();
            }
        }
    }

    return presetSpringWidth;
}

int JoyDPad::getButtonsPresetSpringHeight()
{
    int presetSpringHeight = 0;

    QHash<int, JoyDPadButton*> temphash;
    temphash.insert(JoyDPadButton::DpadUp, buttons.value(JoyDPadButton::DpadUp));
    temphash.insert(JoyDPadButton::DpadDown, buttons.value(JoyDPadButton::DpadDown));
    temphash.insert(JoyDPadButton::DpadLeft, buttons.value(JoyDPadButton::DpadLeft));
    temphash.insert(JoyDPadButton::DpadRight, buttons.value(JoyDPadButton::DpadRight));
    if (currentMode == EightWayMode)
    {
        temphash.insert(JoyDPadButton::DpadLeftUp, buttons.value(JoyDPadButton::DpadLeftUp));
        temphash.insert(JoyDPadButton::DpadRightUp, buttons.value(JoyDPadButton::DpadRightUp));
        temphash.insert(JoyDPadButton::DpadRightDown, buttons.value(JoyDPadButton::DpadRightDown));
        temphash.insert(JoyDPadButton::DpadLeftDown, buttons.value(JoyDPadButton::DpadLeftDown));
    }

    QHashIterator<int, JoyDPadButton*> iter(temphash);
    while (iter.hasNext())
    {
        if (!iter.hasPrevious())
        {
            JoyDPadButton *button = iter.next().value();
            presetSpringHeight = button->getSpringHeight();
        }
        else
        {
            JoyDPadButton *button = iter.next().value();
            int temp = button->getSpringHeight();
            if (temp != presetSpringHeight)
            {
                presetSpringHeight = 0;
                iter.toBack();
            }
        }
    }

    return presetSpringHeight;
}

void JoyDPad::setButtonsSensitivity(double value)
{
    QHashIterator<int, JoyDPadButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->setSensitivity(value);
    }
}

double JoyDPad::getButtonsPresetSensitivity()
{
    double presetSensitivity = 1.0;

    QHash<int, JoyDPadButton*> temphash;
    temphash.insert(JoyDPadButton::DpadUp, buttons.value(JoyDPadButton::DpadUp));
    temphash.insert(JoyDPadButton::DpadDown, buttons.value(JoyDPadButton::DpadDown));
    temphash.insert(JoyDPadButton::DpadLeft, buttons.value(JoyDPadButton::DpadLeft));
    temphash.insert(JoyDPadButton::DpadRight, buttons.value(JoyDPadButton::DpadRight));
    if (currentMode == EightWayMode)
    {
        temphash.insert(JoyDPadButton::DpadLeftUp, buttons.value(JoyDPadButton::DpadLeftUp));
        temphash.insert(JoyDPadButton::DpadRightUp, buttons.value(JoyDPadButton::DpadRightUp));
        temphash.insert(JoyDPadButton::DpadRightDown, buttons.value(JoyDPadButton::DpadRightDown));
        temphash.insert(JoyDPadButton::DpadLeftDown, buttons.value(JoyDPadButton::DpadLeftDown));
    }

    QHashIterator<int, JoyDPadButton*> iter(temphash);
    while (iter.hasNext())
    {
        if (!iter.hasPrevious())
        {
            JoyDPadButton *button = iter.next().value();
            presetSensitivity = button->getSensitivity();
        }
        else
        {
            JoyDPadButton *button = iter.next().value();
            double temp = button->getSensitivity();
            if (temp != presetSensitivity)
            {
                presetSensitivity = 1.0;
                iter.toBack();
            }
        }
    }

    return presetSensitivity;
}

QHash<int, JoyDPadButton*> JoyDPad::getApplicableButtons()
{
    QHash<int, JoyDPadButton*> temphash;
    temphash.insert(JoyDPadButton::DpadUp, buttons.value(JoyDPadButton::DpadUp));
    temphash.insert(JoyDPadButton::DpadDown, buttons.value(JoyDPadButton::DpadDown));
    temphash.insert(JoyDPadButton::DpadLeft, buttons.value(JoyDPadButton::DpadLeft));
    temphash.insert(JoyDPadButton::DpadRight, buttons.value(JoyDPadButton::DpadRight));
    if (currentMode == EightWayMode)
    {
        temphash.insert(JoyDPadButton::DpadLeftUp, buttons.value(JoyDPadButton::DpadLeftUp));
        temphash.insert(JoyDPadButton::DpadRightUp, buttons.value(JoyDPadButton::DpadRightUp));
        temphash.insert(JoyDPadButton::DpadRightDown, buttons.value(JoyDPadButton::DpadRightDown));
        temphash.insert(JoyDPadButton::DpadLeftDown, buttons.value(JoyDPadButton::DpadLeftDown));
    }

    return temphash;
}

void JoyDPad::setButtonsSmoothing(bool enabled)
{
    QHashIterator<int, JoyDPadButton*> iter(buttons);
    while (iter.hasNext())
    {
        JoyDPadButton *button = iter.next().value();
        button->setSmoothing(enabled);
    }
}

bool JoyDPad::getButtonsPresetSmoothing()
{
    bool presetSmoothing = false;

    QHash<int, JoyDPadButton*> temphash = getApplicableButtons();
    QHashIterator<int, JoyDPadButton*> iter(temphash);
    while (iter.hasNext())
    {
        if (!iter.hasPrevious())
        {
            JoyDPadButton *button = iter.next().value();
            presetSmoothing = button->isSmoothingEnabled();
        }
        else
        {
            JoyDPadButton *button = iter.next().value();
            bool temp = button->isSmoothingEnabled();
            if (temp != presetSmoothing)
            {
                presetSmoothing = false;
                iter.toBack();
            }
        }
    }

    return presetSmoothing;
}
