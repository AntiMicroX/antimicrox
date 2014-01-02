#ifndef GAMECONTROLLERTRIGGER_H
#define GAMECONTROLLERTRIGGER_H

#include <QObject>
#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <SDL2/SDL_gamecontroller.h>

#include "gamecontrollertriggerbutton.h"
#include <joyaxis.h>

class GameControllerTrigger : public JoyAxis
{
    Q_OBJECT
public:
    explicit GameControllerTrigger(int index, int originset, QObject *parent = 0);

    virtual QString getXmlName();
    virtual QString getPartialName(bool forceFullFormat, bool displayNames);

    virtual int getDefaultDeadZone();
    virtual int getDefaultMaxZone();
    virtual ThrottleTypes getDefaultThrottle();

    virtual void writeConfig(QXmlStreamWriter *xml);

    static const int AXISDEADZONE;
    static const int AXISMAXZONE;
    static const ThrottleTypes DEFAULTTHROTTLE;

    static const QString xmlName;

signals:

public slots:

};

#endif // GAMECONTROLLERTRIGGER_H
