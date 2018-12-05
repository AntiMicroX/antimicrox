REM Build script used to compile SDL2. It is mainly a reference for which
REM options need to be used in order to compile.

REM Move to directory that contains the SDL source directory.
REM Just use a static location for now.
cd "C:\Users\Travis\Downloads\SDL2-Source"

REM Make build directory and change cwd to build
mkdir build
cd build

REM Generate Makefile with specific options
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="%TEMP%\SDL2" -DSDL_STATIC=OFF -DVIDEO_OPENGLES=OFF -DRENDER_D3D=OFF ..\SDL2-2.0.4

REM Compile
mingw32-make
mingw32-make install

REM Get out of build directory
cd ..

