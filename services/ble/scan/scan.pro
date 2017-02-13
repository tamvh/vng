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
    src/files/etc/vng/services/ble/scan.json \
    src/files/vng.blescan.init \
    src/files/etc/init.d/services/ble/scan.init
