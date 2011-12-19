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

LOCAL_SHARED_LIBRARIES :=       \
        libbinder               \
        libutils                \
        libcutils               \
        libui                   \
        libdl                   \
        libsurfaceflinger_client

LOCAL_MODULE := libstagefrighthw

include $(BUILD_SHARED_LIBRARY)
endif

