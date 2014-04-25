#ifndef XMLCONFIGREADER_H
#define XMLCONFIGREADER_H

#include <QObject>
#include <QXmlStreamReader>
#include <QFile>

#include "inputdevice.h"
#include "joystick.h"

#ifdef USE_SDL_2
#include "gamecontroller/gamecontroller.h"
#endif

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
    void initDeviceTypes();

    QXmlStreamReader *xml;
    QString fileName;
    QFile *configFile;
    InputDevice* joystick;
    QStringList deviceTypes;

signals:
    
public slots:
    void configJoystick(InputDevice *joystick);

};

#endif // XMLCONFIGREADER_H
