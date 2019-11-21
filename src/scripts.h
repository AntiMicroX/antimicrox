#ifndef SCRIPTS_H
#define SCRIPTS_H

#include <QDialog>
#include <QHash>

#include <SDL2/SDL_joystick.h>

class InputDevice;

namespace Ui {
class Scripts;
}

class Scripts : public QDialog
{
    Q_OBJECT

public:
    explicit Scripts(QMap<SDL_JoystickID, InputDevice*>* joysticks, QWidget *parent = nullptr);
    ~Scripts();

private:

    Ui::Scripts *ui;

    QMap<SDL_JoystickID, InputDevice*>* m_joysticks;
};

#endif // SCRIPTS_H
