
#pragma once
#ifndef GLOBALVARIABLES_H
#define GLOBALVARIABLES_H

#include <QObject>
#include <QList>


namespace GlobalVariables
{
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
      static const int DEFAULTKEYREPEATRATE; // unsigned
      static const int DEFAULTTURBOINTERVAL;
      static const int DEFAULTMOUSESPEEDX;
      static const int DEFAULTMOUSESPEEDY;
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

      //static QRegExp emptyGUID;
      static QRegExp emptyUniqueID;
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
      static const int DEFAULTDIAGONALRANGE;
      static const double DEFAULTCIRCLE;
      static const int DEFAULTSTICKDELAY;
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

  class JoyDPadButton
  {
      public:

      static const QString xmlName;
  };

}

#endif // GLOBALVARIABLES_H
