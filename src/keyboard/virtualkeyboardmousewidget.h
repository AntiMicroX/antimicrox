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

#ifndef VIRTUALKEYBOARDMOUSEWIDGET_H
#define VIRTUALKEYBOARDMOUSEWIDGET_H

#include <QObject>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHash>
#include <QString>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QMenu>
#include <QAction>

#include "virtualkeypushbutton.h"
#include "virtualmousepushbutton.h"
#include <joybutton.h>
#include <advancebuttondialog.h>

class VirtualKeyboardMouseWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit VirtualKeyboardMouseWidget(JoyButton *button, QWidget *parent = 0);
    explicit VirtualKeyboardMouseWidget(QWidget *parent = 0);
    bool isKeyboardTabVisible();

protected:
    void setupVirtualKeyboardLayout();
    QVBoxLayout* setupMainKeyboardLayout();
    QVBoxLayout* setupAuxKeyboardLayout();
    QVBoxLayout* setupKeyboardNumPadLayout();

    void setupMouseControlLayout();
    VirtualKeyPushButton* createNewKey(QString xcodestring);
    QPushButton* createNoneKey();
    void populateTopRowKeys();
    QPushButton* createOtherKeysMenu();

    virtual void resizeEvent(QResizeEvent *event);

    JoyButton *button;
    QWidget *keyboardTab;
    QWidget *mouseTab;
    //QLabel *mouseHorizSpeedLabel;
    //QLabel *mouseVertSpeedLabel;
    //QSpinBox *mouseHorizSpeedSpinBox;
    //QSpinBox *mouseVertSpeedSpinBox;
    QPushButton *noneButton;
    QPushButton *mouseSettingsPushButton;
    //QCheckBox *mouseChangeTogether;
    //QComboBox *mouseModeComboBox;
    QMenu *otherKeysMenu;

    static QHash<QString, QString> topRowKeys;

signals:
    void selectionFinished();
    void selectionCleared();
    void selectionMade(int keycode, unsigned int alias);
    void selectionMade(JoyButtonSlot *slot);

public slots:
    void establishVirtualKeyboardSingleSignalConnections();
    void establishVirtualMouseSignalConnections();
    void establishVirtualKeyboardAdvancedSignalConnections();
    void establishVirtualMouseAdvancedSignalConnections();

private slots:
    void processSingleKeyboardSelection(int keycode, unsigned int alias);
    void processAdvancedKeyboardSelection(int keycode, unsigned int alias);
    void processSingleMouseSelection(JoyButtonSlot *tempslot);
    void processAdvancedMouseSelection(JoyButtonSlot *tempslot);
    void clearButtonSlots();
    void clearButtonSlotsFinish();
    void openMouseSettingsDialog();
    void enableMouseSettingButton();
    void setButtonFontSizes();
    void otherKeysActionSingle(bool triggered);
    void otherKeysActionAdvanced(bool triggered);
};

#endif // VIRTUALKEYBOARDMOUSEWIDGET_H
