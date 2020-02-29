#-------------------------------------------------
#
# Project created by QtCreator 2019-11-05T21:20:56
#
#-------------------------------------------------

QT       += core gui
LIBS += -lpthread libwsock32 libws2_32
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test
TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    Server.cpp \
    Config.cpp \
    WinsockEnv.cpp

HEADERS  += mainwindow.h \
    Config.h \
    Server.h \
    WinsockEnv.h

FORMS    += mainwindow.ui

DISTFILES += \
    ../cat.png
