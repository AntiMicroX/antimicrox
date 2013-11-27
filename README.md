# antimicro

## Description

Graphical program used to map keyboard keys and mouse buttons to gamepad buttons.
This program is useful for playing games with no gamepad support.

## License

This program is licensed under the GPL v.3. Please read the gpl.txt text document
included with the source code if you would like to read the terms of the license.
The license can also be found online at [http://www.gnu.org/licenses/gpl.txt](http://www.gnu.org/licenses/gpl.txt).


## Build Dependencies

This program is written in C++ using the [Qt](http://qt-project.org/)
framework. A C++ compiler and a proper C++ build environment will need to be
installed on your system prior to building this program. Under Debian and
Debian-based distributions like Ubuntu, the
easiest way to get a base build environment set up is to install the
meta-package **build-essential**. The following packages are required to be
installed on your system in order to build this program:

* g++
* libqt4-dev
* libsdl1.2-dev
* libxtst-dev
* libX11-dev


## Building under Linux

In order to build this program, open a terminal and cd into the antimicro/src
directory. Enter the following commands in order to build the program:

* cd antimicro/src
* qmake
* make
* make updateqm
* sudo make install

The installation path of antimicro can be customized by specifying the
INSTALL_PREFIX variable for qmake.

* qmake INSTALL_PREFIX=/usr

This will install the antimicro executable to /usr/bin/antimicro.
By default, the executable will be installed to /usr/local/bin/antimicro.

Also, running "make updateqm" is only required if you would like to enable
translations for the application.

## Building under Windows

**Instructions provided by aybe @ https://github.com/aybe**

* You will need Qt with MinGW support: http://qt-project.org/downloads

* download SDL development package : http://www.libsdl.org/release/SDL-devel-1.2.15-mingw32.tar.gz

* open the archive and drop the 'SDL-1.2.15' folder in the 'antimicro' folder

* open the project (antimicro.pro) in Qt Creator

* the first-time wizard selects MSVC as the default compiler, untick it and select MinGW instead

* set build to 'Release' (menu Build -> Open Build and Run Kit Selector) and 'Build All' (Ctrl+Shift+B)

* a 'build-antimicro-Desktop_Qt_5_1_1_MinGW_32bit-release/release' folder appears in 'antimicro' folder

* the application will need SDL.DLL, find it in SDL-1.2.15\bin folder and copy it next to it

* it will also need the following Qt DLLs that you will find inside C:\Qt\5.1.1\mingw48_32\bin

        icudt51.dll
        icuin51.dll
        icuuc51.dll
        libgcc_s_dw2-1.dll
        libstdc++-6.dll
        libwinpthread-1.dll
        Qt5Core.dll
        Qt5Gui.dll
        Qt5Network.dll
        Qt5Widgets.dll

* copy them next to the application executable

* last dependency is the Qt plugin platform 'qwindows.dll' located in C:\Qt\5.1.1\mingw48_32\plugins\platforms\

* create a sub-folder named 'platforms' and copy it there

* you have just built AntiMicro for Windows :D


## Ideas For Future Features

This section is where some of the ideas for future features
for this program will be written. Since the 1.0 release,
there has not been a plan for what to implement in future
versions of AntiMicro. Hopefully, this will help change that.

* Gamepad Status window. A window to show buttons presses and current axes values.
* ~~DONE. Ensure SDL 2 compatibility. Looking at the docs for SDL 2, it seems
like the main API has not really changed so AntiMicro should be compatible.~~
* Setting to not display buttons that are not currently in use.
This will especially help people using a PS3 controller.
* Copy set assignments to a new set. This will make creating
slightly altered sets easier.
* ~~DONE. Spring mode preview. There should be some way in the program
to show the area that spring mode will use based on the settings
specified. Either that or the program should implement something
that will allow the user to select the region on screen that
should be used; this would work like what would be seen in
screencap programs.~~
* ~~DONE. Implement mouse wheel as a type of movement instead of a button press.~~
* ~~DONE. Show identifier given by SDL along with plain joystick identifier.~~
* A mouse setting that will move the mouse cursor to an exact spot on the
screen based on the position of an axis. This would work similar to spring
mode except that it will not automatically return the mouse cursor
to the center of the screen. This idea is mainly intended for the touchpad
on the PS4 controller.

Along with this, I will put the simple mind map that I am using to
write ideas for future development into the repository for this
program. Opening the mind map will require the use of the program
FreeMind which can be downloaded from
[http://freemind.sourceforge.net/wiki/index.php/Main_Page](http://freemind.sourceforge.net/wiki/index.php/Main_Page).

## Shoutout

A big inspiration for this program was the program QJoyPad
([http://qjoypad.sourceforge.net/](http://qjoypad.sourceforge.net/)).
I have been a user of the program for years and it is unfortunate that the
program is no longer being maintained. The source code for QJoyPad was an
invaluable resource when I made the first version of this program and the UI
for this program mostly resembles QJoyPad.

## Credits

Travis Nickles <nickles.travis@gmail.com>

## Translators

Belleguic Terence <hizo@free.fr> - French  
Jay Alexander Fleming <tito.nehru.naser@gmail.com> - Serbian  
VaGNaroK <vagnarokalkimist@gmail.com> - Brazilian Portuguese
 