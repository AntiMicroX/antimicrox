#ifndef SDLEVENTREADER_H
#define SDLEVENTREADER_H

#include <QObject>
#include <QMap>

#ifdef USE_SDL_2
#include <SDL2/SDL.h>
#else
#include <SDL/SDL.h>
#endif

#include "joystick.h"
#include "inputdevice.h"
#include "antimicrosettings.h"

class SDLEventReader : public QObject
{
    Q_OBJECT
public:
    explicit SDLEventReader(QMap<SDL_JoystickID, InputDevice*> *joysticks, AntiMicroSettings *settings, QObject *parent = 0);
    ~SDLEventReader();

    bool isSDLOpen();

protected:
    void initSDL();
    void closeSDL();
    void clearEvents();

    QMap<SDL_JoystickID, InputDevice*> *joysticks;
    bool sdlIsOpen;
    AntiMicroSettings *settings;

signals:
    void eventRaised();
    void finished();
    void sdlStarted();
    void sdlClosed();

public slots:
    void performWork();
    void stop();
    void refresh();

private slots:
    void secondaryRefresh();

};

#endif // SDLEVENTREADER_H
