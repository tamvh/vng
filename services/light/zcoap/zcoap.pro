TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += $$PWD/../../../sdk/src
INCLUDEPATH += src
DEPENDPATH += $$PWD/../../../sdk/src

SOURCES += \
    src/application.cpp \
    src/main.cpp \
    src/cantcoap/cantcoap.cpp \
    src/cantcoap/nethelper.cpp \
    src/zmqtt.cpp

DISTFILES += \
    src/files/etc/vng/services/light/zcoap.json \
    src/CMakeLists.txt \
    src/files/etc/init.d/services/light/zcoap.init \
    Makefile \
    src/cantcoap/Doxyfile


HEADERS += \
    src/timerfd.h \
    src/application.h \
    src/cantcoap/cantcoap.h \
    src/cantcoap/dbg.h \
    src/cantcoap/nethelper.h \
    src/cantcoap/sysdep.h \
    src/cantcoap/uthash.h \
    src/zmqtt.h
