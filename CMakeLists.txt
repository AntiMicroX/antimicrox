## antimicrox Gamepad to KB+M event mapper
## Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
## Copyright (C) 2020 Jagoda Górska <juliagoda.pl@protonmail.com>
## Copyright (C) 2020 Paweł Kotiuk <kotiuk@zohomail.eu>
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


cmake_minimum_required(VERSION 3.12)
project(antimicrox)

if(POLICY CMP0071)
    cmake_policy(SET CMP0071 NEW)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" "${CMAKE_ROOT}/Modules")

set(CMAKE_INCLUDE_CURRENT_DIR ON)


if(UNIX AND NOT APPLE AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "force cmake prefix to be set for /usr" FORCE)
endif(UNIX AND NOT APPLE AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

include(CheckCXXCompilerFlag)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message("Build type: ${CMAKE_BUILD_TYPE}")

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC" AND MSVC_TOOLSET_VERSION GREATER_EQUAL 141)
    set(COMPILER_IS_MSVC TRUE)
endif()

if(NOT COMPILER_IS_MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wcast-align -Wunreachable-code -g -O0 -fno-omit-frame-pointer")
    # ADDITIONAL DEBUG FLAGS
    # set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wstrict-null-sentinel -Wstrict-overflow=5 -Wundef -Wno-unused  -Wswitch-enum")
    if(UNIX AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -rdynamic")
    endif(UNIX AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /analyze- /D_CRT_SECURE_NO_WARNINGS")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W3 /GS /Od /sdl /wd4996 /RTC1 /Gd /Oy-  /permissive-")
endif(NOT COMPILER_IS_MSVC)

# The version number.
set(ANTIMICROX_MAJOR_VERSION 3)
set(ANTIMICROX_MINOR_VERSION 5)
set(ANTIMICROX_PATCH_VERSION 1)

if(UNIX)
    option(WITH_X11 "Compile with support for X11." ON)
    option(WITH_UINPUT "Compile with support for uinput. uinput will be usable to simulate events." ON)
    option(INSTALL_UINPUT_UDEV_RULES "Generate udev rules allowing users using uinput without root permissions." ON)
    option(WITH_XTEST "Compile with support for XTest.  XTest will be usable to simulate events." ON)
    option(APPDATA "Build project with AppData file support." ON)
endif(UNIX)

if(WIN32)
    option(PORTABLE_WIN "Compile app for portability" OFF)
endif(WIN32)

option(USE_QT6_BY_DEFAULT "Use Qt6 if available (if not available fallback to QT5)" OFF)
option(UPDATE_TRANSLATIONS "Call lupdate to update translation files from source." OFF)
option(TRANS_REMOVE_OBSOLETE "Add -noobsolete option to lupdate command to get rid of old text entries" OFF)
option(ATTACH_FAKE_CLASSES "Fake classes can be used in application to tests functionalities" OFF)
option(ANTIMICROX_PKG_VERSION "Manually define version of package displayed in info tab" OFF)
option(CHECK_FOR_UPDATES "Enable checking for updates using GitHub REST API." OFF)
option(BUILD_DOCS "Build documentation" OFF)
option(WITH_TESTS "Allow tests for classes" OFF)

if(WITH_TESTS)
    message("Tests enabled")
endif(WITH_TESTS)

if(ANTIMICROX_PKG_VERSION)
    message("This build will be marked as: ${ANTIMICROX_PKG_VERSION}")
    add_compile_definitions(ANTIMICROX_PKG_VERSION="${ANTIMICROX_PKG_VERSION}")
endif(ANTIMICROX_PKG_VERSION)

if(CHECK_FOR_UPDATES)
    message("This build will check for updates using GitHub API.")
    add_compile_definitions(CHECK_FOR_UPDATES="${CHECK_FOR_UPDATES}")
endif(CHECK_FOR_UPDATES)

if(UNIX)
    if(WITH_XTEST AND NOT WITH_X11)
        set(WITH_XTEST OFF)
        message("Cannot use XTest without X11. Disabling XTest support.")
    endif(WITH_XTEST AND NOT WITH_X11)

    if(WITH_XTEST)
        message("XTest support allowed for simulating events.")
    endif(WITH_XTEST)

    if(WITH_UINPUT)
        message("uinput support allowed for simulating events.")
    else()
        set(INSTALL_UINPUT_UDEV_RULES OFF)
    endif(WITH_UINPUT)

    if(NOT WITH_XTEST AND NOT WITH_UINPUT)
        message(FATAL_ERROR "No system is defined for simulating events.")
    endif(NOT WITH_XTEST AND NOT WITH_UINPUT)
endif(UNIX)

###############################
# PLATFORM-INDEPENDENT FILES
###############################

set(antimicrox_MAIN src/main.cpp)

set(antimicrox_SOURCES
        src/antimicrosettings.cpp
        src/antkeymapper.cpp
        src/applaunchhelper.cpp
        src/autoprofileinfo.cpp
        src/axisvaluebox.cpp
        src/commandlineutility.cpp
        src/common.cpp
        src/dpadcontextmenu.cpp
        src/dpadpushbutton.cpp
        src/dpadpushbuttongroup.cpp
        src/event.cpp
        src/eventhandlerfactory.cpp
        src/eventhandlers/baseeventhandler.cpp
        src/gamecontroller/gamecontroller.cpp
        src/gamecontroller/gamecontrollerdpad.cpp
        src/gamecontroller/gamecontrollerset.cpp
        src/gamecontroller/gamecontrollertrigger.cpp
        src/gamecontroller/gamecontrollertriggerbutton.cpp
        src/gamecontrollerexample.cpp
        src/globalvariables.cpp
        src/gui/aboutdialog.cpp
        src/gui/addeditautoprofiledialog.cpp
        src/gui/advancebuttondialog.cpp
        src/gui/advancestickassignmentdialog.cpp
        src/gui/axiseditdialog.cpp
        src/gui/buttoneditdialog.cpp
        src/gui/calibration.cpp
        src/gui/dpadeditdialog.cpp
        src/gui/editalldefaultautoprofiledialog.cpp
        src/gui/extraprofilesettingsdialog.cpp
        src/gui/flashbuttonwidget.cpp
        src/gui/gamecontrollermappingdialog.cpp
        src/gui/joyaxiswidget.cpp
        src/gui/joybuttonwidget.cpp
        src/gui/joycontrolstickeditdialog.cpp
        src/gui/joydpadbuttonwidget.cpp
        src/gui/joysensoreditdialog.cpp
        src/gui/joystickstatuswindow.cpp
        src/gui/joytabwidget.cpp
        src/gui/joytabwidgetcontainer.cpp
        src/gui/mainsettingsdialog.cpp
        src/gui/mainwindow.cpp
        src/gui/mousesettingsdialog.cpp
        src/gui/qkeydisplaydialog.cpp
        src/gui/quicksetdialog.cpp
        src/gui/setaxisthrottledialog.cpp
        src/gui/setnamesdialog.cpp
        src/gui/slotitemlistwidget.cpp
        src/haptictriggerps5.cpp
        src/inputdaemon.cpp
        src/inputdevice.cpp
        src/inputdevicebitarraystatus.cpp
        src/inputdevicecalibration.cpp
        src/joyaccelerometersensor.cpp
        src/joyaxis.cpp
        src/joyaxiscontextmenu.cpp
        src/joybuttoncontextmenu.cpp
        src/joybuttonmousehelper.cpp
        src/joybuttonslot.cpp
        src/joybuttonstatusbox.cpp
        src/joybuttontypes/joybutton.cpp
        src/joybuttontypes/joyaccelerometerbutton.cpp
        src/joybuttontypes/joyaxisbutton.cpp
        src/joybuttontypes/joycontrolstickbutton.cpp
        src/joybuttontypes/joycontrolstickmodifierbutton.cpp
        src/joybuttontypes/joydpadbutton.cpp
        src/joybuttontypes/joygradientbutton.cpp
        src/joybuttontypes/joygyroscopebutton.cpp
        src/joybuttontypes/joysensorbutton.cpp
        src/joycontrolstick.cpp
        src/joycontrolstickbuttonpushbutton.cpp
        src/joycontrolstickcontextmenu.cpp
        src/joycontrolstickpushbutton.cpp
        src/joycontrolstickstatusbox.cpp
        src/joydpad.cpp
        src/joygyroscopesensor.cpp
        src/joysensor.cpp
        src/joysensorbuttonpushbutton.cpp
        src/joysensorcontextmenu.cpp
        src/joysensorfactory.cpp
        src/joysensorpreset.cpp
        src/joysensorpushbutton.cpp
        src/joysensorstatusbox.cpp
        src/joystick.cpp
        src/keyboard/virtualkeyboardmousewidget.cpp
        src/keyboard/virtualkeypushbutton.cpp
        src/keyboard/virtualmousepushbutton.cpp
        src/localantimicroserver.cpp
        src/logger.cpp
        src/mousedialog/mouseaxissettingsdialog.cpp
        src/mousedialog/mousebuttonsettingsdialog.cpp
        src/mousedialog/mousecontrolsticksettingsdialog.cpp
        src/mousedialog/mousedpadsettingsdialog.cpp
        src/mousedialog/mousesensorsettingsdialog.cpp
        src/mousedialog/springmoderegionpreview.cpp
        src/mousedialog/uihelpers/mouseaxissettingsdialoghelper.cpp
        src/mousedialog/uihelpers/mousebuttonsettingsdialoghelper.cpp
        src/mousedialog/uihelpers/mousecontrolsticksettingsdialoghelper.cpp
        src/mousedialog/uihelpers/mousedpadsettingsdialoghelper.cpp
        src/mousehelper.cpp
        src/pt1filter.cpp
        src/qtkeymapperbase.cpp
        src/sdleventreader.cpp
        src/sensorpushbuttongroup.cpp
        src/setjoystick.cpp
        src/simplekeygrabberbutton.cpp
        src/statisticsestimator.cpp
        src/stickpushbuttongroup.cpp
        src/uihelpers/advancebuttondialoghelper.cpp
        src/uihelpers/buttoneditdialoghelper.cpp
        src/uihelpers/dpadcontextmenuhelper.cpp
        src/uihelpers/dpadeditdialoghelper.cpp
        src/uihelpers/gamecontrollermappingdialoghelper.cpp
        src/uihelpers/joyaxiscontextmenuhelper.cpp
        src/uihelpers/joycontrolstickcontextmenuhelper.cpp
        src/uihelpers/joycontrolstickeditdialoghelper.cpp
        src/uihelpers/joysensoriothreadhelper.cpp
        src/uihelpers/joytabwidgethelper.cpp
        src/vdpad.cpp
        src/xml/inputdevicexml.cpp
        src/xml/joyaxisxml.cpp
        src/xml/joybuttonslotxml.cpp
        src/xml/joybuttonxml.cpp
        src/xml/joydpadxml.cpp
        src/xml/setjoystickxml.cpp
        src/xmlconfigmigration.cpp
        src/xmlconfigreader.cpp
        src/xmlconfigwriter.cpp
        )

set(antimicrox_HEADERS
        src/antimicrosettings.h
        src/antkeymapper.h
        src/applaunchhelper.h
        src/autoprofileinfo.h
        src/axisvaluebox.h
        src/commandlineutility.h
        src/dpadcontextmenu.h
        src/dpadpushbutton.h
        src/dpadpushbuttongroup.h
        src/eventhandlerfactory.h
        src/eventhandlers/baseeventhandler.h
        src/gamecontroller/gamecontroller.h
        src/gamecontroller/gamecontrollerdpad.h
        src/gamecontroller/gamecontrollerset.h
        src/gamecontroller/gamecontrollertrigger.h
        src/gamecontroller/gamecontrollertriggerbutton.h
        src/gamecontroller/xml/gamecontrollerdpadxml.h
        src/gamecontroller/xml/gamecontrollertriggerxml.h
        src/gamecontroller/xml/gamecontrollerxml.h
        src/gamecontrollerexample.h
        src/globalvariables.h
        src/gui/aboutdialog.h
        src/gui/addeditautoprofiledialog.h
        src/gui/advancebuttondialog.h
        src/gui/advancestickassignmentdialog.h
        src/gui/axiseditdialog.h
        src/gui/buttoneditdialog.h
        src/gui/calibration.h
        src/gui/dpadeditdialog.h
        src/gui/editalldefaultautoprofiledialog.h
        src/gui/extraprofilesettingsdialog.h
        src/gui/flashbuttonwidget.h
        src/gui/gamecontrollermappingdialog.h
        src/gui/joyaxiswidget.h
        src/gui/joybuttonwidget.h
        src/gui/joycontrolstickeditdialog.h
        src/gui/joydpadbuttonwidget.h
        src/gui/joysensoreditdialog.h
        src/gui/joystickstatuswindow.h
        src/gui/joytabwidget.h
        src/gui/joytabwidgetcontainer.h
        src/gui/mainsettingsdialog.h
        src/gui/mainwindow.h
        src/gui/mousesettingsdialog.h
        src/gui/qkeydisplaydialog.h
        src/gui/quicksetdialog.h
        src/gui/setaxisthrottledialog.h
        src/gui/setnamesdialog.h
        src/gui/slotitemlistwidget.h
        src/haptictriggerps5.h
        src/haptictriggermodeps5.h
        src/inputdaemon.h
        src/inputdevice.h
        src/inputdevicebitarraystatus.h
        src/inputdevicecalibration.h
        src/joyaccelerometersensor.h
        src/joyaxis.h
        src/joyaxiscontextmenu.h
        src/joybuttoncontextmenu.h
        src/joybuttonmousehelper.h
        src/joybuttonslot.h
        src/joybuttonstatusbox.h
        src/joybuttontypes/joybutton.h
        src/joybuttontypes/joyaccelerometerbutton.h
        src/joybuttontypes/joyaxisbutton.h
        src/joybuttontypes/joycontrolstickbutton.h
        src/joybuttontypes/joycontrolstickmodifierbutton.h
        src/joybuttontypes/joydpadbutton.h
        src/joybuttontypes/joygradientbutton.h
        src/joybuttontypes/joygyroscopebutton.h
        src/joybuttontypes/joysensorbutton.h
        src/joycontrolstick.h
        src/joycontrolstickbuttonpushbutton.h
        src/joycontrolstickcontextmenu.h
        src/joycontrolstickpushbutton.h
        src/joycontrolstickstatusbox.h
        src/joydpad.h
        src/joygyroscopesensor.h
        src/joysensor.h
        src/joysensorbuttonpushbutton.h
        src/joysensorcontextmenu.h
        src/joysensordirection.h
        src/joysensorfactory.h
        src/joysensorpreset.h
        src/joysensorpushbutton.h
        src/joysensorstatusbox.h
        src/joysensortype.h
        src/joystick.h
        src/keyboard/virtualkeyboardmousewidget.h
        src/keyboard/virtualkeypushbutton.h
        src/keyboard/virtualmousepushbutton.h
        src/localantimicroserver.h
        src/logger.h
        src/mousedialog/mouseaxissettingsdialog.h
        src/mousedialog/mousebuttonsettingsdialog.h
        src/mousedialog/mousecontrolsticksettingsdialog.h
        src/mousedialog/mousedpadsettingsdialog.h
        src/mousedialog/springmoderegionpreview.h
        src/mousedialog/mousesensorsettingsdialog.h
        src/mousedialog/uihelpers/mouseaxissettingsdialoghelper.h
        src/mousedialog/uihelpers/mousebuttonsettingsdialoghelper.h
        src/mousedialog/uihelpers/mousecontrolsticksettingsdialoghelper.h
        src/mousedialog/uihelpers/mousedpadsettingsdialoghelper.h
        src/mousehelper.h
        src/pt1filter.h
        src/qtkeymapperbase.h
        src/sdleventreader.h
        src/sensorpushbuttongroup.h
        src/setjoystick.h
        src/simplekeygrabberbutton.h
        src/statisticsestimator.h
        src/stickpushbuttongroup.h
        src/uihelpers/advancebuttondialoghelper.h
        src/uihelpers/buttoneditdialoghelper.h
        src/uihelpers/dpadcontextmenuhelper.h
        src/uihelpers/dpadeditdialoghelper.h
        src/uihelpers/gamecontrollermappingdialoghelper.h
        src/uihelpers/joyaxiscontextmenuhelper.h
        src/uihelpers/joycontrolstickcontextmenuhelper.h
        src/uihelpers/joycontrolstickeditdialoghelper.h
        src/uihelpers/joysensoriothreadhelper.h
        src/uihelpers/joytabwidgethelper.h
        src/vdpad.h
        src/xml/inputdevicexml.h
        src/xml/joyaxisxml.h
        src/xml/joybuttonslotxml.h
        src/xml/joybuttonxml.h
        src/xml/joydpadxml.h
        src/xml/setjoystickxml.h
        src/xmlconfigmigration.h
        src/xmlconfigreader.h
        src/xmlconfigwriter.h
        )

if(ATTACH_FAKE_CLASSES)
    LIST(APPEND antimicrox_SOURCES
            src/fakeclasses/xbox360wireless.cpp
            )

    LIST(APPEND antimicrox_HEADERS
            src/fakeclasses/xbox360wireless.h
            )
endif(ATTACH_FAKE_CLASSES)

set(antimicrox_FORMS
        src/gui/aboutdialog.ui
        src/gui/addeditautoprofiledialog.ui
        src/gui/advancebuttondialog.ui
        src/gui/advancestickassignmentdialog.ui
        src/gui/axiseditdialog.ui
        src/gui/buttoneditdialog.ui
        src/gui/calibration.ui
        src/gui/capturedwindowinfodialog.ui
        src/gui/dpadeditdialog.ui
        src/gui/editalldefaultautoprofiledialog.ui
        src/gui/extraprofilesettingsdialog.ui
        src/gui/gamecontrollermappingdialog.ui
        src/gui/joycontrolstickeditdialog.ui
        src/gui/joysensoreditdialog.ui
        src/gui/joystickstatuswindow.ui
        src/gui/mainsettingsdialog.ui
        src/gui/mainwindow.ui
        src/gui/mousesettingsdialog.ui
        src/gui/qkeydisplaydialog.ui
        src/gui/quicksetdialog.ui
        src/gui/setaxisthrottledialog.ui
        src/gui/setnamesdialog.ui
        )

set(antimicrox_RESOURCES src/resources.qrc)

###############################
# PLATFORM-DEPENDENT FILES
###############################

if(UNIX)
    if(WITH_X11)
        LIST(APPEND antimicrox_SOURCES src/x11extras.cpp
                src/qtx11keymapper.cpp
                src/unixcapturewindowutility.cpp
                src/autoprofilewatcher.cpp
                src/gui/capturedwindowinfodialog.cpp
                )
        LIST(APPEND antimicrox_HEADERS src/x11extras.h
                src/qtx11keymapper.h
                src/unixcapturewindowutility.h
                src/autoprofilewatcher.h
                src/gui/capturedwindowinfodialog.h
                )

        if(WITH_XTEST)
            LIST(APPEND antimicrox_SOURCES src/eventhandlers/xtesteventhandler.cpp)
            LIST(APPEND antimicrox_HEADERS src/eventhandlers/xtesteventhandler.h)
        endif(WITH_XTEST)
    endif(WITH_X11)

    if(WITH_UINPUT)
        LIST(APPEND antimicrox_SOURCES src/qtuinputkeymapper.cpp
                src/uinputhelper.cpp
                src/eventhandlers/uinputeventhandler.cpp
                )
        LIST(APPEND antimicrox_HEADERS src/qtuinputkeymapper.h
                src/uinputhelper.h
                src/eventhandlers/uinputeventhandler.h
                )
    endif(WITH_UINPUT)

elseif(WIN32)
    LIST(APPEND antimicrox_SOURCES
        src/autoprofilewatcher.cpp
        src/winextras.cpp
         src/qtwinkeymapper.cpp
         src/gui/winappprofiletimerdialog.cpp
         src/gui/capturedwindowinfodialog.cpp
         src/eventhandlers/winsendinputeventhandler.cpp
         src/joykeyrepeathelper.cpp
    )
    LIST(APPEND antimicrox_HEADERS
        src/autoprofilewatcher.h
        src/winextras.h
        src/qtwinkeymapper.h
        src/gui/winappprofiletimerdialog.h
        src/gui/capturedwindowinfodialog.h
        src/eventhandlers/winsendinputeventhandler.h
        src/joykeyrepeathelper.h
    )
    LIST(APPEND antimicrox_FORMS
        src/gui/winappprofiletimerdialog.ui    
    )
endif(UNIX)

###############################
# DEFINITIONS
###############################

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

###############################
# REQUIRED PACKAGES
###############################

#QT
set(QT_REQUIRED_VERSION 5.10)
set(CMAKE_AUTOMOC TRUE) # required by moc preprocessor used in QT

if(USE_QT6_BY_DEFAULT)
    find_package(Qt6 COMPONENTS Core)
    if(NOT Qt6_FOUND)
        message("QT6 not found. Falling back to QT5.")
    endif()
else()
    find_package(Qt5 COMPONENTS Core)
    if(NOT Qt5_FOUND)
        message("QT5 not found. Falling back to QT6.")
    endif()
endif()

if (Qt6_FOUND OR NOT Qt5_FOUND)
    find_package(Qt6 ${QT_REQUIRED_VERSION} 
        COMPONENTS Core Widgets Gui Network LinguistTools Concurrent
        REQUIRED)
    find_package(QT NAMES Qt6 REQUIRED) # used to get version number in QT_VERSION_MAJOR and QT_VERSION

    QT_WRAP_UI(antimicrox_FORMS_HEADERS ${antimicrox_FORMS})
    QT_ADD_RESOURCES(antimicrox_RESOURCES_RCC ${antimicrox_RESOURCES})
else()
    find_package(Qt5 ${QT_REQUIRED_VERSION} 
        COMPONENTS Core Widgets Gui Network LinguistTools Concurrent
        REQUIRED)
    find_package(QT NAMES Qt5 REQUIRED) # used to get version number in QT_VERSION_MAJOR and QT_VERSION

    QT5_WRAP_UI(antimicrox_FORMS_HEADERS ${antimicrox_FORMS})
    QT5_ADD_RESOURCES(antimicrox_RESOURCES_RCC ${antimicrox_RESOURCES})
endif()
message("Found QT: ${QT_VERSION}")

add_subdirectory("share/antimicrox/translations")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# SDL
set(SDL_REQUIRED_VERSION 2.0.6)
find_package(SDL2 REQUIRED)

# Others
if(WITH_X11)
    find_package(X11 REQUIRED)
endif(WITH_X11)

###############################
# LINKER LIBRARIES
###############################

LIST(APPEND QT_LIBS Qt${QT_VERSION_MAJOR}::Core)
LIST(APPEND QT_LIBS Qt${QT_VERSION_MAJOR}::Gui)
LIST(APPEND QT_LIBS Qt${QT_VERSION_MAJOR}::Widgets)
LIST(APPEND QT_LIBS Qt${QT_VERSION_MAJOR}::Concurrent)
LIST(APPEND QT_LIBS Qt${QT_VERSION_MAJOR}::Network)

if(UNIX)
    if(WITH_X11)
        LIST(APPEND X11_LIBS ${X11_X11_LIB})
        LIST(APPEND X11_LIBS ${X11_Xi_LIB})
    endif(WITH_X11)

    if(WITH_XTEST)
        LIST(APPEND EXTRA_LIBS ${X11_XTest_LIB})
    endif(WITH_XTEST)

     # necessary ifwe use find_package for SDL2
     #    if(NOT DEFINED SDL2_LIBRARIES)
#        set(SDL2_LIBRARIES SDL2::SDL2)
#    endif()
elseif (WIN32)
    list(APPEND WIN_LIBS "psapi")
endif(UNIX)


include_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
        "${CMAKE_CURRENT_BINARY_DIR}/src"
        ${PROJECT_SOURCE_DIR}
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/src/gui"
        ${SDL2_INCLUDE_DIRS}
)


# Store executable in a bin subdir. Needed here so translations can be loaded.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

###############################
# COMPILE & LINK
###############################
if(UNIX)
    add_executable(antimicrox
        ${antimicrox_MAIN}
        ${antimicrox_HEADERS_MOC}
        ${antimicrox_SOURCES}
        ${antimicrox_FORMS_HEADERS}
        ${antimicrox_RESOURCES_RCC}
        )
elseif(WIN32)
    # The WIN32 is required to specify a GUI application.
    add_executable(antimicrox WIN32
        ${antimicrox_MAIN}
        ${antimicrox_HEADERS_MOC}
        ${antimicrox_SOURCES}
        ${antimicrox_FORMS_HEADERS}
        ${antimicrox_RESOURCES_RCC}
        src/antimicrox_windows_icon.rc # Embedding icon into Windows executable
    )
    target_link_libraries(antimicrox
        ${WIN_LIBS}
    )

    if(PORTABLE_WIN)
        add_definitions(-DWIN_PORTABLE_PACKAGE)
    endif()
endif(UNIX)

target_link_libraries(antimicrox
        ${QT_LIBS}
        ${X11_LIBS}
        ${SDL2_LIBRARIES}
        ${EXTRA_LIBS}
        )

target_include_directories(antimicrox PUBLIC
        ${SDL2_INCLUDE_DIRS}/SDL2
        )

###############################
# INSTALL
###############################

# Specify out directory for final executable.
install(TARGETS antimicrox RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

if(UNIX)
    find_package(ECM REQUIRED NO_MODULE)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_DIR})
    include(ECMInstallIcons)

    ecm_install_icons(ICONS share/icons/application/16x16/apps/16-apps-io.github.antimicrox.antimicrox.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons)
    ecm_install_icons(ICONS share/icons/application/24x24/apps/24-apps-io.github.antimicrox.antimicrox.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons)
    ecm_install_icons(ICONS share/icons/application/32x32/apps/32-apps-io.github.antimicrox.antimicrox.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons)
    ecm_install_icons(ICONS share/icons/application/48x48/apps/48-apps-io.github.antimicrox.antimicrox.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons)
    ecm_install_icons(ICONS share/icons/application/64x64/apps/64-apps-io.github.antimicrox.antimicrox.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons)
    ecm_install_icons(ICONS share/icons/application/128x128/apps/128-apps-io.github.antimicrox.antimicrox.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons)
    ecm_install_icons(ICONS share/icons/application/scalable/apps/sc-apps-io.github.antimicrox.antimicrox.svg DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons)

    ecm_install_icons(ICONS share/icons/breeze_themed/48-apps-antimicrox_trayicon.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons THEME breeze)
    ecm_install_icons(ICONS src/images/48-apps-antimicrox_trayicon.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons)

    install(FILES CHANGELOG.md DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/doc/antimicrox")
    install(FILES other/io.github.antimicrox.antimicrox.desktop DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
    install(FILES other/io.github.antimicrox.antimicrox.xml DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/mime/packages")

    # Copy current Changelog file to location that the resource file expects.
    install(FILES CHANGELOG.md DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/antimicrox")
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

if(UNIX)
    if(INSTALL_UINPUT_UDEV_RULES)
        message("Udev rules installation enabled.")
        install(FILES other/60-antimicrox-uinput.rules DESTINATION "/usr/lib/udev/rules.d/")
    endif(INSTALL_UINPUT_UDEV_RULES)
endif(UNIX)

# Copy libraries needed by Windows
if(WIN32)
    add_custom_command(TARGET antimicrox POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "dll files linked with antimicrox executable: $<TARGET_RUNTIME_DLLS:antimicrox> "
        COMMAND_EXPAND_LISTS
        )
    add_custom_command(TARGET antimicrox POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:antimicrox> $<TARGET_FILE_DIR:antimicrox>
        COMMAND_EXPAND_LISTS
        )

    find_path(SDL2_DLL_LOCATION_DIR "SDL2.dll" HINTS "${SDL2_LIBRARY_DIRS}/../bin")

    #these three lines assume SDL2 in standard location TODO do it better
    find_path(GCC_DLL "libgcc_s_seh-1.dll" HINTS "${SDL2_LIBRARY_DIRS}/../bin")
    find_path(PTHREAD_DLL_DIR "libwinpthread-1.dll" HINTS "${SDL2_LIBRARY_DIRS}/../bin")
    find_path(LIBCPP_DLL_DIR "libstdc++-6.dll" HINTS "${SDL2_LIBRARY_DIRS}/../bin")
    list(APPEND DLL_FILES
        "${CMAKE_CURRENT_BINARY_DIR}/bin/Qt${QT_VERSION_MAJOR}Core.dll"
        "${CMAKE_CURRENT_BINARY_DIR}/bin/Qt${QT_VERSION_MAJOR}Gui.dll"
        "${CMAKE_CURRENT_BINARY_DIR}/bin/Qt${QT_VERSION_MAJOR}Concurrent.dll"
        "${CMAKE_CURRENT_BINARY_DIR}/bin/Qt${QT_VERSION_MAJOR}Network.dll"
        "${CMAKE_CURRENT_BINARY_DIR}/bin/Qt${QT_VERSION_MAJOR}Widgets.dll"
        "${SDL2_DLL_LOCATION_DIR}/SDL2.dll"
        "${GCC_DLL}/libgcc_s_seh-1.dll"
        "${PTHREAD_DLL_DIR}/libwinpthread-1.dll"
        "${LIBCPP_DLL_DIR}/libstdc++-6.dll"
        )

    if(QT_VERSION_MAJOR EQUAL 5)
        list(APPEND DLL_FILES
            "${CMAKE_CURRENT_BINARY_DIR}/bin/libEGL.dll"
            )
    endif(QT_VERSION_MAJOR EQUAL 5)
    

    if(CHECK_FOR_UPDATES)
        message("Searching for SSL DLL-s.")
        find_path(LIBCRYPTO_DLL_DIR "libcrypto-1_1-x64.dll" HINTS "${SDL2_LIBRARY_DIRS}/../bin")
        find_path(LIBSSL_DLL_DIR "libssl-1_1-x64.dll" HINTS "${SDL2_LIBRARY_DIRS}/../bin")
        list(APPEND DLL_FILES
        "${LIBCRYPTO_DLL_DIR}/libcrypto-1_1-x64.dll"
        "${LIBSSL_DLL_DIR}/libssl-1_1-x64.dll"
        )
    endif(CHECK_FOR_UPDATES)

    install(FILES ${DLL_FILES} TYPE BIN )

    # Required platform plugin
    get_target_property(QtCore_LOCATION Qt${QT_VERSION_MAJOR}::Core LOCATION)
    get_filename_component(QT_DLL_DIR ${QtCore_LOCATION} PATH)
    install(FILES
        ${QT_DLL_DIR}/../plugins/platforms/qwindows.dll
        DESTINATION ./bin/platforms/)
endif(WIN32)

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

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/src/)

if(WITH_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif(WITH_TESTS)

# Install SDL database with linked License file
if(UNIX)
    install(FILES share/gamecontrollerdb_linux.txt DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/antimicrox/" RENAME gamecontrollerdb.txt)
elseif(WIN32)
    install(FILES share/gamecontrollerdb_windows.txt DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/antimicrox/" RENAME gamecontrollerdb.txt)
endif(UNIX)
install(FILES share/LICENSE_SDL_GameControllerDB DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/antimicrox/")

###############################
# PACKAGES BUILDING WITH CPACK
###############################

include(InstallRequiredSystemLibraries)


set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Program used to map keyboard keys and mouse controls to a gamepad.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/AntiMicroX/antimicrox/")


set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

set(CPACK_PACKAGE_VERSION_MAJOR "${ANTIMICROX_MAJOR_VERSION}")
set(CPACK_PACKAGE_VERSION_MINOR "${ANTIMICROX_MINOR_VERSION}")
set(CPACK_PACKAGE_VERSION_PATCH "${ANTIMICROX_PATCH_VERSION}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CPACK_PACKAGE_VERSION_PATCH "${CPACK_PACKAGE_VERSION_PATCH}-dbg")
endif()

set(CPACK_PACKAGE_FILE_NAME "antimicrox-${ANTIMICROX_MAJOR_VERSION}.${ANTIMICROX_MINOR_VERSION}.${ANTIMICROX_PATCH_VERSION}")
if(WIN32)
    if(PORTABLE_WIN)
        set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-PortableWindows")
    else(PORTABLE_WIN)
        set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-Windows")
    endif(PORTABLE_WIN)
endif(WIN32)
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-${CMAKE_HOST_SYSTEM_PROCESSOR}")

set(CPACK_PACKAGE_INSTALL_DIRECTORY "AntiMicroX")

if(UNIX)
    set(CPACK_STRIP_FILES "")
    set(CPACK_SOURCE_STRIP_FILES "")
endif()

# Building .deb package
if(CPACK_GENERATOR STREQUAL "DEB")
    if(QT_VERSION_MAJOR EQUAL 5)
        # TODO use regular, not dev packages
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5widgets5 (>= ${QT_REQUIRED_VERSION}), libsdl2-2.0-0 (>= ${SDL_REQUIRED_VERSION}), libc6")
    else()
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6widgets6 (>= ${QT_REQUIRED_VERSION}), libsdl2-2.0-0 (>= ${SDL_REQUIRED_VERSION}), libc6")
    endif(QT_VERSION_MAJOR EQUAL 5)
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "pktiuk <kotiuk@zohomail.eu>")
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "AntiMicroX is a graphical program used to map gamepad buttons to keyboard, mouse, scripts and macros.

It is a new fork of discontinued AntiMicro.")

    message("Preparing documentation for DEB package")
    add_custom_target(package_docummentation ALL)

    #Compress changelog and save it as share/doc/xournalpp/changelog.Debian.gz
    add_custom_command(TARGET package_docummentation PRE_BUILD
            COMMAND gzip -c -9 -n "${PROJECT_SOURCE_DIR}/CHANGELOG.md" > "changelog.gz" VERBATIM)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz" DESTINATION "share/doc/antimicrox/")

    #Strip binaries from unnecessary notes, comments, etc
    add_custom_command(TARGET antimicrox POST_BUILD
            COMMAND strip --strip-unneeded --remove-section=.comment --remove-section=.note "${CMAKE_CURRENT_BINARY_DIR}/bin/antimicrox" VERBATIM)
endif()

if(WIN32 AND NOT UNIX)
    if(NOT PORTABLE_WIN)
        # We set NSIS as generator for Windows
        set(CPACK_GENERATOR NSIS)

        # There is a bug in NSI that does not handle full UNIX paths properly.
        # Make sure there is at least one set of four backlashes.
        set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/src/images\\\\antimicrox.ico")
        set(CPACK_NSIS_MUI_ICON "${CPACK_PACKAGE_ICON}")
        set(CPACK_NSIS_MUI_UNIICON "${CPACK_PACKAGE_ICON}")
        set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\antimicrox.exe")

        set(CPACK_NSIS_DISPLAY_NAME "AntiMicroX")
        set(CPACK_NSIS_PACKAGE_NAME "AntiMicroX")
        set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\github.com/AntiMicroX/antimicrox/wiki")
        set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\github.com/AntiMicroX/antimicrox")
        set(CPACK_NSIS_MODIFY_PATH ON)
    else()
        set(CPACK_GENERATOR ZIP)
    endif()
endif()

set(CPACK_PACKAGE_EXECUTABLES "antimicrox;AntiMicroX")
set(CPACK_CREATE_DESKTOP_LINKS "antimicrox")
include(CPack)

if(WIN32 AND NOT UNIX AND NOT PORTABLE_WIN)
configure_file(
    ${PROJECT_SOURCE_DIR}/cmake/modules/NSIS.definitions.nsh.in
    ${CMAKE_CURRENT_BINARY_DIR}/assets/nsis/NSIS.definitions.nsh
    )
endif()


###############################
# DOCS
###############################

if(BUILD_DOCS)
    message("Doxygen build enabled")
    find_package(Doxygen)

    if (DOXYGEN_FOUND)
        # set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/other/Doxyfile_config.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/docs/Doxyfile)

        # request to configure the file
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

        # note the option ALL which allows to build the docs together with the application
        add_custom_target( doxygen_docs ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM )

    else (DOXYGEN_FOUND)
      message(WARNING "Doxygen need to be installed to generate the doxygen documentation")
    endif (DOXYGEN_FOUND)
endif(BUILD_DOCS)
