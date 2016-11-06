## antimicro Gamepad to KB+M event mapper
## Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.


# Debian Wheezy has a package for version 2.8.9 so
# that will be the minimum supported version.
cmake_minimum_required(VERSION 2.8.9)
project(antimicro)

if(WIN32)
    # Cause cmake to fail if Qt location is not specified.
    if(NOT CMAKE_PREFIX_PATH)
        message(FATAL_ERROR "Please set CMAKE_PREFIX_PATH to the Qt installation directory. Exiting.")
    endif(NOT CMAKE_PREFIX_PATH)

    # CMP0020: Automatically link Qt executables to qtmain target on Windows.
    cmake_policy(SET CMP0020 NEW)
endif(WIN32)

# The version number.
set(ANTIMICRO_MAJOR_VERSION 2)
set(ANTIMICRO_MINOR_VERSION 23)
set(ANTIMICRO_PATCH_VERSION 0)

option(USE_SDL_2 "Use SDL 2 libraries rather than SDL 1." ON)

if(UNIX)
    option(WITH_X11 "Compile with support for X11." ON)
    option(WITH_UINPUT "Compile with support for uinput. uinput will be usable to simulate events." OFF)
    option(WITH_XTEST "Compile with support for XTest.  XTest will be usable to simulate events." ON)
    option(APPDATA "Build project with AppData file support." OFF)
endif(UNIX)

option(UPDATE_TRANSLATIONS "Call lupdate to update translation files from source." OFF)
option(TRANS_KEEP_OBSOLETE "Do not specify -no-obsolete when calling lupdate." OFF)

if(WIN32)
    option(PORTABLE_PACKAGE "Create portable Windows package" OFF)
    #option(TARGET_ARCH "Choose which version of some libraries to use. (x86, x86_64)" "x86")
    option(WITH_VMULTI "Compile with support for vmulti." OFF)
    option(PERFORM_SIGNING "Sign final executable." OFF)

    if(PORTABLE_PACKAGE)
        message("Portable package mode build")

        add_definitions(-DWIN_PORTABLE_PACKAGE)

        # Only way to force install target to be dependent on createprofiledir.
        add_custom_target(createprofiledir)
        add_custom_command(TARGET createprofiledir PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/profiles"
        )
        install(CODE "execute_process(COMMAND ${CMAKE_BUILD_TOOL} createprofiledir WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\")")
        install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/profiles" DESTINATION ${CMAKE_INSTALL_PREFIX})
    endif(PORTABLE_PACKAGE)

    # Find target architecture based on the compiler.
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(TARGET_ARCH "x86_64")
    else()
        set(TARGET_ARCH "x86")
    endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

    #if(TARGET_ARCH)
    #    set(ARCH_OPTIONS "x86" "x86_64")
    #    list(FIND ARCH_OPTIONS ${TARGET_ARCH} ARCH_INDEX)
    #    if(ARCH_INDEX EQUAL -1)
    #        message(FATAL_ERROR "No valid architecture choice used. Exiting.")
    #    endif(ARCH_INDEX EQUAL -1)
    #endif(TARGET_ARCH)

    if(WITH_VMULTI)
        add_definitions(-DWITH_VMULTI)
    endif(WITH_VMULTI)
endif(WIN32)

# Use pkg-config to find SDL library.
if(UNIX)
    find_package(PkgConfig REQUIRED)
    #include(FindPkgConfig)

    if(USE_SDL_2)
        pkg_check_modules(SDL2 REQUIRED sdl2)
    elseif(NOT USE_SDL_2)
        pkg_check_modules(SDL REQUIRED sdl)
    endif(USE_SDL_2)

    if(WITH_X11)
        find_package(X11 REQUIRED)
    endif(WITH_X11)

    if(WITH_XTEST AND NOT WITH_X11)
        set(WITH_XTEST OFF)
        message("Cannot use XTest without X11. Disabling XTest support.")
    endif(WITH_XTEST AND NOT WITH_X11)

    if(WITH_XTEST)
        message("XTest support allowed for simulating events.")
    endif(WITH_XTEST)

    if(WITH_UINPUT)
        message("uinput support allowed for simulating events.")
    endif(WITH_UINPUT)

    if(NOT WITH_XTEST AND NOT WITH_UINPUT)
        message(FATAL_ERROR "No system is defined for simulating events.")
    endif(NOT WITH_XTEST AND NOT WITH_UINPUT)
endif(UNIX)

