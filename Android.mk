ifeq ($(BUILD_WITH_FULL_STAGEFRIGHT),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    WrsOMXPlugin.cpp


LOCAL_CFLAGS := $(PV_CFLAGS_MINUS_VISIBILITY)

#enable log
#LOCAL_CFLAGS += -DLOG_NDEBUG=0

LOCAL_C_INCLUDES:= \
		$(TOP)/frameworks/base/include/media/stagefright/openmax \
        $(TARGET_OUT_HEADERS)/libva \
        $(TARGET_OUT_HEADERS)/libwrs_omxil_intel_mrst_psb \
	$(TOP)/frameworks/base/include/media/stagefright \

LOCAL_SHARED_LIBRARIES :=       \
        libbinder               \
        libutils                \
        libcutils               \
        libui                   \
        libdl                   \
        libva                   \
        libva-android           \
        libva-tpi               \
        libsurfaceflinger_client

LOCAL_MODULE := libstagefrighthw

include $(BUILD_SHARED_LIBRARY)
endif

