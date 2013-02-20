#include "buttontempconfig.h"
#include "event.h"

ButtonTempConfig::ButtonTempConfig(QObject *parent) :
    QObject(parent)
{
    turbo = false;
    toggle = false;
    turboInterval = 100;
    assignments = new QList<JoyButtonSlot*> ();
    mouseSpeedX = 20;
    mouseSpeedY = 20;
}

ButtonTempConfig::ButtonTempConfig(JoyButton *button, QObject *parent) :
    QObject(parent)
{
    turbo = button->isUsingTurbo();
    toggle = button->getToggleState();
    if (button->getTurboInterval() > 0)
    {
        turboInterval = button->getTurboInterval();
    }
    else
    {
        turboInterval = 100;
    }
    assignments = new QList<JoyButtonSlot*> ();
    mouseSpeedX = button->getMouseSpeedX();
    mouseSpeedY = button->getMouseSpeedY();
}

ButtonTempConfig::~ButtonTempConfig()
{
    delete assignments;
}

QString ButtonTempConfig::getSlotsSummary()
{
    QString newlabel;
    int slotCount = assignments->count();

    if (slotCount > 0)
    {
        JoyButtonSlot *slot = assignments->first();
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
