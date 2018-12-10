#ifndef JOYBUTTONXML_H
#define JOYBUTTONXML_H

#include <QObject>
#include <QPointer>

class JoyButton;
class JoyButtonSlotXml;
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
    QPointer<JoyButtonSlotXml> m_btnSlotXml;

};

#endif // JOYBUTTONXML_H
