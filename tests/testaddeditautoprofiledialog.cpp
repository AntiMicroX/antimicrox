#include "addeditautoprofiledialog.h"

#include <QtTest/QtTest>


class TestAddEditAutoProfileDialog: public QObject
{
    Q_OBJECT

public:
    TestAddEditAutoProfileDialog(QObject* parent = 0);

private slots:

private:
    AddEditAutoProfileDialog addEditAutoProfileDialog;
};

TestAddEditAutoProfileDialog::TestAddEditAutoProfileDialog(QObject* parent) :
    QObject(parent),
    addEditAutoProfileDialog()
{
    QTestEventLoop::instance().enterLoop(1);
}

// QTEST_MAIN(TestAddEditAutoProfileDialog)
#include "testaddeditautoprofiledialog.moc"
