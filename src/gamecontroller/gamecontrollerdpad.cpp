#include "gamecontrollerdpad.h"

const QString GameControllerDPad::xmlName = "dpad";

GameControllerDPad::GameControllerDPad(JoyButton *upButton, JoyButton *downButton, JoyButton *leftButton, JoyButton *rightButton,
                                       int index, int originset, SetJoystick *parentSet, QObject *parent) :
    VDPad(upButton, downButton, leftButton, rightButton, index, originset, parentSet, parent)
{
}

QString GameControllerDPad::getName(bool forceFullFormat, bool displayName)
{
    QString label;

    if (!dpadName.isEmpty() && displayName)
    {
        if (forceFullFormat)
        {
            label.append(tr("DPad")).append(" ");
        }

        label.append(dpadName);
    }
    else if (!defaultDPadName.isEmpty())
    {
        if (forceFullFormat)
        {
            label.append(tr("DPad")).append(" ");
        }

        label.append(defaultDPadName);
    }
    else
    {
        label.append(tr("DPad")).append(" ");
        label.append(QString::number(getRealJoyNumber()));
    }

    return label;
}

QString GameControllerDPad::getXmlName()
{
    return this->xmlName;
}
