#-------------------------------------------------
#
# Project created by QtCreator 2012-11-13T22:35:33
#
#-------------------------------------------------

#USE_SDL_2 = 1

isEmpty(INSTALL_PREFIX) {
    unix {
        INSTALL_PREFIX = /usr/local
    } else:win32 {
        INSTALL_PREFIX = $${OUT_PWD}/../antimicro
    }
}

win32 {
    CONFIG(debug, debug|release) {
        DESTDIR = $$OUT_PWD/debug
    } else {
        DESTDIR = $$OUT_PWD/release
    }
}

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

unix {
    target.path = $$INSTALL_PREFIX/bin

    desktop.path = $$INSTALL_PREFIX/share/applications
    desktop.files = ../other/antimicro.desktop

    deskicon.path = $$INSTALL_PREFIX/share/pixmaps
    deskicon.files = images/antimicro.png
} else:win32 {
    target.path = $$INSTALL_PREFIX
}

TRANSLATIONS = $$files(../share/antimicro/translations/antimicro_*.ts)

# qmfiles will be used to specify the path of the initial compiled translations
# before installing
qmfiles.path = $$INSTALL_PREFIX/share/antimicro/translations
for(tsfile, TRANSLATIONS): qmfiles.files += $$replace(tsfile, ".ts", ".qm")

