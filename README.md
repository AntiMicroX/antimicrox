# antimicro

## We've Moved!

As of May 24, 2016, antimicro has moved from
[https://github.com/Ryochan7/antimicro](https://github.com/Ryochan7/antimicro) to
[https://github.com/AntiMicro/antimicro](https://github.com/AntiMicro/antimicro).
Additionally, project management has passed from Travis (Ryochan7) to the AntiMicro
organization due to Travis having other interests and priorities.

So, thank you for your patience as we settle in. And a special thank you to the
following GitHub users who have helped us make the transition:
* 7185
* DarkStarSword
* est31
* ProfessorKaos64
* qwerty12
* WAZAAAAA0
* zzpxyx

## Description

antimicro is a graphical program used to map keyboard keys and mouse controls
to a gamepad. This program is useful for playing PC games using a gamepad that
do not have any form of built-in gamepad support. However, you can use this 
program to control any desktop application with a gamepad; on Linux, this
means that your system has to be running an X environment in order to run
this program.

This program is currently supported under various Linux
distributions, Windows (Vista and later), and FreeBSD. At the time of writing
this, antimicro works in Windows XP but, since Windows XP is no longer
supported, running the program in Windows XP will not be officially supported.
However, efforts will be made to not intentionally break compatibility
with Windows XP.

Also, FreeBSD support will be minimal for now. I don't use BSD on
a daily basis so the main support for FreeBSD is being offered by Anton. He
has graciously made a port of antimicro for FreeBSD that you can find
at the following URL: [http://www.freshports.org/x11/antimicro/](http://www.freshports.org/x11/antimicro/).

## License

This program is licensed under the GPL v.3. Please read the gpl.txt text document
included with the source code if you would like to read the terms of the license.
The license can also be found online at
[http://www.gnu.org/licenses/gpl.txt](http://www.gnu.org/licenses/gpl.txt).

## Download
Source code archives and Windows binaries are available from the antimicro
Releases section on GitHub:

[https://github.com/AntiMicro/antimicro/releases](https://github.com/AntiMicro/antimicro/releases)

AntiMicro is currently in the official Fedora repository and can be installed with

`$ sudo dnf install antimicro`

For Debian and Debian-based distributions, such as Mint, Ubuntu, and Steam OS, please check the LibreGeek
Repositories generously hosted by ProfessorKaos64:

[http://packages.libregeek.org/](http://packages.libregeek.org/)

AntiMicro is currently available for Slackware via SlackBuilds, thanks to NK and Klaatu:

[https://slackbuilds.org/result/?search=antimicro&sv=](https://slackbuilds.org/result/?search=antimicro&sv=)

Ubuntu users may also check the antimicro page on Launchpad:

[https://launchpad.net/~ryochan7/+archive/ubuntu/antimicro](https://launchpad.net/~ryochan7/+archive/ubuntu/antimicro)

## Command line

    Usage: antimicro [options] [profile]

    Options:
    -h, --help                     Print help text.
    -v, --version                  Print version information.
    --tray                         Launch program in system tray only.
    --no-tray                      Launch program with the tray menu disabled.
    --hidden                       Launch program without the main window
                                   displayed.
    --profile <location>           Launch program with the configuration file
                                   selected as the default for selected
                                   controllers. Defaults to all controllers.
    --profile-controller <value>   Apply configuration file to a specific
                                   controller. Value can be a
                                   controller index, name, or GUID.
    --unload [<value>]             Unload currently enabled profile(s).
                                   Value can be a controller index, name, or GUID.
    --startSet <number> [<value>]  Start joysticks on a specific set.
                                   Value can be a controller index, name, or GUID.
    -d, --daemon                   Launch program as a daemon.
    --log-level (debug|info)       Enable logging.
    --eventgen (xtest|uinput)      Choose between using XTest support and uinput
                                   support for event generation. Default: xtest.
    -l, --list                     Print information about joysticks detected by
                                   SDL.
    --map <value>                  Open game controller mapping window of selected
                                   controller. Value can be a controller index or
                                   GUID.

## Pre-made Profiles

There is a repository for pre-made antimicro profiles. Using a pre-made
profile, you can have a controller layout that is suitable for playing a
game without having to map everything yourself. It makes using antimicro
really convenient. In order to use those pre-made profiles, you have to be
running at least antimicro version 2.0 and antimicro must have been compiled
with SDL 2 support.

[https://github.com/AntiMicro/antimicro-profiles](https://github.com/AntiMicro/antimicro-profiles)

## Wiki

An effort is now being done to utilize the Wiki section on GitHub more.
Please check out the Wiki at [https://github.com/AntiMicro/antimicro/wiki](https://github.com/AntiMicro/antimicro/wiki)
to read various help pages that have been created. The Wiki is currently
open to all GitHub users so feel free to add a new page or modify an
existing page.

## Build Dependencies

This program is written in C++ using the [Qt](https://www.qt.io/)
framework. A C++ compiler and a proper C++ build environment will need to be
installed on your system prior to building this program. Under Debian and
Debian-based distributions like Ubuntu, the
easiest way to get a base build environment set up is to install the
meta-package **build-essential**. The following packages are required to be
installed on your system in order to build this program:

* g++
* cmake
* libqt4-dev (Qt 4 support) or qttools5-dev and qttools5-dev-tools (Qt 5
support)
* libsdl1.2-dev (SDL 1.2) or libsdl2-dev (SDL 2)
* libxi-dev (optional. Needed to compile with X11 and uinput support)
* libxtst-dev (optional. Needed to compile with XTest support)
* libX11-dev (optional when compiled with Qt 5 support)

## Building under Linux

In order to build this program, open a terminal and cd into the antimicro
directory. Enter the following commands in order to build the program:

    cd antimicro
    mkdir build && cd build
    cmake ..
    make
    sudo make install

The installation path of antimicro can be customized by specifying the
CMAKE_INSTALL_PREFIX variable while running cmake.

    cmake -DCMAKE_INSTALL_PREFIX=/usr ..

This will install the antimicro executable to /usr/bin/antimicro.
By default, the executable will be installed to /usr/local/bin/antimicro.

The cmake step will use pkg-config to attempt to find any SDL
libraries that you have installed. The project is set up to
look for a copy of SDL 2 followed by SDL 1.2. This behavior should work
fine for most people. You can override this behavior by using the -DUSE_SDL_2
option when you run cmake. Using -DUSE_SDL_2=ON when you run cmake will mean
that you want antimicro compiled with SDL 2 support. Using -DUSE_SDL_2=OFF when
you run cmake will mean that you want antimicro compiled with SDL 1.2 support.

Here is an example of how to specify that you want antimicro to be compiled
with SDL 2 support when you run qmake.

    cmake -DUSE_SDL_2=ON ..

## Building under Windows

**Instructions provided by aybe @ https://github.com/aybe.
Modified by Travis Nickles.**

* Download and install CMake: http://www.cmake.org/cmake/resources/software.html

* You will need Qt with MinGW support: https://www.qt.io/download-open-source/. The
current version of Qt that is being used to create builds is 5.6.0.

* download SDL development package : http://www.libsdl.org/release/SDL2-devel-2.0.3-mingw.tar.gz

* open the archive and drop the 'SDL2-2.0.3' folder in the 'antimicro' folder

* open the project (CMakeLists.txt) in Qt Creator The CMake Wizard will appear
the first time you open the project in Qt Creator.

* Choose a Build Location. The recommendation is to create a "build" folder
under the root antimicro folder and choose that for the build location.

* In the Run CMake section, in the Arguments field, please input
```-DCMAKE_PREFIX_PATH=<Path to MinGW Qt install>```
-DCMAKE_BUILD_TYPE=Release. Replace "```<Path to MinGW Qt install>```"
with the actual path to your Qt installation. The default path for version
Qt 5.6.0 is C:\Qt\Qt5.6.0\5.6\mingw49_32\.

* Choose "MinGW Generator" for the Generator option in the Run CMake section

* Click the Run CMake button and then click Finish

* In the main IDE window, open the Build menu and select "Build All" (Ctrl+Shift+B)

* The application will need SDL2.DLL. A build step has been added to
CMakeLists.txt in order to automate the process. Click the "Projects" icon
in the sidebar to bring up the "Build Settings" section. Within
"Build Steps", click the "Details" button on the Make entry. In the expanded
menu, uncheck the "all" checkbox and then check the "copy_sdl_dll" checkbox
and run "Build All".

* At this point, antimicro has been built for Windows and is runnable
from Qt Creator. A couple more steps are required in order to make a
distributable package.

* Under "Build Settings", expand the Make menu and check the "install" and
"install_dlls" checkboxes.

* Under the "Build" menu in the main window, select "Run CMake" and
add ```-DCMAKE_INSTALL_PREFIX=<DIR>``` option and replace ```<DIR>``` with the directory
that you want to install the application. The default for me is
C:\Program Files (x86)\AntiMicro\ although I use a different directory
when bundling the Window version for other users.

* Run "Build All" to have the application and required DLLs installed
into the final location that will be ready for distribution.

### 64 bit build

* Some additional steps are required in order to compile a 64 bit version of
antimicro. The first step is to download a packaged version of Qt and MinGW
compiled for 64 bit from the Qt-x64 project @
http://sourceforge.net/projects/qtx64/.

* You will have to manually create a new Kit in Qt Creator. In the main Qt
Creator window, click the "Projects" button in the sidebar to bring up the
"Build Settings" page for the project. Click on the "Manage Kits" button
near the top of the page. Manually add the 64 bit compiled Qt version under
"Qt Versions", add the 64 bit MinGW under "Compilers", and add the 64 bit
gdb.exe under "Debuggers".

* After creating a new kit in Qt Creator, bring up the "Build Settings" page
for the project. Hover over the currently selected kit name and click the
arrow that appears, hover over "Change Kit" and select the proper 64 bit kit
that you created earlier.

* Perform a clean on the project or delete the build directory that CMake is
using. After that, choose the "Run CMake" option under the "Build" menu entry.
The arguments that you pass to CMake will have to be changed. You will have
to edit ```-DCMAKE_PREFIX_PATH=<Path to 64 bit MinGW Qt install>``` variable
and have it point to the 64 bit compiled version Qt. Also, make sure to add
```-DTARGET_ARCH=x86_64``` so that CMake will use the proper SDL libraries while
building the program and copy the proper Qt and SDL DLLs if you perform an
**install_dlls**.

## Building the Windows Installer Package (MSI)

(these instructions have been tested with WiX 3.8)

* you need to have WiX installed, grab it at http://wixtoolset.org/

* the building process relies on the WIX environment, it is recommended that you download the installer instead of the binaries as it it will set it up for you

* if Qt Creator is running while you install or upgrade to a newer version then make sure to restart it as it will either not find that environment variable or fetch the old (incorrect) value from the previous version

* to build the MSI package, click on the "Projects" icon in the sidebar,
click the "Details" button on the make entry, uncheck all other options
and check the "buildmsi" box.

* currently it relies on INSTALL to copy files at the location they are harvested, this might change in the future

Notes about the WXS file and the building process :

* the WXS file has been generated with WixEdit and manually modified to contain relative paths, it will only work from the 'windows' sub-folder (or any other)

* WixCop can be run against the WXS file and it should not point out any errors as the WXS has been corrected previously with the -F switch

* CNDL1113 warning : shortucts are advertised, left as-is as a nice feature about them is that if the program gets corrupted it will be repaired by Windows Installer, by design the shortcuts will not point to antimicro.exe as a regular LNK file

* LGHT1073 warning : SDL2.DLL does not specify its language in the language column, not a big deal; it could be recompiled but it's pretty much a time waste as it would only prevent this warning

* all of these warnings have been made silent through the use of command-line switches.

* built MSI package will be placed in /windows

## Testing under Linux

If you are having problems with antimicro detecting a controller or
detecting all axes and buttons, you should test the controller outside of
antimicro to check if the problem is with antimicro or not. The two endorsed
programs for testing gamepads outside of antimicro are **sdl-jstest**
(**sdl2-jstest**) and **evtest**. SDL 2 utilizes evdev on Linux so performing
testing with older programs that use joydev won't be as helpful since some
devices behave a bit differently between the two systems.

[https://github.com/Grumbel/sdl-jstest/](https://github.com/Grumbel/sdl-jstest/)

## Support

In order to obtain support, you can post an issue on the antimicro GitHub page
or you can email me at jeff@jsbackus.com. Please include **antimicro**
somewhere in the subject line of the email message or it might be skipped.

[https://github.com/AntiMicro/antimicro](https://github.com/AntiMicro/antimicro)

## Ideas For Future Features

This section is where some of the ideas for future features
for this program will be written.

* Allow buttons to be bound to actions.
* Use uinput by default and fallback to XTest if necessary. (MOSTLY DONE)
* Move simulated event generation to a new thread.
* ~~Allow logging as long as it doesn't cause button lag.~~
* Allow notes to be added to a profile in various places.

Along with this, I will put the simple mind map that I am using to
write ideas for future development into the repository for this
program. The mind map will include extra notes that are not available in
this README. Opening the mind map will require the use of the program
FreeMind which can be downloaded from
[http://freemind.sourceforge.net/wiki/index.php/Main_Page](http://freemind.sourceforge.net/wiki/index.php/Main_Page).

## Translating

New translations as well as updates to current translations are always welcome.
Please refer to
[https://github.com/AntiMicro/antimicro/wiki/Translating-AntiMicro](https://github.com/AntiMicro/antimicro/wiki/Translating-AntiMicro)

## Shoutout

A big inspiration for this program was the program QJoyPad
([http://qjoypad.sourceforge.net/](http://qjoypad.sourceforge.net/)).
I was a user of the program for years and it is unfortunate that the
program is no longer being maintained. The source code for QJoyPad was an
invaluable resource when I made the first version of this program and the UI
for this program mostly resembles QJoyPad.

## Credits

### Original Developer
Travis Nickles <nickles.travis@gmail.com>

### Contributors

Zerro Alvein  
aybe  
Jeff Backus <jeff@jsbackus.com>  
Arthur Moore  
Anton Tornqvist <antont@inbox.lv>

### Translators

VaGNaroK <vagnarokalkimist@gmail.com> - Brazilian Portuguese  
zzpxyx - Chinese  
Belleguic Terence <hizo@free.fr> - French  
Leonard Koenig <leonard.r.koenig@googlemail.com> - German  
phob - German  
tou omiya - Japanese  
Dmitriy Koshel <form.eater@gmail.com> - Russian  
Jay Alexander Fleming <tito.nehru.naser@gmail.com> - Serbian  
burunduk - Ukrainian  
Flavio HR <flavio.hrx@gmail.com> - Spanish  
WAZAAAAA - wazaaaaa00<@>gmail<.>com - Italian  
