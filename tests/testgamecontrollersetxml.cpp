// SPDX-License-Identifier: GPL-3.0-or-later

#include "antimicrosettings.h"
#include "gamecontroller/gamecontroller.h"
#include "gamecontroller/gamecontrollerset.h"
#include "joyaxis.h"
#include "joybuttontypes/joybutton.h"
#include "logger.h"
#include "vdpad.h"
#include "xml/joyaxisxml.h"
#include "xml/joybuttonxml.h"

#include <QStandardPaths>
#include <QTemporaryDir>
#include <QXmlStreamReader>
#include <QtTest>

#include <memory>

#if !SDL_VERSION_ATLEAST(2, 0, 14)
    #error "Profile XML tests require SDL 2.0.14 or newer for virtual joysticks"
#endif

class TestGameControllerSetXml : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    void invalidElements_data();
    void invalidElements();
    void triggerAliases_data();
    void triggerAliases();
    void validDPad();
    void reloadDoesNotRetainParsers();

  private:
    void readSet(const QString &contents);

    QTemporaryDir m_directory;
    std::unique_ptr<AntiMicroSettings> m_settings;
    std::unique_ptr<GameController> m_device;
    SDL_GameController *m_controller = nullptr;
    int m_virtual_index = -1;
};

void TestGameControllerSetXml::initTestCase()
{
    QVERIFY(m_directory.isValid());
    QCoreApplication::setOrganizationName("AntiMicroXTests");
    QCoreApplication::setApplicationName("ProfileXmlTests");
    QStandardPaths::setTestModeEnabled(true);
    Logger::createInstance(nullptr, Logger::LOG_NONE);
    QCOMPARE(SDL_Init(SDL_INIT_GAMECONTROLLER), 0);
    m_settings = std::make_unique<AntiMicroSettings>(m_directory.filePath("settings.ini"), QSettings::IniFormat);
}

void TestGameControllerSetXml::cleanupTestCase()
{
    m_settings.reset();
    SDL_Quit();
    delete Logger::getInstance();
}

void TestGameControllerSetXml::init()
{
    m_virtual_index =
        SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER, SDL_CONTROLLER_AXIS_MAX, SDL_CONTROLLER_BUTTON_MAX, 0);
    QVERIFY2(m_virtual_index >= 0, SDL_GetError());
    m_controller = SDL_GameControllerOpen(m_virtual_index);
    QVERIFY2(m_controller != nullptr, SDL_GetError());
    m_device = std::make_unique<GameController>(m_controller, m_virtual_index, m_settings.get(), 0, nullptr);
}

void TestGameControllerSetXml::cleanup()
{
    m_device.reset();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    if (m_controller != nullptr)
        SDL_GameControllerClose(m_controller);
    m_controller = nullptr;
    if (m_virtual_index >= 0)
        QCOMPARE(SDL_JoystickDetachVirtual(m_virtual_index), 0);
    m_virtual_index = -1;
}

void TestGameControllerSetXml::readSet(const QString &contents)
{
    QXmlStreamReader xml(QStringLiteral("<set index=\"1\">") + contents + QStringLiteral("</set>"));
    QVERIFY(xml.readNextStartElement());
    m_device->getActiveSetJoystick()->readConfig(&xml);
    QVERIFY2(!xml.hasError(), qPrintable(xml.errorString()));
    QCOMPARE(xml.tokenType(), QXmlStreamReader::EndElement);
    QCOMPARE(xml.name().toString(), QStringLiteral("set"));
}

void TestGameControllerSetXml::invalidElements_data()
{
    QTest::addColumn<QString>("element");
    QTest::addColumn<QString>("index");
    const QStringList elements = {"button", "dpad", "trigger", "stick"};
    const QStringList indices = {"", "0", "-1", "-2147483648", "2147483647", "2147483648", "abc", "999"};
    for (const QString &element : elements)
    {
        for (const QString &index : indices)
        {
            const QByteArray row = (element + ":" + index).toUtf8();
            QTest::newRow(row.constData()) << element << index;
        }
    }
    QTest::newRow("unsupported-trigger-3") << QString("trigger") << QString("3");
    QTest::newRow("unsupported-trigger-4") << QString("trigger") << QString("4");
}

void TestGameControllerSetXml::invalidElements()
{
    QFETCH(QString, element);
    QFETCH(QString, index);
    // Nested names must not escape a rejected element; subsequent valid mappings must still load.
    readSet(QStringLiteral("<name>Original</name><%1 index=\"%2\"><name>Ignored</name>"
                           "<button index=\"2\"><toggle>true</toggle></button></%1>"
                           "<button index=\"1\"><toggle>true</toggle></button>")
                .arg(element, index));
    auto *set = m_device->getActiveSetJoystick();
    QCOMPARE(set->getName(), QStringLiteral("Original"));
    QVERIFY(set->getJoyButton(0)->getToggleState());
    QVERIFY(!set->getJoyButton(1)->getToggleState());
}

void TestGameControllerSetXml::triggerAliases_data()
{
    QTest::addColumn<int>("index");
    QTest::addColumn<int>("axis");
    QTest::newRow("legacy-left") << 1 << int(SDL_CONTROLLER_AXIS_TRIGGERLEFT);
    QTest::newRow("legacy-right") << 2 << int(SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
    QTest::newRow("left") << 5 << int(SDL_CONTROLLER_AXIS_TRIGGERLEFT);
    QTest::newRow("right") << 6 << int(SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
}

void TestGameControllerSetXml::triggerAliases()
{
    QFETCH(int, index);
    QFETCH(int, axis);
    readSet(QStringLiteral("<trigger index=\"%1\"><deadZone>4321</deadZone></trigger>"
                           "<button index=\"1\"><toggle>true</toggle></button>")
                .arg(index));
    auto *set = m_device->getActiveSetJoystick();
    QCOMPARE(set->getJoyAxis(axis)->getDeadZone(), 4321);
    QVERIFY(set->getJoyButton(0)->getToggleState());
}

void TestGameControllerSetXml::validDPad()
{
    readSet(QStringLiteral("<dpad index=\"1\"><mode>eight-way</mode></dpad>"
                           "<button index=\"1\"><toggle>true</toggle></button>"));
    auto *set = m_device->getActiveSetJoystick();
    QCOMPARE(set->getVDPad(0)->getJoyMode(), JoyDPad::EightWayMode);
    QVERIFY(set->getJoyButton(0)->getToggleState());
}

void TestGameControllerSetXml::reloadDoesNotRetainParsers()
{
    auto *set = m_device->getActiveSetJoystick();
    for (int i = 0; i < 20; i++)
        readSet(QStringLiteral("<button index=\"1\"><toggle>true</toggle></button>"
                               "<trigger index=\"5\"><deadZone>4321</deadZone></trigger>"
                               "<dpad index=\"1\"><mode>eight-way</mode></dpad>"));
    QVERIFY(set->findChildren<JoyButtonXml *>(QString(), Qt::FindDirectChildrenOnly).isEmpty());
    QVERIFY(set->findChildren<JoyAxisXml *>(QString(), Qt::FindDirectChildrenOnly).isEmpty());
}

QTEST_MAIN(TestGameControllerSetXml)
#include "testgamecontrollersetxml.moc"
