#ifndef JOYDPADXML_H
#define JOYDPADXML_H

#include <QObject>

class JoyDPad;
class QXmlStreamReader;
class QXmlStreamWriter;

class JoyDPadXml : public QObject
{
    Q_OBJECT
public:
    explicit JoyDPadXml(JoyDPad* joydpad, QObject *parent = nullptr);

    void readConfig(QXmlStreamReader *xml); // JoyDPadXml class
    void writeConfig(QXmlStreamWriter *xml); // JoyDPadXml class

    bool readMainConfig(QXmlStreamReader *xml);

private:

    JoyDPad* m_joydpad;
};

#endif // JOYDPADXML_H
