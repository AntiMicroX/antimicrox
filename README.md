# antimicroX 2.25

## this program is not a part of the official AntiMicro, just like I never was. I cannot help with problems with version 2.23 and below


1. [Description](#description)  
2. [License](#license)  
3. [Command Line](#command-line) 
4. [Wiki](#wiki)
5. [Build Dependencies](#build-dependencies)  
    a. [Building Under Linux](#building-under-linux)  
    b. [Running With Docker](#running-with-docker)  
    c. [Building with Flatpak](#building-with-flatpak)  
6. [Testing Under Linux](#testing-under-linux)  
7. [Support](#support)  
8. [Bugs](#bugs)  
9. [Shoutout](#shoutout)  
10. [Credits](#credits)  

<br/>

## Description

antimicroX is a graphical program used to map keyboard keys and mouse controls
to a gamepad. This program is useful for playing PC games using a gamepad that
do not have any form of built-in gamepad support. However, you can use this 
program to control any desktop application with a gamepad; on Linux, this
means that your system has to be running an X environment in order to run
this program.

This program is currently supported under various Linux
distributions, Windows (Vista and later), and FreeBSD. However functionality of application
has been tested only on Linux (Arch Linux and Ubuntu). More systems will be tested in the future.

Informations about all developers from AntiMicro team and main creator (Ryochan7) are included in 
application. Their **old versions** are here:

https://github.com/AntiMicro/antimicro  
https://github.com/Ryochan7/antimicro  

<br/>

## License

This program is licensed under the GPL v.3. Please read the gpl.txt text document
included with the source code if you would like to read the terms of the license.
The license can also be found online at
http://www.gnu.org/licenses/gpl.txt

<br/>

## Command Line

    Usage: antimicroX [options] [profile]

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

<br/>

## Wiki

[Look here](https://github.com/juliagoda/antimicroX/wiki)

<br/>

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
* itstool (extracts messages from XML files and outputs PO template files, then merges translations from MO files to create translated XML files)
* gettext
* libqt5x11extras5-dev

<br/>

### Building Under Linux

In order to build this program, open a terminal and cd into the antimicroX
directory. Enter the following commands in order to build the program:

    cd antimicroX
    mkdir build && cd build
    cmake ..
    make
    sudo make install
    
or

```
cd antimicrox
cmake --build build
cmake --install build
```


If you're an Arch Linux or Arch Linux based distribution user:

```
git clone https://github.com/archlinux-lucjan/archlinux-packages.git
cd antimicrox-git
makepkg -sric
```

or

```
trizen -S antimicrox-git
```

from [AUR](https://aur.archlinux.org/packages/antimicrox-git/)

 **Distro** | **Maintainer** | **Package**  |
| :--------- | :------------- | :----------- |
| Arch Linux | Piotr Górski   | [antimicrox-git](https://aur.archlinux.org/packages/antimicrox-git) <sup>AUR</sup> |


#### Building deb package
Already built .deb files are available on [Release Page](https://github.com/juliagoda/antimicroX/releases)

```
    cd antimicroX
    mkdir build && cd build
    cmake ..
    cpack
```


<br/>

### Running with Docker

If you want to run application without building process and choose between various distributions, then [look here](https://hub.docker.com/r/juliagoda/antimicrox)

<br/>

### Building with Flatpak

#### Additional Dependencies

* flatpak 
* flatpak-builder


#### Compilation & Installation

This tutorial is about installing antimicroX with flatpak locally. As first you need a com.github.juliagoda.antimicroX.json file, that is placed in main folder. You can for example copy and paste content to local file. Link to raw content of json file is [here](https://raw.githubusercontent.com/juliagoda/antimicroX/master/com.github.juliagoda.antimicroX.json).


1. Download and install Flatpak repo:

`flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo`

2. Add a runtime with Qt and all KDE Frameworks 5

`flatpak install flathub org.kde.Platform//5.11 org.kde.Sdk//5.11`

3. Create new directories for building and creating repo

```
mkdir build-dir
mkdir repo
```

4. Build antimicroX

`flatpak-builder build-dir com.github.juliagoda.antimicroX.json`

5. Test the build of application

`flatpak-builder --run build-dir com.github.juliagoda.antimicroX.json antimicroX`

6. Create repository for application

`flatpak-builder --repo=repo --force-clean build-dir com.github.juliagoda.antimicroX.json`

7. Add the repository to flatpak locally

`flatpak --user remote-add --no-gpg-verify antimicroX repo`

8. Install application

`flatpak --user install antimicroX com.github.juliagoda.antimicroX`


#### Run antimicroX

If you have installed antimicroX locally with success, you can run application:

`flatpak run com.github.juliagoda.antimicroX`



#### Updating

To update all your installed applications and runtimes to the latest version, execute:

`flatpak update`


#### Uninstall antimicroX

1. Delete a remote repository

`flatpak --user remote-delete antimicroX`

2. Delete application

`flatpak uninstall com.github.juliagoda.antimicroX`

<br/>

## Testing Under Linux

If you are having problems with antimicroX detecting a controller or
detecting all axes and buttons, you should test the controller outside of
antimicroX to check if the problem is with antimicroX or not. The two endorsed
programs for testing gamepads outside of antimicroX are **sdl-jstest**
(**sdl2-jstest**) and **evtest**. SDL2 utilizes evdev on Linux so performing
testing with older programs that use joydev won't be as helpful since some
devices behave a bit differently between the two systems. Another method also exist, 
which can be found [here](https://github.com/juliagoda/SDL_JoystickButtonNames).

<br/>

## Support

In order to obtain support, you can post an issue [here](https://github.com/juliagoda/antimicroX/issues) .

<br/>

## Bugs

Application's bugs will be fixed. There are created tests for detecting bugs. Of course, I'm open to proposals or questions from users. 

<br/>

## Shoutout

A big, original inspiration for this program was the program QJoyPad.

<br/>

## Credits

<br/>

### Contributors

juliagoda

<br/>

### Original Developer

Travis Nickles <nickles.travis@gmail.com>

<br/>

### Old Contributors

Zerro Alvein  
aybe  
Jeff Backus <jeff@jsbackus.com>  
Arthur Moore  
Anton Tornqvist <antont@inbox.lv>
mtdeguzis

<br/>

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
