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

# TODO
set(SDL2_DIR "C:/Users/HP/Desktop/prog/antimicrox/SDL2-2.0.12/i686-w64-mingw32")

cmake_minimum_required(VERSION 3.6)
set(QT_REQUIRED_VERSION 5.8)
set(SDL_REQUIRED_VERSION 2.0.6)
project(antimicrox)

# already default?
# if(POLICY CMP0071)
#     cmake_policy(SET CMP0071 NEW)
# endif()

# if(UNIX)
#     find_package(ECM REQUIRED NO_MODULE)
#     set(CMAKE_MODULE_PATH ${ECM_MODULE_DIR})
# endif(UNIX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(UNIX AND NOT APPLE AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "force cmake prefix to be set for /usr" FORCE)
endif(UNIX AND NOT APPLE AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

include(CheckCXXCompilerFlag)
include(GNUInstallDirs)

if(UNIX)
    include(ECMInstallIcons)
endif(UNIX)

CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)

if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif(COMPILER_SUPPORTS_CXX11)

# The version number.
set(ANTIMICROX_MAJOR_VERSION 3)
set(ANTIMICROX_MINOR_VERSION 1)
set(ANTIMICROX_PATCH_VERSION 3)

option(WITH_TESTS "Allow tests for classes" OFF)

if(WITH_TESTS)
    message("Tests enabled")
endif(WITH_TESTS)

if(UNIX)
    option(WITH_X11 "Compile with support for X11." ON)
    option(WITH_UINPUT "Compile with support for uinput. uinput will be usable to simulate events." ON)
    option(WITH_XTEST "Compile with support for XTest.  XTest will be usable to simulate events." ON)
    option(APPDATA "Build project with AppData file support." ON)
endif(UNIX)

option(UPDATE_TRANSLATIONS "Call lupdate to update translation files from source." OFF)
option(TRANS_KEEP_OBSOLETE "Add -noobsolete option to lupdate command to get rid of old text entries" OFF)
option(ATTACH_FAKE_CLASSES "Fake classes can be used in application to tests functionalities" OFF)


if(UNIX)
    # find_package(PkgConfig REQUIRED)

    # pkg_check_modules(SDL2 REQUIRED sdl2)
#    find_package(SDL2 REQUIRED) -> will fail on some systems

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

if(WIN32)
    option(PORTABLE_PACKAGE "Create portable Windows package" OFF)
    #option(TARGET_ARCH "Choose which version of some libraries to use. (x86, x86_64)" "x86")
    # option(WITH_VMULTI "Compile with support for vmulti." OFF)
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

    # if(WITH_VMULTI)
    #     add_definitions(-DWITH_VMULTI)
    # endif(WITH_VMULTI)
endif(WIN32)

set(antimicrox_MAIN src/main.cpp)

