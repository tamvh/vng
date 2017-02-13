TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += $$PWD/../../../sdk/src
INCLUDEPATH += src
DEPENDPATH += $$PWD/../../../sdk/src

SOURCES += \
    src/application.cpp \
    src/httpclient.cpp \
    src/main.cpp \
    src/zcloudclient.cpp

DISTFILES += \
    src/files/etc/vng/services/light/sensor.json \
    src/files/etc/init.d/services/light/sensor.init \
    src/CMakeLists.txt

HEADERS += \
    src/timerfd.h \
    src/zcloudclient.h \
    src/zclouddef.h \
    src/zclouddevice.h \
    src/application.h \
    src/cloudclient.h \
    src/httpclient.h \
    src/upload.h
