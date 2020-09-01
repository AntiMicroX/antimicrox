## Build Dependencies

### Dependencies
This program is written in C++ using the [Qt](https://www.qt.io/)
framework. A C++ compiler and a proper C++ build environment will need to be installed on your system prior to building this program. Under Debian and Debian-based distributions like Ubuntu, the easiest way to get a base build environment set up is to install the meta-package **build-essential**. The following packages are required to be
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
mkdir build && cd build
cmake --build .
sudo cmake --install .
```
<br/>


#### Building deb package

Already built .deb files are available on [Release Page](https://github.com/AntiMicroX/antimicroX/releases)

```bash
    cd antimicroX
    mkdir build && cd build
    cmake .. -DCPACK_GENERATOR="DEB"
    cmake --build . --target package
```


<br/>

#### Building AppImage

Create build directory
```bash
mkdir build && cd ./build
```

<br/>

Download tools used for creating appimages (and make them executable)
```bash  
wget https://github.com/linuxdeploy/linuxdeploy/releases/downloacontinuous/linuxdeploy-x86_64.AppImage
wget https://github.com/AppImage/AppImageKit/releases/downloacontinuous/appimagetool-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releasedownload/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x linuxdeploy-x86_64.AppImage
chmod +x appimagetool-x86_64.AppImage
chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
```

<br/>

Build antimicroX and install it in AppDir directory
```bash
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make
make install DESTDIR=AppDir
```

<br/>

Create AppImage file
```bash
./linuxdeploy-x86_64.AppImage --appdir AppDir --plugin qt
./appimagetool-x86_64.AppImage AppDir/ --no-appstream
```
<br/>

### Running with Docker

If you want to run application without building process and choose between various distributions, then [look here](https://hub.docker.com/r/juliagoda/antimicrox)  

<br/>

All full tags variations:

- juliagoda/antimicrox:latest
- juliagoda/antimicrox:3.0-ubuntu-bionic
- juliagoda/antimicrox:3.0-fedora-latest
- juliagoda/antimicrox:3.0-suseleap15.2


Because the docker likes to replace the README on the docker hub website with this one. I need to add informations about how to run image:  

You should as first:

`git pull juliagoda/antimicrox:3.0-ubuntu-bionic`

where "3.0-ubuntu-bionic" is a tag and can be replaced by other chosen tag. Next we have to create group docker and add user to it. [Look here](https://docs.docker.com/engine/install/linux-postinstall/). To run GUI docker apps:

`xhost +local:docker`

To finally run image:

`docker run -it -e DISPLAY=unix$DISPLAY --mount type=bind,source=/dev/input,target=/dev/input --device /dev/input --mount type=bind,source=/home/$USER,target=/home/$USER --net=host -e HOME=$HOME --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" --device /dev/dri:/dev/dri --workdir=$HOME antimicrox:3.0-ubuntu-bionic`

This allows the use your files from your home directory and the use of your connected devices without other workarounds

<br/>