/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail.com>
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

#include "antimicrosettings.h"
#include "globalvariables.h"
#include "inputdevice.h"
#include "joybuttontypes/joybutton.h"

#include <QDebug>
#include <QMapIterator>
#include <QThread>

#ifdef Q_OS_WIN
    #include <winextras.h>
#endif

AppLaunchHelper::AppLaunchHelper(AntiMicroSettings *settings, bool graphical, QObject *parent)
    : QObject(parent)
{
    this->settings = settings;
    this->graphical = graphical;
}

void AppLaunchHelper::initRunMethods()
{
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
    bool smoothingEnabled = settings->value("Mouse/Smoothing", false).toBool();

    if (smoothingEnabled)
    {
        int historySize = settings->value("Mouse/HistorySize", 0).toInt();

        if (historySize > 0)
        {
            JoyButton::setMouseHistorySize(historySize, GlobalVariables::JoyButton::MAXIMUMMOUSEHISTORYSIZE,
                                           GlobalVariables::JoyButton::mouseHistorySize,
                                           &GlobalVariables::JoyButton::mouseHistoryX,
                                           &GlobalVariables::JoyButton::mouseHistoryY);
        }

        double weightModifier = settings->value("Mouse/WeightModifier", 0.0).toDouble();

        if (weightModifier > 0.0)
        {
            JoyButton::setWeightModifier(weightModifier, GlobalVariables::JoyButton::MAXIMUMWEIGHTMODIFIER,
                                         GlobalVariables::JoyButton::weightModifier);
        }
    }
}

void AppLaunchHelper::changeMouseRefreshRate()
{
    int refreshRate = settings->value("Mouse/RefreshRate", 0).toInt();

    if (refreshRate > 0)
    {
        JoyButton::setMouseRefreshRate(refreshRate, GlobalVariables::JoyButton::mouseRefreshRate,
                                       GlobalVariables::JoyButton::IDLEMOUSEREFRESHRATE, JoyButton::getMouseHelper(),
                                       &GlobalVariables::JoyButton::mouseHistoryX,
                                       &GlobalVariables::JoyButton::mouseHistoryY, JoyButton::getTestOldMouseTime(),
                                       JoyButton::getStaticMouseEventTimer());
    }
}

void AppLaunchHelper::changeGamepadPollRate()
{
    int pollRate = settings->value("GamepadPollRate", GlobalVariables::AntimicroSettings::defaultSDLGamepadPollRate).toInt();
    if (pollRate > 0)
    {
        JoyButton::setGamepadRefreshRate(pollRate, GlobalVariables::JoyButton::gamepadRefreshRate,
                                         JoyButton::getMouseHelper());
    }
}

void AppLaunchHelper::printControllerList(QMap<SDL_JoystickID, InputDevice *> *joysticks)
{
    PRINT_STDOUT() << QObject::tr("# of joysticks found: %1").arg(joysticks->size()) << "\n"
                   << "\n"
                   << QObject::tr("List Joysticks:") << "\n"
                   << QObject::tr("---------------") << "\n";
    QMapIterator<SDL_JoystickID, InputDevice *> iter(*joysticks);
    int indexNumber = 1;

    while (iter.hasNext())
    {
        InputDevice *tempdevice = iter.next().value();
        PRINT_STDOUT() << QObject::tr("Joystick %1:").arg(indexNumber) << "\n" << tempdevice->getDescription();

        if (iter.hasNext())
        {
            PRINT_STDOUT() << "\n";
            indexNumber++;
        }
    }
}

void AppLaunchHelper::changeSpringModeScreen()
{
    int springScreen =
        settings->value("Mouse/SpringScreen", GlobalVariables::AntimicroSettings::defaultSpringScreen).toInt();

    if (springScreen >= QGuiApplication::screens().count())
    {
        springScreen = -1;
        settings->setValue("Mouse/SpringScreen", GlobalVariables::AntimicroSettings::defaultSpringScreen);
        settings->sync();
    }

    JoyButton::setSpringModeScreen(springScreen, GlobalVariables::JoyButton::springModeScreen);
}

#ifdef Q_OS_WIN
void AppLaunchHelper::checkPointerPrecision()
{
    WinExtras::grabCurrentPointerPrecision();
    bool disableEnhandedPoint =
        settings->value("Mouse/DisableWinEnhancedPointer", AntiMicroSettings::defaultDisabledWinEnhanced).toBool();
    if (disableEnhandedPoint)
    {
        WinExtras::disablePointerPrecision();
    }
}

void AppLaunchHelper::appQuitPointerPrecision()
{
    bool disableEnhancedPoint =
        settings->value("Mouse/DisableWinEnhancedPointer", AntiMicroSettings::defaultDisabledWinEnhanced).toBool();
    if (disableEnhancedPoint && !WinExtras::isUsingEnhancedPointerPrecision())
    {
        WinExtras::enablePointerPrecision();
    }
}
#endif

void AppLaunchHelper::revertMouseThread()
{
    JoyButton::indirectStaticMouseThread(QThread::currentThread(), JoyButton::getStaticMouseEventTimer(),
                                         JoyButton::getMouseHelper());
}

void AppLaunchHelper::changeMouseThread(QThread *thread)
{
    JoyButton::setStaticMouseThread(thread, JoyButton::getStaticMouseEventTimer(), JoyButton::getTestOldMouseTime(),
                                    GlobalVariables::JoyButton::IDLEMOUSEREFRESHRATE, JoyButton::getMouseHelper());
}

void AppLaunchHelper::establishMouseTimerConnections() { JoyButton::establishMouseTimerConnections(); }

AntiMicroSettings *AppLaunchHelper::getSettings() const { return settings; }
