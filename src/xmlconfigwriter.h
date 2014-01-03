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
    bool hasError();
    QString getErrorString();

protected:
    QXmlStreamWriter *xml;
    QString fileName;
    QFile *configFile;
    InputDevice* joystick;
    bool writerError;
    QString writerErrorString;

signals:
    
public slots:
    void write(InputDevice* joystick);

};

#endif // XMLCONFIGWRITER_H
