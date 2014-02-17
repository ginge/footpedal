#-------------------------------------------------
#
# Project created by QtCreator 2014-02-15T00:04:50
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FootPedalController
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    console.cpp \
    packethandler.cpp \
    keydisplayform.cpp

HEADERS  += mainwindow.h \
    console.h \
    USBKeyboard.h \
    packethandler.h \
    keydisplayform.h

FORMS    += mainwindow.ui \
    keydisplayform.ui
