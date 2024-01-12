################################
## Add specified modules here ##
################################
$(info CONNAC version: $(CONNAC_VER))
$(info Segment: $(SEGMENT))

ifeq ($(SEGMENT), SP)
    # build ko by connac version
    ifeq ($(CONNAC_VER), 3_0)
        obj-m += build/connac3x/6985_6639/
    else ifeq ($(CONNAC_VER), 2_0)
        obj-m += build/connac2x/6897/
        obj-m += build/connac2x/6893/
    endif
else
    KO_CODE_PATH := $(if $(filter /%,$(src)),,$(srctree)/)$(src)

    include $(KO_CODE_PATH)/Kbuild.main
endif
