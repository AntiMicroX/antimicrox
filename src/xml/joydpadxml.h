#ifndef JOYDPADXML_H
#define JOYDPADXML_H

#include <QObject>

class QXmlStreamReader;
class QXmlStreamWriter;

template<class T>
class JoyDPadXml
{

public:
    JoyDPadXml(T* joydpad, QObject *parent = nullptr);

    void readConfig(QXmlStreamReader *xml); // JoyDPadXml class
    void writeConfig(QXmlStreamWriter *xml); // JoyDPadXml class

    bool readMainConfig(QXmlStreamReader *xml);

private:

    T* m_joydpad;
};

//#include "joydpadxml.cpp"

#endif // JOYDPADXML_H
