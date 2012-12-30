#ifndef XMLCONFIGWRITER_H
#define XMLCONFIGWRITER_H

#include <QObject>
#include <QFile>
#include <QXmlStreamWriter>

#include "joystick.h"
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
    Joystick* joystick;

signals:
    
public slots:
    void write(Joystick* joystick);

};

#endif // XMLCONFIGWRITER_H
