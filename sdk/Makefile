##############################################
# OpenWrt Makefile for vng.iot.sdk library
# 
##############################################

include $(TOPDIR)/rules.mk

# Name and release number of this package
PKG_NAME:=vng.iot.sdk
PKG_RELEASE:=1


# This specifies the directory where we're going to build the program.  
# The root build directory, $(BUILD_DIR), is by default the build_mipsel 
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

# Specify package information for this program. 
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION 
# variable below and uncomment the Kamikaze define
# directive for the description below
define Package/$(PKG_NAME)
        NAME:=vng.iot.sdk
        SECTION:=libs
        CATEGORY:=VNG
        TITLE:=VNG iot sdk
        DEPENDS:=+libstdcpp +libjson-c +libpahomqtt +libmosquitto +libcoap
endef


define Package/$(PKG_NAME)/description
        VNG Corp. IoT software development kits
endef


# Specify what needs to be done to prepare for building the package.
# In our case, we need to copy the source files to the build directory.
# This is NOT the default.  The default uses the PKG_SOURCE_URL and the
# PKG_SOURCE which is not defined here to download the source from the web.
# In order to just build a simple program that we have just written, it is
# much easier to do it this way.
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


# Specify where and how to install the program. Since we only have one file.
# The $(1) variable represents the root directory on the router running 
# OpenWrt. The $(INSTALL_DIR) variable contains a command to prepare the install 
# directory if it does not already exist.  Likewise $(INSTALL_BIN) contains the 
# command to copy the binary file from its current location (in our case the build
# directory) to the install directory.
define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/lib$(PKG_NAME).* $(1)/usr/lib/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	mkdir -p $(1)/usr/include/vng
	mkdir -p $(1)/usr/include/vng/iot
	mkdir -p $(1)/usr/include/vng/iot/sdk
	$(CP) $(PKG_BUILD_DIR)/*.h $(1)/usr/include/vng/iot/sdk

	mkdir -p $(1)/usr/include/vng/iot/sdk/ble
	$(CP) $(PKG_BUILD_DIR)/ble/*.h $(1)/usr/include/vng/iot/sdk/ble

	mkdir -p $(1)/usr/include/vng/iot/sdk/ble/gap
	$(CP) $(PKG_BUILD_DIR)/ble/gap/*.h $(1)/usr/include/vng/iot/sdk/ble/gap

	mkdir -p $(1)/usr/include/vng/iot/sdk/comm
	$(CP) $(PKG_BUILD_DIR)/comm/*.h $(1)/usr/include/vng/iot/sdk/comm

	mkdir -p $(1)/usr/include/vng/iot/sdk/comm/ble
	$(CP) $(PKG_BUILD_DIR)/comm/ble/*.h $(1)/usr/include/vng/iot/sdk/comm/ble

	mkdir -p $(1)/usr/include/vng/iot/sdk/core
	$(CP) $(PKG_BUILD_DIR)/core/*.h $(1)/usr/include/vng/iot/sdk/core

	mkdir -p $(1)/usr/include/vng/iot/sdk/devices/
	$(CP) $(PKG_BUILD_DIR)/devices/*.h $(1)/usr/include/vng/iot/sdk/devices

	mkdir -p $(1)/usr/include/vng/iot/sdk/net
	$(CP) $(PKG_BUILD_DIR)/net/*.h $(1)/usr/include/vng/iot/sdk/net

	mkdir -p $(1)/usr/include/vng/iot/sdk/net/paho
	$(CP) $(PKG_BUILD_DIR)/net/paho/*.h $(1)/usr/include/vng/iot/sdk/net/paho
	
	mkdir -p $(1)/usr/include/vng/iot/sdk/net/mosquitto
	$(CP) $(PKG_BUILD_DIR)/net/mosquitto/*.h $(1)/usr/include/vng/iot/sdk/net/mosquitto
	
	mkdir -p $(1)/usr/include/vng/iot/sdk/std
	$(CP) $(PKG_BUILD_DIR)/std/*.h $(1)/usr/include/vng/iot/sdk/std

	mkdir -p $(1)/usr/include/vng/iot/sdk/utils
	$(CP) $(PKG_BUILD_DIR)/utils/*.h $(1)/usr/include/vng/iot/sdk/utils

	mkdir -p $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/lib$(PKG_NAME).* $(1)/usr/lib/
endef


# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.
$(eval $(call BuildPackage,$(PKG_NAME)))
