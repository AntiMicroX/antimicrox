#include "joyaxisbuttonxml.h"
#include "joybuttontypes/joyaxisbutton.h"

JoyAxisButtonXml::JoyAxisButtonXml(JoyAxisButton* joyAxisBtn, QObject* parent) : JoyButtonXml(joyAxisBtn, parent)
{
    m_joyAxisBtn = joyAxisBtn;
}
