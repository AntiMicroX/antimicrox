#ifndef SCRIPTS_H
#define SCRIPTS_H

#include <QDialog>


class InputDevice;

namespace Ui {
class Scripts;
}

class Scripts : public QDialog
{
    Q_OBJECT

public:
    explicit Scripts(InputDevice* joystick, QWidget *parent = nullptr);
    ~Scripts();

protected:



private:

    Ui::Scripts *ui;

    InputDevice* m_joystick;
};

#endif // SCRIPTS_H
