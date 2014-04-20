#ifndef SDLEVENTREADER_H
#define SDLEVENTREADER_H

#include <QObject>
#include <QHash>
#ifdef USE_SDL_2
#include <SDL2/SDL.h>
#else
#include <SDL/SDL.h>
#endif

#include "joystick.h"
#include "inputdevice.h"

class SDLEventReader : public QObject
{
    Q_OBJECT
public:
    explicit SDLEventReader(QHash<SDL_JoystickID, InputDevice*> *joysticks, QObject *parent = 0);
    ~SDLEventReader();
    SDL_Event& getCurrentEvent();
    bool isSDLOpen();

protected:
    void initSDL();
    void closeSDL();
    void clearEvents();

    QHash<SDL_JoystickID, InputDevice*> *joysticks;
    SDL_Event currentEvent;
    bool sdlIsOpen;

signals:
    void eventRaised();
    void finished();
    void sdlStarted();

public slots:
    void performWork();
    void stop();
    void refresh();

private slots:
    void secondaryRefresh();

};

#endif // SDLEVENTREADER_H
