#-------------------------------------------------
#
# Project created by QtCreator 2012-11-13T22:35:33
#
#-------------------------------------------------

isEmpty(INSTALL_PREFIX) {
    INSTALL_PREFIX = /usr/local
}


QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

target.path = $$INSTALL_PREFIX/bin

desktop.path = $$INSTALL_PREFIX/share/applications
desktop.files = ../other/antimicro.desktop

deskicon.path = $$INSTALL_PREFIX/share/pixmaps
deskicon.files = images/antimicro.png

TRANSLATIONS = ../share/antimicro/translations/antimicro_en.ts \
    ../share/antimicro/translations/antimicro_fr.ts \
    ../share/antimicro/translations/antimicro_sr.ts

programtranslations.path = $$INSTALL_PREFIX/share/antimicro/translations
programtranslations.files = ../share/antimicro/translations/antimicro_en.qm \
    ../share/antimicro/translations/antimicro_fr.qm \
    ../share/antimicro/translations/antimicro_sr.qm

equals(OUT_PWD, $$PWD) {
    updateqm.commands = lrelease $$_PRO_FILE_

    finaltranslations.path = $$programtranslations.path
    finaltranslations.files = $$programtranslations.files

} else {
    finaltranslations.path = $$programtranslations.path

    for(transfile, TRANSLATIONS): fulltranslations += $$OUT_PWD/$$transfile

    for(qmfile, programtranslations.files): finaltranslations.files += $$OUT_PWD/$$qmfile

    updateqm.commands = $(MKDIR) $${OUT_PWD}/../share/antimicro/translations && \
        $(COPY_DIR) $$PWD/../share/antimicro/translations $${OUT_PWD}/../share/antimicro && \
        lrelease $$fulltranslations
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
    xmlconfigmigration.cpp \
    setjoystick.cpp \
    sdleventreader.cpp \
    setaxisthrottledialog.cpp \
    x11info.cpp \
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
    mousedialog/mousedpadsettingsdialog.cpp

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
    xmlconfigmigration.h \
    setjoystick.h \
    sdleventreader.h \
    setaxisthrottledialog.h \
    x11info.h \
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
    mousedialog/mousedpadsettingsdialog.h

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


LIBS += -lSDL -lXtst -lX11

RESOURCES += \
    resources.qrc

INSTALLS += target desktop deskicon finaltranslations

OTHER_FILES += \
    ../gpl.txt \
    ../other/antimicro.desktop

QMAKE_CLEAN += $$finaltranslations.files
