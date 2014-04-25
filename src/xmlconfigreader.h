#ifndef XMLCONFIGREADER_H
#define XMLCONFIGREADER_H

#include <QObject>
#include <QXmlStreamReader>
#include <QFile>

#include "inputdevice.h"
#include "joystick.h"
#include "gamecontroller/gamecontroller.h"
#include "common.h"

class XMLConfigReader : public QObject
{
    Q_OBJECT
public:
    explicit XMLConfigReader(QObject *parent = 0);
    ~XMLConfigReader();
    void setJoystick(InputDevice *joystick);
    void setFileName(QString filename);
    QString getErrorString();
    bool hasError();
    bool read();


protected:
    QXmlStreamReader *xml;
    QString fileName;
    QFile *configFile;
    InputDevice* joystick;

signals:
    
public slots:
    void configJoystick(InputDevice *joystick);

};

#endif // XMLCONFIGREADER_H
