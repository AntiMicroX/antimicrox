// SPDX-License-Identifier: GPL-3.0-or-later

#include "startupshortcut.h"

#include <QDir>
#include <QTemporaryDir>
#include <QtTest>

class TestStartupShortcut : public QObject
{
    Q_OBJECT

  private slots:
    void init();
    void enableCreatesShortcut();
    void repeatedEnablePreservesShortcut();
    void existingCustomShortcutIsPreserved();
    void disableRemovesOnlyShortcut();
    void repeatedDisableSucceeds();
    void emptyPlaceholderIsRepaired();
    void nonemptyFileIsPreserved();
    void directoryIsPreserved();
    void missingDirectoryFailsCleanly();
    void missingApplicationPreservesPlaceholder();

  private:
    void writeFile(const QString &path, const QByteArray &contents);
    QByteArray contents(const QString &path);
    QTemporaryDir m_directory;
    QString m_application;
    QString m_shortcut;
};

void TestStartupShortcut::writeFile(const QString &path, const QByteArray &data)
{
    QFile file(path);
    QVERIFY2(file.open(QIODevice::WriteOnly), qPrintable(file.errorString()));
    QCOMPARE(file.write(data), qint64(data.size()));
}

QByteArray TestStartupShortcut::contents(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return QByteArray();
    return file.readAll();
}

void TestStartupShortcut::init()
{
    QVERIFY(m_directory.isValid());
    // Each case gets a separate folder; no real Startup folder is ever accessed.
    const QString folder = m_directory.filePath(QString::fromLatin1(QTest::currentTestFunction()));
    QVERIFY(QDir().mkpath(folder));
    m_application = folder + QString::fromUtf8("/Anti Micro X \xc3\xa9.exe");
    m_shortcut = folder + "/antimicrox.lnk";
    writeFile(m_application, "application contents");
}

void TestStartupShortcut::enableCreatesShortcut()
{
    QVERIFY(StartupShortcut::setEnabled(m_application, m_shortcut, true));
    QVERIFY(QFileInfo(m_shortcut).isSymLink());
    QCOMPARE(QFileInfo(m_shortcut).symLinkTarget(), QFileInfo(m_application).absoluteFilePath());
}

void TestStartupShortcut::repeatedEnablePreservesShortcut()
{
    QVERIFY(StartupShortcut::setEnabled(m_application, m_shortcut, true));
    const QByteArray original = contents(m_shortcut);
    for (int i = 0; i < 3; i++)
    {
        QVERIFY(StartupShortcut::setEnabled(m_application, m_shortcut, true));
        QVERIFY(QFileInfo(m_shortcut).isSymLink());
        QCOMPARE(contents(m_shortcut), original);
    }
}

void TestStartupShortcut::existingCustomShortcutIsPreserved()
{
    const QString customTarget = m_directory.filePath("custom.exe");
    writeFile(customTarget, "custom application");
    QVERIFY(QFile::link(customTarget, m_shortcut));
    const QByteArray original = contents(m_shortcut);
    QVERIFY(StartupShortcut::setEnabled(m_application, m_shortcut, true));
    QCOMPARE(QFileInfo(m_shortcut).symLinkTarget(), QFileInfo(customTarget).absoluteFilePath());
    QCOMPARE(contents(m_shortcut), original);
}

void TestStartupShortcut::disableRemovesOnlyShortcut()
{
    QVERIFY(QFile::link(m_application, m_shortcut));
    QVERIFY(StartupShortcut::setEnabled(m_application, m_shortcut, false));
    QVERIFY(!QFileInfo(m_shortcut).exists());
    QVERIFY(!QFileInfo(m_shortcut).isSymLink());
    QCOMPARE(contents(m_application), QByteArray("application contents"));
}

void TestStartupShortcut::repeatedDisableSucceeds()
{
    for (int i = 0; i < 3; i++)
        QVERIFY(StartupShortcut::setEnabled(m_application, m_shortcut, false));
    QVERIFY(!QFileInfo(m_shortcut).exists());
}

void TestStartupShortcut::emptyPlaceholderIsRepaired()
{
    writeFile(m_shortcut, "");
    QVERIFY(StartupShortcut::setEnabled(m_application, m_shortcut, true));
    QVERIFY(QFileInfo(m_shortcut).isSymLink());
    QCOMPARE(QFileInfo(m_shortcut).symLinkTarget(), QFileInfo(m_application).absoluteFilePath());
}

void TestStartupShortcut::nonemptyFileIsPreserved()
{
    writeFile(m_shortcut, "unrecognized content");
    QVERIFY(!StartupShortcut::setEnabled(m_application, m_shortcut, true));
    QCOMPARE(contents(m_shortcut), QByteArray("unrecognized content"));
}

void TestStartupShortcut::directoryIsPreserved()
{
    QVERIFY(QDir().mkdir(m_shortcut));
    QVERIFY(!StartupShortcut::setEnabled(m_application, m_shortcut, true));
    QVERIFY(QFileInfo(m_shortcut).isDir());
    QVERIFY(!StartupShortcut::setEnabled(m_application, m_shortcut, false));
    QVERIFY(QFileInfo(m_shortcut).isDir());
}

void TestStartupShortcut::missingDirectoryFailsCleanly()
{
    const QString path = m_directory.filePath("missing/startup/antimicrox.lnk");
    QVERIFY(!StartupShortcut::setEnabled(m_application, path, true));
    QVERIFY(!QFileInfo(path).exists());
}

void TestStartupShortcut::missingApplicationPreservesPlaceholder()
{
    writeFile(m_shortcut, "");
    QVERIFY(!StartupShortcut::setEnabled(m_directory.filePath("missing.exe"), m_shortcut, true));
    QVERIFY(QFileInfo(m_shortcut).isFile());
    QCOMPARE(QFileInfo(m_shortcut).size(), qint64(0));
}

QTEST_GUILESS_MAIN(TestStartupShortcut)
#include "teststartupshortcut.moc"
