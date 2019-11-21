#include "aboutdialog.h"

#include <QtTest/QtTest>
#include <QLabel>
#include <QTextBrowser>
#include <QPlainTextEdit>

class TestAboutDialog: public QObject
{
    Q_OBJECT

public:
    TestAboutDialog(QObject* parent = 0);

private slots:
    void testVersion();
    void infoTextBrowserEmpty();
    void changelogPlainEmpty();
    void textBrowserDevsEmpty();
    void licenseInfoEmpty();
    void creditsInfoEmpty();

private:
    AboutDialog aboutDialog;
};

TestAboutDialog::TestAboutDialog(QObject* parent) :
    QObject(parent),
    aboutDialog()
{
    QTestEventLoop::instance().enterLoop(1);
}

void TestAboutDialog::infoTextBrowserEmpty()
{
    QTextBrowser* infoTextBrowser = aboutDialog.findChild<QTextBrowser*>("infoTextBrowser");
    QVERIFY2(!infoTextBrowser->toPlainText().isEmpty(), "infoTextBrowser is empty");
}

void TestAboutDialog::changelogPlainEmpty()
{
    QPlainTextEdit* changelogPlainTextEdit = aboutDialog.findChild<QPlainTextEdit*>("changelogPlainTextEdit");
    QVERIFY2(!changelogPlainTextEdit->toPlainText().isEmpty(), "changelogPlainTextEdit is empty");
}

void TestAboutDialog::textBrowserDevsEmpty()
{
    QPlainTextEdit* developersInfo = aboutDialog.findChild<QPlainTextEdit*>("plainTextEdit");
    QVERIFY2(!developersInfo->toPlainText().isEmpty(), "plainTextEdit for info about developers is empty");
}

void TestAboutDialog::licenseInfoEmpty()
{
    QTextBrowser* licenseInfo = aboutDialog.findChild<QTextBrowser*>("textBrowser_2");
    QVERIFY2(!licenseInfo->toPlainText().isEmpty(), "textBrowser_2 about license information is empty");
}

void TestAboutDialog::creditsInfoEmpty()
{
    QTextBrowser* creditsInfo = aboutDialog.findChild<QTextBrowser*>("textBrowser");
    QVERIFY2(!creditsInfo->toPlainText().isEmpty(), "textBrowser about credits is empty");
}

void TestAboutDialog::testVersion()
{
    QLabel* versionLabel = aboutDialog.findChild<QLabel*>("versionLabel");
    QCOMPARE(versionLabel->text(), QString("2.24.2"));
}

QTEST_MAIN(TestAboutDialog)
#include "testaboutdialog.moc"
