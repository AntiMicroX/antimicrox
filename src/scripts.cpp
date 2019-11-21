#include "scripts.h"
#include "ui_scripts.h"

#include "inputdevice.h"


Scripts::Scripts(InputDevice *joystick, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Scripts),
    m_joystick(joystick)
{

    ui->setupUi(this);

    ui->tabWidget->setTabText(0, m_joystick->getSDLName());
}


Scripts::~Scripts()
{
    delete ui;
}
