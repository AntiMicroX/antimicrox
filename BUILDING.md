# Building AntiMicroX

- [Building AntiMicroX](#building-antimicrox)
  - [Build Dependencies](#build-dependencies)
  - [Building Under Linux](#building-under-linux)
    - [Build Options for CMake](#build-options-for-cmake)
    - [Universal Options](#universal-options)
    - [Linux Options](#linux-options)
  - [Building deb package](#building-deb-package)
  - [Building AppImage](#building-appimage)
  - [Building Flatpak](#building-flatpak)

## Build Dependencies

This program is written in C++ using the [Qt](https://www.qt.io/)
framework. A C++ compiler and a proper C++ build environment will need to be installed on your system prior to building this program. Under Debian and Debian-based distributions like Ubuntu, the easiest way to get a base build environment set up is to install the meta-package **build-essential**. The following packages are required to be
installed on your system in order to build this program:

- `g++` from `gcc`
- `cmake`
- `extra-cmake-modules`
- `qttools5-dev` and `qttools5-dev-tools` (`qt5-tools` on distros based on Arch Linux) (Qt5 support)
- `libsdl2-dev` (`sdl2` on distros based on Arch Linux) (SDL2)
- `libxi-dev` (`libxi` on distros based on Arch Linux) (Optional. Needed to compile with X11 and uinput support)
- `libxtst-dev` (`libxtst` on distros based on Arch Linux) (Optional. Needed to compile with XTest support)
- `libx11-dev` (`libx11` on distros based on Arch Linux) (Needed to compile with Qt5 support)
- `itstool` (extracts messages from XML files and outputs PO template files, then merges translations from MO files to create translated XML files)
- `gettext`
- `libqt5x11extras5-dev` (`qt5-x11extras` on distros based on Arch Linux)

## Building Under Linux

In order to build this program, open a terminal and cd into the antimicrox
directory. Enter the following commands in order to build the program:

```bash
cd antimicrox
mkdir build && cd build
cmake ..
make
sudo make install
```

or

```bash
cd antimicrox
mkdir build && cd build
cmake --build .
sudo cmake --install .
```

### Build Options for CMake

There are a few application specific options that can be used when running
cmake to build antimicrox. The following file will attempt to list some of those
options and describe their use in the project.


### Universal Options

    -DUPDATE_TRANSLATIONS

Default: OFF. Set updateqm target to call lupdate in order to update 
translation files from source.

    -DTRANS_KEEP_OBSOLETE
    
Default: OFF. Do not specify -noobsolete option when calling lupdate 
command for qm files. -noobsolete is a method for getting rid of obsolete text entries

    -DWITH_TESTS
    
Default: OFF. Allows for the launch of test sources with unit tests

### Linux Options

    -DAPPDATA

Default: ON. Build the project with AppData support.

    -DWITH_UINPUT

Default: ON. Compile the program with uinput support.

    -DWITH_X11

Default: ON. Compile the program with X11 support.

    -DWITH_XTEST

Default: ON. Compile the program with XTest support.

## Building deb package

Already built .deb files are available on [Release Page](https://github.com/AntiMicroX/antimicrox/releases)

```bash
cd antimicrox
mkdir build && cd build
cmake .. -DCPACK_GENERATOR="DEB"
cmake --build . --target package
```

## Building AppImage

Create build directory
```bash
mkdir build && cd ./build
```

Download tools used for creating appimages (and make them executable)
```bash  
wget https://github.com/linuxdeploy/linuxdeploy/releases/downloacontinuous/linuxdeploy-x86_64.AppImage
wget https://github.com/AppImage/AppImageKit/releases/downloacontinuous/appimagetool-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releasedownload/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
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
./appimagetool-x86_64.AppImage AppDir/ --no-appstream
```

## Building Flatpak

The command builds the package into the `build` folder and installs the created flatpak.
The Flathub manifest can be located in [Flathub's Github repo](https://github.com/flathub/io.github.antimicrox.antimicrox).

```bash
flatpak install flathub org.kde.Platform//5.11 org.kde.Sdk//5.11
flatpak-builder --user --install build/ other/io.github.antimicrox.antimicrox.yml --force-clean
```
