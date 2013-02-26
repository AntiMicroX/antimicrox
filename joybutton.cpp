#include <QDebug>
#include <QThread>
#include <cmath>

#include "joybutton.h"
#include "event.h"

const QString JoyButton::xmlName = "button";

JoyButton::JoyButton(QObject *parent) :
    QObject(parent)
{
    this->reset();
    index = 0;
}

JoyButton::JoyButton(int index, QObject *parent) :
    QObject(parent)
{
    this->reset();
    this->index = index;
}

void JoyButton::joyEvent(bool pressed)
{
    if (toggle && pressed && (pressed != isDown))
    {
        isButtonPressed = !isButtonPressed;
        isDown = true;
        emit clicked(index);
        createDeskEvent();
    }
    else if (toggle && !pressed && isDown)
    {
        isDown = false;
        emit released(index);
    }

    else if (!toggle && (pressed != isButtonPressed))
    {
        if (pressed)
        {
            emit clicked(index);
        }
        else
        {
            emit released(index);
        }

        isButtonPressed = pressed;

        if (isButtonPressed && useTurbo)
        {
            turboEvent();
            connect(&timer, SIGNAL(timeout()), this, SLOT(turboEvent()));
        }
        else if (!isButtonPressed && useTurbo)
        {
            timer.stop();
            disconnect(&timer, SIGNAL(timeout()), 0, 0);
            if (isKeyPressed)
            {
                turboEvent();
            }
        }

        if (!useTurbo)
        {
            createDeskEvent();
        }
    }
}

int JoyButton::getJoyNumber()
{
    return index;
}

int JoyButton::getRealJoyNumber()
{
    return index + 1;
}

void JoyButton::setJoyNumber(int index)
{
    this->index = index;
}

void JoyButton::setToggle(bool toggle)
{
    this->toggle = toggle;
}

void JoyButton::setTurboInterval(int interval)
{
    this->turboInterval = interval;
}

void JoyButton::reset()
{
    assignments.clear();

    isKeyPressed = isButtonPressed = false;
    toggle = false;
    turboInterval = 0;
    isDown = false;
    useTurbo = false;
    mouseSpeedX = 30;
    mouseSpeedY = 30;
    timer.stop();
}

void JoyButton::reset(int index)
{
    JoyButton::reset();
    this->index = index;
}

bool JoyButton::getToggleState()
{
    return toggle;
}

int JoyButton::getTurboInterval()
{
    return turboInterval;
}

void JoyButton::turboEvent()
{
    QListIterator<JoyButtonSlot*> iter(assignments);
    if (!isKeyPressed)
    {
        while(iter.hasNext())
        {
            JoyButtonSlot *slot = iter.next();
            int tempcode = slot->getSlotCode();
            JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();
            sendevent(tempcode, true, mode);
        }
        isKeyPressed = true;
        timer.start(100);
    }
    else
    {
        while(iter.hasNext())
        {
            JoyButtonSlot *slot = iter.next();
            int tempcode = slot->getSlotCode();
            JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();
            sendevent(tempcode, false, mode);
        }
        isKeyPressed = false;
        timer.start(turboInterval - 100);
    }
}

JoyButton::~JoyButton()
{
    timer.stop();
}

void JoyButton::createDeskEvent()
{
    QListIterator<JoyButtonSlot*> iter(assignments);
    while(iter.hasNext())
    {
        JoyButtonSlot *slot = iter.next();
        int tempcode = slot->getSlotCode();
        JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();
        if (mode != JoyButtonSlot::JoyMouseMovement)
        {
            sendevent(tempcode, isButtonPressed, mode);
        }
        else
        {
            slot->getMouseInterval()->restart();
            mouseEvent(slot);
        }
    }
}

