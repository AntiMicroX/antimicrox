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
    void selectionMade(int keycode);
    void selectionMade(JoyButtonSlot *slot);

public slots:
    void establishVirtualKeyboardSingleSignalConnections();
    void establishVirtualMouseSignalConnections();
    void establishVirtualKeyboardAdvancedSignalConnections();
    void establishVirtualMouseAdvancedSignalConnections();

private slots:
    void processSingleKeyboardSelection(int keycode, unsigned int alias);
    void processAdvancedKeyboardSelection(int keycode);
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
