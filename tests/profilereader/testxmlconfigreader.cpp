// SPDX-License-Identifier: GPL-3.0-or-later
#include "globalvariables.h"
#include "xml/inputdevicexml.h"
#include "xmlconfigmigration.h"
#include "xmlconfigreader.h"

#include <QDir>
#include <QFile>
#include <QPointer>
#include <QTemporaryDir>
#include <QXmlStreamReader>
#include <QtTest>

// Compile the production reader. Controlled collaborators isolate file selection,
// opening, errors and retries from SDL/device parsing and legacy migration.
class InputDevice
{
  public:
    QString profileName;
    int reads = 0;
};
const QString GlobalVariables::Joystick::xmlName = "joystick";
const QString GlobalVariables::GameController::xmlName = "gamecontroller";
InputDeviceXml::InputDeviceXml(InputDevice *device, QObject *parent)
    : QObject(parent)
    , m_inputDevice(device)
{
}
void InputDeviceXml::writeConfig(QXmlStreamWriter *) { QFAIL("Unexpected write"); }
void InputDeviceXml::readConfig(QXmlStreamReader *xml)
{
    ++m_inputDevice->reads;
    while (xml->readNextStartElement())
    {
        if (xml->name() == QStringLiteral("profilename"))
            m_inputDevice->profileName = xml->readElementText();
        else
            xml->skipCurrentElement();
    }
}
XMLConfigMigration::XMLConfigMigration(QXmlStreamReader *xml, QObject *parent)
    : QObject(parent)
    , reader(xml)
    , fileVersion(6)
{
}
bool XMLConfigMigration::requiresMigration() { return false; }
QString XMLConfigMigration::migrate()
{
    QTest::qFail("Migration is outside these file-access tests", __FILE__, __LINE__);
    return {};
}

class TestXmlConfigReader : public QObject
{
    Q_OBJECT
  private slots:
    void init();
    void cleanup();
    void invalidSetup_data();
    void invalidSetup();
    void validProfile_data();
    void validProfile();
    void deletedAfterSelection();
    void directoryIsNotAProfile();
    void retryAfterFileCreation();
    void retryAfterInvalidRoot();
    void switchingToMissingFileNeverReloadsPreviousProfile();
    void switchingToExistingFile();
    void repeatedReadStartsAtBeginning();
    void changingFilenameReleasesPreviousFile();

