#-------------------------------------------------
#
# Project created by QtCreator 2012-11-13T22:35:33
#
#-------------------------------------------------

QT       += core gui

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
    axisvaluebox.cpp

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
    axisvaluebox.h

FORMS    += mainwindow.ui \
    buttoneditdialog.ui \
    dpadbuttoneditdialog.ui \
    axiseditdialog.ui


LIBS += -lSDL -lXtst -lX11

RESOURCES += \
    resources.qrc

