include $(TOPDIR)/rules.mk

PKG_NAME:=senderAPP
PKG_VERSION:=3.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)


include $(INCLUDE_DIR)/package.mk

define Package/senderAPP
	SECTION:=CSI
	CATEGORY:=CSI
	TITLE:=switch with sendData
endef

define Package/senderAPP/description
	switch with sendData	
endef

define Build/Prepare
	echo changed for 5 times
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS)
endef

define Package/senderAPP/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/senderAPP $(1)/bin/
endef

$(eval $(call BuildPackage,senderAPP))