void JoyButton::mouseEvent(JoyButtonSlot *buttonslot)
{
    QTime* mouseInterval = buttonslot->getMouseInterval();

    int mousemode = buttonslot->getSlotCode();
    int mousespeed;
    int timeElapsed = mouseInterval->elapsed();

    if (mousemode == JoyButtonSlot::MouseRight)
    {
        mousespeed = mouseSpeedX;
    }
    else if (mousemode == JoyButtonSlot::MouseLeft)
    {
        mousespeed = mouseSpeedX;
    }
    else if (mousemode == JoyButtonSlot::MouseDown)
    {
        mousespeed = mouseSpeedY;
    }
    else if (mousemode == JoyButtonSlot::MouseUp)
    {
        mousespeed = mouseSpeedY;
    }

    if (isButtonPressed && timeElapsed >= 1)
    {
        int mouse1 = 0;
        int mouse2 = 0;
        double sumDist = buttonslot->getDistance();

        if (mousemode == JoyButtonSlot::MouseRight)
        {
            sumDist += (mousespeed * JoyButtonSlot::JOYSPEED * timeElapsed) / 1000.0;
            int distance = (int)floor(sumDist + 0.5);
            mouse1 = distance;
        }
        else if (mousemode == JoyButtonSlot::MouseLeft)
        {
            sumDist += (mousespeed * JoyButtonSlot::JOYSPEED * timeElapsed) / 1000.0;
            int distance = (int)floor(sumDist + 0.5);
            mouse1 = -distance;
        }
        else if (mousemode == JoyButtonSlot::MouseDown)
        {
            sumDist += (mousespeed * JoyButtonSlot::JOYSPEED * timeElapsed) / 1000.0;
            int distance = (int)floor(sumDist + 0.5);
            mouse2 = distance;
        }
        else if (mousemode == JoyButtonSlot::MouseUp)
        {
            sumDist += (mousespeed * JoyButtonSlot::JOYSPEED * timeElapsed) / 1000.0;
            int distance = (int)floor(sumDist + 0.5);
            mouse2 = -distance;
        }

        if (sumDist < 1.0)
        {
            buttonslot->setDistance(sumDist);
        }
        else if (sumDist >= 1.0)
        {
            sendevent(mouse1, mouse2);
            sumDist = 0.0;

            buttonslot->setDistance(sumDist);
        }

        mouseInterval->restart();
    }

    if (isButtonPressed)
    {
        QMetaObject::invokeMethod(this, "mouseEvent", Qt::QueuedConnection, Q_ARG(JoyButtonSlot*, buttonslot));
    }
    else
    {
        buttonslot->setDistance(0.0);
        mouseInterval->restart();
    }
}

void JoyButton::setUseTurbo(bool useTurbo)
{
    this->useTurbo = useTurbo;
}

bool JoyButton::isUsingTurbo()
{
    return useTurbo;
}

QString JoyButton::getXmlName()
{
    return this->xmlName;
}