set(antimicro_SOURCES src/main.cpp
    src/mainwindow.cpp
    src/joybuttonwidget.cpp
    src/joystick.cpp
    src/joybutton.cpp
    src/joybuttontypes/joygradientbutton.cpp
    src/event.cpp
    src/inputdaemon.cpp
    src/joyaxis.cpp
    src/joyaxiswidget.cpp
    src/joydpad.cpp
    src/joybuttontypes/joydpadbutton.cpp
    src/axiseditdialog.cpp
    src/xmlconfigreader.cpp
    src/xmlconfigwriter.cpp
    src/joytabwidget.cpp
    src/axisvaluebox.cpp
    src/advancebuttondialog.cpp
    src/simplekeygrabberbutton.cpp
    src/joybuttonslot.cpp
    src/joybuttontypes/joyaxisbutton.cpp
    src/aboutdialog.cpp
    src/setjoystick.cpp
    src/sdleventreader.cpp
    src/setaxisthrottledialog.cpp
    src/keyboard/virtualkeypushbutton.cpp
    src/keyboard/virtualkeyboardmousewidget.cpp
    src/keyboard/virtualmousepushbutton.cpp
    src/buttoneditdialog.cpp
    src/commandlineutility.cpp
    src/joycontrolstick.cpp
    src/joybuttontypes/joycontrolstickbutton.cpp
    src/joybuttontypes/joycontrolstickmodifierbutton.cpp
    src/joycontrolstickeditdialog.cpp
    src/joycontrolstickpushbutton.cpp
    src/joycontrolstickbuttonpushbutton.cpp
    src/joycontrolstickstatusbox.cpp
    src/advancestickassignmentdialog.cpp
    src/dpadpushbutton.cpp
    src/dpadeditdialog.cpp
    src/vdpad.cpp
    src/joydpadbuttonwidget.cpp
    src/quicksetdialog.cpp
    src/mousehelper.cpp
    src/mousesettingsdialog.cpp
    src/mousedialog/mousecontrolsticksettingsdialog.cpp
    src/mousedialog/mouseaxissettingsdialog.cpp
    src/mousedialog/mousebuttonsettingsdialog.cpp
    src/mousedialog/mousedpadsettingsdialog.cpp
    src/joytabwidgetcontainer.cpp
    src/mousedialog/springmoderegionpreview.cpp
    src/joystickstatuswindow.cpp
    src/joybuttonstatusbox.cpp
    src/qtkeymapperbase.cpp
    src/flashbuttonwidget.cpp
    src/xmlconfigmigration.cpp
    src/qkeydisplaydialog.cpp
    src/antkeymapper.cpp
    src/inputdevice.cpp
    src/mainsettingsdialog.cpp
    src/gamecontroller/gamecontrollertriggerbutton.cpp
    src/setnamesdialog.cpp
    src/autoprofileinfo.cpp
    src/addeditautoprofiledialog.cpp
    src/editalldefaultautoprofiledialog.cpp
    src/common.cpp
    src/localantimicroserver.cpp
    src/extraprofilesettingsdialog.cpp
    src/antimicrosettings.cpp
    src/joybuttoncontextmenu.cpp
    src/joycontrolstickcontextmenu.cpp
    src/dpadcontextmenu.cpp
    src/joyaxiscontextmenu.cpp
    src/stickpushbuttongroup.cpp
    src/dpadpushbuttongroup.cpp
    src/joybuttonmousehelper.cpp
    src/logger.cpp
    src/inputdevicebitarraystatus.cpp
    src/applaunchhelper.cpp
    src/slotitemlistwidget.cpp
    src/eventhandlers/baseeventhandler.cpp
    src/eventhandlerfactory.cpp
    src/uihelpers/advancebuttondialoghelper.cpp
    src/uihelpers/buttoneditdialoghelper.cpp
    src/uihelpers/joytabwidgethelper.cpp
    src/uihelpers/joyaxiscontextmenuhelper.cpp
    src/uihelpers/joycontrolstickcontextmenuhelper.cpp
    src/uihelpers/dpadcontextmenuhelper.cpp
    src/uihelpers/dpadeditdialoghelper.cpp
    src/uihelpers/joycontrolstickeditdialoghelper.cpp
    src/uihelpers/gamecontrollermappingdialoghelper.cpp
    src/mousedialog/uihelpers/mouseaxissettingsdialoghelper.cpp
    src/mousedialog/uihelpers/mousebuttonsettingsdialoghelper.cpp
    src/mousedialog/uihelpers/mousecontrolsticksettingsdialoghelper.cpp
    src/mousedialog/uihelpers/mousedpadsettingsdialoghelper.cpp
)

