TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += $$PWD/../../../sdk/src
DEPENDPATH += $$PWD/../../../sdk/src

HEADERS += \
    src/application.h \
    src/cloud/cloudmqttclient.h \
    src/cloud/iotcloud.h

SOURCES += \
    src/main.cpp \
    src/application.cpp \
    src/cloud/cloudmqttclient.cpp \
    src/cloud/iotcloud.cpp

DISTFILES += \
    src/CMakeLists.txt \
    src/files/etc/vng/services/access/reader.json \
    src/files/etc/init.d/services/access/reader.init \
    Makefile