set(antimicrox_SOURCES
        src/aboutdialog.cpp
        src/addeditautoprofiledialog.cpp
        src/advancebuttondialog.cpp
        src/advancestickassignmentdialog.cpp
        src/antimicrosettings.cpp
        src/antkeymapper.cpp
        src/applaunchhelper.cpp
        src/autoprofileinfo.cpp
        src/axiseditdialog.cpp
        src/axisvaluebox.cpp
        src/buttoneditdialog.cpp
        src/calibration.cpp
        src/commandlineutility.cpp
        src/common.cpp
        src/dpadcontextmenu.cpp
        src/dpadeditdialog.cpp
        src/dpadpushbutton.cpp
        src/dpadpushbuttongroup.cpp
        src/editalldefaultautoprofiledialog.cpp
        src/event.cpp
        src/eventhandlerfactory.cpp
        src/eventhandlers/baseeventhandler.cpp
        src/extraprofilesettingsdialog.cpp
        src/flashbuttonwidget.cpp
        src/gamecontroller/gamecontroller.cpp
        src/gamecontroller/gamecontrollerdpad.cpp
        src/gamecontroller/gamecontrollerset.cpp
        src/gamecontroller/gamecontrollertrigger.cpp
        src/gamecontroller/gamecontrollertriggerbutton.cpp
        src/gamecontroller/xml/gamecontrollerdpadxml.cpp
        src/gamecontroller/xml/gamecontrollertriggerxml.cpp
        src/gamecontroller/xml/gamecontrollerxml.cpp
        src/gamecontrollerexample.cpp
        src/gamecontrollermappingdialog.cpp
        src/globalvariables.cpp
        src/inputdaemon.cpp
        src/inputdevice.cpp
        src/inputdevicebitarraystatus.cpp
        src/joyaxis.cpp
        src/joyaxiscontextmenu.cpp
        src/joyaxiswidget.cpp
        src/joybutton.cpp
        src/joybuttoncontextmenu.cpp
        src/joybuttonmousehelper.cpp
        src/joybuttonslot.cpp
        src/joybuttonstatusbox.cpp
        src/joybuttontypes/joyaxisbutton.cpp
        src/joybuttontypes/joycontrolstickbutton.cpp
        src/joybuttontypes/joycontrolstickmodifierbutton.cpp
        src/joybuttontypes/joydpadbutton.cpp
        src/joybuttontypes/joygradientbutton.cpp
        src/joybuttonwidget.cpp
        src/joycontrolstick.cpp
        src/joycontrolstickbuttonpushbutton.cpp
        src/joycontrolstickcontextmenu.cpp
        src/joycontrolstickeditdialog.cpp
        src/joycontrolstickpushbutton.cpp
        src/joycontrolstickstatusbox.cpp
        src/joydpad.cpp
        src/joydpadbuttonwidget.cpp
        src/joystick.cpp
        src/joystickstatuswindow.cpp
        src/joytabwidget.cpp
        src/joytabwidgetcontainer.cpp
        src/keyboard/virtualkeyboardmousewidget.cpp
        src/keyboard/virtualkeypushbutton.cpp
        src/keyboard/virtualmousepushbutton.cpp
        src/localantimicroserver.cpp
        src/logger.cpp
        src/mainsettingsdialog.cpp
        src/mainwindow.cpp
        src/messagehandler.cpp
        src/mousedialog/mouseaxissettingsdialog.cpp
        src/mousedialog/mousebuttonsettingsdialog.cpp
        src/mousedialog/mousecontrolsticksettingsdialog.cpp
        src/mousedialog/mousedpadsettingsdialog.cpp
        src/mousedialog/springmoderegionpreview.cpp
        src/mousedialog/uihelpers/mouseaxissettingsdialoghelper.cpp
        src/mousedialog/uihelpers/mousebuttonsettingsdialoghelper.cpp
        src/mousedialog/uihelpers/mousecontrolsticksettingsdialoghelper.cpp
        src/mousedialog/uihelpers/mousedpadsettingsdialoghelper.cpp
        src/mousehelper.cpp
        src/mousesettingsdialog.cpp
        src/qkeydisplaydialog.cpp
        src/qtkeymapperbase.cpp
        src/quicksetdialog.cpp
        src/sdleventreader.cpp
        src/setaxisthrottledialog.cpp
        src/setjoystick.cpp
        src/setnamesdialog.cpp
        src/simplekeygrabberbutton.cpp
        src/slotitemlistwidget.cpp
        src/stickpushbuttongroup.cpp
        src/uihelpers/advancebuttondialoghelper.cpp
        src/uihelpers/buttoneditdialoghelper.cpp
        src/uihelpers/dpadcontextmenuhelper.cpp
        src/uihelpers/dpadeditdialoghelper.cpp
        src/uihelpers/gamecontrollermappingdialoghelper.cpp
        src/uihelpers/joyaxiscontextmenuhelper.cpp
        src/uihelpers/joycontrolstickcontextmenuhelper.cpp
        src/uihelpers/joycontrolstickeditdialoghelper.cpp
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
        src/aboutdialog.h
        src/addeditautoprofiledialog.h
        src/advancebuttondialog.h
        src/advancestickassignmentdialog.h
        src/antimicrosettings.h
        src/antkeymapper.h
        src/applaunchhelper.h
        src/autoprofileinfo.h
        src/axiseditdialog.h
        src/axisvaluebox.h
        src/buttoneditdialog.h
        src/calibration.h
        src/commandlineutility.h
        src/dpadcontextmenu.h
        src/dpadeditdialog.h
        src/dpadpushbutton.h
        src/dpadpushbuttongroup.h
        src/editalldefaultautoprofiledialog.h
        src/eventhandlerfactory.h
        src/eventhandlers/baseeventhandler.h
        src/extraprofilesettingsdialog.h
        src/flashbuttonwidget.h
        src/gamecontroller/gamecontroller.h
        src/gamecontroller/gamecontrollerdpad.h
        src/gamecontroller/gamecontrollerset.h
        src/gamecontroller/gamecontrollertrigger.h
        src/gamecontroller/gamecontrollertriggerbutton.h
        src/gamecontroller/xml/gamecontrollerdpadxml.h
        src/gamecontroller/xml/gamecontrollertriggerxml.h
        src/gamecontroller/xml/gamecontrollerxml.h
        src/gamecontrollerexample.h
        src/gamecontrollermappingdialog.h
        src/globalvariables.h
        src/inputdaemon.h
        src/inputdevice.h
        src/inputdevicebitarraystatus.h
        src/joyaxis.h
        src/joyaxiscontextmenu.h
        src/joyaxiswidget.h
        src/joybutton.h
        src/joybuttoncontextmenu.h
        src/joybuttonmousehelper.h
        src/joybuttonslot.h
        src/joybuttonstatusbox.h
        src/joybuttontypes/joyaxisbutton.h
        src/joybuttontypes/joycontrolstickbutton.h
        src/joybuttontypes/joycontrolstickmodifierbutton.h
        src/joybuttontypes/joydpadbutton.h
        src/joybuttontypes/joygradientbutton.h
        src/joybuttonwidget.h
        src/joycontrolstick.h
        src/joycontrolstickbuttonpushbutton.h
        src/joycontrolstickcontextmenu.h
        src/joycontrolstickeditdialog.h
        src/joycontrolstickpushbutton.h
        src/joycontrolstickstatusbox.h
        src/joydpad.h
        src/joydpadbuttonwidget.h
        src/joystick.h
        src/joystickstatuswindow.h
        src/joytabwidget.h
        src/joytabwidgetcontainer.h
        src/keyboard/virtualkeyboardmousewidget.h
        src/keyboard/virtualkeypushbutton.h
        src/keyboard/virtualmousepushbutton.h
        src/localantimicroserver.h
        src/logger.h
        src/mainsettingsdialog.h
        src/mainwindow.h
        src/messagehandler.h
        src/mousedialog/mouseaxissettingsdialog.h
        src/mousedialog/mousebuttonsettingsdialog.h
        src/mousedialog/mousecontrolsticksettingsdialog.h
        src/mousedialog/mousedpadsettingsdialog.h
        src/mousedialog/springmoderegionpreview.h
        src/mousedialog/uihelpers/mouseaxissettingsdialoghelper.h
        src/mousedialog/uihelpers/mousebuttonsettingsdialoghelper.h
        src/mousedialog/uihelpers/mousecontrolsticksettingsdialoghelper.h
        src/mousedialog/uihelpers/mousedpadsettingsdialoghelper.h
        src/mousehelper.h
        src/mousesettingsdialog.h
        src/qkeydisplaydialog.h
        src/qtkeymapperbase.h
        src/quicksetdialog.h
        src/sdleventreader.h
        src/setaxisthrottledialog.h
        src/setjoystick.h
        src/setnamesdialog.h
        src/simplekeygrabberbutton.h
        src/slotitemlistwidget.h
        src/stickpushbuttongroup.h
        src/uihelpers/advancebuttondialoghelper.h
        src/uihelpers/buttoneditdialoghelper.h
        src/uihelpers/dpadcontextmenuhelper.h
        src/uihelpers/dpadeditdialoghelper.h
        src/uihelpers/gamecontrollermappingdialoghelper.h
        src/uihelpers/joyaxiscontextmenuhelper.h
        src/uihelpers/joycontrolstickcontextmenuhelper.h
        src/uihelpers/joycontrolstickeditdialoghelper.h
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
        src/aboutdialog.ui
        src/addeditautoprofiledialog.ui
        src/advancebuttondialog.ui
        src/advancestickassignmentdialog.ui
        src/axiseditdialog.ui
        src/buttoneditdialog.ui
        src/calibration.ui
        src/capturedwindowinfodialog.ui
        src/dpadeditdialog.ui
        src/editalldefaultautoprofiledialog.ui
        src/extraprofilesettingsdialog.ui
        src/gamecontrollermappingdialog.ui
        src/joycontrolstickeditdialog.ui
        src/joystickstatuswindow.ui
        src/mainsettingsdialog.ui
        src/mainwindow.ui
        src/mousesettingsdialog.ui
        src/qkeydisplaydialog.ui
        src/quicksetdialog.ui
        src/setaxisthrottledialog.ui
        src/setnamesdialog.ui
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
                src/capturedwindowinfodialog.cpp
                )
        LIST(APPEND antimicrox_HEADERS src/x11extras.h
                src/qtx11keymapper.h
                src/unixcapturewindowutility.h
                src/autoprofilewatcher.h
                src/capturedwindowinfodialog.h
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
    LIST(APPEND antimicro_SOURCES
        src/winextras.cpp
         src/qtwinkeymapper.cpp
        #  src/winappprofiletimerdialog.cpp
        #  src/capturedwindowinfodialog.cpp
         src/eventhandlers/winsendinputeventhandler.cpp
         src/joykeyrepeathelper.cpp
    )
    LIST(APPEND antimicro_HEADERS
        src/winextras.h
        src/qtwinkeymapper.h
        # src/winappprofiletimerdialog.h
        # src/capturedwindowinfodialog.h
        src/eventhandlers/winsendinputeventhandler.h
        src/joykeyrepeathelper.h
    )
    # LIST(APPEND antimicro_FORMS src/winappprofiletimerdialog.ui)
    # Add Windows specific resource file used for application icons.
    # LIST(APPEND antimicro_RESOURCES src/resources_windows.qrc)

#     if(WITH_VMULTI)
#         list(APPEND antimicro_SOURCES src/qtvmultikeymapper.cpp
#              src/eventhandlers/winvmultieventhandler.cpp
# #            vmulti/client/client.c
#         )
#         list(APPEND antimicro_HEADERS src/qtvmultikeymapper.h
#              src/eventhandlers/winvmultieventhandler.h
#         )
#     endif(WITH_VMULTI)

endif(UNIX)

###############################
# DEFINITIONS
###############################

add_definitions(-DUSE_SDL_2)

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
# PACKAGES
###############################

if(UNIX)
    find_package(Qt5Widgets ${QT_REQUIRED_VERSION} REQUIRED)
    find_package(Qt5Core ${QT_REQUIRED_VERSION} REQUIRED)
    find_package(Qt5Gui ${QT_REQUIRED_VERSION} REQUIRED)
    find_package(Qt5Network ${QT_REQUIRED_VERSION} REQUIRED)
    find_package(Qt5LinguistTools ${QT_REQUIRED_VERSION} REQUIRED)
    find_package(Qt5Concurrent ${QT_REQUIRED_VERSION} REQUIRED)
    #find_package(Qt5Test REQUIRED)

    QT5_WRAP_CPP(antimicrox_HEADERS_MOC ${antimicrox_HEADERS})
    QT5_WRAP_UI(antimicrox_FORMS_HEADERS ${antimicrox_FORMS})
    QT5_ADD_RESOURCES(antimicrox_RESOURCES_RCC ${antimicrox_RESOURCES})

    add_subdirectory("share/antimicrox/translations")

    set(CMAKE_POSITION_INDEPENDENT_CODE ON)

elseif(WIN32)
    find_package(Qt5Widgets REQUIRED)
    find_package(Qt5Core REQUIRED)
    find_package(Qt5Gui REQUIRED)
    find_package(Qt5Network REQUIRED)
    find_package(Qt5LinguistTools REQUIRED)
    find_package(Qt5Concurrent ${QT_REQUIRED_VERSION} REQUIRED)

    QT5_WRAP_UI(antimicro_FORMS_HEADERS ${antimicro_FORMS})
    QT5_ADD_RESOURCES(antimicro_RESOURCES_RCC ${antimicro_RESOURCES})
    add_subdirectory("share/antimicrox/translations")

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

    # LIST(APPEND LIBS ${Qt5Widgets_LIBRARIES}
    #     ${Qt5Core_LIBRARIES}
    #     ${Qt5Gui_LIBRARIES}
    #     ${Qt5Network_LIBRARIES}
    # )
endif(UNIX)

find_package(SDL2 REQUIRED)

###############################
# LINKER LIBRARIES
###############################

LIST(APPEND QT_LIBS Qt5::Core)
LIST(APPEND QT_LIBS Qt5::Gui)
LIST(APPEND QT_LIBS Qt5::Widgets)
LIST(APPEND QT_LIBS Qt5::Concurrent)
LIST(APPEND QT_LIBS Qt5::Network)

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
    # include_directories(${SDL2_INCLUDE_DIRS})
    message("SDL2 include dir: ${SDL2_INCLUDE_DIRS}")
    # list(APPEND LIBS ${SDL2_LIBRARIES})

    add_definitions(-DUNICODE -D_UNICODE)

    list(APPEND WIN_LIBS "psapi")

    if(WITH_VMULTI)
        include_directories("${PROJECT_SOURCE_DIR}/vmulti/inc")
        list(APPEND WIN_LIBS "hid" "setupapi")
    endif(WITH_VMULTI)
endif(UNIX)

include_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
        "${CMAKE_CURRENT_BINARY_DIR}/src"
        ${PROJECT_SOURCE_DIR}
        "${PROJECT_SOURCE_DIR}/src"
        ${SDL2_INCLUDE_DIRS}
)

