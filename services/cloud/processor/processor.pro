TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += $$PWD/../../../sdk/src
DEPENDPATH += $$PWD/../../../sdk/src

HEADERS += \
    src/application.h \
    src/cloud/iotcloud.h \
    src/cloud/cloudprocesscommand.h \
    src/cloud/controlldeviceadvertise.h \
    src/cloud/addressconvertutils.h

SOURCES += \
    src/main.cpp \
    src/application.cpp \
    src/cloud/iotcloud.cpp \
    src/cloud/cloudprocesscommand.cpp \
    src/cloud/addressconvertutils.cpp

DISTFILES += \
    src/CMakeLists.txt \
    src/files/etc/vng/services/cloud/processor.json \
    src/files/etc/init.d/services/cloud/processor.init \
    Makefile
