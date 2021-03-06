#-------------------------------------------------
#
# Project created by QtCreator 2015-10-16T09:41:01
#
#-------------------------------------------------

# CR100 image should follo version set here
VERSION = 1.19

CONFIG += c++11

include(bridge.user.pri)

#QT       += core
QT       += core spidevice

QT       -= gui
QT       += network

QMAKE_CXXFLAGS += -O0

TARGET = COM5003LWLBridge
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lzeraxmlconfig
LIBS += -lzeramath
LIBS += -lQt5SpiDevice

TARGET = com5003lwlbridge

SOURCES += main.cpp \
    bridgeconfiguration.cpp \
    lwlconnection.cpp \
    bridge.cpp \
    ethconnection.cpp \
    ethparameterdelegate.cpp \
    ethmeasuredelegate.cpp \
    ethoscilloscopedelegate.cpp \
    spiconnection.cpp \
    ethcmddelegate.cpp

HEADERS += \
    bridgeconfigdata.h \
    bridgeconfiguration.h \
    lwlconnection.h \
    bridge.h \
    ethconnection.h \
    ethcmddelegate.h \
    ethparameterdelegate.h \
    ethmeasuredelegate.h \
    ethoscilloscopedelegate.h \
    spi2fpga.h \
    spiconnection.h

OTHER_FILES += \
    bridge.xml \
    bridge.xsd

configxml.path = /etc/zera/com5003lwlbridge/
configxml.files = bridge.xsd \
                  bridge.xml

INSTALLS += configxml

target.path = /usr/bin
INSTALLS += target
