#ifndef GAMECONTROLLERMAPPINGDIALOG_H
#define GAMECONTROLLERMAPPINGDIALOG_H

#include <QDialog>
#include <QHash>
#include <QAbstractButton>

#include "inputdevice.h"
#include "gamecontroller/gamecontroller.h"

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
    QString generateSDLMappingString();

    QString bindingString(SDL_GameControllerButtonBind bind);
    QList<QVariant> bindingValues(SDL_GameControllerButtonBind bind);

    InputDevice *device;

private:
    Ui::GameControllerMappingDialog *ui;

signals:
    void mappingUpdate(QString mapping, InputDevice *device);

private slots:
    void buttonAssign(int buttonindex);
    void axisAssign(int axis, int value);
    void dpadAssign(int dpad, int buttonindex);
    void saveChanges();
    void discardMapping(QAbstractButton *button);
    void enableButtonEvents(int code);
    void obliterate();
};

#endif // GAMECONTROLLERMAPPINGDIALOG_H
