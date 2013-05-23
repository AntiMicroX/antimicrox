#ifndef XMLCONFIGREADER_H
#define XMLCONFIGREADER_H

#include <QObject>
#include <QXmlStreamReader>
#include <QFile>

#include "joystick.h"
#include "common.h"

class XMLConfigReader : public QObject
{
    Q_OBJECT
public:
    explicit XMLConfigReader(QObject *parent = 0);
    ~XMLConfigReader();
    void setJoystick(Joystick *joystick);
    void setFileName(QString filename);

    bool read();


protected:
    QXmlStreamReader *xml;
    QString fileName;
    QFile *configFile;
    Joystick* joystick;

signals:
    
public slots:
    void configJoystick(Joystick *joystick);

};

#endif // XMLCONFIGREADER_H