set(antimicro_HEADERS src/mainwindow.h
    src/joybuttonwidget.h
    src/joystick.h
    src/joybutton.h
    src/joybuttontypes/joygradientbutton.h
    src/inputdaemon.h
    src/joyaxis.h
    src/joyaxiswidget.h
    src/joydpad.h
    src/joybuttontypes/joydpadbutton.h
    src/axiseditdialog.h
    src/xmlconfigreader.h
    src/xmlconfigwriter.h
    src/joytabwidget.h
    src/axisvaluebox.h
    src/advancebuttondialog.h
    src/simplekeygrabberbutton.h
    src/joybuttonslot.h
    src/joybuttontypes/joyaxisbutton.h
    src/aboutdialog.h
    src/setjoystick.h
    src/sdleventreader.h
    src/setaxisthrottledialog.h
    src/keyboard/virtualkeypushbutton.h
    src/keyboard/virtualkeyboardmousewidget.h
    src/keyboard/virtualmousepushbutton.h
    src/buttoneditdialog.h
    src/commandlineutility.h
    src/joycontrolstick.h
    src/joybuttontypes/joycontrolstickbutton.h
    src/joybuttontypes/joycontrolstickmodifierbutton.h
    src/joycontrolstickeditdialog.h
    src/joycontrolstickpushbutton.h
    src/joycontrolstickbuttonpushbutton.h
    src/joycontrolstickstatusbox.h
    src/advancestickassignmentdialog.h
    src/dpadpushbutton.h
    src/dpadeditdialog.h
    src/vdpad.h
    src/joydpadbuttonwidget.h
    src/quicksetdialog.h
    src/mousehelper.h
    src/mousesettingsdialog.h
    src/mousedialog/mousecontrolsticksettingsdialog.h
    src/mousedialog/mouseaxissettingsdialog.h
    src/mousedialog/mousebuttonsettingsdialog.h
    src/mousedialog/mousedpadsettingsdialog.h
    src/joytabwidgetcontainer.h
    src/mousedialog/springmoderegionpreview.h
    src/joystickstatuswindow.h
    src/joybuttonstatusbox.h
    src/qtkeymapperbase.h
    src/flashbuttonwidget.h
    src/xmlconfigmigration.h
    src/qkeydisplaydialog.h
    src/antkeymapper.h
    src/inputdevice.h
    src/mainsettingsdialog.h
    src/gamecontroller/gamecontrollertriggerbutton.h
    src/setnamesdialog.h
    src/autoprofileinfo.h
    src/addeditautoprofiledialog.h
    src/editalldefaultautoprofiledialog.h
    src/localantimicroserver.h
    src/extraprofilesettingsdialog.h
    src/antimicrosettings.h
    src/joybuttoncontextmenu.h
    src/joycontrolstickcontextmenu.h
    src/dpadcontextmenu.h
    src/joyaxiscontextmenu.h
    src/stickpushbuttongroup.h
    src/dpadpushbuttongroup.h
    src/joybuttonmousehelper.h
    src/logger.h
    src/inputdevicebitarraystatus.h
    src/applaunchhelper.h
    src/slotitemlistwidget.h
    src/eventhandlers/baseeventhandler.h
    src/eventhandlerfactory.h
    src/uihelpers/advancebuttondialoghelper.h
    src/uihelpers/buttoneditdialoghelper.h
    src/uihelpers/joytabwidgethelper.h
    src/uihelpers/joyaxiscontextmenuhelper.h
    src/uihelpers/joycontrolstickcontextmenuhelper.h
    src/uihelpers/dpadcontextmenuhelper.h
    src/uihelpers/dpadeditdialoghelper.h
    src/uihelpers/joycontrolstickeditdialoghelper.h
    src/uihelpers/gamecontrollermappingdialoghelper.h
    src/mousedialog/uihelpers/mouseaxissettingsdialoghelper.h
    src/mousedialog/uihelpers/mousebuttonsettingsdialoghelper.h
    src/mousedialog/uihelpers/mousecontrolsticksettingsdialoghelper.h
    src/mousedialog/uihelpers/mousedpadsettingsdialoghelper.h
)

set(antimicro_FORMS src/mainwindow.ui
    src/axiseditdialog.ui
    src/advancebuttondialog.ui
    src/aboutdialog.ui
    src/setaxisthrottledialog.ui
    src/buttoneditdialog.ui
    src/joycontrolstickeditdialog.ui
    src/advancestickassignmentdialog.ui
    src/dpadeditdialog.ui
    src/quicksetdialog.ui
    src/mousesettingsdialog.ui
    src/joystickstatuswindow.ui
    src/qkeydisplaydialog.ui
    src/gamecontrollermappingdialog.ui
    src/mainsettingsdialog.ui
    src/setnamesdialog.ui
    src/addeditautoprofiledialog.ui
    src/editalldefaultautoprofiledialog.ui
    src/extraprofilesettingsdialog.ui
    src/capturedwindowinfodialog.ui
)

set(antimicro_RESOURCES src/resources.qrc)

# Files that require SDL 2 support.
if(USE_SDL_2)
    LIST(APPEND antimicro_SOURCES src/gamecontroller/gamecontroller.cpp
         src/gamecontroller/gamecontrollerdpad.cpp
         src/gamecontroller/gamecontrollerset.cpp
         src/gamecontroller/gamecontrollertrigger.cpp
         src/gamecontrollermappingdialog.cpp
         src/gamecontrollerexample.cpp
    )
    LIST(APPEND antimicro_HEADERS src/gamecontroller/gamecontroller.h
        src/gamecontroller/gamecontrollerdpad.h
        src/gamecontroller/gamecontrollerset.h
        src/gamecontroller/gamecontrollertrigger.h
        src/gamecontrollermappingdialog.h
        src/gamecontrollerexample.h
    )