void JoyButton::readConfig(QXmlStreamReader *xml)
{
    if (xml->isStartElement() && xml->name() == getXmlName())
    {
        reset();

        xml->readNextStartElement();
        while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != getXmlName()))
        {
            if (xml->name() == "toggle" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                if (temptext == "true")
                {
                    this->setToggle(true);
                }
            }
            else if (xml->name() == "turbointerval" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setTurboInterval(tempchoice);
            }
            else if (xml->name() == "useturbo" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                if (temptext == "true")
                {
                    this->setUseTurbo(true);
                }
            }
            else if (xml->name() == "mousespeedx" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setMouseSpeedX(tempchoice);
            }
            else if (xml->name() == "mousespeedy" && xml->isStartElement())
            {
                QString temptext = xml->readElementText();
                int tempchoice = temptext.toInt();
                this->setMouseSpeedY(tempchoice);
            }
            else if (xml->name() == "slots" && xml->isStartElement())
            {
                xml->readNextStartElement();
                while (!xml->atEnd() && (!xml->isEndElement() && xml->name() != "slots"))
                {
                    if (xml->name() == "slot" && xml->isStartElement())
                    {
                        JoyButtonSlot *buttonslot = new JoyButtonSlot();
                        buttonslot->readConfig(xml);
                        this->assignments.append(buttonslot);
                    }
                    xml->readNextStartElement();
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

void JoyButton::writeConfig(QXmlStreamWriter *xml)
{
    xml->writeStartElement(getXmlName());
    xml->writeAttribute("index", QString::number(getRealJoyNumber()));

    xml->writeTextElement("toggle", toggle ? "true" : "false");
    xml->writeTextElement("turbointerval", QString::number(turboInterval));
    xml->writeTextElement("useturbo", useTurbo ? "true" : "false");
    xml->writeTextElement("mousespeedx", QString::number(mouseSpeedX));
    xml->writeTextElement("mousespeedy", QString::number(mouseSpeedY));

    xml->writeStartElement("slots");
    QListIterator<JoyButtonSlot*> iter(assignments);
    if (iter.hasNext())
    {
        JoyButtonSlot *buttonslot = iter.next();
        buttonslot->writeConfig(xml);
    }
    xml->writeEndElement();

    xml->writeEndElement();
}

QString JoyButton::getName()
{
    QString newlabel = getPartialName();
    newlabel = newlabel.append(": ").append(getSlotsSummary());
    return newlabel;
}

QString JoyButton::getPartialName()
{
    return QString("Button ").append(QString::number(getRealJoyNumber()));
}

QString JoyButton::getSlotsSummary()
{
    QString newlabel;
    int slotCount = assignments.count();

    if (slotCount > 0)
    {
        JoyButtonSlot *slot = assignments.first();
        int code = slot->getSlotCode();
        if (slot->getSlotMode() == JoyButtonSlot::JoyKeyboard)
        {
            newlabel = newlabel.append(keycodeToKey(code).toUpper());
        }
        else if (slot->getSlotMode() == JoyButtonSlot::JoyMouseButton)
        {
            newlabel = newlabel.append("Mouse ").append(QString::number(code));
        }
        else if (slot->getSlotMode() == JoyButtonSlot::JoyMouseMovement)
        {
            newlabel.append(slot->movementString());
        }

        if (slotCount > 1)
        {
            newlabel = newlabel.append(" ...");
        }
    }
    else
    {
        newlabel = newlabel.append("[NO KEY]");
    }

    return newlabel;
}

void JoyButton::setCustomName(QString name)
{
    customName = name;
}

QString JoyButton::getCustomName()
{
    return customName;
}

void JoyButton::setAssignedSlot(int code, JoyButtonSlot::JoySlotInputAction mode)
{
    JoyButtonSlot *slot = new JoyButtonSlot(code, mode);
    assignments.append(slot);
}

void JoyButton::setAssignedSlot(int code, int index, JoyButtonSlot::JoySlotInputAction mode)
{
    if (index >= 0 && index < assignments.count())
    {
        // Slot already exists. Override code and place into desired slot
        JoyButtonSlot *slot = new JoyButtonSlot(code, mode);
        assignments.insert(index, slot);
    }
    else if (index >= assignments.count())
    {
        // Append code into a new slot
        JoyButtonSlot *slot = new JoyButtonSlot(code, mode);
        assignments.append(slot);
    }
}

QList<JoyButtonSlot*>* JoyButton::getAssignedSlots()
{
    QList<JoyButtonSlot*> *newassign = new QList<JoyButtonSlot*> (assignments);
    return newassign;
}

void JoyButton::setMouseSpeedX(int speed)
{
    if (speed >= 1 && speed <= 200)
    {
        mouseSpeedX = speed;
    }
}

int JoyButton::getMouseSpeedX()
{
    return mouseSpeedX;
}

void JoyButton::setMouseSpeedY(int speed)
{
    if (speed >= 1 && speed <= 200)
    {
        mouseSpeedY = speed;
    }
}

int JoyButton::getMouseSpeedY()
{
    return mouseSpeedY;
}
