<map version="0.9.0">
<!-- To view this file, download free mind mapping software FreeMind from http://freemind.sourceforge.net -->
<node CREATED="1379712845116" ID="ID_571348173" MODIFIED="1386798703703" TEXT="AntiMicro Future Developments">
<node CREATED="1379712868552" FOLDED="true" ID="ID_1152197684" MODIFIED="1392361496339" POSITION="right" TEXT="Mouse Curves">
<icon BUILTIN="button_ok"/>
<node CREATED="1385413655567" ID="ID_1859704771" MODIFIED="1385413672185" TEXT="Curve options were taken from QJoyPad. Linear is still used as the default."/>
</node>
<node CREATED="1379712878989" FOLDED="true" ID="ID_1601152391" MODIFIED="1392361529837" POSITION="left" TEXT="Gamepad Status/Diagnosis">
<icon BUILTIN="button_ok"/>
<node CREATED="1386295949051" ID="ID_1843601206" MODIFIED="1386296073732" TEXT="Status window shows pieces of information provided by SDL: number of buttons, number of axes, number of hats and controller display name. The window also shows the current raw values of all axes, hats, and buttons. This window will be useful for better showing exactly how SDL sees the controller."/>
</node>
<node CREATED="1379712921288" ID="ID_1904560551" MODIFIED="1392361620864" POSITION="right" TEXT="Tap Time?">
<icon BUILTIN="button_ok"/>
<node CREATED="1386296158070" ID="ID_1324519805" MODIFIED="1392361566372" TEXT="This is mainly useful for Release slot events and Pause slot events. This will help ensure that a key is held down for long enough for a game to properly catch the keydown event.">
<icon BUILTIN="info"/>
</node>
</node>
<node CREATED="1379713052871" FOLDED="true" ID="ID_362469693" MODIFIED="1392361620066" POSITION="left" TEXT="SDL 2 compatibility?">
<icon BUILTIN="button_ok"/>
<node CREATED="1385413319581" HGAP="246" ID="ID_315431873" MODIFIED="1386729226005" VSHIFT="-44">
<richcontent TYPE="NODE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Most of the Joystick API is the same but there are some problems.
    </p>
    <p>
      
    </p>
    <p>
      The biggest problem is regarding the Xbox 360 controller under Windows. SDL 2 uses XInput with an Xbox 360 controller as opposed to the old behavior of using DirectInput. The button layout of the 360 controller is completely different when using XInput. Because of this, any Windows
    </p>
    <p>
      controller profile made with a version of AntiMicro built using SDL 1.2 will not
    </p>
    <p>
      work on a version built with SDL 2.
    </p>
  </body>
</html></richcontent>
<icon BUILTIN="info"/>
</node>
<node CREATED="1386727091342" FOLDED="true" HGAP="241" ID="ID_606065535" MODIFIED="1392361600287" TEXT="Allow compilation against SDL 2 using Joystick API" VSHIFT="-23">
<icon BUILTIN="button_ok"/>
<node CREATED="1386728036714" HGAP="52" ID="ID_653911559" MODIFIED="1392361578740" TEXT="Different flags must be passed to SDL_Init. Use SDL_INIT_JOYSTICK. Do not use SDL_INIT_VIDEO.&#xa;&#xa;Enabling video support without actually creating a window in SDL causes&#xa;some major problems with joystick support in SDL 2; no axes support and only button up events are provided. Enabling video&#xa;was required in SDL 1.2 because that would also enable the events system&#xa;in SDL." VSHIFT="-35">
<icon BUILTIN="info"/>
</node>
</node>
<node CREATED="1386727260315" HGAP="331" ID="ID_367093659" MODIFIED="1388425763979" TEXT="Allow use of GameController API. Can be used to allow for unified configuration files.&#xa;&#xa;Hopefully, there will be a way to implement a gamecontroller mapping program&#xa;so that controllers not currently supported by SDL can still use the API.&#xa;Think Steam Big Picture controller configuration." VSHIFT="37">
<icon BUILTIN="button_ok"/>
</node>
</node>
<node CREATED="1379713072288" ID="ID_1167223802" MODIFIED="1385518028334" POSITION="right" TEXT="Mouse Wheel as Movement?">
<icon BUILTIN="button_ok"/>
</node>
<node CREATED="1379713112228" ID="ID_2402610" MODIFIED="1379713123344" POSITION="left" TEXT="Disable Buttons from Display?"/>
<node CREATED="1379713177327" ID="ID_1294383668" MODIFIED="1385413564002" POSITION="right" TEXT="Spring Mode Window Preview">
<icon BUILTIN="button_ok"/>
</node>
<node CREATED="1379719159629" ID="ID_1413165164" MODIFIED="1385413551068" POSITION="left" TEXT="Gamepad Identifier (Using SDL_JoystickName?)">
<icon BUILTIN="button_ok"/>
</node>
<node CREATED="1379742754449" ID="ID_146831130" MODIFIED="1379742761336" POSITION="right" TEXT="Copy Set Assignments to New Set"/>
</node>
</map>