endif(USE_SDL_2)

# Platform dependent files.
if(UNIX)
    if(WITH_X11)
        LIST(APPEND antimicro_SOURCES src/x11extras.cpp
             src/qtx11keymapper.cpp
             src/unixcapturewindowutility.cpp
             src/autoprofilewatcher.cpp
             src/capturedwindowinfodialog.cpp
        )
        LIST(APPEND antimicro_HEADERS src/x11extras.h
             src/qtx11keymapper.h
             src/unixcapturewindowutility.h
             src/autoprofilewatcher.h
             src/capturedwindowinfodialog.h
        )

        if(WITH_XTEST)
            LIST(APPEND antimicro_SOURCES src/eventhandlers/xtesteventhandler.cpp)
            LIST(APPEND antimicro_HEADERS src/eventhandlers/xtesteventhandler.h)
        endif(WITH_XTEST)
    endif(WITH_X11)

    if(WITH_UINPUT)
        LIST(APPEND antimicro_SOURCES src/qtuinputkeymapper.cpp
             src/uinputhelper.cpp
             src/eventhandlers/uinputeventhandler.cpp
        )
        LIST(APPEND antimicro_HEADERS src/qtuinputkeymapper.h
             src/uinputhelper.h
             src/eventhandlers/uinputeventhandler.h
        )
    endif(WITH_UINPUT)

elseif(WIN32)
    LIST(APPEND antimicro_SOURCES src/winextras.cpp
         src/qtwinkeymapper.cpp
         src/winappprofiletimerdialog.cpp
         src/autoprofilewatcher.cpp
         src/capturedwindowinfodialog.cpp
         src/eventhandlers/winsendinputeventhandler.cpp
         src/joykeyrepeathelper.cpp
    )
    LIST(APPEND antimicro_HEADERS src/winextras.h
        src/qtwinkeymapper.h
        src/winappprofiletimerdialog.h
        src/autoprofilewatcher.h
        src/capturedwindowinfodialog.h
        src/eventhandlers/winsendinputeventhandler.h
        src/joykeyrepeathelper.h
    )
    LIST(APPEND antimicro_FORMS src/winappprofiletimerdialog.ui)
    # Add Windows specific resource file used for application icons.
    LIST(APPEND antimicro_RESOURCES src/resources_windows.qrc)

    if(WITH_VMULTI)
        list(APPEND antimicro_SOURCES src/qtvmultikeymapper.cpp
             src/eventhandlers/winvmultieventhandler.cpp
#            vmulti/client/client.c
        )
        list(APPEND antimicro_HEADERS src/qtvmultikeymapper.h
             src/eventhandlers/winvmultieventhandler.h
        )
    endif(WITH_VMULTI)
endif(UNIX)

set(USE_QT5 OFF)
set(USE_QT4 OFF)

if(UNIX)
    # Check if Qt 4 or 5 was specified using an environment variable
    # or by specifying -DQT_QMAKE_EXECUTABLE.
    # Otherwise, check for Qt libraries.
    if("$ENV{QT_SELECT}" EQUAL 5)
        set(USE_QT5 ON)
    elseif(QT_QMAKE_EXECUTABLE MATCHES ".*/qmake-qt5")
        set(USE_QT5 ON)
    elseif ("$ENV{QT_SELECT}" EQUAL 4)
        set(USE_QT4 ON)
    elseif(QT_QMAKE_EXECUTABLE MATCHES ".*/qmake-qt4")
        set(USE_QT4 ON)
    else()
        find_package(Qt5Core QUIET)
        if(Qt5Core_FOUND)
            set(USE_QT5 ON)
        else()
            find_package(Qt4 QUIET)
            if(QT_FOUND)
                set(USE_QT4 ON)
            else()
                message(FATAL_ERROR "No Qt libraries could be found.")
            endif(QT_FOUND)
        endif(Qt5Core_FOUND)
    endif("$ENV{QT_SELECT}" EQUAL 5)
elseif(WIN32)
    # Use Qt5 on Windows.
    set(USE_QT5 ON)
endif(UNIX)

if(USE_QT5)
    message("Compiling with Qt5 support")
elseif(USE_QT4)
    message("Compiling with Qt4 support")
    if(NOT WITH_X11)
        message(FATAL_ERROR "Cannot build application using Qt4 without X11 support.")
    endif(NOT WITH_X11)
else()
    message(FATAL_ERROR "No Qt version was specified.")
endif(USE_QT5)

if(USE_SDL_2)
    add_definitions(-DUSE_SDL_2)
endif(USE_SDL_2)

if (WIN32)
    if(PERFORM_SIGNING)
        add_definitions(-DPERFORM_SIGNING)
    endif(PERFORM_SIGNING)
endif (WIN32)

if(UNIX)
    if(WITH_X11)
        add_definitions(-DWITH_X11)
    endif(WITH_X11)

    if(WITH_XTEST)
        add_definitions(-DWITH_XTEST)
    endif(WITH_XTEST)

    if(WITH_UINPUT)
        add_definitions(-DWITH_UINPUT)
    endif(WITH_UINPUT)
