#ifndef GAMECONTROLLERMAPPINGDIALOG_H
#define GAMECONTROLLERMAPPINGDIALOG_H

#include <QDialog>
#include <QHash>
#include <QAbstractButton>

#include "inputdevice.h"
#include "gamecontroller.h"

namespace Ui {
class GameControllerMappingDialog;
}

class GameControllerMappingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GameControllerMappingDialog(InputDevice *device, QWidget *parent = 0);
    ~GameControllerMappingDialog();

    static QHash<int, QString> tempaliases;
    static QHash<SDL_GameControllerButton, int> buttonPlacement;
    static QHash<SDL_GameControllerAxis, int> axisPlacement;

protected:
    void populateGameControllerBindings(GameController *controller);
    void removeControllerMapping();
    void enableDeviceConnections();
    void disableDeviceConnections();

    QString bindingString(SDL_GameControllerButtonBind bind);
    QList<QVariant> bindingValues(SDL_GameControllerButtonBind bind);

    InputDevice *device;

private:
    Ui::GameControllerMappingDialog *ui;

signals:
    void mappingUpdate(QString mapping, InputDevice *device);

private slots:
    void testButtonAssign(int buttonindex);
    void testAxisAssign(int axis, int value);
    void testDPadAssign(int dpad, int buttonindex);
    void testsave();
    void testOther(QAbstractButton *button);
};

#endif // GAMECONTROLLERMAPPINGDIALOG_H
