/* antimicroX Gamepad to KB+M event mapper
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

#include "globalvariables.h"
#include "messagehandler.h"
#include "inputdevice.h"
#include "joybutton.h"
#include "antimicrosettings.h"

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

    outstream << QObject::tr("# of joysticks found: %1").arg(joysticks->size()) << endl;
    outstream << endl;
    outstream << QObject::tr("List Joysticks:") << endl;
    outstream << QObject::tr("---------------") << endl;
    QMapIterator<SDL_JoystickID, InputDevice*> iter(*joysticks);
    int indexNumber = 1;

    while (iter.hasNext())
    {
        InputDevice *tempdevice = iter.next().value();
        outstream << QObject::tr("Joystick %1:").arg(indexNumber) << endl;
        outstream << "  " << QObject::tr("Index:           %1").arg(tempdevice->getRealJoyNumber()) << endl;
        //outstream << "  " << QObject::tr("GUID:            %1").arg(tempdevice->getGUIDString()) << endl;
        outstream << "  " << QObject::tr("UniqueID:            %1").arg(tempdevice->getUniqueIDString()) << endl;
        outstream << "  " << QObject::tr("GUID:            %1").arg(tempdevice->getGUIDString()) << endl;
        outstream << "  " << QObject::tr("VendorID:            %1").arg(tempdevice->getVendorString()) << endl;
        outstream << "  " << QObject::tr("ProductID:            %1").arg(tempdevice->getProductIDString()) << endl;
        outstream << "  " << QObject::tr("Product Version:            %1").arg(tempdevice->getProductVersion()) << endl;
        outstream << "  " << QObject::tr("Name:            %1").arg(tempdevice->getSDLName()) << endl;
        QString gameControllerStatus = tempdevice->isGameController() ?
                                       QObject::tr("Yes") : QObject::tr("No");
        outstream << "  " << QObject::tr("Game Controller: %1").arg(gameControllerStatus) << endl;

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