endif(UNIX)

if (UNIX)
    if (USE_QT5)
        # Find includes in corresponding build directories
        #set(CMAKE_INCLUDE_CURRENT_DIR ON)
        # Instruct CMake to run moc automatically when needed.
        set(CMAKE_AUTOMOC ON)

        find_package(Qt5Widgets REQUIRED)
        find_package(Qt5Core REQUIRED)
        find_package(Qt5Gui REQUIRED)
        find_package(Qt5Network REQUIRED)
        find_package(Qt5LinguistTools REQUIRED)

        QT5_WRAP_CPP(antimicro_HEADERS_MOC ${antimicro_HEADERS})
        QT5_WRAP_UI(antimicro_FORMS_HEADERS ${antimicro_FORMS})
        QT5_ADD_RESOURCES(antimicro_RESOURCES_RCC ${antimicro_RESOURCES})
        add_subdirectory("share/antimicro/translations")

        include_directories(${Qt5Widgets_INCLUDE_DIRS})
        add_definitions(${Qt5Widgets_DEFINITIONS})
        include_directories(${Qt5Core_INCLUDE_DIRS})
        add_definitions(${Qt5Core_DEFINITIONS})
        include_directories(${Qt5Gui_INCLUDE_DIRS})
        add_definitions(${Qt5Gui_DEFINITIONS})
        include_directories(${Qt5Network_INCLUDE_DIRS})
        add_definitions(${Qt5Network_DEFINITIONS})

        set(CMAKE_POSITION_INDEPENDENT_CODE ON)

        # Add compiler flags for building executables (-fPIE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

        LIST(APPEND LIBS ${Qt5Widgets_LIBRARIES}
            ${Qt5Core_LIBRARIES}
            ${Qt5Gui_LIBRARIES}
            ${Qt5Network_LIBRARIES}
        )
    else()
        find_package(Qt4 REQUIRED)
        set(QT_USE_QTNETWORK TRUE)

        QT4_WRAP_CPP(antimicro_HEADERS_MOC ${antimicro_HEADERS})
        QT4_WRAP_UI(antimicro_FORMS_HEADERS ${antimicro_FORMS})
        QT4_ADD_RESOURCES(antimicro_RESOURCES_RCC ${antimicro_RESOURCES})

        add_subdirectory("share/antimicro/translations")

        include(${QT_USE_FILE})
        add_definitions(${QT_DEFINITIONS})
        list(APPEND LIBS ${QT_LIBRARIES})
    endif(USE_QT5)

elseif(WIN32)
    # Find includes in corresponding build directories
    #set(CMAKE_INCLUDE_CURRENT_DIR ON)
    # Instruct CMake to run moc automatically when needed.
    set(CMAKE_AUTOMOC ON)

    find_package(Qt5Widgets REQUIRED)
    find_package(Qt5Core REQUIRED)
    find_package(Qt5Gui REQUIRED)
    find_package(Qt5Network REQUIRED)
    find_package(Qt5LinguistTools REQUIRED)

    QT5_WRAP_CPP(antimicro_HEADERS_MOC ${antimicro_HEADERS})
    QT5_WRAP_UI(antimicro_FORMS_HEADERS ${antimicro_FORMS})
    QT5_ADD_RESOURCES(antimicro_RESOURCES_RCC ${antimicro_RESOURCES})
    add_subdirectory("share/antimicro/translations")

    include_directories(${Qt5Widgets_INCLUDE_DIRS})
    add_definitions(${Qt5Widgets_DEFINITIONS})
    include_directories(${Qt5Core_INCLUDE_DIRS})
    add_definitions(${Qt5Core_DEFINITIONS})
    include_directories(${Qt5Gui_INCLUDE_DIRS})
    add_definitions(${Qt5Gui_DEFINITIONS})
    include_directories(${Qt5Network_INCLUDE_DIRS})
    add_definitions(${Qt5Network_DEFINITIONS})

    set(CMAKE_POSITION_INDEPENDENT_CODE ON)

    # Add compiler flags for building executables (-fPIE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

    LIST(APPEND LIBS ${Qt5Widgets_LIBRARIES}
        ${Qt5Core_LIBRARIES}
        ${Qt5Gui_LIBRARIES}
        ${Qt5Network_LIBRARIES}
    )
endif(UNIX)

if(UNIX)
    if(WITH_X11)
        LIST(APPEND LIBS ${X11_X11_LIB})
        LIST(APPEND LIBS ${X11_Xi_LIB})
    endif(WITH_X11)

    if(WITH_XTEST)
        LIST(APPEND LIBS ${X11_XTest_LIB})
    endif(WITH_XTEST)

    if(USE_SDL_2)
        list(APPEND LIBS ${SDL2_LIBRARIES})
    else()
        list(APPEND LIBS ${SDL_LIBRARIES})
    endif(USE_SDL_2)
