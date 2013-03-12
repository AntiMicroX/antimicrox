#ifndef SDLEVENTREADER_H
#define SDLEVENTREADER_H

#include <QObject>
#include <QHash>
#include <SDL/SDL.h>

#include "joystick.h"

class SDLEventReader : public QObject
{
    Q_OBJECT
public:
    explicit SDLEventReader(QHash<int, Joystick*> *joysticks, QObject *parent = 0);
    ~SDLEventReader();
    SDL_Event& getCurrentEvent();

protected:
    void initSDL();
    void closeSDL();

    QHash<int, Joystick*> *joysticks;
    SDL_Event currentEvent;
    bool sdlIsOpen;

signals:
    void eventRaised();
    void finished();

public slots:
    void performWork();
    void stop();
    //void reset();
    void refresh();

private slots:
    //void refreshJoysticks ();

};

#endif // SDLEVENTREADER_H
