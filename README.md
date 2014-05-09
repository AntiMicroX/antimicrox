# antimicro

## Description

AntiMicro is a graphical program used to map keyboard keys and mouse controls
to a gamepad. This program is useful for playing PC games using a gamepad that
do not have any form of built-in gamepad support. However, you can use this 
program to control any desktop application with a gamepad; on Linux, this
means that your system has to be running an X environment in order to run
this program.

This program is currently supported under various Linux
distributions and Windows Vista and later. At the time of writing this,
AntiMicro works in Windows XP but, since Windows XP is no longer supported,
running the program in Windows XP will not be officially supported.
However, efforts will be made to not intentionally break compatibility
with Windows XP.

## License

This program is licensed under the GPL v.3. Please read the gpl.txt text document
included with the source code if you would like to read the terms of the license.
The license can also be found online at
[http://www.gnu.org/licenses/gpl.txt](http://www.gnu.org/licenses/gpl.txt).

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
* libsdl1.2-dev (or libsdl2-dev)
* libxtst-dev
* libX11-dev

## Building under Linux

In order to build this program, open a terminal and cd into the antimicro/src
directory. Enter the following commands in order to build the program:

    cd antimicro/src
    qmake
    make
    sudo make install

The installation path of antimicro can be customized by specifying the
INSTALL_PREFIX variable while running qmake.

    qmake INSTALL_PREFIX=/usr

This will install the antimicro executable to /usr/bin/antimicro.
By default, the executable will be installed to /usr/local/bin/antimicro.

The qmake step will use pkg-config to attempt to find any SDL
libraries that you have installed. The project is set up to
look for a copy of SDL 2 followed by SDL 1.2. This behavior should work
fine for most people. You can override this behavior by using the USE_SDL_2
variable when you run qmake. Using USE_SDL_2=1 when you run qmake will mean
that you want AntiMicro compiled with SDL 2 support. Using USE_SDL_2=0 when
you run qmake will mean that you want AntiMicro compiled with SDL 1.2 support.

Here is an example of how to specify that you want AntiMicro to be compiled
with SDL 2 support when you run qmake.

    qmake USE_SDL_2=1

## Building under Windows

**Instructions provided by aybe @ https://github.com/aybe**

* You will need Qt with MinGW support: http://qt-project.org/downloads

* download SDL development package : http://www.libsdl.org/release/SDL2-devel-2.0.3-mingw.tar.gz

* open the archive and drop the 'SDL2-2.0.3' folder in the 'antimicro' folder

* open the project (antimicro.pro) in Qt Creator

* the first-time wizard selects MSVC as the default compiler, untick it and select MinGW instead

* set build to 'Release' (menu Build -> Open Build and Run Kit Selector) and 'Build All' (Ctrl+Shift+B)

* a 'build-antimicro-Desktop_Qt_5_1_1_MinGW_32bit-release/release' folder appears in 'antimicro' folder

* the application will need SDL2.DLL, find it in SDL2-2.0.3\bin folder and copy it next to it

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

##Building the Windows Installer Package (MSI)

(these instructions have been tested with WiX 3.8)

* you need to have WiX installed, grab it at http://wixtoolset.org/

* the building process relies on the WIX environment, it is recommended that you download the installer instead of the binaries as it it will set it up for you

* if Qt Creator is running while you install or upgrade to a newer version then make sure to restart it as it will either not find that environment variable or fetch the old (incorrect) value from the previous version

* to build the MSI package, add a 'Make Build Step' after a 'make install' step to the project and specify 'buildmsi' as the argument, at your convenience you'll disable/enable it as needed

* currently it relies on INSTALL to copy files at the location they are harvested, this might change in the future

Notes about the WXS file and the building process :

* the WXS file has been generated with WixEdit and manually modified to contain relative paths, it will only work from the 'windows' sub-folder (or any other)

* WixCop can be run against the WXS file and it should not point out any errors as the WXS has been corrected previously with the -F switch

* CNDL1113 warning : shortucts are advertised, left as-is as a nice feature about them is that if the program gets corrupted it will be repaired by Windows Installer, by design the shortcuts will not point to antimicro.exe as a regular LNK file

* LGHT1073 warning : SDL2.DLL does not specify its language in the language column, not a big deal; it could be recompiled but it's pretty much a time waist as it would only prevent this warning

* all of these warnings have been made silent through the use of command-line switches.

* built MSI package will be placed in /windows

## Ideas For Future Features

This section is where some of the ideas for future features
for this program will be written.

* ~~Setting to not display buttons that are not currently in use.
This will especially help people using a PS3 controller.~~
* Copy set assignments to a new set. This will make creating
slightly altered sets easier.
* ~~Improve SteamOS support.~~
* Write documentation for portions of the program.
* ~~Add Delay slot type.~~
* Convert code base to use CMake as opposed to qmake.
* ~~Add profile name. Use filename as a backup name.~~

Along with this, I will put the simple mind map that I am using to
write ideas for future development into the repository for this
program. The mind map will include extra notes that are not available in
this README. Opening the mind map will require the use of the program
FreeMind which can be downloaded from
[http://freemind.sourceforge.net/wiki/index.php/Main_Page](http://freemind.sourceforge.net/wiki/index.php/Main_Page).

## Shoutout

A big inspiration for this program was the program QJoyPad
([http://qjoypad.sourceforge.net/](http://qjoypad.sourceforge.net/)).
I was a user of the program for years and it is unfortunate that the
program is no longer being maintained. The source code for QJoyPad was an
invaluable resource when I made the first version of this program and the UI
for this program mostly resembles QJoyPad.

## Credits

Travis Nickles <nickles.travis@gmail.com>  
aybe  
Zerro Alvein

## Translators

Belleguic Terence <hizo@free.fr> - French  
Jay Alexander Fleming <tito.nehru.naser@gmail.com> - Serbian  
VaGNaroK <vagnarokalkimist@gmail.com> - Brazilian Portuguese