elseif (WIN32)
    if(USE_SDL_2)
        set(SDL2_LIBRARY "")
        set(SDL2_LIBRARY_DIR "")
        set(SDL2_INCLUDE_DIR "")
        set(SDL2_DLL_LOCATION_DIR "")

        if (MSYS OR MINGW)
            find_package(PkgConfig QUIET)
            if(PKG_CONFIG_FOUND)
                #include(FindPkgConfig)

                pkg_check_modules(SDL2 QUIET sdl2)
                if(SDL2_FOUND)
                    #find_path(SDL2_INCLUDE_DIR "SDL2/SDL.h" HINTS ${SDL2_INCLUDE_DIRS})

                    set(SDL2_LIBRARY ${SDL2_LIBRARIES})
                    set(SDL2_LIBRARY_DIR ${SDL2_LIBRARY_DIRS})
                    set(SDL2_INCLUDE_DIR ${SDL2_INCLUDE_DIRS})
                    unset(SDL2_DLL_LOCATION_DIR)
                    find_path(SDL2_DLL_LOCATION_DIR "SDL2.dll" HINTS "${SDL2_LIBRARY_DIRS}/../bin")
                    #set(SDL2_DLL_LOCATION_DIR "${SDL2_LIBRARY_DIRS}/../bin")

                    list(APPEND LIBS ${SDL2_LIBRARIES})
                endif(SDL2_FOUND)
            endif(PKG_CONFIG_FOUND)
        endif(MSYS OR MINGW)

        if (NOT SDL2_LIBRARY)
            # Perform extra voodoo to get proper library paths and include
            # proper headers.
	    file(GLOB SDL2_BASE_DIR "${PROJECT_SOURCE_DIR}/SDL2-*.*.*/")

	    if (SDL2_BASE_DIR)
                unset(SDL2_LIBRARY)

                if(TARGET_ARCH STREQUAL "x86_64")
                    set(SDL2_LIBRARY_DIR "${SDL2_BASE_DIR}/x86_64-w64-mingw32/lib")
                    set(SDL2_INCLUDE_DIR "${SDL2_BASE_DIR}/x86_64-w64-mingw32/include")
                    set(SDL2_DLL_LOCATION_DIR "${SDL2_BASE_DIR}/x86_64-w64-mingw32/bin")

                    find_library(SDL2_LIBRARY SDL2 ${SDL2_LIBRARY_DIR})
                    list(APPEND LIBS ${SDL2_LIBRARY})
                    include_directories(${SDL2_INCLUDE_DIR})
                else()
                    set(SDL2_LIBRARY_DIR "${SDL2_BASE_DIR}/i686-w64-mingw32/lib")
                    set(SDL2_INCLUDE_DIR "${SDL2_BASE_DIR}/i686-w64-mingw32/include")
                    set(SDL2_DLL_LOCATION_DIR "${SDL2_BASE_DIR}/i686-w64-mingw32/bin")

                    find_library(SDL2_LIBRARY SDL2 ${SDL2_LIBRARY_DIR})
                    list(APPEND LIBS ${SDL2_LIBRARY})
                    include_directories(${SDL2_INCLUDE_DIR})
                endif(TARGET_ARCH STREQUAL "x86_64")
	    endif(SDL2_BASE_DIR)  
        endif(NOT SDL2_LIBRARY)

        add_definitions(-DUNICODE -D_UNICODE)
    endif(USE_SDL_2)

    list(APPEND LIBS "psapi")

    if(WITH_VMULTI)
        include_directories("${PROJECT_SOURCE_DIR}/vmulti/inc")
        list(APPEND LIBS "hid" "setupapi")
    endif(WITH_VMULTI)
endif (UNIX)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories("${PROJECT_SOURCE_DIR}/src")

if(UNIX)
    # Store executable in a bin subdir. Needed here so translations can be loaded.
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
endif(UNIX)

if(USE_QT5)
    if(UNIX)
        add_executable(antimicro ${antimicro_SOURCES}
            ${antimicro_FORMS_HEADERS}
            ${antimicro_RESOURCES_RCC}
        )
    elseif(WIN32)
        # The WIN32 is required to specify a GUI application.
        add_executable(antimicro WIN32 ${antimicro_SOURCES}
            ${antimicro_FORMS_HEADERS}
            ${antimicro_RESOURCES_RCC}
            src/antimicro.rc
        )
    endif(UNIX)

else()
    add_executable(antimicro ${antimicro_SOURCES}
        ${antimicro_HEADERS_MOC}
        ${antimicro_FORMS_HEADERS}
        ${antimicro_RESOURCES_RCC}
    )

endif(USE_QT5)

# Add link libraries.
#message(${LIBS})
target_link_libraries(antimicro ${LIBS})

# Specify out directory for final executable.
if(UNIX)
	install(TARGETS antimicro RUNTIME DESTINATION "bin")
