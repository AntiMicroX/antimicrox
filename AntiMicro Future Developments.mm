<map version="0.9.0">
<!-- To view this file, download free mind mapping software FreeMind from http://freemind.sourceforge.net -->
<node CREATED="1379712845116" ID="ID_571348173" MODIFIED="1379712975572" TEXT="AntiMicro Future Developments">
<node CREATED="1379712868552" FOLDED="true" ID="ID_1152197684" MODIFIED="1386296245672" POSITION="right" TEXT="Mouse Curves">
<icon BUILTIN="button_ok"/>
<node CREATED="1385413655567" ID="ID_1859704771" MODIFIED="1385413672185" TEXT="Curve options were taken from QJoyPad. Linear is still used as the default."/>
</node>
<node CREATED="1379712878989" FOLDED="true" ID="ID_1601152391" MODIFIED="1386296120777" POSITION="left" TEXT="Gamepad Status/Diagnosis">
<icon BUILTIN="button_ok"/>
<node CREATED="1386295949051" ID="ID_1843601206" MODIFIED="1386296073732" TEXT="Status window shows pieces of information provided by SDL: number of buttons, number of axes, number of hats and controller display name. The window also shows the current raw values of all axes, hats, and buttons. This window will be useful for better showing exactly how SDL sees the controller."/>
</node>
<node CREATED="1379712921288" FOLDED="true" ID="ID_1904560551" MODIFIED="1386296255012" POSITION="right" TEXT="Tap Time?">
<icon BUILTIN="help"/>
<node CREATED="1386296158070" ID="ID_1324519805" MODIFIED="1386296197745" TEXT="I still haven&apos;t found a really good use case for this yet."/>
</node>
<node CREATED="1379713052871" FOLDED="true" ID="ID_362469693" MODIFIED="1386296150605" POSITION="left" TEXT="SDL 2 compatibility?">
<icon BUILTIN="yes"/>
<node CREATED="1385413319581" ID="ID_315431873" MODIFIED="1385413526247">
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
