#ifndef JOYBUTTONSLOTXML_H
#define JOYBUTTONSLOTXML_H

#include <QObject>

#include "joybuttonslot.h"

class QXmlStreamReader;
class QXmlStreamWriter;

class JoyButtonSlotXml : public JoyButtonSlot
{
    Q_OBJECT

public:
     JoyButtonSlotXml(JoyButtonSlot *joyBtnSlot, QObject *parent = nullptr);

     virtual void readConfig(QXmlStreamReader *xml);
     virtual void writeConfig(QXmlStreamWriter *xml);

private:
     JoyButtonSlot* m_joyBtnSlot;
};

#endif // JOYBUTTONSLOTXML_H
