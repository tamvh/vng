#*******************************************************************************
#  Copyright (c) 2016 VNG Corp. iot lab
# 
#  Contributors:
#     Trung Bui Quang(trungbq#@vng.com.vn) - initial version
#*******************************************************************************/

# Note: on OS X you should install XCode and the associated command-line tools

SET(PROJECT_NAME "vng.iot.sdk")
PROJECT("${PROJECT_NAME}" CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

## build settings
SET(VNGIOTSDK_VERSION_MAJOR 1)
SET(VNGIOTSDK_VERSION_MINOR 0)
SET(VNGIOTSDK_VERSION_PATCH 0)
SET(VNGIOTSDK_VERSION ${VNGIOTSDK_VERSION_MAJOR}.${VNGIOTSDK_VERSION_MINOR}.${VNGIOTSDK_VERSION_PATCH})

EXECUTE_PROCESS(COMMAND date -u OUTPUT_VARIABLE BUILD_TIMESTAMP)
STRING(STRIP ${BUILD_TIMESTAMP} BUILD_TIMESTAMP)

## build options
SET(VNGIOTSDK_WITH_SSL FALSE CACHE BOOL "Flag that defines whether to build ssl-enabled binaries too. ")
SET(VNGIOTSDK_BUILD_DOCUMENTATION FALSE CACHE BOOL "Create and install the HTML based API documentation (requires Doxygen)")
SET(VNGIOTSDK_BUILD_SAMPLES FALSE CACHE BOOL "Build sample programs")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(-DUSE_EPOLL -DUSE_LIBJSONC -DWITH_POSIX -DDEBUG_PRINT)
add_library("${PROJECT_NAME}"
	    SHARED
            net/mosquitto/netmosquittochannel.cpp
            ble/gap/blegappacket.cpp
            comm/ble/commbleadvertise.cpp
            comm/ble/commblecommand.cpp
            comm/ble/commblereceive.cpp
            comm/commmessage.cpp
            core/coreapplication.cpp
	    core/coreexecute.cpp
	    core/coreapplication.cpp
	    core/corelock.cpp
	    core/coresemaphore.cpp
	    core/coresynchronize.cpp
	    core/corevariant.cpp
            devices/devicesprotocol.cpp
            devices/devicesdevice.cpp
            net/netaddress.cpp
            net/netapplication.cpp
            net/netchannel.cpp
            net/netsubscribe.cpp
            net/paho/netpahochannel.cpp
#            net/coap/netcoapendpoint.cpp
            utils/utilscrc16.cpp)

SET(CPACK_PACKAGE_VERSION_MAJOR ${VNGIOTSDK_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${VNGIOTSDK_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${VNGIOTSDK_VERSION_PATCH})
SET_TARGET_PROPERTIES("${PROJECT_NAME}"
	PROPERTIES
	VERSION ${VNGIOTSDK_VERSION}
	SOVERSION ${VNGIOTSDK_VERSION_MAJOR})
    
TARGET_LINK_LIBRARIES("${PROJECT_NAME}" json-c paho-mqtt3c mosquitto coap-1)
INSTALL(TARGETS "${PROJECT_NAME}"
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib)

INCLUDE(CPack)
