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

#include <QTextStream>
#include <QMapIterator>
#include <QDesktopWidget>

#include "applaunchhelper.h"

#ifdef Q_OS_WIN
#include <winextras.h>
#endif

AppLaunchHelper::AppLaunchHelper(AntiMicroSettings *settings, bool graphical,
                                 QObject *parent) :
    QObject(parent)
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
            JoyButton::setMouseHistorySize(historySize);
        }

        double weightModifier = settings->value("Mouse/WeightModifier", 0.0).toDouble();
        if (weightModifier > 0.0)
        {
            JoyButton::setWeightModifier(weightModifier);
        }
    }
}

void AppLaunchHelper::changeMouseRefreshRate()
{
    int refreshRate = settings->value("Mouse/RefreshRate", 0).toInt();
    if (refreshRate > 0)
    {
        JoyButton::setMouseRefreshRate(refreshRate);
    }
}

void AppLaunchHelper::changeGamepadPollRate()
{
    unsigned int pollRate = settings->value("GamepadPollRate",
                                            AntiMicroSettings::defaultSDLGamepadPollRate).toUInt();
    if (pollRate > 0)
    {
        JoyButton::setGamepadRefreshRate(pollRate);
    }
}

void AppLaunchHelper::printControllerList(QMap<SDL_JoystickID, InputDevice *> *joysticks)
{
    QTextStream outstream(stdout);

    outstream << QObject::tr("# of joysticks found: %1").arg(joysticks->size()) << endl;
    outstream << endl;
    outstream << QObject::tr("List Joysticks:") << endl;
    outstream << QObject::tr("---------------") << endl;
    QMapIterator<SDL_JoystickID, InputDevice*> iter(*joysticks);
    unsigned int indexNumber = 1;
    while (iter.hasNext())
    {
        InputDevice *tempdevice = iter.next().value();
        outstream << QObject::tr("Joystick %1:").arg(indexNumber) << endl;
        outstream << "  " << QObject::tr("Index:           %1").arg(tempdevice->getRealJoyNumber()) << endl;
#ifdef USE_SDL_2
        outstream << "  " << QObject::tr("GUID:            %1").arg(tempdevice->getGUIDString()) << endl;
#endif
        outstream << "  " << QObject::tr("Name:            %1").arg(tempdevice->getSDLName()) << endl;
#ifdef USE_SDL_2
        QString gameControllerStatus = tempdevice->isGameController() ?
                                       QObject::tr("Yes") : QObject::tr("No");
        outstream << "  " << QObject::tr("Game Controller: %1").arg(gameControllerStatus) << endl;
#endif

        outstream << "  " << QObject::tr("# of Axes:       %1").arg(tempdevice->getNumberRawAxes()) << endl;
        outstream << "  " << QObject::tr("# of Buttons:    %1").arg(tempdevice->getNumberRawButtons()) << endl;
        outstream << "  " << QObject::tr("# of Hats:       %1").arg(tempdevice->getNumberHats()) << endl;

        if (iter.hasNext())
        {
            outstream << endl;
            indexNumber++;
        }
    }
}

void AppLaunchHelper::changeSpringModeScreen()
{
    QDesktopWidget deskWid;
    int springScreen = settings->value("Mouse/SpringScreen",
                                       AntiMicroSettings::defaultSpringScreen).toInt();

    if (springScreen >= deskWid.screenCount())
    {
        springScreen = -1;
        settings->setValue("Mouse/SpringScreen",
                           AntiMicroSettings::defaultSpringScreen);
        settings->sync();
    }

    JoyButton::setSpringModeScreen(springScreen);
}

#ifdef Q_OS_WIN
void AppLaunchHelper::checkPointerPrecision()
{
    WinExtras::grabCurrentPointerPrecision();
    bool disableEnhandedPoint = settings->value("Mouse/DisableWinEnhancedPointer",
                                                AntiMicroSettings::defaultDisabledWinEnhanced).toBool();
    if (disableEnhandedPoint)
    {
        WinExtras::disablePointerPrecision();
    }
}

void AppLaunchHelper::appQuitPointerPrecision()
{
    bool disableEnhancedPoint = settings->value("Mouse/DisableWinEnhancedPointer",
                                                AntiMicroSettings::defaultDisabledWinEnhanced).toBool();
    if (disableEnhancedPoint && !WinExtras::isUsingEnhancedPointerPrecision())
    {
        WinExtras::enablePointerPrecision();
    }
}

#endif

void AppLaunchHelper::revertMouseThread()
{
    JoyButton::indirectStaticMouseThread(QThread::currentThread());
}

void AppLaunchHelper::changeMouseThread(QThread *thread)
{
    JoyButton::setStaticMouseThread(thread);
}

void AppLaunchHelper::establishMouseTimerConnections()
{
    JoyButton::establishMouseTimerConnections();
}
