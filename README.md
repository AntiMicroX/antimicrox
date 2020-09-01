# <img src="./src/images/antimicroX.png" alt="Icon" width="60"/> AntiMicroX 3.0.1 


1. [Description](#description)  
2. [License](#license)  
3. [Command Line](#command-line) 
4. [Install](#installation)
5. [Wiki](#wiki)
6. [Testing Under Linux](#testing-under-linux)  
7. [AntiMicroX Profiles](#antimicrox-profiles)
8. [Support](#support)  
9. [Bugs](#bugs)  
10. [Shoutout](#shoutout)  
11. [Credits](#credits)  
12. [Participation in the translation of AntiMicroX](#participation-in-the-translation-of-antimicrox)

<br/>

## Description

antimicroX is a graphical program used to map keyboard keys and mouse controls
to a gamepad. This program is useful for playing PC games using a gamepad that
do not have any form of built-in gamepad support. However, you can use this 
program to control any desktop application with a gamepad; on Linux, this
means that your system has to be running an X environment in order to run
this program.

This program is currently supported under various Linux
distributions.

This application is continuation of project called `AntiMicro`, which was later abandoned and revived by juliagoda.

Legacy repositories:
- First AntiMicroX repository: https://github.com/juliagoda/antimicroX
- Second, maintained by organization: https://github.com/AntiMicro/antimicro
- First, original AntiMicro repository: https://github.com/Ryochan7/antimicro

**Screenshots:**
<table border="0px" >
<tr><td>

<img src="./other/appdata/screenshots/controller_main_window_dark_theme_1.png" alt="Main Window" />
  </td><td>
    <img src="./other/appdata/screenshots/controller_mapping_dark_theme.png" alt="Mapping" />
  </td></tr>

  <tr><td>
    <img src="./other/appdata/screenshots/controller_callibration_dark_theme.png" alt="Calibration" />

  </td><td>
    <img src="./other/appdata/screenshots/controller_configure_dark_theme.png" alt="Configuration" />
  </td></tr>
</table>
<br/>


## License

This program is licensed under the GPL v.3. Please read the LICENSE text document
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
                                   enabled xtest and uinput options on Linux. Default: xtest.
    -l, --list                     Print information about joysticks detected by
                                   SDL. Use only if you have sdl library. You can 
                                   check your controller index, name or 
                                   even GUID.  

<br/>


## Installation

### Fedora
```
dnf install antimicroX
```

### Arch Linux or Arch Linux based distributions:

```
trizen -S antimicrox-git
```

### Debian/Ubuntu-based distributions:
Download from the [release site](https://github.com/AntiMicroX/antimicroX/releases) and install `.deb` package.

### AppImage
Download from the [release site](https://github.com/AntiMicroX/antimicroX/releases).
<br/>It is recommended to use [AppImageLauncher](https://github.com/TheAssassin/AppImageLauncher) with this package.

<br/>

## Wiki

[Look here](https://github.com/juliagoda/antimicroX/wiki)

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

## AntiMicroX Profiles

If you would like to send the profile you are using for your application or find something 
for yourself, [here](https://github.com/AntiMicroX/antimicroX-profiles) is the forked repository. If you want to report a bug, ask 
a question or share a suggestion, you can do that on the antimicroX page or on the
[antimicroX-profiles](https://github.com/AntiMicroX/antimicroX-profiles) page.

<br/>

## Support

In order to obtain support, you can post an issue [here](https://github.com/AntiMicroX/antimicroX/issues).

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

Full list is available [here](https://github.com/AntiMicroX/antimicroX/graphs/contributors)

<br/>

### Original Developer

Travis Nickles <nickles.travis@gmail.com>

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
WAZAAAAA - <wazaaaaa00@gmail.com> - Italian  
juliagoda <juliagoda.pl@protonmail.com> - Polish

<br/>

## Participation in the translation of AntiMicroX

If you want to participate in the translation of the program, but your language is not yet on the support list. Please contact us.

However, if your language is already supported in the program, you must have the Qt Linguist program installed, which is usually included in the qt5-tools package, to conveniently translate the application. However, before you do that, make sure that you don't already have this program installed. All you have to do is open it, and then use it to open a file with the ts extension.

If a red exclamation mark appears in the translation field, it means that your translation does not end with the same sign as the original content. Try to place commas or dots in appropriate places and often save your progress.
