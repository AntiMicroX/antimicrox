#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <SDL2/SDL_gamecontroller.h>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "inputdevice.h"
#include "gamecontrollerdpad.h"
#include "gamecontrollerset.h"

class GameController : public InputDevice
{
    Q_OBJECT
public:
    explicit GameController(SDL_GameController *controller, QObject *parent = 0);

    virtual QString getName();
    virtual QString getSDLName();
    virtual QString getGUIDString(); // GUID available on SDL 2.
    virtual QString getXmlName();
    virtual void closeSDLDevice();
    virtual SDL_JoystickID getSDLJoystickID();

    virtual int getNumberRawButtons();
    virtual int getNumberRawAxes();
    virtual int getNumberRawHats();

    QString getBindStringForAxis(int index);
    QString getBindStringForButton(int index);

    SDL_GameControllerButtonBind getBindForAxis(int index);
    SDL_GameControllerButtonBind getBindForButton(int index);

    virtual void readConfig(QXmlStreamReader *xml);
    virtual void writeConfig(QXmlStreamWriter *xml);

    static const QString xmlName;

protected:
    SDL_GameController *controller;

signals:


public slots:

protected slots:
    virtual void axisActivatedEvent(int setindex, int axisindex, int value);
    virtual void buttonClickEvent(int setindex, int buttonindex);
    virtual void buttonReleaseEvent(int setindex, int buttonindex);
    virtual void axisButtonDownEvent(int setindex, int axisindex, int buttonindex);
    virtual void axisButtonUpEvent(int setindex, int axisindex, int buttonindex);
    virtual void dpadButtonDownEvent(int setindex, int dpadindex, int buttonindex);
    virtual void dpadButtonUpEvent(int setindex, int dpadindex, int buttonindex);
};

#endif // GAMECONTROLLER_H
