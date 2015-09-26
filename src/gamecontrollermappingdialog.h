/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
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

#ifndef GAMECONTROLLERMAPPINGDIALOG_H
#define GAMECONTROLLERMAPPINGDIALOG_H

#include <QDialog>
#include <QHash>
#include <QList>
#include <QAbstractButton>

#include "uihelpers/gamecontrollermappingdialoghelper.h"
#include "inputdevice.h"
#include "gamecontroller/gamecontroller.h"
#include "antimicrosettings.h"

namespace Ui {
class GameControllerMappingDialog;
}

class GameControllerMappingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GameControllerMappingDialog(InputDevice *device, AntiMicroSettings *settings, QWidget *parent = 0);
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
    void populateAxisDeadZoneComboBox();

    QString bindingString(SDL_GameControllerButtonBind bind);
    QList<QVariant> bindingValues(SDL_GameControllerButtonBind bind);

    InputDevice *device;
    AntiMicroSettings *settings;
    unsigned int buttonGrabs;
    QList<int> eventTriggerAxes;
    QList<int> originalAxesDeadZones;
    GameControllerMappingDialogHelper helper;
    int currentDeadZoneValue;
    bool usingGameController;

private:
    Ui::GameControllerMappingDialog *ui;

signals:
    void mappingUpdate(QString mapping, InputDevice *device);

private slots:
    void buttonAssign(int buttonindex);
    void axisAssign(int axis, int value);
    void dpadAssign(int dpad, int buttonindex);
    void buttonRelease(int buttonindex);
    void axisRelease(int axis, int value);
    void dpadRelease(int dpad, int buttonindex);
    void saveChanges();
    void discardMapping(QAbstractButton *button);
    void enableButtonEvents(int code);
    void obliterate();
    void changeButtonDisplay();
    void changeAxisDeadZone(int index);
    void updateLastAxisLineEdit(int value);
    void updateLastAxisLineEditRaw(int index, int value);
};

#endif // GAMECONTROLLERMAPPINGDIALOG_H
