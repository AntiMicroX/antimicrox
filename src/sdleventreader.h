#ifndef SDLEVENTREADER_H
#define SDLEVENTREADER_H

#include <QObject>
#include <QMap>
#include <QTimer>

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
    explicit SDLEventReader(QMap<SDL_JoystickID, InputDevice*> *joysticks,
                            AntiMicroSettings *settings,
                            QObject *parent = 0);
    ~SDLEventReader();

    bool isSDLOpen();

protected:
    void initSDL();
    void closeSDL();
    void clearEvents();
    int CheckForEvents();

    QMap<SDL_JoystickID, InputDevice*> *joysticks;
    bool sdlIsOpen;
    AntiMicroSettings *settings;
    unsigned int pollRate;
    QTimer pollRateTimer;

signals:
    void eventRaised();
    void finished();
    void sdlStarted();
    void sdlClosed();

public slots:
    void performWork();
    void stop();
    void refresh();
    void updatePollRate(unsigned int tempPollRate);

private slots:
    void secondaryRefresh();

};

#endif // SDLEVENTREADER_H
