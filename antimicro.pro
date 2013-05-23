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
desktop.files = other/antimicro.desktop

deskicon.path = $$INSTALL_PREFIX/share/pixmaps
deskicon.files = images/antimicro.png

TRANSLATIONS = share/antimicro/translations/antimicro_en.ts \
    share/antimicro/translations/antimicro_fr.ts

programtranslations.path = $$INSTALL_PREFIX/share/antimicro/translations
programtranslations.files = share/translations/antimicro/antimicro_en.qm \
    share/translations/antimicro/antimicro_fr.qm

equals(OUT_PWD, $$PWD) {
    updateqm.commands = lrelease $$_PRO_FILE_
} else {
    for(transfile, TRANSLATIONS): fulltranslations += $$OUT_PWD/$$transfile

    for(qmfile, programtranslations.files): finaltranslations += $$OUT_PWD/$$qmfile

    updateqm.commands = $(MKDIR) $${OUT_PWD}/share/antimicro/translations && \
        $(COPY_DIR) $$PWD/share/antimicro/translations $${OUT_PWD}/share/antimicro && \
        lrelease $$fulltranslations
}

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
    commandlineutility.cpp

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
    commandlineutility.h

FORMS    += mainwindow.ui \
    axiseditdialog.ui \
    advancebuttondialog.ui \
    aboutdialog.ui \
    setaxisthrottledialog.ui \
    buttoneditdialog.ui


LIBS += -lSDL -lXtst -lX11

RESOURCES += \
    resources.qrc

INSTALLS += target desktop deskicon programtranslations

OTHER_FILES += \
    gpl.txt \
    other/antimicro.desktop

