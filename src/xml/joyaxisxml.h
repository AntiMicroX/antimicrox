#pragma once
#ifndef JOYAXISXML_H
#define JOYAXISXML_H

#include <QObject>
#include <QPointer>

class JoyAxis;
class JoyButtonXml;
class QXmlStreamReader;
class QXmlStreamWriter;

class JoyAxisXml : public QObject
{
    Q_OBJECT

public:
    explicit JoyAxisXml(JoyAxis* axis, QObject *parent = nullptr);
    ~JoyAxisXml();

    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

    virtual bool readMainConfig(QXmlStreamReader *xml);
    virtual bool readButtonConfig(QXmlStreamReader *xml);

private:
    JoyAxis* m_joyAxis;
    QPointer<JoyButtonXml> joyButtonXmlNAxis;
    QPointer<JoyButtonXml> joyButtonXmlPAxis;
};

#endif // JOYAXISXML_H
