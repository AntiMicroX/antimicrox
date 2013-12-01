#-------------------------------------------------
#
# Project created by QtCreator 2012-11-13T22:35:33
#
#-------------------------------------------------

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
programtranslations.path = $$INSTALL_PREFIX/share/antimicro/translations
for(tsfile, TRANSLATIONS): programtranslations.files += $$replace(tsfile, ".ts", ".qm")

equals(OUT_PWD, $$PWD) {
    unix {
        updateqm.commands = $$[QT_INSTALL_BINS]/lrelease $$_PRO_FILE_
    } else:win32 {
        updateqm.commands = $$[QT_INSTALL_BINS]\\lrelease.exe $$_PRO_FILE_
    }

    finaltranslations.path = $$programtranslations.path
    finaltranslations.files = $$programtranslations.files

} else {
    finaltranslations.path = $$programtranslations.path

    for(transfile, TRANSLATIONS) {
        unix {
            fulltranslations += $$OUT_PWD/$$transfile
        } else:win32 {
            fulltranslations += $$DESTDIR/$$replace(transfile, "\.\.", "")
        }
    }

    for(qmfile, programtranslations.files) {
        unix {
            finaltranslations.files += $$OUT_PWD/$$qmfile
        } else:win32 {
            finaltranslations.files += $$DESTDIR/$$replace(qmfile, "\.\.", "")
        }
    }

    TRANSLATION_IN_DIR = $${PWD}/../share/antimicro/translations
    unix {
        TRANSLATION_OUT_DIR = $${OUT_PWD}/../share/antimicro/translations
    } else:win32 {
        TRANSLATION_OUT_DIR = $${DESTDIR}/share/antimicro/translations
    }

    unix {
        updateqm.commands = $(MKDIR) $${TRANSLATION_OUT_DIR} && \
            $(COPY_DIR) $${TRANSLATION_IN_DIR} $${TRANSLATION_OUT_DIR} && \
            $$[QT_INSTALL_BINS]/lrelease $$fulltranslations
    } else:win32 {
        greaterThan(QT_MAJOR_VERSION, 4) {
            TRANSLATION_IN_DIR = $$shell_path($$TRANSLATION_IN_DIR)
            TRANSLATION_OUT_DIR = $$shell_path($$TRANSLATION_OUT_DIR)
        } else {
            TRANSLATION_IN_DIR = $$replace(TRANSLATION_DIR, "/", "\\")
            TRANSLATION_OUT_DIR = $$replace(TRANSLATION_OUT_DIR, "/", "\\")
        }

        updateqm.commands = $(MKDIR) $${TRANSLATION_OUT_DIR} & \
            $(COPY_DIR) $${TRANSLATION_DIR} $${TRANSLATION_OUT_DIR} & \
            $$[QT_INSTALL_BINS]\\lrelease.exe $$fulltranslations
    }
}

finaltranslations.CONFIG += no_check_exist

updateqm.target = updateqm

QMAKE_EXTRA_TARGETS += updateqm

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
    joybuttonstatusbox.cpp


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
    joybuttonstatusbox.h


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
  LIBS += -lSDL -lXtst -lX11
} else:win32 {
  LIBS += -L"..\SDL-1.2.15\lib" -lSDL
  INCLUDEPATH += "..\SDL-1.2.15\include"
  msvc: {
    INCLUDEPATH +== "$$PWD"
  }
}


RESOURCES += \
    resources.qrc

win32 {
  RC_FILE += antimicro.rc
}

INSTALLS += target finaltranslations
unix {
    INSTALLS += desktop deskicon
} else:win32 {
    extradlls = $$[QT_INSTALL_BINS]\\icudt51.dll \
        $$[QT_INSTALL_BINS]\\icuin51.dll \
        $$[QT_INSTALL_BINS]\\icuuc51.dll \
        $$[QT_INSTALL_BINS]\\libgcc_s_dw2-1.dll \
        $$[QT_INSTALL_BINS]\\libwinpthread-1.dll \
        $$[QT_INSTALL_BINS]\\Qt5Core.dll \
        $$[QT_INSTALL_BINS]\\Qt5Gui.dll \
        $$[QT_INSTALL_BINS]\\Qt5Network.dll \
        $$[QT_INSTALL_BINS]\\Qt5Widgets.dll \
        ..\\SDL-1.2.15\\bin\\SDL.dll
        $$[QT_INSTALL_BINS]\\libstdc++-6.dll

    copy_dlls.path = $$replace(INSTALL_PREFIX, "/", "\\")
    for(dllfile, extradlls) {
        copy_dlls.extra +=  $(COPY) \"$$dllfile\" $${copy_dlls.path} &
    }

    copy_platforms_dll.path = $${copy_dlls.path}\\platforms
    copy_platforms_dll.extra = $(COPY) \"$$[QT_INSTALL_BINS]\\..\\plugins\\platforms\\qwindows.dll\" $${copy_platforms_dll.path}

    INSTALLS += copy_dlls copy_platforms_dll
}


OTHER_FILES += \
    ../gpl.txt \
    ../other/antimicro.desktop

win32 {
  OTHER_FILES += \
  antimicro.rc
}

QMAKE_CLEAN += $$finaltranslations.files
