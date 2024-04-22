include $(TOPDIR)/rules.mk

PKG_NAME:=receiverAPP
PKG_VERSION:=3.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)


include $(INCLUDE_DIR)/package.mk

define Package/receiverAPP
	SECTION:=CSI
	CATEGORY:=CSI
	TITLE:=switch with recvCSI
endef

define Package/receiverAPP/description
	switch with recvCSI
endef

define Build/Prepare
	echo changed for 1 times
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS)
endef

define Package/receiverAPP/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/receiverAPP $(1)/bin/
endef

$(eval $(call BuildPackage,receiverAPP))

