#ifndef GAMECONTROLLERTRIGGER_H
#define GAMECONTROLLERTRIGGER_H

#include <QObject>

#include <SDL2/SDL_gamecontroller.h>

#include "joyaxis.h"

class GameControllerTrigger : public JoyAxis
{
    Q_OBJECT
public:
    explicit GameControllerTrigger(int index, int originset, QObject *parent = 0);

    virtual bool isDefault();

signals:

public slots:

};

#endif // GAMECONTROLLERTRIGGER_H
