# antimicro

## Description

Graphical program used to map keyboard keys and mouse buttons to gamepad buttons.
Useful for playing games with no gamepad support.

## License

This program is licensed under the GPL v.3. Please read the gpl.txt text document
included with the source code if you would like to read the terms of the license.
The license can also be found online at [http://www.gnu.org/licenses/gpl.txt](http://www.gnu.org/licenses/gpl.txt).


## Build Dependencies

The following packages are required to be installed on your system in order to build this program:

* libqt4-dev
* libsdl1.2-dev
* libxtst-dev
* libX11-dev


## Building

In order to build this program, open a terminal and cd into the antimicro/src directory.
Enter the following commands in order to build the program:

* cd antimicro/src
* qmake
* make
* make updateqm
* sudo make install

The installation path of antimicro can be customized by specifying the INSTALL_PREFIX variable for qmake.

* qmake INSTALL_PREFIX=/usr

This will install the antimicro executable to /usr/bin/antimicro.
By default, the executable will be installed to /usr/local/bin/antimicro.

Also, running "make updateqm" is only required if you would like to enable translations
for the application.

## Shoutout

A big inspiration for this program was the program QJoyPad ([http://qjoypad.sourceforge.net/](http://qjoypad.sourceforge.net/)).
I have been a user of the program for years and it is unfortunate that the program is no longer being maintained.
The source code for QJoyPad was an invaluable resource when I made the first version of this program and the UI for this program mostly resembles QJoyPad.

## Credits

Travis Nickles <nickles.travis@gmail.com>

## Translators

Belleguic Terence <hizo@free.fr>
