/* antimicroX Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef SDLEVENTREADER_H
#define SDLEVENTREADER_H

#include "joystick.h"

class InputDevice;
class AntiMicroSettings;


class SDLEventReader : public QObject
{
    Q_OBJECT

public:
    explicit SDLEventReader(QMap<SDL_JoystickID, InputDevice*> *joysticks,
                            AntiMicroSettings *settings,
                            QObject *parent = nullptr);
    ~SDLEventReader();

    bool isSDLOpen();

    QMap<SDL_JoystickID, InputDevice*> *getJoysticks() const;
    AntiMicroSettings *getSettings() const;
    QTimer const& getPollRateTimer();

protected:
    void initSDL();
    void closeSDL();
    void clearEvents();
    int eventStatus();

signals:
    void eventRaised();
    void finished();
    void sdlStarted();
    void sdlClosed();

public slots:
    void performWork();
    void stop();
    void refresh();
    void updatePollRate(int tempPollRate); // (unsigned)
    void resetJoystickMap();
    void quit();
    void closeDevices();
    void haltServices();

private slots:
    void secondaryRefresh();

private:
    QMap<SDL_JoystickID, InputDevice*> *joysticks;
    bool sdlIsOpen;
    AntiMicroSettings *settings;
    int pollRate;
    QTimer pollRateTimer;

};

#endif // SDLEVENTREADER_H
