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
        DESTDIR = $$OUT_PWD/release
    } else {
        DESTDIR = $$OUT_PWD/debug
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

    !exists($${TRANSLATION_OUT_DIR}): mkpath($${TRANSLATION_OUT_DIR})

    unix {
        updateqm.commands = $(COPY_DIR) $${TRANSLATION_IN_DIR} $${TRANSLATION_OUT_DIR} && \
            $$[QT_INSTALL_BINS]/lrelease $$fulltranslations
    } else:win32 {
        greaterThan(QT_MAJOR_VERSION, 4) {
            TRANSLATION_IN_DIR = $$shell_path($$TRANSLATION_IN_DIR)
            TRANSLATION_OUT_DIR = $$shell_path($$TRANSLATION_OUT_DIR)
        } else {
            TRANSLATION_IN_DIR = $$replace(TRANSLATION_DIR, "/", "\\")
            TRANSLATION_OUT_DIR = $$replace(TRANSLATION_OUT_DIR, "/", "\\")
        }

        updateqm.commands = $(COPY_DIR) $${TRANSLATION_DIR} $${TRANSLATION_OUT_DIR} & \
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
    joytabwidgetcontainer.cpp


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
    joytabwidgetcontainer.h


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
    mousesettingsdialog.ui


unix {
  LIBS += -lSDL -lXtst -lX11
} else:win32 {
  #LIBS += -L"" -lSDL
  #INCLUDEPATH += ""
  msvc: {
    INCLUDEPATH +== "$$PWD"
  }
}


RESOURCES += \
    resources.qrc

INSTALLS += target finaltranslations
unix {
    INSTALLS += desktop deskicon
}


OTHER_FILES += \
    ../gpl.txt \
    ../other/antimicro.desktop

QMAKE_CLEAN += $$finaltranslations.files
