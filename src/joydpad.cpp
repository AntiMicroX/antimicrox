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
            if (prevDirection & JoyDPadButton::DpadUp)
            {
                prevButton = buttons.value(JoyDPadButton::DpadUp);
                prevButton->joyEvent(false, ignoresets);
            }

            if (prevDirection & JoyDPadButton::DpadRight)
            {
                prevButton = buttons.value(JoyDPadButton::DpadRight);
                prevButton->joyEvent(false, ignoresets);
            }

            if (prevDirection & JoyDPadButton::DpadDown)
            {
                prevButton = buttons.value(JoyDPadButton::DpadDown);
                prevButton->joyEvent(false, ignoresets);
            }

            if (prevDirection & JoyDPadButton::DpadLeft)
            {
                prevButton = buttons.value(JoyDPadButton::DpadLeft);
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
            if (value & JoyDPadButton::DpadUp)
            {
                curButton = buttons.value(JoyDPadButton::DpadUp);
                curButton->joyEvent(true, ignoresets);
            }

            if (value & JoyDPadButton::DpadRight)
            {
                curButton = buttons.value(JoyDPadButton::DpadRight);
                curButton->joyEvent(true, ignoresets);
            }

            if (value & JoyDPadButton::DpadDown)
            {
                curButton = buttons.value(JoyDPadButton::DpadDown);
                curButton->joyEvent(true, ignoresets);
            }

            if (value & JoyDPadButton::DpadLeft)
            {
                curButton = buttons.value(JoyDPadButton::DpadLeft);
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
