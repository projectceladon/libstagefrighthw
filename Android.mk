# Copyright (C) 2009 The Android Open Source Project
# Copyright (C) 2019-2022 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

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
	liblog			\
        libstagefright_foundation

LOCAL_MODULE := libstagefrighthw
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := intel
ifeq ($(shell test $(PLATFORM_SDK_VERSION) -ge 27; echo $$?), 0)
LOCAL_HEADER_LIBRARIES += media_plugin_headers
endif
include $(BUILD_SHARED_LIBRARY)

PREBUILT_PROJECT := libstagefrighthw
include $(BUILD_PREBUILT_BUNDLE_CREATE)
endif

