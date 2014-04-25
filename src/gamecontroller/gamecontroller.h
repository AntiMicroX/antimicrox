#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <SDL2/SDL_gamecontroller.h>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <inputdevice.h>
#include "gamecontrollerdpad.h"
#include "gamecontrollerset.h"

class GameController : public InputDevice
{
    Q_OBJECT
public:
    explicit GameController(SDL_GameController *controller, int deviceIndex, QObject *parent = 0);

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
    void readJoystickConfig(QXmlStreamReader *xml);

    SDL_GameController *controller;

signals:


public slots:

protected slots:
    virtual void axisActivatedEvent(int setindex, int axisindex, int value);
    virtual void buttonClickEvent(int buttonindex);
    virtual void buttonReleaseEvent(int buttonindex);
};

#endif // GAMECONTROLLER_H
