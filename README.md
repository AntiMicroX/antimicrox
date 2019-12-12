# antimicro 2.24.2


1. [Description](#description)  
2. [License](#license)  
3. [Command Line](#command-line)  
4. [Build Dependencies](#build-dependencies)  
    a. [Building Under Linux](#building-under-linux)  
    b. [Building Under Windows](#building-under-windows)  
    c. [Building with Flatpak](#building-with-flatpak)  
5. [Testing Under Linux](#testing-under-linux)  
6. [Support](#support)  
7. [Bugs](#bugs)  
8. [Shoutout](#shoutout)  
9. [Credits](#credits)  



## Description

antimicro is a graphical program used to map keyboard keys and mouse controls
to a gamepad. This program is useful for playing PC games using a gamepad that
do not have any form of built-in gamepad support. However, you can use this 
program to control any desktop application with a gamepad; on Linux, this
means that your system has to be running an X environment in order to run
this program.

This program is currently supported under various Linux
distributions, Windows (Vista and later), and FreeBSD. However functionality of application
has been tested only on Linux (Arch Linux and Ubuntu). More systems will be tested in the future.

Informations about all developers from AntiMicro team and main creator (Ryochan7) are included in 
application. Their versions are here:

https://github.com/AntiMicro/antimicro  
https://github.com/Ryochan7/antimicro  


## License

This program is licensed under the GPL v.3. Please read the gpl.txt text document
included with the source code if you would like to read the terms of the license.
The license can also be found online at
http://www.gnu.org/licenses/gpl.txt



## Command Line

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
    -d, --daemon                   Launch program as a daemon. Use only on Linux.
    --log-level (debug|info)       Enable logging. Default: debug.
    --log-file <filename>          Choose a file for writing logs
    --eventgen (xtest|uinput)      Choose between using XTest support and uinput
                                   support for event generation. Use only if you have
                                   enabled xtest and uinput options on Linux or vmulti
                                   on Windows. Default: xtest.
    -l, --list                     Print information about joysticks detected by
                                   SDL. Use only if you have sdl library. You can 
                                   check your controller index, name or 
                                   even GUID.



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
* extra-cmake-modules
* qttools5-dev and qttools5-dev-tools (qt5-tools on distros based on Arch Linux) (Qt5 support)
* libsdl2-dev (sdl2 on distros based on Arch Linux) (SDL2)
* libxi-dev (libxi on distros based on Arch Linux) (Optional. Needed to compile with X11 and uinput support)
* libxtst-dev (libxtst on distros based on Arch Linux) (Optional. Needed to compile with XTest support)
* libx11-dev (libx11 on distros based on Arch Linux) (Needed to compile with Qt5 support)


### Building Under Linux

In order to build this program, open a terminal and cd into the antimicro
directory. Enter the following commands in order to build the program:

    cd antimicro
    mkdir build && cd build
    cmake ..
    make
    sudo make install
    
If you're an Arch Linux or Arch Linux based distribution user:

```
git clone https://github.com/archlinux-lucjan/archlinux-packages.git
cd antimicro-git
makepkg -sric
```

The cmake step will use pkg-config to attempt to find SDL
libraries that you have installed. The project is set up to
look for a copy of SDL2. This behavior should work
fine for most people. You can override this behavior by using the -DUSE_SDL_2
option when you run cmake. Using -DUSE_SDL_2=ON when you run cmake will mean
that you want antimicro compiled with SDL2 support. However you don't have to do that,
because the option is set as default in CMakeLists.txt.


### Building Under Windows

*Instructions provided by aybe @ https://github.com/aybe
and modified by Travis Nickles.*

* Download and install CMake: http://www.cmake.org/cmake/resources/software.html .

* You will need Qt with MinGW support: https://www.qt.io/download-open-source/ . The
current stable version of Qt that is being used to create builds is 5.10.0 .

* Download SDL development package : https://www.libsdl.org/release/SDL2-devel-2.0.8-mingw.tar.gz .

* Open the archive and drop the 'SDL2-2.0.8' folder in the 'antimicro' folder.

* Open the project (CMakeLists.txt) in Qt Creator. The CMake Wizard will appear
the first time you open the project in Qt Creator.

* Choose a Build Location. The recommendation is to create a "build" folder
under the root antimicro folder and choose that for the build location.

* In the Run CMake section, in the Arguments field, please input
```-DCMAKE_PREFIX_PATH=<Path to MinGW Qt install>```
```-DCMAKE_BUILD_TYPE=Release```. Replace "```<Path to MinGW Qt install>```"
with the actual path to your Qt installation. The default path for version
Qt 5.10.0 is C:\Qt\Qt5.10.0\mingw53_32 .

* Choose "MinGW Generator" for the Generator option in the Run CMake section.

* Click the Run CMake button and then click Finish.

* In the main IDE window, open the Build menu and select "Build All" (Ctrl+Shift+B).

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

##### 64-bit Build

* Some additional steps are required in order to compile a 64-bit version of
antimicro. The first step is to download a packaged version of Qt, MSYS shell environment,
the MinGW 64 C++ compiler, and a series of libraries @
https://github.com/iat-cener/tonatiuh/wiki/Installing-MinGW-For-Windows64 .

* You will have to manually create a new Kit in Qt Creator. In the main Qt
Creator window, click the "Projects" button in the sidebar to bring up the
"Build Settings" page for the project. Click on the "Manage Kits" button
near the top of the page. Manually add the 64-bit compiled Qt version under
"Qt Versions", add the 64-bit MinGW under "Compilers", and add the 64 bit
gdb.exe under "Debuggers".

* After creating a new kit in Qt Creator, bring up the "Build Settings" page
for the project. Hover over the currently selected kit name and click the
arrow that appears, hover over "Change Kit" and select the proper 64-bit kit
that you created earlier.

* Perform a clean on the project or delete the build directory that CMake is
using. After that, choose the "Run CMake" option under the "Build" menu entry.
The arguments that you pass to CMake will have to be changed. You will have
to edit ```-DCMAKE_PREFIX_PATH=<Path to 64 bit MinGW Qt install>``` variable
and have it point to the 64 bit compiled version Qt. Also, make sure to add
```-DTARGET_ARCH=x86_64``` so that CMake will use the proper SDL libraries while
building the program and copy the proper Qt and SDL DLLs if you perform an
**install_dlls**.

##### Building The Windows Installer Package (MSI)

*These instructions have been tested with WiX 3.8*

* You need to have WiX installed, grab it at http://wixtoolset.org/ .

* The building process relies on the WIX environment, it is recommended that you download the installer instead of the binaries as it it will set it up for you.

* If Qt Creator is running while you install or upgrade to a newer version then make sure to restart it as it will either not find that environment variable or fetch the old (incorrect) value from the previous version.

* To build the MSI package, click on the "Projects" icon in the sidebar,
click the "Details" button on the make entry, uncheck all other options
and check the "buildmsi" box.

* Currently it relies on INSTALL to copy files at the location they are harvested, this might change in the future.

Notes about the WXS file and the building process:

* The WXS file has been generated with WixEdit and manually modified to contain relative paths, it will only work from the 'windows' sub-folder (or any other).

* WixCop can be run against the WXS file and it should not point out any errors as the WXS has been corrected previously with the -F switch.

* CNDL1113 warning : shortucts are advertised, left as-is as a nice feature about them is that if the program gets corrupted it will be repaired by Windows Installer, by design the shortcuts will not point to antimicro.exe as a regular LNK file.

* LGHT1073 warning : SDL2.DLL does not specify its language in the language column, not a big deal; it could be recompiled but it's pretty much a time waste as it would only prevent this warning.

* All of these warnings have been made silent through the use of command-line switches.

* Built MSI package will be placed in /windows .


### Building with Flatpak

#### Additional Dependencies

* flatpak 
* flatpak-builder


#### Compilation & Installation

This tutorial is about installing antimicro with flatpak locally. As first you need a com.github.juliagoda.antimicro.json file, that is placed in main folder. You can for example copy and paste content to local file. Link to raw content of json file is [here](https://raw.githubusercontent.com/juliagoda/antimicro/master/com.github.juliagoda.antimicro.json).


1. Download and install Flatpak repo:

`flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo`

2. Add a runtime with Qt and all KDE Frameworks 5

`flatpak install flathub org.kde.Platform//5.11 org.kde.Sdk//5.11`

3. Create new directories for building and creating repo

```
mkdir build-dir
mkdir repo
```

4. Build antimicro

`flatpak-builder build-dir com.github.juliagoda.antimicro.json`

5. Test the build of application

`flatpak-builder --run build-dir com.github.juliagoda.antimicro.json antimicro`

6. Create repository for application

`flatpak-builder --repo=repo --force-clean build-dir com.github.juliagoda.antimicro.json`

7. Add the repository to flatpak locally

`flatpak --user remote-add --no-gpg-verify antimicro repo`

8. Install application

`flatpak --user install antimicro com.github.juliagoda.antimicro`


#### Run antimicro

If you have installed antimicro locally with success, you can run application:

`flatpak run com.github.juliagoda.antimicro`


#### Updating

To update all your installed applications and runtimes to the latest version, execute:

`flatpak update`


#### Uninstall antimicro

1. Delete a remote repository

`flatpak --user remote-delete antimicro`

2. Delete application

`flatpak uninstall com.github.juliagoda.antimicro`



## Testing Under Linux

If you are having problems with antimicro detecting a controller or
detecting all axes and buttons, you should test the controller outside of
antimicro to check if the problem is with antimicro or not. The two endorsed
programs for testing gamepads outside of antimicro are **sdl-jstest**
(**sdl2-jstest**) and **evtest**. SDL2 utilizes evdev on Linux so performing
testing with older programs that use joydev won't be as helpful since some
devices behave a bit differently between the two systems. Another method also exist, 
which can be found [here](https://github.com/juliagoda/SDL_JoystickButtonNames).

## Support

In order to obtain support, you can post an issue [here](https://github.com/juliagoda/antimicro/issues) .

## Bugs

Application's bugs will be fixed. There are created tests for detecting bugs. Of course, I'm open to proposals or questions from users. 


## Shoutout

A big, original inspiration for this program was the program QJoyPad.


## Credits

### Contributors

juliagoda

### Original Developer

Travis Nickles <nickles.travis@gmail.com>

### Old Contributors

Zerro Alvein  
aybe  
Jeff Backus <jeff@jsbackus.com>  
Arthur Moore  
Anton Tornqvist <antont@inbox.lv>
mtdeguzis

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
juliagoda <juliagoda.pl@protonmail.com> - Polish
