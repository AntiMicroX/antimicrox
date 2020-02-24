#include "buttoneditdialog.h"

#include <QtTest/QtTest>

/*
if slotSummaryLabel->text() == tr("[NO KEY]") and lastJoyButton is nullptr lastJoyButton->getSlotsString().replace("&", "&&") == tr("[NO KEY]") then advancedPushButton should be disabled

if slotSummaryLabel->text() != tr("[NO KEY]") and lastJoyButton is not nullptr and lastJoyButton->getSlotsString().replace("&", "&&") != tr("[NO KEY]") then
advancedPushButton should be enabled

toggleCheckBox should be checked if lastJoyButton->getToggleState()

turboCheckBox should be checked if lastJoyButton->isUsingTurbo()

buttonNameLineEdit cannot be disabled

actionNameLineEdit cannot be disabled

buttonBox->Cancel cannot be disabled

virtualKeyMouseTabWidget->tab(0)->->findChildren<VirtualKeyPushButton*> ()
check if each
QString getXcodestring() const; != ""
QString getDisplayString() const; != tr("[NO KEY]")
int getKeycode() const; != 0
int getQkeyalias() const; != 0
only if it's not a none button and otherKeysPushbutton

QList<VirtualMousePushButton*> newlist = mouseTab->findChildren<VirtualMousePushButton*>();
QListIterator<VirtualMousePushButton*> iter(newlist);

while (iter.hasNext())
{
   VirtualMousePushButton *mousebutton = iter.next();
   // test if getMouseCode() is not null or less
}

getOtherKeysMenu()-actions()
if AntKeyMapper::getInstance()->returnQtKey(oneFromListAction) != 0
*/


class TestButtonEditDialog: public QObject
{
    Q_OBJECT

public:
    TestButtonEditDialog(QObject* parent = 0);

private slots:

private:
    ButtonEditDialog buttonEditDialog;
};

TestButtonEditDialog::TestButtonEditDialog(QObject* parent) :
    QObject(parent),
    buttonEditDialog()
{
    QTestEventLoop::instance().enterLoop(1);
}

// QTEST_MAIN(TestButtonEditDialog)
#include "testbuttoneditdialog.moc"
