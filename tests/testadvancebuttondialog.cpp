#include "advancebuttondialog.h"

#include <QtTest/QtTest>


class TestAdvanceButtonDialog: public QObject
{
    Q_OBJECT

public:
    TestAdvanceButtonDialog(QObject* parent = 0);

private slots:

private:
    AdvanceButtonDialog advanceButtonDialog;
};

TestAdvanceButtonDialog::TestAdvanceButtonDialog(QObject* parent) :
    QObject(parent),
    advanceButtonDialog()
{
    QTestEventLoop::instance().enterLoop(1);
}

// QTEST_MAIN(TestAdvanceButtonDialog)
#include "testadvancebuttondialog.moc"
