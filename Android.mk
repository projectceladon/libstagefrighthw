
ifeq ($(INTEL_STAGEFRIGHT),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    WrsOMXPlugin.cpp


LOCAL_CFLAGS := $(PV_CFLAGS_MINUS_VISIBILITY)

#enable log
#LOCAL_CFLAGS += -DLOG_NDEBUG=0

ifeq ($(USE_INTEL_MDP),true)
    LOCAL_CFLAGS += -DUSE_INTEL_MDP
endif

ifeq ($(TARGET_HAS_ISV), true)
	LOCAL_CFLAGS +=-DTARGET_HAS_ISV
endif

ifeq ($(PRODUCT_FEAT_VPU_VERISILICON),true)
    LOCAL_CFLAGS += -DUSE_HANTRO_OMX_CORE
endif
ifeq ($(PRODUCT_FEAT_VPU_ALLEGRO),true)
    LOCAL_CFLAGS += -DUSE_ALLEGRO_OMX_CORE
endif


LOCAL_C_INCLUDES:= \
        $(call include-path-for, frameworks-native)/media/hardware \
        $(call include-path-for, frameworks-native)/media/openmax

LOCAL_SHARED_LIBRARIES :=       \
        libbinder               \
        libutils                \
        libcutils               \
        libui                   \
        libdl                   \
        libstagefright_foundation

LOCAL_MODULE := libstagefrighthw
LOCAL_MODULE_OWNER := intel

include $(BUILD_SHARED_LIBRARY)

PREBUILT_PROJECT := libstagefrighthw
include $(BUILD_PREBUILT_BUNDLE_CREATE)
endif

