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

#ifndef GLOBALVARIABLES_H
#define GLOBALVARIABLES_H

#include <QList>
#include <QObject>
#include <QRegularExpression>

namespace GlobalVariables {
class JoyButton
{
  public:
    static const QString xmlName;

    static int IDLEMOUSEREFRESHRATE;
    static int mouseHistorySize;
    // Get active mouse movement refresh rate
    static int mouseRefreshRate;
    static int springModeScreen;
    // gamepad poll rate used by the application in ms
    static int gamepadRefreshRate;

    static double cursorRemainderX;
    static double cursorRemainderY;
    static double mouseSpeedModifier;
    // Weight modifier in the range of 0.0 - 1.0
    static double weightModifier;

    static const bool DEFAULTTOGGLE;
    static const bool DEFAULTUSETURBO;
    static const bool DEFAULTCYCLERESETACTIVE;
    static const bool DEFAULTRELATIVESPRING;

    static const double DEFAULTMOUSESPEEDMOD;
    static const double DEFAULTSENSITIVITY;
    static const double DEFAULTEASINGDURATION;
    static const double MINIMUMEASINGDURATION;
    static const double MAXIMUMEASINGDURATION;
    static const double DEFAULTWEIGHTMODIFIER;
    static const double MAXIMUMWEIGHTMODIFIER;
    static const double DEFAULTEXTRACCELVALUE;
    static const double DEFAULTMINACCELTHRESHOLD;
    static const double DEFAULTMAXACCELTHRESHOLD;
    static const double DEFAULTSTARTACCELMULTIPLIER;
    static const double DEFAULTACCELEASINGDURATION;

    // Define default values for many properties.
    static const int ENABLEDTURBODEFAULT;
    static const int DEFAULTKEYREPEATDELAY; // unsigned
    static const int DEFAULTKEYREPEATRATE;  // unsigned
    static const int DEFAULTTURBOINTERVAL;
    static const int DEFAULTMOUSESPEEDX;
    static const int DEFAULTMOUSESPEEDY;
    static const int MAXMOUSESPEED;
    static const int DEFAULTSETSELECTION;
    static const int DEFAULTSPRINGWIDTH;
    static const int DEFAULTSPRINGHEIGHT;
    static const int DEFAULTWHEELX;
    static const int DEFAULTWHEELY;
    static const int DEFAULTCYCLERESET;
    static const int DEFAULTMOUSEHISTORYSIZE;
    static const int MAXIMUMMOUSEHISTORYSIZE;
    static const int MAXIMUMMOUSEREFRESHRATE;
    static const int DEFAULTIDLEMOUSEREFRESHRATE;
    static const int MINCYCLERESETTIME;
    static const int MAXCYCLERESETTIME;
    static const int DEFAULTSPRINGRELEASERADIUS;

    static QHash<int, int> activeKeys;
    static QHash<int, int> activeMouseButtons;
    static QList<double> mouseHistoryX;
    static QList<double> mouseHistoryY;
};

class AntimicroSettings
{
  public:
    static const bool defaultDisabledWinEnhanced;
    static const bool defaultAssociateProfiles;
    static const int defaultSpringScreen;
    static const int defaultSDLGamepadPollRate;
};

class InputDevice
{
  public:
    static const int NUMBER_JOYSETS;
    static const int DEFAULTKEYPRESSTIME;
    static const int RAISEDDEADZONE;
    static const int DEFAULTKEYREPEATDELAY;
    static const int DEFAULTKEYREPEATRATE;

    // static QRegularExpression emptyGUID;
    static QRegularExpression emptyUniqueID;
};

class JoyAxis
{
  public:
    static const int AXISMIN;
    static const int AXISMAX;
    static const int AXISDEADZONE;
    static const int AXISMAXZONE;

    static const float JOYSPEED;

    static const QString xmlName;
};

class HapticTriggerPs5
{
  public:
    static const int CLICKSTRENGTH;
    static const int RIGIDSTRENGTH;
    static const int RIGIDGRADIENTSTRENGTH;
    static const int VIBRATIONSTRENGTH;
    static const int RANGE;
    static const int FREQUENCY;
};

#ifdef WITH_X11

class X11Extras
{
  public:
    static const QString mouseDeviceName;
    static const QString keyboardDeviceName;
    static const QString xtestMouseDeviceName;

    static QString _customDisplayString;
};

#endif

class GameController
{
  public:
    static const QString xmlName;
};

class GameControllerDPad
{
  public:
    static const QString xmlName;
};

class GameControllerTrigger
{
  public:
    static const int AXISDEADZONE;
    static const int AXISMAXZONE;

    static const QString xmlName;
};

class GameControllerTriggerButton
{
  public:
    static const QString xmlName;
};

class InputDaemon
{
  public:
    static const int GAMECONTROLLERTRIGGERRELEASE;
};

class VDPad
{
  public:
    static const QString xmlName;
};

class SetJoystick
{
  public:
    static const int MAXNAMELENGTH;
    static const int RAISEDDEADZONE;
};

class Joystick
{
  public:
    static const QString xmlName;
};

class JoyDPad
{
  public:
    static const QString xmlName;
    static const int DEFAULTDPADDELAY; // unsigned
};

class JoyControlStick
{
  public:
    static const double PI;

    // Define default values for stick properties.
    static const int DEFAULTDEADZONE;
    static const int DEFAULTMAXZONE;
    static const int DEFAULTMODIFIERZONE;
    static const bool DEFAULTMODIFIERZONEINVERTED;
    static const int DEFAULTDIAGONALRANGE;
    static const double DEFAULTCIRCLE;
    static const int DEFAULTSTICKDELAY;
};

class JoySensor
{
  public:
    static const double ACCEL_MIN;
    static const double ACCEL_MAX;
    static const double GYRO_MIN;
    static const double GYRO_MAX;
    static const double DEFAULTDEADZONE;
    static const int DEFAULTDIAGONALRANGE;
    static const unsigned int DEFAULTSENSORDELAY;
};

class JoyButtonSlot
{
  public:
    static const int JOYSPEED;
    static const QString xmlName;
    static const int MAXTEXTENTRYDISPLAYLENGTH;
};

class AdvanceButtonDialog
{
  public:
    static const int MINIMUMTURBO;
};

class JoyAxisButton
{
  public:
    static const QString xmlName;
};

class JoyControlStickButton
{
  public:
    static const QString xmlName;
};

class JoyControlStickModifierButton
{
  public:
    static const QString xmlName;
};

class JoySensorButton
{
  public:
    static const QString xmlName;
};

class JoyDPadButton
{
  public:
    static const QString xmlName;
};

} // namespace GlobalVariables

#endif // GLOBALVARIABLES_H
