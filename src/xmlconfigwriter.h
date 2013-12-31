#ifndef XMLCONFIGWRITER_H
#define XMLCONFIGWRITER_H

#include <QObject>
#include <QFile>
#include <QXmlStreamWriter>

#include "inputdevice.h"
#include "common.h"

class XMLConfigWriter : public QObject
{
    Q_OBJECT
public:
    explicit XMLConfigWriter(QObject *parent = 0);
    ~XMLConfigWriter();
    void setFileName(QString filename);

protected:
    QXmlStreamWriter *xml;
    QString fileName;
    QFile *configFile;
    InputDevice* joystick;

signals:
    
public slots:
    void write(InputDevice* joystick);

};

#endif // XMLCONFIGWRITER_H