if(UNIX)
    # Store executable in a bin subdir. Needed here so translations can be loaded.
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
endif(UNIX)

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
        # TODO create this with windows .ico icon
        # src/antimicrox.rc
    )
endif(UNIX)

target_link_libraries(antimicrox
        ${QT_LIBS}
        ${X11_LIBS}
        ${WIN_LIBS}
        ${SDL2_LIBRARIES}
        ${EXTRA_LIBS}
        )

###############################
# INSTALL
###############################

# Specify out directory for final executable.
if(UNIX)
    install(TARGETS antimicrox RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
    #           install(TARGETS antilib DESTINATION "${CMAKE_INSTALL_LIBDIR}")
    install(FILES ${antimicrox_HEADERS} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/antimicrox")
elseif(WIN32)
	install(TARGETS antimicrox RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
endif(UNIX)

if(UNIX)
    ecm_install_icons(ICONS src/icons/application/16x16/apps/16-apps-io.github.antimicrox.antimicrox.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons)
    ecm_install_icons(ICONS src/icons/application/24x24/apps/24-apps-io.github.antimicrox.antimicrox.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons)
    ecm_install_icons(ICONS src/icons/application/32x32/apps/32-apps-io.github.antimicrox.antimicrox.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons)
    ecm_install_icons(ICONS src/icons/application/48x48/apps/48-apps-io.github.antimicrox.antimicrox.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons)
    ecm_install_icons(ICONS src/icons/application/64x64/apps/64-apps-io.github.antimicrox.antimicrox.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons)
    ecm_install_icons(ICONS src/icons/application/128x128/apps/128-apps-io.github.antimicrox.antimicrox.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons)
    ecm_install_icons(ICONS src/icons/application/scalable/apps/sc-apps-io.github.antimicrox.antimicrox.svg DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons)

    ecm_install_icons(ICONS src/images/breeze_themed/48-apps-antimicrox_trayicon.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons THEME breeze)
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

###############################
# WINDOWS PACKAGING WARNING UNTESTED
###############################

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
              sign /v /a /s ROOT /n antimicrox ${CMAKE_INSTALL_PREFIX}/antimicrox.exe)")
    endif(PERFORM_SIGNING)

    # Set variables needed for MSI building.
    set(MSIFOLDER "${PROJECT_SOURCE_DIR}/windows")
    set(WIXENV $ENV{WIX})

    # Use a different file for each architecture due
    # to some DLL differences.
    if(TARGET_ARCH STREQUAL "x86_64")
        set(WIXWXS "${MSIFOLDER}/antimicro_64.wxs")
    else()
        set(WIXWXS "${MSIFOLDER}/antimicrox.wxs")
    endif(TARGET_ARCH STREQUAL "x86_64")

    set(WIXOBJ "${MSIFOLDER}/antimicrox.wixobj")

    # Use version number in output file name.
    set(MSIFILENAME "${ANTIMICRO_MAJOR_VERSION}.${ANTIMICRO_MINOR_VERSION}")
    if(ANTIMICRO_PATCH_VERSION AND NOT ANTIMICRO_PATCH_VERSION EQUAL 0)
        set(MSIFILENAME "${MSIFILENAME}.${ANTIMICRO_PATCH_VERSION}")
    endif(ANTIMICRO_PATCH_VERSION AND NOT ANTIMICRO_PATCH_VERSION EQUAL 0)

    # Change output file depending on the target architecture.
    if(TARGET_ARCH STREQUAL "x86_64")
        set(WIXMSI "${MSIFOLDER}/antimicrox-${MSIFILENAME}-win64.msi")
    else()
        set(WIXMSI "${MSIFOLDER}/antimicrox-${MSIFILENAME}-win32.msi")
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

###############################
# TESTING
###############################

if(WITH_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif(WITH_TESTS)

###############################
# CPACK BUILD
###############################

include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Program used to map keyboard keys and mouse controls to a gamepad.")


set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

set(CPACK_PACKAGE_VERSION_MAJOR "${ANTIMICROX_MAJOR_VERSION}")
set(CPACK_PACKAGE_VERSION_MINOR "${ANTIMICROX_MINOR_VERSION}")
set(CPACK_PACKAGE_VERSION_PATCH "${ANTIMICROX_PATCH_VERSION}")

set(CPACK_PACKAGE_FILE_NAME "antimicrox-${ANTIMICROX_MAJOR_VERSION}.${ANTIMICROX_MINOR_VERSION}.${ANTIMICROX_PATCH_VERSION}-${CMAKE_HOST_SYSTEM_PROCESSOR}")

set(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")

if(UNIX)
    set(CPACK_STRIP_FILES "")
    set(CPACK_SOURCE_STRIP_FILES "")
endif()

# Building .deb package
if(CPACK_GENERATOR STREQUAL "DEB")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "qtbase5-dev (>= ${QT_REQUIRED_VERSION}), libsdl2-2.0-0 (>= ${SDL_REQUIRED_VERSION}), libc6")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "pktiuk <kotiuk@zohomail.eu>")
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "AntiMicroX is a graphical program used to map gamepad buttons to keyboard, mouse, scripts and macros.

It is a new fork of discontinued antimicrox.")

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

set(CPACK_PACKAGE_EXECUTABLES "antimicrox" "antimicrox")
include(CPack)
