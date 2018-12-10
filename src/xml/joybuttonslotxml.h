#ifndef JOYBUTTONSLOTXML_H
#define JOYBUTTONSLOTXML_H

class JoyButtonSlot;
class QXmlStreamReader;
class QXmlStreamWriter;

#include <QObject>

class JoyButtonSlotXml : public QObject
{
    Q_OBJECT
public:
    explicit JoyButtonSlotXml(JoyButtonSlot* btnSlot, QObject *parent = nullptr);

    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

signals:

public slots:

private:
    JoyButtonSlot* m_btnSlot;
};

#endif // JOYBUTTONSLOTXML_H
