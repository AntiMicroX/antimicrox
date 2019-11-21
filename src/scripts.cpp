#include "scripts.h"
#include "ui_scripts.h"


Scripts::Scripts(QMap<SDL_JoystickID, InputDevice *> *joysticks, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Scripts),
    m_joysticks(joysticks)
{
    ui->setupUi(this);

}

Scripts::~Scripts()
{
    delete ui;
}
