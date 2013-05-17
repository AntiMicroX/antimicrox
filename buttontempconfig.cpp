#include <QListIterator>

#include "buttontempconfig.h"
#include "event.h"

ButtonTempConfig::ButtonTempConfig(QObject *parent) :
    QObject(parent)
{
    turbo = false;
    toggle = false;
    turboInterval = 100;
    assignments = new QList<JoyButtonSlot*> ();
    mouseSpeedX = 50;
    mouseSpeedY = 50;
    setSelection = -1;
    originset = 0;
    setSelectionCondition = JoyButton::SetChangeDisabled;
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
    assignments = new QList<JoyButtonSlot*> (*button->getAssignedSlots());
    mouseSpeedX = button->getMouseSpeedX();
    mouseSpeedY = button->getMouseSpeedY();

    setSelection = -1;
    setSelectionCondition = JoyButton::SetChangeDisabled;

    originset = button->getOriginSet();
    setSelection = button->getSetSelection();
    setSelectionCondition = button->getChangeSetCondition();
}

ButtonTempConfig::~ButtonTempConfig()
{
    if (assignments)
    {
        delete assignments;
        assignments = 0;
    }
}

QString ButtonTempConfig::getSlotsSummary()
{
    QString newlabel;
    int slotCount = assignments->size();

    if (slotCount > 0)
    {
        JoyButtonSlot *slot = assignments->first();
        newlabel = newlabel.append(slot->getSlotString());

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

bool ButtonTempConfig::containsSequence()
{
    bool result = false;

    QListIterator<JoyButtonSlot*> tempiter(*assignments);
    while (tempiter.hasNext() && !result)
    {
        JoyButtonSlot *slot = tempiter.next();
        JoyButtonSlot::JoySlotInputAction mode = slot->getSlotMode();
        if (mode == JoyButtonSlot::JoyPause ||
            mode == JoyButtonSlot::JoyHold ||
            mode == JoyButtonSlot::JoyDistance
           )
        {
            result = true;
        }
    }

    return result;
}
