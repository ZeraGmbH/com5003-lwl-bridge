#-------------------------------------------------
#
# Project created by QtCreator 2015-10-16T09:41:01
#
#-------------------------------------------------

CONFIG += c++11

QT       += core
QT       += core spidevice

QT       -= gui
QT       += network

include(bridge.user.pri)

TARGET = COM5003LWLBridge
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lzeraxmlconfig
LIBS += -lzeramath
LIBS += -lQt5SpiDevice

SOURCES += src/main.cpp \
    src/bridgeconfiguration.cpp \
    src/lwlconnection.cpp \
    src/bridge.cpp \
    src/ethconnection.cpp \
    src/ethcmdserializer.cpp \
    src/ethparameterdelegate.cpp \
    src/ethmeasuredelegate.cpp \
    src/ethoscilloscopedelegate.cpp \
    src/ethcmddelegate.cpp \
    src/spiconnection.cpp

HEADERS += \
    src/bridgeconfigdata.h \
    src/bridgeconfiguration.h \
    src/lwlconnection.h \
    src/bridge.h \
    src/ethconnection.h \
    src/ethcmddelegate.h \
    src/ethcmdserializer.h \
    src/ethparameterdelegate.h \
    src/ethmeasuredelegate.h \
    src/ethoscilloscopedelegate.h \
    src/spi2fpga.h \
    src/spiconnection.h

OTHER_FILES += \
    src/bridge.xml \
    src/bridge.xsd

configxml.path = /etc/zera/com5003bridge
configxml.files = bridge.xsd \
                  bridge.xml

INSTALLS += configxml

