#-------------------------------------------------
#
# Project created by QtCreator 2016-11-07T14:38:11
#
#-------------------------------------------------

QT += core gui
QT += serialport
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ClientControl
TEMPLATE = app
#SRC_PATH = ../../../wiced_tools/ClientUI

SOURCES += main.cpp\
        mainwindow.cpp \
        audio_src.cpp \
        dm.cpp \
        hf.cpp \
        spp.cpp \
        ag.cpp \
        ble_hidd.cpp \
        hid_host.cpp \
        home_kit.cpp \
        avrc_tg.cpp \
        avrc_ct.cpp \
        iap2.cpp \
        download.cpp \
        gatt.cpp \
        audio_snk.cpp \
        bsg.cpp

# spy tracing only supported in WIN32
win32: SOURCES += spy.cpp

HEADERS  += mainwindow.h \
            download.h \
            avrc.h

FORMS    += mainwindow.ui

INCLUDEPATH += ../include

# ws2_32.lib path might need to be adjusted on user PC, for example 
# C:\WINDDK\7600.16385.0\lib\win7\i386\ws2_32.lib
win32: LIBS += -lQt5Network ws2_32.lib

RESOURCES     = resources.qrc

DISTFILES += \
    README.txt
