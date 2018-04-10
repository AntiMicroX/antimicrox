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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    this->settings = settings;
    this->graphical = graphical;
}

void AppLaunchHelper::initRunMethods()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int refreshRate = settings->value("Mouse/RefreshRate", 0).toInt();
    if (refreshRate > 0)
    {
        JoyButton::setMouseRefreshRate(refreshRate);
    }
}

void AppLaunchHelper::changeGamepadPollRate()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    int pollRate = settings->value("GamepadPollRate",
                                            AntiMicroSettings::defaultSDLGamepadPollRate).toUInt();
    if (pollRate > 0)
    {
        JoyButton::setGamepadRefreshRate(pollRate);
    }
}

void AppLaunchHelper::printControllerList(QMap<SDL_JoystickID, InputDevice *> *joysticks)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
        outstream << "  " << QObject::trUtf8("GUID:            %1").arg(tempdevice->getGUIDString()) << endl;
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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

    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;
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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

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
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyButton::indirectStaticMouseThread(QThread::currentThread());
}

void AppLaunchHelper::changeMouseThread(QThread *thread)
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyButton::setStaticMouseThread(thread);
}

void AppLaunchHelper::establishMouseTimerConnections()
{
    qDebug() << "[" << __FILE__ << ": " << __LINE__ << "] " << __FUNCTION__;

    JoyButton::establishMouseTimerConnections();
}
