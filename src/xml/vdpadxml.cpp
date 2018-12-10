#include "vdpadxml.h"
#include "vdpad.h"

VDPadXml::VDPadXml(VDPad* joyvdpad, QObject *parent) : JoyDPadXml(joyvdpad, parent)
{
    m_vdpad = joyvdpad;
}
