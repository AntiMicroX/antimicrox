#include <QHashIterator>

#include "joydpad.h"

JoyDPad::JoyDPad(QObject *parent) :
    QObject(parent)
{
    buttons = QHash<int, JoyDPadButton*> ();
    prevDirection = JoyDPadButton::DpadCentered;
    populateButtons();
    originset = 0;
}

JoyDPad::JoyDPad(int index, int originset, QObject *parent) :
    QObject(parent)
{
    this->index = index;
    buttons = QHash<int, JoyDPadButton*> ();
    prevDirection = JoyDPadButton::DpadCentered;
    populateButtons();
    this->originset = originset;
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

int JoyDPad::getRealJoyNumber()
{
    return index + 1;
}

void JoyDPad::readConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && xml->name() == "dpad")
    {
        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != "dpad"))
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
    xml->writeStartElement("dpad");
    xml->writeAttribute("index", QString::number(index+1));

    QHash<int, JoyDPadButton*>::iterator i;
    for (i = buttons.begin(); i != buttons.end(); i++)
    {
        JoyDPadButton *button = (*i);
        button->writeConfig(xml);
    }

    xml->writeEndElement();
}

void JoyDPad::joyEvent(int value, bool ignoresets)
{
    JoyDPadButton *curButton;
    JoyDPadButton *prevButton;
    if (value != prevDirection)
    {
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

        /*if (prevDirection & JoyDPadButton::DpadRightUp)
        {
            prevButton = buttons.value(JoyDPadButton::DpadRightUp);
            prevButton->joyEvent(false);
        }
        if (prevDirection & JoyDPadButton::DpadRightDown)
        {
            prevButton = buttons.value(JoyDPadButton::DpadRightDown);
            prevButton->joyEvent(false);
        }
        if (prevDirection & JoyDPadButton::DpadLeftUp)
        {
            prevButton = buttons.value(JoyDPadButton::DpadLeftUp);
            prevButton->joyEvent(false);
        }
        if (prevDirection & JoyDPadButton::DpadLeftDown)
        {
            prevButton = buttons.value(JoyDPadButton::DpadLeftDown);
            prevButton->joyEvent(false);
        }*/

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
        /*if (value & JoyDPadButton::DpadRightUp)
        {
            curButton = buttons.value(JoyDPadButton::DpadRightUp);
            curButton->joyEvent(true);
        }
        if (value & JoyDPadButton::DpadRightDown)
        {
            curButton = buttons.value(JoyDPadButton::DpadRightDown);
            curButton->joyEvent(true);
        }
        if (value & JoyDPadButton::DpadLeftUp)
        {
            curButton = buttons.value(JoyDPadButton::DpadLeftUp);
            curButton->joyEvent(true);
        }
        if (value & JoyDPadButton::DpadLeftDown)
        {
            curButton = buttons.value(JoyDPadButton::DpadLeftDown);
            curButton->joyEvent(true);
        }*/

    }

    prevDirection = value;


    /*if (value == JoyDPadButton::DpadCentered && prevDirection != JoyDPadButton::DpadCentered)
    {
        JoyDPadButton *button = buttons.value(prevDirection);
        button->joyEvent(false);
    }
    else if (value != prevDirection)
    {
        if (prevDirection != JoyDPadButton::DpadCentered)
        {
            JoyDPadButton *prevButton = buttons.value(prevDirection);
            prevButton->joyEvent(false);
        }

        JoyDPadButton *curButton = buttons.value(value);
        curButton->joyEvent(true);
    }

    prevDirection = value;*/
}

QHash<int, JoyDPadButton*>* JoyDPad::getJoyButtons()
{
    return &buttons;
}

int JoyDPad::getCurrentDirection()
{
    return prevDirection;
}
