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
#define LOG_NDEBUG 0
#define LOG_TAG "IntelHwRenderer"
#include <utils/Log.h>

#include "IntelHwRenderer.h"

#include <media/stagefright/MediaDebug.h>
#include <surfaceflinger/ISurface.h>
#include <ui/Overlay.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <va/va.h>

#ifdef __cplusplus
}
#endif

#include <va/va_android.h>
#include <vabuffer.h>

#include <sys/time.h>


namespace android {

static int g_framecount = 0;
static unsigned long g_start = 0;
static unsigned long g_end = 0;
////////////////////////////////////////////////////////////////////////////////

IntelHwRenderer::IntelHwRenderer(
    const sp<ISurface> &surface,
    size_t displayWidth, size_t displayHeight,
    size_t decodedWidth, size_t decodedHeight,
    OMX_COLOR_FORMATTYPE colorFormat)
    :mISurface(surface)
    ,mDisplayWidth(displayWidth)
    ,mDisplayHeight(displayHeight)
    ,mDecodedWidth(decodedWidth)
    ,mDecodedHeight(decodedHeight)
    ,mColorFormat(colorFormat)
    ,mInitCheck(OK) {
    g_framecount = 0;
    g_start = 0;
    g_end = 0;
    LOGV("---- Constructor ---\n");
}

IntelHwRenderer::~IntelHwRenderer() {
    //LOGV("---- Destructor ---g_framecount=%d, g_start=%lu, g_end=%lu\n", g_framecount, g_start, g_end);
    float duration = (float)(g_end - g_start) / 1000.0f;
    float fps = (float) g_framecount / duration;
    LOGV("duration=%f, fps=%f\n", duration, fps);
}

void IntelHwRenderer::render(const void *data,
    size_t size, void *platformPrivate) {
    LOGV("---- start rendering ---\n");
    ++g_framecount;
    if(0 == g_start) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        g_start = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }
        
    VABuffer *vaBuf = (VABuffer *)(data);
    VAStatus va_status;
    LOGV("In IntelHwRenderer vadisplay = %x surfaceid = %x aDataLen = %d\n", 
                        vaBuf->display, vaBuf->surface, size);
    va_status = vaSyncSurface(vaBuf->display, vaBuf->surface);
    LOGV("In IntelHwRenderer vaSyncSurface() returns = %x\n", va_status);
    LOGV("vaBuf->frame_structure = 0x%x", vaBuf->frame_structure);
    va_status = vaPutSurface(vaBuf->display, vaBuf->surface,
                        mISurface, 0, 0, mDecodedWidth, mDecodedHeight,
                        0, 0, mDisplayWidth, mDisplayHeight,
                        NULL, 0, 0);
    LOGV("In IntelHwRenderer vaPutSurface() returns = %x\n", va_status);
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        g_end = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        //LOGV("vaPutSurface() g_end = %lu\n", g_end);
    }

}

}  // namespace android

