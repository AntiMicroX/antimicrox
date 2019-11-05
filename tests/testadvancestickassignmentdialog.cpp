#include "advancestickassignmentdialog.h"

#include <QtTest/QtTest>


class TestAdvanceStickAssignmentDialog: public QObject
{
    Q_OBJECT

public:
    TestAdvanceStickAssignmentDialog(QObject* parent = 0);

private slots:

private:
    AdvanceStickAssignmentDialog advanceStickAssignmentDialog;
};

TestAdvanceStickAssignmentDialog::TestAdvanceStickAssignmentDialog(QObject* parent) :
    QObject(parent),
    advanceStickAssignmentDialog()
{
    QTestEventLoop::instance().enterLoop(1);
}

// QTEST_MAIN(TestAdvanceButtonDialog)
#include "testadvancestickassignmentdialog.moc"

