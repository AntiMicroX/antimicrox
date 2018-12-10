#ifndef VDPADXML_H
#define VDPADXML_H

#include "xml/joydpadxml.h"

class VDPad;

class VDPadXml : public JoyDPadXml
{
public:
    VDPadXml(VDPad* joyvdpad, QObject *parent = nullptr);

private:
    VDPad* m_vdpad;
};

#endif // VDPADXML_H
