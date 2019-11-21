#include "axiseditdialog.h"

#include <QtTest/QtTest>


class TestAxisEditDialog: public QObject
{
    Q_OBJECT

public:
    TestAxisEditDialog(QObject* parent = 0);

private slots:

private:
    AxisEditDialog axisEditDialog;
};

TestAxisEditDialog::TestAxisEditDialog(QObject* parent) :
    QObject(parent),
    axisEditDialog()
{
    QTestEventLoop::instance().enterLoop(1);
}

// QTEST_MAIN(TestAxisEditDialog)
#include "testaxiseditdialog.moc"


