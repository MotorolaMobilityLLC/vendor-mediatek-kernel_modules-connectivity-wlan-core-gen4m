LOCAL_PATH := $(call my-dir)

ifeq ($(MTK_WLAN_SUPPORT), yes)

ifeq ($(WLAN_CHIP_ID), common)
	# for layer decoupling 2.0, we have to build all configurations
	WLAN_CHIP_ID := 6893
	WIFI_CHIP := CONNAC2X2_SOC3_0
	WIFI_IP_SET := 1
	CONNAC_VER := 2_0
	WIFI_NAME := wlan_drv_gen4m_6893
	WIFI_CHRDEV_MODULE := wmt_chrdev_wifi_connac2.ko
	include $(LOCAL_PATH)/build_wlan_drv.mk
else
	WIFI_NAME := wlan_drv_gen4m
	WIFI_CHRDEV_MODULE := wmt_chrdev_wifi.ko
	include $(LOCAL_PATH)/build_wlan_drv.mk
endif

endif
