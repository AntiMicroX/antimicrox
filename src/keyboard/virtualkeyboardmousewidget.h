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

#include "keyboard/virtualkeypushbutton.h"
#include "keyboard/virtualmousepushbutton.h"
#include "joybutton.h"
#include "advancebuttondialog.h"

class VirtualKeyboardMouseWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit VirtualKeyboardMouseWidget(JoyButton *button, QWidget *parent = 0);
    explicit VirtualKeyboardMouseWidget(QWidget *parent = 0);
    bool isKeyboardTabVisible();

protected:
    JoyButton *button;
    QWidget *keyboardTab;
    QWidget *mouseTab;
    QLabel *mouseHorizSpeedLabel;
    QLabel *mouseVertSpeedLabel;
    QSpinBox *mouseHorizSpeedSpinBox;
    QSpinBox *mouseVertSpeedSpinBox;
    QPushButton *noneButton;
    QCheckBox *mouseChangeTogether;

    static QHash<QString, QString> topRowKeys;

    void setupVirtualKeyboardLayout();
    QVBoxLayout* setupMainKeyboardLayout();
    QVBoxLayout* setupAuxKeyboardLayout();
    QVBoxLayout* setupKeyboardNumPadLayout();

    void setupMouseControlLayout();
    VirtualKeyPushButton* createNewKey(QString xcodestring);
    QPushButton* createNoneKey();
    void populateTopRowKeys();

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
    void processSingleKeyboardSelection(int keycode);
    void processAdvancedKeyboardSelection(int keycode);
    void processSingleMouseSelection(JoyButtonSlot *tempslot);
    void processAdvancedMouseSelection(JoyButtonSlot *tempslot);
    void clearButtonSlots();
    void clearButtonSlotsFinish();
    void syncSpeedSpinBoxes();
    void moveSpeedsTogether(int value);
    void updateHorizontalSpeedConvertLabel(int value);
    void updateVerticalSpeedConvertLabel(int value);
    void setButtonMouseHorizSpeed(int value);
    void setButtonMouseVertiSpeed(int value);
};

#endif // VIRTUALKEYBOARDMOUSEWIDGET_H
