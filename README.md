# antimicro

## Description

Graphical program used to map keyboard and mouse buttons to gamepad buttons.
Useful for games with no gamepad support.

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

In order to build this program, open a terminal and cd into the antimicro directory.
Enter the following commands in order to build the program:

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

## Roadmap to 1.0

This program has evolved pretty quickly since I started the project. I believe that
this program has now obtained an identity beyond just being considered an updated version of
QJoyPad. Now that a lot of the features that I have wanted are now implemented in the program,
I have started to take into consideration what it will take before I can actually consider this program
worthy of a 1.0 release. I have started to compile a list of possible features that I would
like incorporated in this program before it is worthy of a 1.0 release.

* Combine axes in order to specify controller sticks
	* This will allow for more control over the range that dictates a diagonal press
* Configuration menu for specifying aspects of a controller (number of sticks, buttons, d-pads, etc.)
* Allow for sticks and d-pads to be considered 8-way controls with a button associated
with each direction
* Implement more slot actions for macros: Cycle Stop, Distance, Release

## Credits

Travis Nickles <nickles.travis@gmail.com>

## Translators

Belleguic Terence <hizo@free.fr>
