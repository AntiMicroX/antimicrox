#ifndef GAMECONTROLLERSET_H
#define GAMECONTROLLERSET_H

#include <QObject>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <SDL2/SDL_gamecontroller.h>

#include <setjoystick.h>
#include "gamecontrollerdpad.h"
#include "gamecontrollertrigger.h"

class GameControllerSet : public SetJoystick
{
    Q_OBJECT
public:
    explicit GameControllerSet(InputDevice *device, int index, QObject *parent = 0);

    virtual void refreshAxes();

    virtual void readConfig(QXmlStreamReader *xml);

protected:
    void populateSticksDPad();

signals:

public slots:
    virtual void reset();
};

#endif // GAMECONTROLLERSET_H
