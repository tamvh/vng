#-------------------------------------------------
#
# Project created by QtCreator 2016-11-02T11:13:26
#
#-------------------------------------------------

QT       -= core gui

TARGET = sdk
TEMPLATE = lib

DEFINES += SDK_LIBRARY USE_LIBJSONC WITH_POSIX USE_EPOLL
INCLUDEPATH += src
SOURCES += \
    src/core/coreapplication.cpp \
    src/core/coreexecute.cpp \
    src/core/corelock.cpp \
    src/core/coresemaphore.cpp \
    src/core/coresynchronize.cpp \
    src/core/corevariant.cpp \
    src/utils/utilscrc16.cpp \
    src/devices/ble/gap/devicesblegappacket.cpp \
    src/devices/devicesdevice.cpp \
    src/devices/devicesprotocol.cpp \
    src/comm/commmessage.cpp \
    src/comm/ble/commbleadvertise.cpp \
    src/comm/ble/commblereceive.cpp \
    src/comm/ble/commblecommand.cpp \
    src/ble/gap/blegappacket.cpp \
    src/net/netaddress.cpp \
    src/net/netapplication.cpp \
    src/net/netchannel.cpp \
    src/net/netsubscribe.cpp \
    src/net/paho/netpahochannel.cpp \
    src/net/coap/netcoapendpoint.cpp \
    src/net/mosquitto/netmosquittochannel.cpp \
    src/ble/gap/blegapsubscribe.cpp \
    src/ble/gap/blegapcommand.cpp \
    src/ble/blecommand.cpp

HEADERS += \
    src/core/corecallback.h \
    src/core/corecommand.h \
    src/core/coreexecute.h \
    src/core/corelock.h \
    src/core/coresemaphore.h \
    src/core/coresynchronize.h \
    src/core/coreutils.h \
    src/core/corevariant.h \
    src/std/stdhash.h \
    src/std/stdlist.h \
    src/sdkdefs.h \
    src/utils/utilscrc16.h \
    src/core/coreapplication.h \
    src/devices/devicesdevice.h \
    src/devices/devicesprotocol.h \
    src/comm/commmessage.h \
    src/comm/ble/commbleadvertise.h \
    src/comm/ble/commblereceive.h \
    src/comm/ble/commblecommand.h \
    src/ble/gap/blegappacket.h \
    src/ble/bledefs.h \
    src/net/netaddress.h \
    src/net/netapplication.h \
    src/net/netchannel.h \
    src/net/netsubscribe.h \
    src/net/paho/netpahochannel.h \
    src/utils/utilsprint.h \
    src/net/coap/netcoapendpoint.h \
    src/net/mosquitto/netmosquittochannel.h \
    src/ble/gap/blegapsubscribe.h \
    src/ble/gap/blegapevent.h \
    src/ble/gap/blegapcommand.h \
    src/ble/blecommand.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
INCLUDEPATH += /usr/local/include

DISTFILES += \
    src/CMakeLists.txt \
    Makefile



