// SPDX-License-Identifier: GPL-3.0-or-later

#include "profilemigration.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

class TestProfileMigration : public QObject
{
    Q_OBJECT

  private slots:
    void init();
    void completeSource_data();
    void completeSource();
    void invalidSource_data();
    void invalidSource();
    void invalidConversion_data();
    void invalidConversion();
    void unicodeSave_data();
    void unicodeSave();
    void newProfile();
    void missingDirectory();
    void existingDirectory();
    void retryClearsError();

  private:
    QByteArray readFile();
    void seedProfile();
    QTemporaryDir m_directory;
    QString m_path;
};

void TestProfileMigration::init()
{
    QVERIFY(m_directory.isValid());
    const QString folder = m_directory.filePath(QString::fromLatin1(QTest::currentTestFunction()) +
                                                QString::fromLatin1(QTest::currentDataTag()));
    QVERIFY(QDir().mkpath(folder));
    m_path = folder + "/profile.amgp";
}

QByteArray TestProfileMigration::readFile()
{
    QFile file(m_path);
    if (!file.open(QIODevice::ReadOnly))
        return QByteArray();
    return file.readAll();
}

void TestProfileMigration::seedProfile()
{
    QFile file(m_path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QCOMPARE(file.write("original profile"), qint64(16));
}

void TestProfileMigration::completeSource_data()
{
    QTest::addColumn<QString>("source");
    QTest::newRow("empty-legacy") << QString("<joystick configversion=\"2\"/>");
    QTest::newRow("nested") << QString("<joystick configversion=\"2\"><sets><set index=\"1\"/></sets></joystick>");
    QTest::newRow("unicode") << QString::fromUtf8(
        "<joystick configversion=\"2\"><profilename>Caf\xc3\xa9</profilename></joystick>");
}

void TestProfileMigration::completeSource()
{
    QFETCH(QString, source);
    QXmlStreamReader reader(source);
    QVERIFY(reader.readNextStartElement());
    const QString copied = ProfileMigration::readCompleteXml(reader);
    QVERIFY2(!reader.hasError(), qPrintable(reader.errorString()));
    QVERIFY(!copied.isEmpty());
    QXmlStreamReader check(copied);
    QVERIFY(check.readNextStartElement());
    QCOMPARE(check.name().toString(), QString("joystick"));
    QCOMPARE(check.attributes().value("configversion").toString(), QString("2"));
    while (!check.atEnd())
        check.readNext();
    QVERIFY2(!check.hasError(), qPrintable(check.errorString()));
    if (source.contains(QString::fromUtf8("Caf\xc3\xa9")))
        QVERIFY(copied.contains(QString::fromUtf8("Caf\xc3\xa9")));
}

void TestProfileMigration::invalidSource_data()
{
    QTest::addColumn<QString>("source");
    QTest::newRow("truncated") << QString("<joystick configversion=\"2\"><sets>");
    QTest::newRow("mismatch") << QString("<joystick configversion=\"2\"></sets>");
    QTest::newRow("extra-root") << QString("<joystick configversion=\"2\"/><joystick/>");
    QTest::newRow("unknown-entity") << QString("<joystick configversion=\"2\">&missing;</joystick>");
}

void TestProfileMigration::invalidSource()
{
    QFETCH(QString, source);
    QXmlStreamReader reader(source);
    QVERIFY(reader.readNextStartElement());
    QVERIFY(ProfileMigration::readCompleteXml(reader).isEmpty());
    QVERIFY(reader.hasError());
    // The reader's historical premature-end workaround must not hide migration errors.
    QCOMPARE(reader.error(), QXmlStreamReader::CustomError);
    QVERIFY(!reader.errorString().isEmpty());
}

void TestProfileMigration::invalidConversion_data()
{
    QTest::addColumn<QString>("converted");
    QTest::newRow("empty") << QString();
    QTest::newRow("truncated") << QString("<joystick>");
    QTest::newRow("mismatch") << QString("<joystick></sets>");
    QTest::newRow("extra-root") << QString("<joystick/><joystick/>");
    QTest::newRow("unsupported-encoding") << QString("<?xml version=\"1.0\" encoding=\"invalid-encoding\"?><joystick/>");
}

void TestProfileMigration::invalidConversion()
{
    QFETCH(QString, converted);
    seedProfile();
    const QDir folder(QFileInfo(m_path).absolutePath());
    const QStringList before = folder.entryList(QDir::AllEntries | QDir::Hidden);
    QString error;
    QVERIFY(!ProfileMigration::save(m_path, converted, error));
    QVERIFY(!error.isEmpty());
    QCOMPARE(readFile(), QByteArray("original profile"));
    QCOMPARE(folder.entryList(QDir::AllEntries | QDir::Hidden), before);
}

void TestProfileMigration::unicodeSave_data()
{
    QTest::addColumn<QString>("name");
    QTest::newRow("accent") << QString::fromUtf8("Caf\xc3\xa9");
    QTest::newRow("cjk") << QString::fromUtf8("\xe6\xbc\xa2\xe5\xad\x97");
    QTest::newRow("emoji") << QString::fromUtf8("Game \xf0\x9f\x8e\xae");
}

void TestProfileMigration::unicodeSave()
{
    QFETCH(QString, name);
    seedProfile();
    const QString contents = QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                     "<joystick configversion=\"6\"><profilename>%1</profilename></joystick>")
                                 .arg(name);
    QString error;
    QVERIFY2(ProfileMigration::save(m_path, contents, error), qPrintable(error));
    QVERIFY(error.isEmpty());
    QCOMPARE(readFile(), contents.toUtf8());
    QXmlStreamReader check(readFile());
    QVERIFY(check.readNextStartElement());
    QVERIFY(check.readNextStartElement());
    QCOMPARE(check.readElementText(), name);
}

void TestProfileMigration::newProfile()
{
    QString error;
    QVERIFY2(ProfileMigration::save(m_path, "<joystick/>", error), qPrintable(error));
    QCOMPARE(readFile(), QByteArray("<joystick/>"));
}

void TestProfileMigration::missingDirectory()
{
    QString error;
    QVERIFY(!ProfileMigration::save(m_path + "/missing/profile.amgp", "<joystick/>", error));
    QVERIFY(!error.isEmpty());
    QVERIFY(!QFileInfo::exists(m_path));
}

void TestProfileMigration::existingDirectory()
{
    QVERIFY(QDir().mkdir(m_path));
    QString error;
    QVERIFY(!ProfileMigration::save(m_path, "<joystick/>", error));
    QVERIFY(!error.isEmpty());
    QVERIFY(QFileInfo(m_path).isDir());
}

void TestProfileMigration::retryClearsError()
{
    QString error;
    QVERIFY(!ProfileMigration::save(m_path, "<joystick>", error));
    QVERIFY(!error.isEmpty());
    QVERIFY2(ProfileMigration::save(m_path, "<joystick/>", error), qPrintable(error));
    QVERIFY(error.isEmpty());
}

QTEST_GUILESS_MAIN(TestProfileMigration)
#include "testprofilemigration.moc"