equals(OUT_PWD, $$PWD) {
    unix {
        updateqm.commands = $$[QT_INSTALL_BINS]/lrelease $$_PRO_FILE_
    } else:win32 {
        updateqm.commands = $$[QT_INSTALL_BINS]\\lrelease.exe $$_PRO_FILE_
    }

    # Compile translations files in source tree
    compiledtranslations.path = $$qmfiles.path
    compiledtranslations.files = $$qmfiles.files

} else {
    # Specify path of final installed qm files
    compiledtranslations.path = $$qmfiles.path

    # Determine source translation directory and intermediate output
    # translation directory
    TRANSLATION_IN_DIR = $${PWD}/../share/antimicro/translations
    unix {
        TRANSLATION_OUT_DIR = $${OUT_PWD}/../share/antimicro/translations
    } else:win32 {
        TRANSLATION_OUT_DIR = $${DESTDIR}/share/antimicro/translations
    }

    # Use intermediate variable in order to not interfere with lupdate
    for(transfile, TRANSLATIONS) {
        unix {
            fulltranslations += $$TRANSLATION_OUT_DIR/$$basename(transfile)
        } else:win32 {
            # Using the raw path so the separators must be changed
            fulltranslations += $$replace(TRANSLATION_OUT_DIR, "/", "\\")\\$$basename(transfile)
        }
    }

    # Build qm install files list
    for(qmfile, qmfiles.files) {
        unix {
            compiledtranslations.files += $$TRANSLATION_OUT_DIR/$$basename(qmfile)
        } else:win32 {
            compiledtranslations.files += $$TRANSLATION_OUT_DIR/$$basename(qmfile)
        }
    }

    # Copy .ts files to build directory and compile
    # to .qm files
    unix {
        updateqm.commands = $(MKDIR) $${TRANSLATION_OUT_DIR} && \
            $(COPY_DIR) $${TRANSLATION_IN_DIR}/* $${TRANSLATION_OUT_DIR} && \
            $$[QT_INSTALL_BINS]/lrelease $$fulltranslations
    } else:win32 {
        # Use shell_path function here for reference. Not available in Qt4
        greaterThan(QT_MAJOR_VERSION, 4) {
            TRANSLATION_IN_DIR = $$shell_path($$TRANSLATION_IN_DIR)
            TRANSLATION_OUT_DIR = $$shell_path($$TRANSLATION_OUT_DIR)
        } else {
            TRANSLATION_IN_DIR = $$replace(TRANSLATION_IN_DIR, "/", "\\")
            TRANSLATION_OUT_DIR = $$replace(TRANSLATION_OUT_DIR, "/", "\\")
        }

        updateqm.commands = $(MKDIR) $${TRANSLATION_OUT_DIR} & \
            $(COPY_DIR) $${TRANSLATION_IN_DIR} $${TRANSLATION_OUT_DIR} & \
            $$[QT_INSTALL_BINS]\\lrelease.exe $$fulltranslations
    }
}

# qm files will not exist before running qmake. Need this to workaround that
compiledtranslations.CONFIG += no_check_exist

# Add updateqm rule to QMAKE_EXTRA_TARGETS
updateqm.target = updateqm
QMAKE_EXTRA_TARGETS += updateqm

# Create rule to copy required SDL dll file to output build directory
win32 {
    copy_sdl_dll.commands = $(COPY) ..\\SDL-1.2.15\\bin\\SDL.dll $$replace(DESTDIR, "/", "\\")
    copy_sdl_dll.target = copy_sdl_dll

    QMAKE_EXTRA_TARGETS += copy_sdl_dll
}

TARGET = antimicro
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    joybuttonwidget.cpp \
    joystick.cpp \
    joybutton.cpp \
    event.cpp \
    inputdaemon.cpp \
    joyaxis.cpp \
    joyaxiswidget.cpp \
    joydpad.cpp \
    joydpadbutton.cpp \
    axiseditdialog.cpp \
    xmlconfigreader.cpp \
    xmlconfigwriter.cpp \
    joytabwidget.cpp \
    axisvaluebox.cpp \
    advancebuttondialog.cpp \
    simplekeygrabberbutton.cpp \
    joybuttonslot.cpp \
    joyaxisbutton.cpp \
    aboutdialog.cpp \
    setjoystick.cpp \
    sdleventreader.cpp \
    setaxisthrottledialog.cpp \
    keyboard/virtualkeypushbutton.cpp \
    keyboard/virtualkeyboardmousewidget.cpp \
    keyboard/virtualmousepushbutton.cpp \
    buttoneditdialog.cpp \
    commandlineutility.cpp \
    joycontrolstick.cpp \
    joycontrolstickbutton.cpp \
    joycontrolstickeditdialog.cpp \
    joycontrolstickpushbutton.cpp \
    joycontrolstickbuttonpushbutton.cpp \
    joycontrolstickstatusbox.cpp \
    advancestickassignmentdialog.cpp \
    virtualdpadpushbutton.cpp \
    dpadpushbutton.cpp \
    dpadeditdialog.cpp \
    vdpad.cpp \
    joydpadbuttonwidget.cpp \
    quicksetdialog.cpp \
    mousehelper.cpp \
    mousesettingsdialog.cpp \
    mousedialog/mousecontrolsticksettingsdialog.cpp \
    mousedialog/mouseaxissettingsdialog.cpp \
    mousedialog/mousebuttonsettingsdialog.cpp \
    mousedialog/mousedpadsettingsdialog.cpp \
    joytabwidgetcontainer.cpp \
    mousedialog/springmoderegionpreview.cpp \
    joystickstatuswindow.cpp \
    joybuttonstatusbox.cpp \
    flashbuttonwidget.cpp


unix {
  SOURCES += x11info.cpp
} else:win32 {
  SOURCES += wininfo.cpp
}

HEADERS  += mainwindow.h \
    joybuttonwidget.h \
    joystick.h \
    joybutton.h \
    event.h \
    inputdaemon.h \
    joyaxis.h \
    joyaxiswidget.h \
    joydpad.h \
    joydpadbutton.h \
    axiseditdialog.h \
    xmlconfigreader.h \
    xmlconfigwriter.h \
    common.h \
    joytabwidget.h \
    axisvaluebox.h \
    advancebuttondialog.h \
    simplekeygrabberbutton.h \
    joybuttonslot.h \
    joyaxisbutton.h \
    aboutdialog.h \
    setjoystick.h \
    sdleventreader.h \
    setaxisthrottledialog.h \
    keyboard/virtualkeypushbutton.h \
    keyboard/virtualkeyboardmousewidget.h \
    keyboard/virtualmousepushbutton.h \
    buttoneditdialog.h \
    commandlineutility.h \
    joycontrolstick.h \
    joycontrolstickbutton.h \
    joycontrolstickdirectionstype.h \
    joycontrolstickeditdialog.h \
    joycontrolstickpushbutton.h \
    joycontrolstickbuttonpushbutton.h \
    joycontrolstickstatusbox.h \
    advancestickassignmentdialog.h \
    virtualdpadpushbutton.h \
    dpadpushbutton.h \
    dpadeditdialog.h \
    vdpad.h \
    joydpadbuttonwidget.h \
    quicksetdialog.h \
    mousehelper.h \
    mousesettingsdialog.h \
    mousedialog/mousecontrolsticksettingsdialog.h \
    mousedialog/mouseaxissettingsdialog.h \
    mousedialog/mousebuttonsettingsdialog.h \
    mousedialog/mousedpadsettingsdialog.h \
    joytabwidgetcontainer.h \
    mousedialog/springmoderegionpreview.h \
    joystickstatuswindow.h \
    joybuttonstatusbox.h \
    flashbuttonwidget.h


unix {
  HEADERS += x11info.h
} else:win32 {
  HEADERS += wininfo.h
}

FORMS    += mainwindow.ui \
    axiseditdialog.ui \
    advancebuttondialog.ui \
    aboutdialog.ui \
    setaxisthrottledialog.ui \
    buttoneditdialog.ui \
    joycontrolstickeditdialog.ui \
    advancestickassignmentdialog.ui \
    dpadeditdialog.ui \
    quicksetdialog.ui \
    mousesettingsdialog.ui \
    joystickstatuswindow.ui


unix {
  !isEmpty(USE_SDL_2) {
    LIBS += -lSDL2
  } else {
    LIBS += -lSDL
  }
  LIBS += -lXtst -lX11
} else:win32 {
  !isEmpty(USE_SDL_2) {
    LIBS += -L"..\SDL2-2.0.1\i686-w64-mingw32\lib" -lSDL2
    INCLUDEPATH += "..\SDL2-2.0.1\i686-w64-mingw32\include"
  } else {
    LIBS += -L"..\SDL-1.2.15\lib" -lSDL
    INCLUDEPATH += "..\SDL-1.2.15\include"
  }
  msvc: {
    INCLUDEPATH +== "$$PWD"
  }
}


RESOURCES += \
    resources.qrc

win32 {
  RC_FILE += antimicro.rc
}

# Force updateqm to run before attempting to install
# compiled translation files
compiledtranslations.depends = updateqm

INSTALLS += target compiledtranslations

# Install platform-dependent files
unix {
    INSTALLS += desktop deskicon
} else:win32 {
    # Copy all required release build DLL files that a packaged
    # release build of the program will need to be able to run
    extradlls = $$[QT_INSTALL_BINS]\\icudt51.dll \
        $$[QT_INSTALL_BINS]\\icuin51.dll \
        $$[QT_INSTALL_BINS]\\icuuc51.dll \
        $$[QT_INSTALL_BINS]\\libgcc_s_dw2-1.dll \
        $$[QT_INSTALL_BINS]\\libwinpthread-1.dll \
        $$[QT_INSTALL_BINS]\\Qt5Core.dll \
        $$[QT_INSTALL_BINS]\\Qt5Gui.dll \
        $$[QT_INSTALL_BINS]\\Qt5Network.dll \
        $$[QT_INSTALL_BINS]\\Qt5Widgets.dll \
        ..\\SDL-1.2.15\\bin\\SDL.dll \
        $$[QT_INSTALL_BINS]\\libstdc++-6.dll

    install_dlls.path = $$replace(INSTALL_PREFIX, "/", "\\")
    for(dllfile, extradlls) {
        install_dlls.extra +=  $(COPY) \"$$dllfile\" $${install_dlls.path} &
        install_dlls.files += $$dllfile
    }

    # Install path will be different for the qwindows.dll platform plugin. Needs to be separated
    # from the other copy commands
    install_platforms_dll.path = $${install_dlls.path}\\platforms
    install_platforms_dll.extra = $(COPY) \"$$[QT_INSTALL_BINS]\\..\\plugins\\platforms\\qwindows.dll\" $${install_platforms_dll.path}
    install_platforms_dll.files = $$[QT_INSTALL_BINS]\\..\\plugins\\platforms\\qwindows.dll

    INSTALLS += install_dlls install_platforms_dll
}

win32 {
    # Build MSI package
    MSIFOLDER = $$shell_path($${PWD}/../windows)
    WIXENV = $$(WIX)
    WIXWXS = \"$$MSIFOLDER\AntiMicro.wxs\"
    WIXOBJ = \"$$MSIFOLDER\AntiMicro.wixobj\"
    WIXMSI = \"$$MSIFOLDER\AntiMicro.msi\"

    isEmpty(WIXENV) {
        buildmsi.commands = @echo MSI package build aborted: WIX environment variable not defined.
    }
    else {
        buildmsi.commands = @echo MSI package build in progress, please wait ... && \
                            \"$$WIXENV\bin\candle.exe\" $$WIXWXS -out $$WIXOBJ -sw1113 && \
                            \"$$WIXENV\bin\light.exe\" $$WIXOBJ -out $$WIXMSI -sw1076 -spdb
    }

    buildmsi.path = MSIFOLDER
    buildmsi.target = buildmsi

    msipackage.files += $$WIXOBJ
    msipackage.files += $$WIXMSI

    QMAKE_EXTRA_TARGETS += buildmsi
    QMAKE_CLEAN += $$msipackage.files
}

OTHER_FILES += \
    ../gpl.txt \
    ../other/antimicro.desktop


# Add application icon on Windows
win32 {
  OTHER_FILES += \
  antimicro.rc
}

# Have intermediate qm files deleted during make clean
QMAKE_CLEAN += $$compiledtranslations.files

!isEmpty(USE_SDL_2) {
  DEFINES += USE_SDL_2
}
