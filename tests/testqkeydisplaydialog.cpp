#include "qkeydisplaydialog.h"

#include <QtTest/QtTest>

/*

label->text() cannot be empty
buttonBox->Close cannot be disabled
nativeTitleLabel->text() == tr("Native Key Value:")
qtKeyTitleLabel->text() == tr("Qt Key Value:")
antiTitleLabel->text() == tr("antimicroX Key Value:")
label->text() is all visible
label->text() contains <a href="http://doc.qt.io/qt-5/qt.html#Key-enum">
label->text() contains <a href="https://github.com/juliagoda/antimicroX">
nativeKeyLabel->text() at the start is 0x00000000 or empty
qtKeyLabel->text() at the start is 0x00000000 or empty
antimicroKeyLabel->text() at the start is 0x00000000 or empty



run keyReleaseEvent(QKeyEvent *event) for all keys, that are physically present:

// qnamespace.h -> enum Key { contains all adresses of keys in Qt
// keysymdef.h and XF86keysym.h contain adresses of keys in X11
// input-event-codes.h contains addresses of keys in uinput

if int virtualkey = event->nativeVirtualKey() is != 0

if it's X11
if X11Extras::getInstance()->getGroup1KeySym(virtualkey) > 0 then check out if

if it's uinput
QtKeyMapperBase *nativeKeyMapper = AntKeyMapper::getInstance()->getNativeKeyMapper();
int tempalias = nativeKeyMapper->returnQtKey(virtualkey);
if AntKeyMapper::getInstance()->returnVirtualKey(tempalias) > 0 then check out if

ui->nativeKeyLabel->text() == QString("0x%1").arg(finalvirtual, 0, 16)
ui->qtKeyLabel->text() == QString("0x%1").arg(event->key(), 0, 16)
ui->antimicroKeyLabel->text() == QString("0x%1").arg(AntKeyMapper::getInstance()->returnQtKey(finalvirtual), 0, 16)

*/

class TestQKeyDisplayDialog: public QObject
{
    Q_OBJECT

public:
    TestQKeyDisplayDialog(QObject* parent = 0);

private slots:

private:
    QKeyDisplayDialog qkeydisplaydialog;
};

TestQKeyDisplayDialog::TestQKeyDisplayDialog(QObject* parent) :
    QObject(parent),
    QKeyDisplayDialog()
{
    QTestEventLoop::instance().enterLoop(1);
}

// QTEST_MAIN(TestQKeyDisplayDialog)
#include "testqkeydisplaydialog.moc"