elseif(WIN32)
	install(TARGETS antimicro RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
endif(UNIX)

if(UNIX)
    install(FILES src/images/antimicro.png DESTINATION "share/pixmaps")
    install(FILES other/antimicro.desktop DESTINATION "share/applications")
    install(FILES other/antimicro.xml DESTINATION "share/mime/packages")
endif(UNIX)

# Add man page for *nix platforms.
if(UNIX)
    add_subdirectory(other)

    # Only way to force install target to be dependent on manpage.
    install(CODE "execute_process(COMMAND ${CMAKE_BUILD_TOOL} manpage WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\")")

    if(APPDATA)
        # Only way to force install target to be dependent on appdata.
        install(CODE "execute_process(COMMAND ${CMAKE_BUILD_TOOL} appdata WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\")")
    endif(APPDATA)
endif(UNIX)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)


# Only way to force install target to be dependent on updateqm.
install(CODE "execute_process(COMMAND ${CMAKE_BUILD_TOOL} updateqm WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\")")

# Use this to use some variables created here in the actual project.
# Modify the config.h.in file using the appropriate variables.
configure_file(
    "${PROJECT_SOURCE_DIR}/src/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
)

# Copy current Changelog file to location that the resource file expects.
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Changelog DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/src/)

if(WIN32)
    #if(TARGET_ARCH STREQUAL "x86_64")
    #    # Copy SDL2.dll to find installation directory.
    #    add_custom_target(copy_sdl_dll
    #        COMMAND ${CMAKE_COMMAND} -E copy "${SDL2_BASE_DIR}/x86_64-w64-mingw32/bin/SDL2.dll" ${CMAKE_CURRENT_BINARY_DIR}
    #    )
    #else()
    #    # Copy SDL2.dll to find installation directory.
    #    add_custom_target(copy_sdl_dll
    #        COMMAND ${CMAKE_COMMAND} -E copy "${SDL2_BASE_DIR}/i686-w64-mingw32/bin/SDL2.dll" ${CMAKE_CURRENT_BINARY_DIR}
    #    )
    #endif(TARGET_ARCH STREQUAL "x86_64")

    add_custom_target(copy_sdl_dll
        COMMAND ${CMAKE_COMMAND} -E copy "${SDL2_DLL_LOCATION_DIR}/SDL2.dll" ${CMAKE_CURRENT_BINARY_DIR}
    )


    # Obtain location of Qt5 DLL files and assign them to a list.
    # This list will only be used for Release builds.
    get_target_property(QTCORE_DLL_LOCATION Qt5::Core LOCATION)
    string(REPLACE "/Qt5Core.dll" "" QTCORE_DLL_LOCATION ${QTCORE_DLL_LOCATION})
    set(EXTRA_QT_DLL_FILES
        "${QTCORE_DLL_LOCATION}/Qt5Core.dll"
        "${QTCORE_DLL_LOCATION}/Qt5Gui.dll"
        "${QTCORE_DLL_LOCATION}/Qt5Network.dll"
        "${QTCORE_DLL_LOCATION}/Qt5Widgets.dll"
    )

    find_library(EXTRA_DLLS_PTHREAD NAMES "libwinpthread-1.dll" )
    find_library(EXTRA_DLLS_LIBCPP NAMES "libstdc++-6.dll" )

    # Don't think find_library will handle versioned DLLS, so we'll try each one until we find something
    foreach( ICU_VER 51 52 53 54 55 56 57 58 59 )
      find_library(ICU_DT_DLL NAMES "icudt${ICU_VER}.dll")
      find_library(ICU_IN_DLL NAMES "icuin${ICU_VER}.dll")
      find_library(ICU_UC_DLL NAMES "icuuc${ICU_VER}.dll")
    endforeach( ICU_VER )

    list(APPEND EXTRA_DLL_FILES
      ${EXTRA_QT_DLL_FILES}
      ${EXTRA_DLLS_PTHREAD}
      ${EXTRA_DLLS_LIBCPP}
      ${ICU_DT_DLL}
      ${ICU_IN_DLL}
      ${ICU_UC_DLL}
      )
     
    # Copy relevant DLL files depending on the chosen architecture.
    if(TARGET_ARCH STREQUAL "x86_64")
      find_library( GCC_DLL "libgcc_s_seh-1.dll" )
      
      list(APPEND EXTRA_DLL_FILES
        ${GCC_DLL}
        "${SDL2_BASE_DIR}/x86_64-w64-mingw32/bin/SDL2.dll"
        )
    else()
      find_library( GCC_DLL "libgcc_s_dw2-1.dll" )
      
      list(APPEND EXTRA_DLL_FILES
	${GCC_DLL}
        "${SDL2_BASE_DIR}/i686-w64-mingw32/bin/SDL2.dll"
        )
    endif(TARGET_ARCH STREQUAL "x86_64")
    
    # Not a DLL file, but needs to travel with SDL2.DLL
    list(APPEND EXTRA_DLL_FILES "${SDL2_BASE_DIR}/README-SDL.txt")

    # Target to copy Qt DLL files.
    add_custom_target(install_extra_dlls)

    # Create destination directory if it does not exist.
    add_custom_command(TARGET install_extra_dlls PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_INSTALL_PREFIX}/"
    )

    foreach(CURRENT_DLL_FILE ${EXTRA_DLL_FILES})
        add_custom_command(TARGET install_extra_dlls PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${CURRENT_DLL_FILE} "${CMAKE_INSTALL_PREFIX}/"
        )
    endforeach()

    # Target to copy qwindows.dll platforms file.
    add_custom_target(install_platforms_dll)
    add_custom_command(TARGET install_platforms_dll PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${QTCORE_DLL_LOCATION}/../plugins/platforms/qwindows.dll"
        "${CMAKE_INSTALL_PREFIX}/platforms/qwindows.dll"
    )

    # Combine the two targets into a single target that will be used
    # while bundling the program.
    add_custom_target(install_dlls)
    add_dependencies(install_dlls
        install_extra_dlls
        install_platforms_dll
    )

    # Copy bundled Qt translation files.
    SET(QT_TRANSLATION_PATH "${QTCORE_DLL_LOCATION}/../translations/" )
    file(GLOB QT_TRANSLATION_FILES "${QT_TRANSLATION_PATH}qt_[^help]*.qm")
    add_custom_target(copy_qt_translations)
    foreach(CURRENT_QM_FILE ${QT_TRANSLATION_FILES})
        set(CURRENT_QM_FILENAME "")
        string(REPLACE ${QT_TRANSLATION_PATH} "" CURRENT_QM_FILENAME ${CURRENT_QM_FILE})
        add_custom_command(TARGET copy_qt_translations PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "${CURRENT_QM_FILE}"
            "${CMAKE_INSTALL_PREFIX}/share/qt/translations/${CURRENT_QM_FILENAME}"
        )
    endforeach()

    # Sign final executable
    if(PERFORM_SIGNING)
      install(CODE "execute_process(COMMAND \"C:/Program Files (x86)/Windows Kits/8.1/bin/x64/signtool.exe\"
              sign /v /a /s ROOT /n antimicro ${CMAKE_INSTALL_PREFIX}/antimicro.exe)")
    endif(PERFORM_SIGNING)

    # Set variables needed for MSI building.
    set(MSIFOLDER "${PROJECT_SOURCE_DIR}/windows")
    set(WIXENV $ENV{WIX})

    # Use a different file for each architecture due
    # to some DLL differences.
    if(TARGET_ARCH STREQUAL "x86_64")
        set(WIXWXS "${MSIFOLDER}/antimicro_64.wxs")
    else()
        set(WIXWXS "${MSIFOLDER}/antimicro.wxs")
    endif(TARGET_ARCH STREQUAL "x86_64")

    set(WIXOBJ "${MSIFOLDER}/antimicro.wixobj")

    # Use version number in output file name.
    set(MSIFILENAME "${ANTIMICRO_MAJOR_VERSION}.${ANTIMICRO_MINOR_VERSION}")
    if(ANTIMICRO_PATCH_VERSION AND NOT ANTIMICRO_PATCH_VERSION EQUAL 0)
        set(MSIFILENAME "${MSIFILENAME}.${ANTIMICRO_PATCH_VERSION}")
    endif(ANTIMICRO_PATCH_VERSION AND NOT ANTIMICRO_PATCH_VERSION EQUAL 0)

    # Change output file depending on the target architecture.
    if(TARGET_ARCH STREQUAL "x86_64")
        set(WIXMSI "${MSIFOLDER}/antimicro-${MSIFILENAME}-win64.msi")
    else()
        set(WIXMSI "${MSIFOLDER}/antimicro-${MSIFILENAME}-win32.msi")
    endif(TARGET_ARCH STREQUAL "x86_64")

    if(NOT WIXENV)
        # Display message when WIX is not set up. No extra target will be added.
        message("MSI package building not possible: WIX environment variable not defined.")
    else()
        # Target to build .msi installer file.
        add_custom_target(buildmsi)

        # Change arch value passed to candle.exe
        set(WIXARCH "")
        if(TARGET_ARCH STREQUAL "x86_64")
            set(WIXARCH "x64")
        else()
            set(WIXARCH "x86")
        endif(TARGET_ARCH STREQUAL "x86_64")

        add_custom_command(TARGET buildmsi PRE_BUILD
            COMMAND "${WIXENV}\\bin\\candle.exe" ${WIXWXS} -out ${WIXOBJ} -sw1113 -arch ${WIXARCH} &&
            "${WIXENV}\\bin\\light.exe" ${WIXOBJ} -out ${WIXMSI} -sw1076 -spdb
        )
        set(WIXFILES ${WIXOBJ} ${WIXMSI})
        set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${WIXFILES}")
    endif(NOT WIXENV)

    # Copy extra files to include in the final archive.
    install(FILES gpl.txt Changelog README.md ProfileTips.md
            DESTINATION ${CMAKE_INSTALL_PREFIX})
endif(WIN32)
