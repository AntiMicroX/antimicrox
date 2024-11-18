# Building AntiMicroX

Most of these packages are already built and available on [Release Page](https://github.com/AntiMicroX/antimicrox/releases), but if you want, you can also build AntiMicroX by yourself.

- [Building AntiMicroX](#building-antimicrox)
  - [Build Dependencies](#build-dependencies)
  - [Basic building](#basic-building)
    - [MSVC building tips](#msvc-building-tips)
    - [Build Options for CMake](#build-options-for-cmake)
    - [Universal Options](#universal-options)
    - [Linux Options](#linux-options)
  - [Building DEB package](#building-deb-package)
  - [Building RPM package](#building-rpm-package)
  - [Building AppImage](#building-appimage)
  - [Building Flatpak](#building-flatpak)
  - [Building Windows installer](#building-windows-installer)

## Build Dependencies

This program is written in C++ using the [Qt](https://www.qt.io/)
framework. A C++ compiler and a proper C++ build environment will need to be installed on your system prior to building this program. Under Debian and Debian-based distributions like Ubuntu, the easiest way to get a base build environment set up is to install the meta-package **build-essential**. The following packages are required to be
installed on your system in order to build this program:

- `g++` from `gcc` (or clang compiler)
- `cmake`
- `extra-cmake-modules`
- QT libraries:
  - For QT6: `qt6-base-dev`, `qt6-tools-dev-tools` `libqt6core5compat6-dev` `qt6-tools-dev`
  - For QT5: `qttools5-dev` and `qttools5-dev-tools` (`qt5-tools` on distros based on Arch Linux)
- `libsdl2-dev` (`sdl2` on distros based on Arch Linux) (SDL2)
- `libxi-dev` (`libxi` on distros based on Arch Linux) (Optional. Needed to compile with X11 and uinput support)
- `libxtst-dev` (`libxtst` on distros based on Arch Linux) (Optional. Needed to compile with XTest support)
- `libx11-dev` (`libx11` on distros based on Arch Linux) (Needed to compile with Qt5 support)
- `itstool` (extracts messages from XML files and outputs PO template files, then merges translations from MO files to create translated XML files)
- `gettext`

<details>
  <summary>Fedora dependencies</summary>

One-liner for installing above dependencies:

```bash
sudo dnf install git make cmake gcc cmake extra-cmake-modules qt6-qttools-devel SDL2-devel libXi-devel libXtst-devel libX11-devel itstool gettext-devel;
```

</details>

<details>
  <summary>Ubuntu/Debian dependencies</summary>

One-liner for installing above dependencies:  

```bash
sudo apt install g++ cmake extra-cmake-modules qt6-base-dev qt6-tools-dev-tools libqt6core5compat6-dev qt6-tools-dev libsdl2-dev libxi-dev libxtst-dev libx11-dev itstool gettext ninja-build
```

QT5 Variant:

```bash
sudo apt install g++ cmake extra-cmake-modules qttools5-dev qttools5-dev-tools libsdl2-dev libxi-dev libxtst-dev libx11-dev itstool gettext ninja-build
```

</details>

<details>
  <summary>Windows dependencies</summary>
In case of Windows you need QT, SDL2 libraries, cmake and compiler (mingw for example).

For setting up your environment you may use `msys2`. Alternatively, you may use `MSVC`.

</details>

## Basic building

This way of building is useful for testing purposes.

In order to build this program, open a terminal and cd into the antimicrox
directory. Enter the following commands in order to:

Clone repository

```bash
git clone https://github.com/AntiMicroX/antimicrox.git
```

Build the program:

```bash
cd antimicrox
mkdir build && cd build
cmake ..
cmake --build .
```

Run built binaries

```
./bin/antimicrox
```

#### MSVC building tips

Recent versions of Visual Studio (2017+) have support for cmake projects. Under Visual Studio 2022, building AntiMicroX is quite straight forward.
- Ensure you have compatable versions of [Qt](https://www.qt.io/download) (5.9 works as of writing,) and [SDL2-devel](https://github.com/libsdl-org/SDL/releases/) installed.
- Open antimicrox as a local folder in VS22. It should pick up the `CMakeLists.txt` and offer an option to open the CMake settings editor. If it doesn't, right click on `CMakeLists.txt` in the solution explorer and select `CMake settings for antimicrox`.
- In the `Command arguments` section, add an argument to tell CMake where to find your Qt; E.g.: `"-DCMAKE_PREFIX_PATH=C:\Qt\5.9\msvc2017_64\lib\cmake"`. As of writing, Qt's msvc2017 works properly through vs22.
- Under the `Cmake variables and cache` section, click the link labeled `Save and generate cmake cache to load variables.
- If the CMake generation fails due to SDL2, find the variables named `SDL2_PATH`, `SDL2_INCLUDE_DIR`, and `SDL2_DLL_LOCATION_DIR` in the list view, and set them properly. You may also need to move the headers in the SDL2 include dir inside a folder named `SDL2` to match their include paths on other systems.
- At this point you should be able to save your changes to regenerate the cmake cache, which will then allow you to build `antimicrox.exe` through Visual Studio.

A recommended way of installation is building package typical for for your system (or building universal one like an AppImage).

<details>
  <summary>Installation using cmake (not recommended)</summary>

This way of installation is not recommended, because it doesn't integrate very well with some environments.

Install:

```bash
sudo cmake --install .
```

Uninstall:

```bash
sudo make uninstall
```

</details>

### Build Options for CMake

There are a few application specific options that can be used when running
cmake to build antimicrox. The following file will attempt to list some of those
options and describe their use in the project.

### Universal Options

    -DBUILD_DOCS

Default: OFF. Build documentation using doxygen (installed doxygen and dot required). Generated files can be found in `docs` directory

    -DUPDATE_TRANSLATIONS

Default: OFF. Set updateqm target to call lupdate in order to update
translation files from source.

    -DCMAKE_BUILD_TYPE

Default: Not defined. You can define build type.  
Debug builds are compiled with enabled debug flags, disabled optimizations and better printing stack trace in case of crash.

    -DTRANS_REMOVE_OBSOLETE

Default: OFF. Do not specify -noobsolete option when calling lupdate
command for qm files. -noobsolete is a method for getting rid of obsolete text entries

    -DCHECK_FOR_UPDATES

Default: OFF. Show update button in GUI when newer version is available. Recommended for builds distributed without package management systems.

    -DWITH_TESTS

Default: OFF. Allows for the launch of test sources with unit tests

    -DANTIMICROX_PKG_VERSION

Default: Not defined. (feature intended for packagers) Manually define version of package displayed in info tab. When not defined building time is displayed instead. Example: `-DANTIMICROX_PKG_VERSION=3.1.7-appimage`

### Linux Options

    -DAPPDATA

Default: ON. Build the project with AppData support.

    -DWITH_UINPUT

Default: ON. Compile the program with uinput support.

    -DINSTALL_UINPUT_UDEV_RULES

Default: ON. During installation process create new udev rule allowing regular users using uinput.

    -DWITH_X11

Default: ON. Compile the program with X11 support.

    -DWITH_XTEST

Default: ON. Compile the program with XTest support.

---

**qDebug output on terminal:**

On some systems, particularly if you do not have QtCreator installed, you may notice that some debug info (in particular qDebug() statements) do not generate any output on a Linux terminal. To fix this, you can create a qtlogging.ini file:

    mkdir ~/.config/QtProject
    printf '[Rules]\n*.debug=true\nqt.*.debug=false\n' > ~/.config/QtProject/qtlogging.ini

Once this file is created and has the debug rules present, it should be picked up and applied automatically the next time you run cmake to create a build.

## Building DEB package

```bash
cd antimicrox
mkdir build && cd build
cmake .. -DCPACK_GENERATOR="DEB"
cmake --build . --target package
```

## Building RPM package

If your distribution doesn't yet have an RPM package, you can easily build one for yourself.

```bash
cd antimicrox
mkdir build && cd build
cmake .. -DCPACK_GENERATOR="RPM"
cmake --build . --target package
```

## Building AppImage

Create build directory

```bash
mkdir build && cd ./build
```

Download tools used for creating appimages (and make them executable)

```bash
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x linuxdeploy-x86_64.AppImage
chmod +x appimagetool-x86_64.AppImage
chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
```

Build antimicrox and install it in AppDir directory

```bash
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make
make install DESTDIR=AppDir
```

Create AppImage file

```bash
./linuxdeploy-x86_64.AppImage --appdir AppDir --plugin qt
./appimagetool-x86_64.AppImage AppDir/
```

## Building Flatpak

The command builds the package into the `build` folder and installs the created flatpak.
The Flathub manifest can be located in [Flathub's Github repo](https://github.com/flathub/io.github.antimicrox.antimicrox).

```bash
flatpak install flathub org.kde.Platform//5.11 org.kde.Sdk//5.11
flatpak-builder --user --install build/ other/io.github.antimicrox.antimicrox.yml --force-clean
```

## Building Windows installer

In case of building Windows package you will need [NSIS](https://nsis.sourceforge.io/Download).

```
cmake -DCMAKE_BUILD_TYPE=Debug -B .\build
cmake --build .\build --target package
```
