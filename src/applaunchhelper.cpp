/* antimicro Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
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

#include "applaunchhelper.h"

#include "globalvariables.h"
#include "messagehandler.h"
#include "inputdevice.h"
#include "joybutton.h"
#include "antimicrosettings.h"

#ifdef Q_OS_WIN
    #include "winextras.h"
#endif

#include <QTextStream>
#include <QMapIterator>
#include <QDesktopWidget>
#include <QThread>
#include <QDebug>


AppLaunchHelper::AppLaunchHelper(AntiMicroSettings *settings, bool graphical,
                                 QObject *parent) :
    QObject(parent)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    this->settings = settings;
    this->graphical = graphical;
}

void AppLaunchHelper::initRunMethods()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    if (graphical)
    {
        establishMouseTimerConnections();
        enablePossibleMouseSmoothing();
        changeMouseRefreshRate();
        changeSpringModeScreen();
        changeGamepadPollRate();

#ifdef Q_OS_WIN
        checkPointerPrecision();
#endif
    }
}

void AppLaunchHelper::enablePossibleMouseSmoothing()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool smoothingEnabled = settings->value("Mouse/Smoothing", false).toBool();

    if (smoothingEnabled)
    {
        int historySize = settings->value("Mouse/HistorySize", 0).toInt();

        if (historySize > 0)
        {
            JoyButton::setMouseHistorySize(historySize, GlobalVariables::JoyButton::MAXIMUMMOUSEHISTORYSIZE, GlobalVariables::JoyButton::mouseHistorySize, &GlobalVariables::JoyButton::mouseHistoryX, &GlobalVariables::JoyButton::mouseHistoryY);
        }

        double weightModifier = settings->value("Mouse/WeightModifier", 0.0).toDouble();

        if (weightModifier > 0.0)
        {
            JoyButton::setWeightModifier(weightModifier, GlobalVariables::JoyButton::MAXIMUMWEIGHTMODIFIER, GlobalVariables::JoyButton::weightModifier);
        }
    }
}

void AppLaunchHelper::changeMouseRefreshRate()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int refreshRate = settings->value("Mouse/RefreshRate", 0).toInt();

    if (refreshRate > 0)
    {
        JoyButton::setMouseRefreshRate(refreshRate, GlobalVariables::JoyButton::mouseRefreshRate, GlobalVariables::JoyButton::IDLEMOUSEREFRESHRATE, JoyButton::getMouseHelper(), &GlobalVariables::JoyButton::mouseHistoryX, &GlobalVariables::JoyButton::mouseHistoryY, JoyButton::getTestOldMouseTime(), JoyButton::getStaticMouseEventTimer());
    }
}

void AppLaunchHelper::changeGamepadPollRate()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    int pollRate = settings->value("GamepadPollRate",
                                            GlobalVariables::AntimicroSettings::defaultSDLGamepadPollRate).toInt();
    if (pollRate > 0)
    {
        JoyButton::setGamepadRefreshRate(pollRate, GlobalVariables::JoyButton::gamepadRefreshRate, JoyButton::getMouseHelper());
    }
}

void AppLaunchHelper::printControllerList(QMap<SDL_JoystickID, InputDevice *> *joysticks)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QTextStream outstream(stdout);

    outstream << QObject::trUtf8("# of joysticks found: %1").arg(joysticks->size()) << endl;
    outstream << endl;
    outstream << QObject::trUtf8("List Joysticks:") << endl;
    outstream << QObject::trUtf8("---------------") << endl;
    QMapIterator<SDL_JoystickID, InputDevice*> iter(*joysticks);
    int indexNumber = 1;

    while (iter.hasNext())
    {
        InputDevice *tempdevice = iter.next().value();
        outstream << QObject::trUtf8("Joystick %1:").arg(indexNumber) << endl;
        outstream << "  " << QObject::trUtf8("Index:           %1").arg(tempdevice->getRealJoyNumber()) << endl;
        //outstream << "  " << QObject::trUtf8("GUID:            %1").arg(tempdevice->getGUIDString()) << endl;
        outstream << "  " << QObject::trUtf8("UniqueID:            %1").arg(tempdevice->getUniqueIDString()) << endl;
        outstream << "  " << QObject::trUtf8("Name:            %1").arg(tempdevice->getSDLName()) << endl;
        QString gameControllerStatus = tempdevice->isGameController() ?
                                       QObject::trUtf8("Yes") : QObject::trUtf8("No");
        outstream << "  " << QObject::trUtf8("Game Controller: %1").arg(gameControllerStatus) << endl;

        outstream << "  " << QObject::trUtf8("# of Axes:       %1").arg(tempdevice->getNumberRawAxes()) << endl;
        outstream << "  " << QObject::trUtf8("# of Buttons:    %1").arg(tempdevice->getNumberRawButtons()) << endl;
        outstream << "  " << QObject::trUtf8("# of Hats:       %1").arg(tempdevice->getNumberHats()) << endl;

        if (iter.hasNext())
        {
            outstream << endl;
            indexNumber++;
        }
    }
}

void AppLaunchHelper::changeSpringModeScreen()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    QDesktopWidget deskWid;
    int springScreen = settings->value("Mouse/SpringScreen",
                                       GlobalVariables::AntimicroSettings::defaultSpringScreen).toInt();

    if (springScreen >= deskWid.screenCount())
    {
        springScreen = -1;
        settings->setValue("Mouse/SpringScreen",
                           GlobalVariables::AntimicroSettings::defaultSpringScreen);
        settings->sync();
    }

    JoyButton::setSpringModeScreen(springScreen, GlobalVariables::JoyButton::springModeScreen);
}

#ifdef Q_OS_WIN
void AppLaunchHelper::checkPointerPrecision()
{

    qInstallMessageHandler(MessageHandler::myMessageOutput);
    WinExtras::grabCurrentPointerPrecision();
    bool disableEnhandedPoint = settings->value("Mouse/DisableWinEnhancedPointer",
                                                GlobalVariables::defaultDisabledWinEnhanced).toBool();
    if (disableEnhandedPoint)
    {
        WinExtras::disablePointerPrecision();
    }
}

void AppLaunchHelper::appQuitPointerPrecision()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    bool disableEnhancedPoint = settings->value("Mouse/DisableWinEnhancedPointer",
                                                GlobalVariables::defaultDisabledWinEnhanced).toBool();
    if (disableEnhancedPoint && !WinExtras::isUsingEnhancedPointerPrecision())
    {
        WinExtras::enablePointerPrecision();
    }
}

#endif

void AppLaunchHelper::revertMouseThread()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButton::indirectStaticMouseThread(QThread::currentThread(), JoyButton::getStaticMouseEventTimer(), JoyButton::getMouseHelper());
}

void AppLaunchHelper::changeMouseThread(QThread *thread)
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButton::setStaticMouseThread(thread, JoyButton::getStaticMouseEventTimer(), JoyButton::getTestOldMouseTime(), GlobalVariables::JoyButton::IDLEMOUSEREFRESHRATE, JoyButton::getMouseHelper());
}

void AppLaunchHelper::establishMouseTimerConnections()
{
    qInstallMessageHandler(MessageHandler::myMessageOutput);

    JoyButton::establishMouseTimerConnections();
}

AntiMicroSettings *AppLaunchHelper::getSettings() const {

    return settings;
}
