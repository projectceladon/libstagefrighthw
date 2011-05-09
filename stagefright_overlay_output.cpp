/*************************************************************************************
 * INTEL CONFIDENTIAL
 * Copyright 2011 Intel Corporation All Rights Reserved.
 * The source code contained or described herein and all documents related
 * to the source code ("Material") are owned by Intel Corporation or its
 * suppliers or licensors. Title to the Material remains with Intel
 * Corporation or its suppliers and licensors. The Material contains trade
 * secrets and proprietary and confidential information of Intel or its
 * suppliers and licensors. The Material is protected by worldwide copyright
 * and trade secret laws and treaty provisions. No part of the Material may
 * be used, copied, reproduced, modified, published, uploaded, posted,
 * transmitted, distributed, or disclosed in any way without Intel’s prior
 * express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery
 * of the Materials, either expressly, by implication, inducement, estoppel or
 * otherwise. Any license under such intellectual property rights must be express
 * and approved by Intel in writing.
 ************************************************************************************/

#include "IntelHwRenderer.h"

#include <media/stagefright/HardwareAPI.h>

using android::sp;
using android::ISurface;
using android::VideoRenderer;

VideoRenderer *createRenderer(
        const sp<ISurface> &surface,
        const char *componentName,
        OMX_COLOR_FORMATTYPE colorFormat,
        size_t displayWidth, size_t displayHeight,
        size_t decodedWidth, size_t decodedHeight) {
    using android::IntelHwRenderer;

    static const int OMX_INTEL_COLOR_FormatRawVa = 0x7FA00E00;
    if (OMX_INTEL_COLOR_FormatRawVa != colorFormat)
        return NULL;

    IntelHwRenderer *renderer =
        new IntelHwRenderer(
                surface, displayWidth, displayHeight,
                decodedWidth, decodedHeight,
                colorFormat);

    if (renderer->initCheck() != android::OK) {
        delete renderer;
        renderer = NULL;
    }

    return renderer;
}

