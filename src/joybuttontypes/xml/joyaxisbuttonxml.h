#ifndef JOYAXISBUTTONXML_H
#define JOYAXISBUTTONXML_H

#include "xml/joybuttonxml.h"

#include <QObject>

class JoyAxisButton;

class JoyAxisButtonXml : public JoyButtonXml
{
public:
    JoyAxisButtonXml(JoyAxisButton* joyAxisBtn, QObject* parent = nullptr);

private:
    JoyAxisButton* m_joyAxisBtn;
};

#endif // JOYAXISBUTTONXML_H
