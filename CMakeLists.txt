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
cmake_minimum_required(VERSION 3.6)
project(antimicro)

if(POLICY CMP0071)
  cmake_policy(SET CMP0071 NEW)
endif()

if(UNIX)
    find_package(ECM REQUIRED NO_MODULE)
    set(CMAKE_MODULE_PATH ${ECM_MODULE_DIR})
endif(UNIX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" "${CMAKE_ROOT}/Modules")

set(CMAKE_INCLUDE_CURRENT_DIR ON)


if (UNIX AND NOT APPLE)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "force cmake prefix to be set for /usr" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
endif(UNIX AND NOT APPLE)

include(CheckCXXCompilerFlag)
include(GNUInstallDirs)

if (UNIX)
    include(ECMInstallIcons)
endif(UNIX)

CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)


if(COMPILER_SUPPORTS_CXX11)
    if (NOT WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wstrict-null-sentinel -Wstrict-overflow=5 -Wundef -Wno-unused -std=c++11")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

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
set(ANTIMICRO_MINOR_VERSION 24)
set(ANTIMICRO_PATCH_VERSION 0)

option(WITH_TESTS "Allow tests for classes" OFF)
option(USE_SDL_2 "Use SDL 2 libraries." ON)

if(WITH_TESTS)
    message("Tests enabled")
endif(WITH_TESTS)

if(UNIX)
    option(WITH_X11 "Compile with support for X11." ON)
    option(WITH_UINPUT "Compile with support for uinput. uinput will be usable to simulate events." OFF)
    option(WITH_XTEST "Compile with support for XTest.  XTest will be usable to simulate events." ON)
    option(APPDATA "Build project with AppData file support." ON)
endif(UNIX)

option(UPDATE_TRANSLATIONS "Call lupdate to update translation files from source." OFF)
option(TRANS_KEEP_OBSOLETE "Do not specify -no-obsolete when calling lupdate." OFF)


if(WIN32)
    option(PORTABLE_PACKAGE "Create portable Windows package" OFF)
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

set(antimicro_MAIN
    src/main.cpp
    )

set(antimicro_SOURCES
    src/mainwindow.cpp
    src/messagehandler.cpp
    src/joybuttonwidget.cpp
    src/joystick.cpp
    src/joybutton.cpp
    src/xml/joybuttonxml.cpp
    src/joybuttontypes/joygradientbutton.cpp
    src/event.cpp
    src/inputdaemon.cpp
    src/joyaxis.cpp
    src/xml/joyaxisxml.cpp
    src/joyaxiswidget.cpp
    src/joydpad.cpp
    src/xml/joydpadxml.cpp
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
    src/xml/inputdevicexml.cpp
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
    src/calibration.cpp
    src/globalvariables.cpp
)

set(antimicro_HEADERS
    src/mainwindow.h
    src/messagehandler.h
    src/joybuttonwidget.h
    src/joystick.h
    src/joybutton.h
    src/xml/joybuttonxml.h
    src/joybuttontypes/joygradientbutton.h
    src/inputdaemon.h
    src/joyaxis.h
    src/xml/joyaxisxml.h
    src/joyaxiswidget.h
    src/joydpad.h
    src/xml/joydpadxml.h
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
    src/xml/inputdevicexml.h
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
    src/calibration.h
    src/globalvariables.h
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
    src/calibration.ui
)

set(antimicro_RESOURCES src/resources.qrc)

# Files that require SDL 2 support.
if(USE_SDL_2)
    LIST(APPEND antimicro_SOURCES
         src/gamecontroller/gamecontroller.cpp
         src/gamecontroller/gamecontrollerdpad.cpp
         src/gamecontroller/gamecontrollerset.cpp
         src/gamecontroller/gamecontrollertrigger.cpp
         src/gamecontrollermappingdialog.cpp
         src/gamecontrollerexample.cpp
         src/gamecontroller/xml/gamecontrollerxml.cpp
         src/gamecontroller/xml/gamecontrollertriggerxml.cpp
    )
    LIST(APPEND antimicro_HEADERS src/gamecontroller/gamecontroller.h
        src/gamecontroller/gamecontrollerdpad.h
        src/gamecontroller/gamecontrollerset.h
        src/gamecontroller/gamecontrollertrigger.h
        src/gamecontrollermappingdialog.h
        src/gamecontrollerexample.h
        src/gamecontroller/xml/gamecontrollerxml.h
        src/gamecontroller/xml/gamecontrollertriggerxml.h
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

        find_package(Qt5Widgets REQUIRED)
        find_package(Qt5Core REQUIRED)
        find_package(Qt5Gui REQUIRED)
        find_package(Qt5Network REQUIRED)
        find_package(Qt5LinguistTools REQUIRED)

        QT5_WRAP_CPP(antimicro_HEADERS_MOC ${antimicro_HEADERS})
        QT5_WRAP_UI(antimicro_FORMS_HEADERS ${antimicro_FORMS})
        QT5_ADD_RESOURCES(antimicro_RESOURCES_RCC ${antimicro_RESOURCES})

        add_subdirectory("share/antimicro/translations")

        set(CMAKE_POSITION_INDEPENDENT_CODE ON)

elseif(WIN32)

    find_package(Qt5Widgets REQUIRED)
    find_package(Qt5Core REQUIRED)
    find_package(Qt5Gui REQUIRED)
    find_package(Qt5Network REQUIRED)
    find_package(Qt5LinguistTools REQUIRED)

    QT5_WRAP_CPP(antimicro_HEADERS_MOC ${antimicro_HEADERS})
    QT5_WRAP_UI(antimicro_FORMS_HEADERS ${antimicro_FORMS})
    QT5_ADD_RESOURCES(antimicro_RESOURCES_RCC ${antimicro_RESOURCES})
    add_subdirectory("share/antimicro/translations")

    set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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
    endif(USE_SDL_2)
elseif (WIN32)
    if(USE_SDL_2)
    
        if(SDL2_LIBRARY_DIR AND SDL2_INCLUDE_DIR)
            list(APPEND LIBS ${SDL2_LIBRARY})
            include_directories(${SDL2_INCLUDE_DIR})
        endif(SDL2_LIBRARY_DIR AND SDL2_INCLUDE_DIR)

        add_definitions(-DUNICODE -D_UNICODE)
    endif(USE_SDL_2)

    list(APPEND LIBS "psapi")

    if(WITH_VMULTI)
        include_directories("${PROJECT_SOURCE_DIR}/vmulti/inc")
        list(APPEND LIBS "hid" "setupapi")
    endif(WITH_VMULTI)
endif (UNIX)


include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    "${CMAKE_CURRENT_BINARY_DIR}/src"
    ${PROJECT_SOURCE_DIR}
    "${PROJECT_SOURCE_DIR}/src"
    )


if(UNIX)
    # Store executable in a bin subdir. Needed here so translations can be loaded.
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
endif(UNIX)

    if(UNIX)
            add_library( antilib
                SHARED
                ${antimicro_HEADERS_MOC}
                ${antimicro_SOURCES}
                ${antimicro_FORMS_HEADERS}
                ${antimicro_RESOURCES_RCC}
            )

            target_link_libraries (antilib Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Network ${SDL2_LIBRARIES} ${LIBS})

            add_executable(antimicro ${antimicro_MAIN})
            target_link_libraries (antimicro antilib Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Network)

    elseif(WIN32)

        add_library( antilib
            SHARED
            ${antimicro_HEADERS_MOC}
            ${antimicro_SOURCES}
            ${antimicro_FORMS_HEADERS}
            ${antimicro_RESOURCES_RCC}
            src/antimicro.rc
        )

        # The WIN32 is required to specify a GUI application.
        add_executable(antimicro ${antimicro_MAIN})
        target_link_libraries (antimicro antilib Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Network ${LIBS})
    endif(UNIX)

    target_compile_definitions(antimicro PUBLIC QT_DEBUG_NO_OUTPUT) # should be commented during development


# Specify out directory for final executable.
if(UNIX)
	install(TARGETS antimicro RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
        install(TARGETS antilib DESTINATION "${CMAKE_INSTALL_LIBDIR}")
        install(FILES ${antimicro_HEADERS} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/antimicro")
elseif(WIN32)
	install(TARGETS antimicro RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
        install(TARGETS antilib
             RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX} COMPONENT runtime
             ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT devel
             PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}" COMPONENT devel)
        install(FILES ${antimicro_HEADERS} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
endif(UNIX)

if(UNIX)
    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-application_exit.png DESTINATION share/antimicro/icons)
    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-archive_insert.png DESTINATION share/antimicro/icons)
    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-dialog_cancel.png DESTINATION share/antimicro/icons)

    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-dialog_close.png DESTINATION share/antimicro/icons)
    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-dialog_ok.png DESTINATION share/antimicro/icons)
    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-document_close.png DESTINATION share/antimicro/icons)

    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-document_open.png DESTINATION share/antimicro/icons)
    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-document_open_folder.png DESTINATION share/antimicro/icons)
    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-document_revert.png DESTINATION share/antimicro/icons)

    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-document_revert_small.png DESTINATION share/antimicro/icons)
    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-document_save.png DESTINATION share/antimicro/icons)
    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-document_save_as.png DESTINATION share/antimicro/icons)

    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-edit_clear.png DESTINATION share/antimicro/icons)
    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-edit_clear_list.png DESTINATION share/antimicro/icons)
    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-edit_delete.png DESTINATION share/antimicro/icons)

    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-edit_select.png DESTINATION share/antimicro/icons)
    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-edit_table_delete_row.png DESTINATION share/antimicro/icons)
    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-edit_table_insert_row_below.png DESTINATION share/antimicro/icons)

    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-games_config_custom.png DESTINATION share/antimicro/icons)
    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-games_config_options.png DESTINATION share/antimicro/icons)
    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-help_about.png DESTINATION share/antimicro/icons)

    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-text_field.png DESTINATION share/antimicro/icons)
    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-view_fullscreen.png DESTINATION share/antimicro/icons)
    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-view_refresh.png DESTINATION share/antimicro/icons)
    ecm_install_icons(ICONS src/icons/icons/16x16/actions/16-actions-view_restore.png DESTINATION share/antimicro/icons)


    ecm_install_icons(ICONS src/icons/application/16x16/apps/16-apps-com.github.juliagoda.antimicro.png DESTINATION share/icons)
    ecm_install_icons(ICONS src/icons/application/24x24/apps/24-apps-com.github.juliagoda.antimicro.png DESTINATION share/icons)
    ecm_install_icons(ICONS src/icons/application/32x32/apps/32-apps-com.github.juliagoda.antimicro.png DESTINATION share/icons)
    ecm_install_icons(ICONS src/icons/application/48x48/apps/48-apps-com.github.juliagoda.antimicro.png DESTINATION share/icons)
    ecm_install_icons(ICONS src/icons/application/64x64/apps/64-apps-com.github.juliagoda.antimicro.png DESTINATION share/icons)

    install(FILES src/images/profile.jpg src/images/antimicro.png src/images/antimicro_trayicon.png src/images/axis.png src/images/button.png src/images/controllermap.png DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/antimicro/images")

    install(FILES src/icons/index.theme DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/antimicro/icons/hicolor")
    install(FILES other/com.github.juliagoda.antimicro.desktop DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
    install(FILES other/com.github.juliagoda.antimicro.xml DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/mime/packages")
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
        "${SDL2_BASE_DIR}/i686-w64-mingw32/bin/SDIRDL2.dll"
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

    # Change outputlib file depending on the target architecture.
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



if(WITH_TESTS)
    enable_testing ()
    add_subdirectory (tests)
endif(WITH_TESTS)
