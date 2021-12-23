/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
 * Copyright (C) 2020 Paweł Kotiuk <kotiuk@zohomail.eu>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    QPlainTextEdit* changelogText = aboutDialog.findChild<QPlainTextEdit*>("changelogText");
    QVERIFY2(!changelogText->toPlainText().isEmpty(), "changelogText is empty");
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
    QCOMPARE(versionLabel->text(), QString("2.25"));
}

QTEST_MAIN(TestAboutDialog)
#include "testaboutdialog.moc"
