#-------------------------------------------------
#
# Project created by QtCreator 2016-05-09T09:51:05
#
#-------------------------------------------------

QT       -= core gui

TARGET = coap
TEMPLATE = lib

DEFINES += COAP_LIBRARY
DEFINES += WITH_POSIX
DEFINES += HAVE_CONFIG_H
DEFINES += _GNU_SOURCE
INCLUDEPATH += include/coap
unix {
    target.path = /usr/lib
    INSTALLS += target
}

SOURCES += \
    src/address.c \
    src/async.c \
    src/block.c \
    src/coap_io.c \
    src/coap_time.c \
    src/debug.c \
    src/encode.c \
    src/hashkey.c \
    src/mem.c \
    src/net.c \
    src/option.c \
    src/pdu.c \
    src/resource.c \
    src/str.c \
    src/subscribe.c \
    src/uri.c

HEADERS += \
    include/coap/address.h \
    include/coap/async.h \
    include/coap/bits.h \
    include/coap/block.h \
    include/coap/coap.h \
    include/coap/coap_io.h \
    include/coap/coap_time.h \
    include/coap/debug.h \
    include/coap/encode.h \
    include/coap/hashkey.h \
    include/coap/libcoap.h \
    include/coap/lwippools.h \
    include/coap/mem.h \
    include/coap/net.h \
    include/coap/option.h \
    include/coap/pdu.h \
    include/coap/prng.h \
    include/coap/resource.h \
    include/coap/str.h \
    include/coap/subscribe.h \
    include/coap/uri.h \
    include/coap/uthash.h \
    include/coap/utlist.h \
    coap_config.h
