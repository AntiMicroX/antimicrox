#ifndef JOYBUTTONXML_H
#define JOYBUTTONXML_H

#include <QObject>
class JoyButton;
class QXmlStreamReader;
class QXmlStreamWriter;

class JoyButtonXml : public QObject
{
    Q_OBJECT

public:
    explicit JoyButtonXml(JoyButton* joyButton, QObject *parent = nullptr);

    virtual bool readButtonConfig(QXmlStreamReader *xml);
    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

private:
    JoyButton* m_joyButton;

};

#endif // JOYBUTTONXML_H