  private:
    void writeProfile(const QString &path,
                      const QByteArray &xml = "<joystick configversion=\"6\"><profilename>Loaded</profilename></joystick>");
    void verifyFailure(XMLConfigReader &reader);
    QTemporaryDir m_directory;
    QString m_path;
    InputDevice m_device;
};
void TestXmlConfigReader::init()
{
    QVERIFY(m_directory.isValid());
    m_path = m_directory.filePath(QString::fromLatin1(QTest::currentTestFunction()) +
                                  QString::fromLatin1(QTest::currentDataTag()) + ".amgp");
    m_device.profileName = "Before";
    m_device.reads = 0;
}
void TestXmlConfigReader::cleanup() { QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete); }
void TestXmlConfigReader::writeProfile(const QString &path, const QByteArray &xml)
{
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QCOMPARE(file.write(xml), qint64(xml.size()));
}
void TestXmlConfigReader::verifyFailure(XMLConfigReader &reader)
{
    QVERIFY(reader.read()); // Historical API returns true on failure.
    QVERIFY(reader.hasError());
    QVERIFY(!reader.getErrorString().isEmpty());
    QCOMPARE(m_device.profileName, QString("Before"));
    QCOMPARE(m_device.reads, 0);
}
void TestXmlConfigReader::invalidSetup_data()
{
    QTest::addColumn<QString>("kind");
    for (const char *kind : {"unset", "empty", "missing", "null-device"})
        QTest::newRow(kind) << QString::fromLatin1(kind);
}
void TestXmlConfigReader::invalidSetup()
{
    QFETCH(QString, kind);
    XMLConfigReader reader;
    reader.setJoystick(&m_device);
    if (kind == "empty")
        reader.setFileName("");
    else if (kind != "unset")
        reader.setFileName(m_path);
    if (kind == "null-device")
    {
        writeProfile(m_path);
        reader.setJoystick(nullptr);
    }
    verifyFailure(reader);
}
void TestXmlConfigReader::validProfile_data()
{
    QTest::addColumn<QByteArray>("root");
    QTest::newRow("joystick") << QByteArray("joystick");
    QTest::newRow("gamecontroller") << QByteArray("gamecontroller");
}
void TestXmlConfigReader::validProfile()
{
    QFETCH(QByteArray, root);
    writeProfile(m_path, "<" + root + " configversion=\"6\"><profilename>Loaded</profilename></" + root + ">");
    XMLConfigReader reader;
    reader.setJoystick(&m_device);
    reader.setFileName(m_path);
    QVERIFY(!reader.read());
    QVERIFY(!reader.hasError());
    QCOMPARE(m_device.profileName, QString("Loaded"));
    QCOMPARE(m_device.reads, 1);
    QVERIFY(!reader.getConfigFile()->isOpen());
}
void TestXmlConfigReader::deletedAfterSelection()
{
    writeProfile(m_path);
    XMLConfigReader reader;
    reader.setJoystick(&m_device);
    reader.setFileName(m_path);
    QVERIFY(QFile::remove(m_path));
    verifyFailure(reader);
}
void TestXmlConfigReader::directoryIsNotAProfile()
{
    QVERIFY(QDir().mkpath(m_path));
    XMLConfigReader reader;
    reader.setJoystick(&m_device);
    reader.setFileName(m_path);
    verifyFailure(reader);
    QVERIFY(!reader.getConfigFile()->isOpen());
}
void TestXmlConfigReader::retryAfterFileCreation()
{
    XMLConfigReader reader;
    reader.setJoystick(&m_device);
    reader.setFileName(m_path);
    verifyFailure(reader);
    writeProfile(m_path);
    QVERIFY(!reader.read());
    QVERIFY(!reader.hasError());
    QVERIFY(reader.getErrorString().isEmpty());
    QCOMPARE(m_device.profileName, QString("Loaded"));
}
void TestXmlConfigReader::retryAfterInvalidRoot()
{
    writeProfile(m_path, "<invalid/>");
    XMLConfigReader reader;
    reader.setJoystick(&m_device);
    reader.setFileName(m_path);
    verifyFailure(reader);
    writeProfile(m_path);
    QVERIFY(!reader.read());
    QVERIFY(!reader.hasError());
    QVERIFY(reader.getErrorString().isEmpty());
    QCOMPARE(m_device.profileName, QString("Loaded"));
}
void TestXmlConfigReader::switchingToMissingFileNeverReloadsPreviousProfile()
{
    writeProfile(m_path);
    XMLConfigReader reader;
    reader.setJoystick(&m_device);
    reader.setFileName(m_path);
    QVERIFY(!reader.read());
    m_device.profileName = "Before";
    m_device.reads = 0;
    reader.setFileName(m_path + ".missing");
    verifyFailure(reader);
}
void TestXmlConfigReader::switchingToExistingFile()
{
    writeProfile(m_path);
    XMLConfigReader reader;
    reader.setJoystick(&m_device);
    reader.setFileName(m_path);
    QVERIFY(!reader.read());
    const QString second = m_path + ".second";
    writeProfile(second, "<joystick configversion=\"6\"><profilename>Second</profilename></joystick>");
    reader.setFileName(second);
    QCOMPARE(reader.getFileName(), second);
    QVERIFY(!reader.read());
    QCOMPARE(m_device.profileName, QString("Second"));
}
void TestXmlConfigReader::repeatedReadStartsAtBeginning()
{
    writeProfile(m_path);
    XMLConfigReader reader;
    reader.setJoystick(&m_device);
    reader.setFileName(m_path);
    QVERIFY(!reader.read());
    m_device.profileName = "Before";
    QVERIFY(!reader.read());
    QCOMPARE(m_device.profileName, QString("Loaded"));
    QCOMPARE(m_device.reads, 2);
}
void TestXmlConfigReader::changingFilenameReleasesPreviousFile()
{
    writeProfile(m_path);
    XMLConfigReader reader;
    reader.setFileName(m_path);
    QPointer<const QFile> previous(reader.getConfigFile());
    reader.setFileName(m_path + ".missing");
    QVERIFY(previous.isNull());
    QCOMPARE(reader.getFileName(), m_path + ".missing");
    QCOMPARE(reader.getConfigFile()->fileName(), m_path + ".missing");
}
QTEST_GUILESS_MAIN(TestXmlConfigReader)
#include "testxmlconfigreader.moc"
