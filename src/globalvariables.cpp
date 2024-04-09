/* antimicrox Gamepad to KB+M event mapper
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail>
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

#include "globalvariables.h"
#include "common.h"

#include <cmath>

// ---- JOYBUTTON --- //

const QString GlobalVariables::JoyButton::xmlName = "button";

// Set default values for many properties.
const int GlobalVariables::JoyButton::ENABLEDTURBODEFAULT = 100;
const double GlobalVariables::JoyButton::DEFAULTMOUSESPEEDMOD = 1.0;
double GlobalVariables::JoyButton::mouseSpeedModifier = GlobalVariables::JoyButton::DEFAULTMOUSESPEEDMOD;
const int GlobalVariables::JoyButton::DEFAULTKEYREPEATDELAY = 600; // 600 ms
const int GlobalVariables::JoyButton::DEFAULTKEYREPEATRATE = 40;   // 40 ms. 25 times per second
const bool GlobalVariables::JoyButton::DEFAULTTOGGLE = false;
const int GlobalVariables::JoyButton::DEFAULTTURBOINTERVAL = 0;
const bool GlobalVariables::JoyButton::DEFAULTUSETURBO = false;
const int GlobalVariables::JoyButton::DEFAULTMOUSESPEEDX = 50;
const int GlobalVariables::JoyButton::DEFAULTMOUSESPEEDY = 50;
const int GlobalVariables::JoyButton::MAXMOUSESPEED = 300;
const int GlobalVariables::JoyButton::DEFAULTSETSELECTION = -1;
const int GlobalVariables::JoyButton::DEFAULTSPRINGWIDTH = 0;
const int GlobalVariables::JoyButton::DEFAULTSPRINGHEIGHT = 0;
const double GlobalVariables::JoyButton::DEFAULTSENSITIVITY = 1.0;
const int GlobalVariables::JoyButton::DEFAULTWHEELX = 20;
const int GlobalVariables::JoyButton::DEFAULTWHEELY = 20;
const bool GlobalVariables::JoyButton::DEFAULTCYCLERESETACTIVE = false;
const int GlobalVariables::JoyButton::DEFAULTCYCLERESET = 0;
const bool GlobalVariables::JoyButton::DEFAULTRELATIVESPRING = false;
const double GlobalVariables::JoyButton::DEFAULTEASINGDURATION = 0.5;
const double GlobalVariables::JoyButton::MINIMUMEASINGDURATION = 0.2;
const double GlobalVariables::JoyButton::MAXIMUMEASINGDURATION = 5.0;
const int GlobalVariables::JoyButton::MINCYCLERESETTIME = 10;
const int GlobalVariables::JoyButton::MAXCYCLERESETTIME = 60000;

const int GlobalVariables::JoyButton::DEFAULTMOUSEHISTORYSIZE = 10;
const double GlobalVariables::JoyButton::DEFAULTWEIGHTMODIFIER = 0.2;
const int GlobalVariables::JoyButton::MAXIMUMMOUSEHISTORYSIZE = 100;
const double GlobalVariables::JoyButton::MAXIMUMWEIGHTMODIFIER = 1.0;
const int GlobalVariables::JoyButton::MAXIMUMMOUSEREFRESHRATE = 16;
int GlobalVariables::JoyButton::IDLEMOUSEREFRESHRATE = (5 * 20);
const int GlobalVariables::JoyButton::DEFAULTIDLEMOUSEREFRESHRATE = 100;
const double GlobalVariables::JoyButton::DEFAULTEXTRACCELVALUE = 2.0;
const double GlobalVariables::JoyButton::DEFAULTMINACCELTHRESHOLD = 10.0;
const double GlobalVariables::JoyButton::DEFAULTMAXACCELTHRESHOLD = 100.0;
const double GlobalVariables::JoyButton::DEFAULTSTARTACCELMULTIPLIER = 0.0;
const double GlobalVariables::JoyButton::DEFAULTACCELEASINGDURATION = 0.1;
const int GlobalVariables::JoyButton::DEFAULTSPRINGRELEASERADIUS = 0;

// Keep references to active keys and mouse buttons.
QHash<int, int> GlobalVariables::JoyButton::activeKeys;
QHash<int, int> GlobalVariables::JoyButton::activeMouseButtons;

// History buffers used for mouse smoothing routine.
QList<double> GlobalVariables::JoyButton::mouseHistoryX;
QList<double> GlobalVariables::JoyButton::mouseHistoryY;

// Carry over remainder of a cursor move for the next mouse event.
double GlobalVariables::JoyButton::cursorRemainderX = 0.0;
double GlobalVariables::JoyButton::cursorRemainderY = 0.0;

double GlobalVariables::JoyButton::weightModifier = 0;
// Mouse history buffer size
int GlobalVariables::JoyButton::mouseHistorySize = 1;

int GlobalVariables::JoyButton::mouseRefreshRate = 5;
int GlobalVariables::JoyButton::springModeScreen = -1;
int GlobalVariables::JoyButton::gamepadRefreshRate = 10;

// ---- ANTIMICROSETTINGS --- //

const bool GlobalVariables::AntimicroSettings::defaultDisabledWinEnhanced = false;
const bool GlobalVariables::AntimicroSettings::defaultAssociateProfiles = true;
const int GlobalVariables::AntimicroSettings::defaultSpringScreen = -1;
const int GlobalVariables::AntimicroSettings::defaultSDLGamepadPollRate = 10; // unsigned

// ---- INPUTDEVICE ---- //

const int GlobalVariables::InputDevice::NUMBER_JOYSETS = 8;
const int GlobalVariables::InputDevice::DEFAULTKEYPRESSTIME = 100;
const int GlobalVariables::InputDevice::RAISEDDEADZONE = 20000;
const int GlobalVariables::InputDevice::DEFAULTKEYREPEATDELAY = 660; // 660 ms
const int GlobalVariables::InputDevice::DEFAULTKEYREPEATRATE = 40;   // 40 ms. 25 times per second

// QRegularExpression GlobalVariables::InputDevice::emptyGUID("^[0]+$");
QRegularExpression GlobalVariables::InputDevice::emptyUniqueID("^[0]+$");

// ---- JOYAXIS ---- //

// Set default values for many properties.
const int GlobalVariables::JoyAxis::AXISMIN = -32767;
const int GlobalVariables::JoyAxis::AXISMAX = 32767;
const int GlobalVariables::JoyAxis::AXISDEADZONE = 6000;
const int GlobalVariables::JoyAxis::AXISMAXZONE = 32000;

// Speed in pixels/second
const float GlobalVariables::JoyAxis::JOYSPEED = 20.0;

const QString GlobalVariables::JoyAxis::xmlName = "axis";

const int GlobalVariables::HapticTriggerPs5::CLICKSTRENGTH = 256;
const int GlobalVariables::HapticTriggerPs5::RIGIDSTRENGTH = 128;
const int GlobalVariables::HapticTriggerPs5::RIGIDGRADIENTSTRENGTH = 256;
const int GlobalVariables::HapticTriggerPs5::VIBRATIONSTRENGTH = 64;
const int GlobalVariables::HapticTriggerPs5::RANGE = 320;
const int GlobalVariables::HapticTriggerPs5::FREQUENCY = 32;

#ifdef WITH_X11

// ---- X11EXTRAS ---- //

const QString GlobalVariables::X11Extras::mouseDeviceName = PadderCommon::mouseDeviceName;
const QString GlobalVariables::X11Extras::keyboardDeviceName = PadderCommon::keyboardDeviceName;
const QString GlobalVariables::X11Extras::xtestMouseDeviceName = QString("Virtual core XTEST pointer");

QString GlobalVariables::X11Extras::_customDisplayString = QString("");

#endif

// ---- GameController ---- //

const QString GlobalVariables::GameController::xmlName = "gamecontroller";

// ---- GameControllerDPad ---- //

const QString GlobalVariables::GameControllerDPad::xmlName = "dpad";

// ---- GameControllerTrigger ---- //

const int GlobalVariables::GameControllerTrigger::AXISDEADZONE = 2000;
const int GlobalVariables::GameControllerTrigger::AXISMAXZONE = 32000;

const QString GlobalVariables::GameControllerTrigger::xmlName = "trigger";

// ---- GameControllerTriggerButton ---- //

const QString GlobalVariables::GameControllerTriggerButton::xmlName = "triggerbutton";

// ---- InputDaemon ---- //

const int GlobalVariables::InputDaemon::GAMECONTROLLERTRIGGERRELEASE = 16384;

// ---- VDPad ---- //

const QString GlobalVariables::VDPad::xmlName = "vdpad";

// ---- SetJoystick ---- //

const int GlobalVariables::SetJoystick::MAXNAMELENGTH = 30;
const int GlobalVariables::SetJoystick::RAISEDDEADZONE = 20000;

// ---- Joystick ---- //

const QString GlobalVariables::Joystick::xmlName = "joystick";

// ---- JoyDPad ---- //

const QString GlobalVariables::JoyDPad::xmlName = "dpad";
const int GlobalVariables::JoyDPad::DEFAULTDPADDELAY = 0;

// ---- JoyControlStick ---- //

// Define Pi here.
const double GlobalVariables::JoyControlStick::PI = acos(-1.0);

// Set default values used for stick properties.
const int GlobalVariables::JoyControlStick::DEFAULTDEADZONE = 8000;
const int GlobalVariables::JoyControlStick::DEFAULTMAXZONE = GlobalVariables::GameControllerTrigger::AXISMAXZONE;
const int GlobalVariables::JoyControlStick::DEFAULTMODIFIERZONE = 8000;
const bool GlobalVariables::JoyControlStick::DEFAULTMODIFIERZONEINVERTED = false;
const int GlobalVariables::JoyControlStick::DEFAULTDIAGONALRANGE = 45;
const double GlobalVariables::JoyControlStick::DEFAULTCIRCLE = 0.0;
const int GlobalVariables::JoyControlStick::DEFAULTSTICKDELAY = 0;

// ---- JoySensor ---- //

const double GlobalVariables::JoySensor::ACCEL_MIN = -90.0;
const double GlobalVariables::JoySensor::ACCEL_MAX = 90.0;
const double GlobalVariables::JoySensor::GYRO_MIN = -360.0;
const double GlobalVariables::JoySensor::GYRO_MAX = 360.0;
const double GlobalVariables::JoySensor::DEFAULTDEADZONE = 20;
const int GlobalVariables::JoySensor::DEFAULTDIAGONALRANGE = 45;
const unsigned int GlobalVariables::JoySensor::DEFAULTSENSORDELAY = 0;

// ---- JoyButtonSlot ---- //

const int GlobalVariables::JoyButtonSlot::JOYSPEED = 20;
const QString GlobalVariables::JoyButtonSlot::xmlName = "slot";
const int GlobalVariables::JoyButtonSlot::MAXTEXTENTRYDISPLAYLENGTH = 40;

// ---- AdvanceButtonDialog ---- //

const int GlobalVariables::AdvanceButtonDialog::MINIMUMTURBO = 2;

// ---- JoyAxisButton ---- //

const QString GlobalVariables::JoyAxisButton::xmlName = "axisbutton";

// ---- JoyControlStickButton ---- //

const QString GlobalVariables::JoyControlStickButton::xmlName = "stickbutton";

// ---- JoyControlStickModifierButton ---- //

const QString GlobalVariables::JoyControlStickModifierButton::xmlName = "stickmodifierbutton";

// ---- JoySensorButton ---- //

const QString GlobalVariables::JoySensorButton::xmlName = "sensorbutton";

// ---- JoyDPadButton ---- //

const QString GlobalVariables::JoyDPadButton::xmlName = "dpadbutton";
