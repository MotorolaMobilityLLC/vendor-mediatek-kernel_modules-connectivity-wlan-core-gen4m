include $(CLEAR_VARS)

LOCAL_MODULE := $(WIFI_NAME).ko
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_REQUIRED_MODULES := $(WIFI_CHRDEV_MODULE)

ifeq ($(CONNAC_VER), 3_0)
else ifeq ($(CONNAC_VER), 2_0)
	LOCAL_REQUIRED_MODULES += conninfra.ko
	LOCAL_REQUIRED_MODULES += connfem.ko
else
	LOCAL_REQUIRED_MODULES += wmt_drv.ko
endif

include $(MTK_KERNEL_MODULE)

WIFI_OPTS := CONFIG_MTK_COMBO_WIFI_HIF=$(WIFI_HIF) MODULE_NAME=$(WIFI_NAME) MTK_COMBO_CHIP=$(WIFI_CHIP) WLAN_CHIP_ID=$(WLAN_CHIP_ID) MTK_ANDROID_WMT=$(WIFI_WMT) WIFI_ENABLE_GCOV=$(WIFI_ENABLE_GCOV) WIFI_IP_SET=$(WIFI_IP_SET) MTK_ANDROID_EMI=$(WIFI_EMI)
WIFI_OPTS += WLAN_CONNAC3_DEV=$(MTK_WLAN_CONNAC3_DEV)
WIFI_OPTS += CONNAC_VER=$(CONNAC_VER)
WIFI_OPTS += MTK_WLAN_SERVICE=yes

$(linked_module): OPTS += $(WIFI_OPTS)
