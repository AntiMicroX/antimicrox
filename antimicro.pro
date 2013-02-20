#-------------------------------------------------
#
# Project created by QtCreator 2012-11-13T22:35:33
#
#-------------------------------------------------

isEmpty(INSTALL_PREFIX)
{
    INSTALL_PREFIX = /usr/local
}

QT       += core gui

target.path = $$INSTALL_PREFIX/bin

TARGET = antimicro
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    joybuttonwidget.cpp \
    joystick.cpp \
    joybutton.cpp \
    event.cpp \
    inputdaemon.cpp \
    buttoneditdialog.cpp \
    keygrabberbutton.cpp \
    joyaxis.cpp \
    joyaxiswidget.cpp \
    joydpad.cpp \
    joydpadbutton.cpp \
    dpadbuttoneditdialog.cpp \
    axiseditdialog.cpp \
    xmlconfigreader.cpp \
    xmlconfigwriter.cpp \
    joytabwidget.cpp \
    axisvaluebox.cpp \
    advancebuttondialog.cpp \
    buttontempconfig.cpp \
    simplekeygrabberbutton.cpp \
    joybuttonslot.cpp \
    joyaxisbutton.cpp \
    aboutdialog.cpp \
    xmlconfigmigration.cpp

HEADERS  += mainwindow.h \
    joybuttonwidget.h \
    joystick.h \
    joybutton.h \
    event.h \
    inputdaemon.h \
    buttoneditdialog.h \
    keygrabberbutton.h \
    joyaxis.h \
    joyaxiswidget.h \
    joydpad.h \
    joydpadbutton.h \
    dpadbuttoneditdialog.h \
    axiseditdialog.h \
    xmlconfigreader.h \
    xmlconfigwriter.h \
    common.h \
    joytabwidget.h \
    axisvaluebox.h \
    advancebuttondialog.h \
    buttontempconfig.h \
    simplekeygrabberbutton.h \
    joybuttonslot.h \
    joyaxisbutton.h \
    aboutdialog.h \
    xmlconfigmigration.h

FORMS    += mainwindow.ui \
    buttoneditdialog.ui \
    dpadbuttoneditdialog.ui \
    axiseditdialog.ui \
    advancebuttondialog.ui \
    aboutdialog.ui


LIBS += -lSDL -lXtst -lX11

RESOURCES += \
    resources.qrc

INSTALLS += target
