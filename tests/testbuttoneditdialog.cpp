#include "buttoneditdialog.h"

#include <QtTest/QtTest>


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
#include "testbuttoneditdialog.cpp"
