TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += $$PWD/../../../sdk/src
DEPENDPATH += $$PWD/../../../sdk/src

HEADERS += \
    src/application.h

SOURCES += \
    src/main.cpp \
    src/application.cpp

DISTFILES += \
    src/CMakeLists.txt \
    src/etc/vng/ble/advertise.json \
    src/files/vng.bleadvertise.init \
    src/files/etc/vng/services/ble/advertise.json \
    src/files/etc/init.d/services/ble/advertise.init \
    Makefile
