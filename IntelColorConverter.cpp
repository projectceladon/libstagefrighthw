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

#include <MediaDebug.h>
#include "IntelColorConverter.h"

namespace android {

#ifdef __cplusplus
extern "C" {
#endif

    ColorConverter *createPlatformSpecificColorConverter(
        OMX_COLOR_FORMATTYPE from, OMX_COLOR_FORMATTYPE to) {
        return new IntelColorConverter(from, to);
    }

#ifdef __cplusplus
}
#endif

IntelColorConverter::IntelColorConverter(
    OMX_COLOR_FORMATTYPE from, OMX_COLOR_FORMATTYPE to)
    : ColorConverter(from, to),
    mIntelSrcFormat(from),
    mIntelDstFormat(to),
    mIntelClip(NULL) {
}

IntelColorConverter::~IntelColorConverter() {
    delete[] mIntelClip;
    mIntelClip = NULL;
}

void IntelColorConverter::convert(
    size_t width, size_t height,
    const void *srcBits, size_t srcSkip,
    void *dstBits, size_t dstSkip) {
    CHECK_EQ(mIntelDstFormat, OMX_COLOR_Format16bitRGB565);

    if (mIntelSrcFormat != OMX_INTEL_COLOR_FormatRawVa)
    {
        ColorConverter::convert(
            width, height, srcBits, srcSkip, dstBits, dstSkip);
    }
    else
    {
        convertIntelYUV420SemiPlanar(
            width, height, srcBits, srcSkip, dstBits, dstSkip);
    }
}

void IntelColorConverter::convertIntelYUV420SemiPlanar(
    size_t width, size_t height,
    const void *srcBits, size_t srcSkip,
    void *dstBits, size_t dstSkip) {
    CHECK_EQ(srcSkip, 0);  // Doesn't really make sense for YUV formats.
    CHECK(dstSkip >= width * 2);
    CHECK((dstSkip & 3) == 0);

    void *surface_buf = NULL;
    const VABuffer *vaBuf = NULL;
    VAImage sourceImage;

    uint8_t *adjustedClip = intelInitClip();

    VAStatus va_status = mapVaData(
                             srcBits, surface_buf, vaBuf, sourceImage);

    CHECK_EQ(va_status, VA_STATUS_SUCCESS);

    uint32_t *dstPtr = (uint32_t *)dstBits;
    const uint8_t *src_y = (const uint8_t *)surface_buf;

    const uint8_t *src_u =
        (const uint8_t *)src_y + sourceImage.offsets[1];

    for (size_t h = 0; h < height; ++h) {
        for (size_t w = 0; w < width; w += 2) {
            signed y_1 = (signed)src_y[w] - 16;
            signed y_2 = (signed)src_y[w + 1] - 16;

            signed v = (signed)src_u[w & ~1] - 128;
            signed u = (signed)src_u[(w & ~1) + 1] - 128;

            signed u_b = u * 517;
            signed u_g = -u * 100;
            signed v_g = -v * 208;
            signed v_r = v * 409;

            signed tmp_1 = y_1 * 298;
            signed b_1 = (tmp_1 + u_b) / 256;
            signed g_1 = (tmp_1 + v_g + u_g) / 256;
            signed r_1 = (tmp_1 + v_r) / 256;

            signed tmp_2 = y_2 * 298;
            signed b_2 = (tmp_2 + u_b) / 256;
            signed g_2 = (tmp_2 + v_g + u_g) / 256;
            signed r_2 = (tmp_2 + v_r) / 256;

            uint32_t rgb_1 =
                ((adjustedClip[b_1] >> 3) << 11)
                | ((adjustedClip[g_1] >> 2) << 5)
                | (adjustedClip[r_1] >> 3);

            uint32_t rgb_2 =
                ((adjustedClip[b_2] >> 3) << 11)
                | ((adjustedClip[g_2] >> 2) << 5)
                | (adjustedClip[r_2] >> 3);

            dstPtr[w / 2] = (rgb_2 << 16) | rgb_1;
        }

        src_y += sourceImage.pitches[0];

        if (h & 1) {
            src_u += sourceImage.pitches[0];
        }

        dstPtr += dstSkip / 4;
    }
    va_status = unmapVaData(vaBuf, sourceImage);
    CHECK_EQ(va_status, VA_STATUS_SUCCESS);
}

VAStatus IntelColorConverter::mapVaData(
    const void *srcBits, void *&surfaceBuf,
    const VABuffer *&vaBuf, VAImage &sourceImage)
{
    VAStatus va_status;

    vaBuf = (const VABuffer *)srcBits;

    va_status = vaDeriveImage(vaBuf->display, vaBuf->surface, &sourceImage);
    if (va_status != VA_STATUS_SUCCESS)
    {
        LOGE("Failed to derive image!");
        return va_status;
    }

    va_status = vaMapBuffer(vaBuf->display, sourceImage.buf, &surfaceBuf);

    if (va_status != VA_STATUS_SUCCESS) {
        LOGE("Failed to map buffer!");
        vaDestroyImage(vaBuf->display, sourceImage.image_id);
    }
    return va_status;
}

VAStatus IntelColorConverter::unmapVaData(
    const VABuffer *vaBuf, VAImage &sourceImage)
{
    VAStatus va_status;
    va_status = vaUnmapBuffer(vaBuf->display, sourceImage.buf);
    if (va_status != VA_STATUS_SUCCESS)
    {
        LOGE("Failed to unmap buffer!");
        vaDestroyImage(vaBuf->display, sourceImage.image_id);
        return va_status;
    }

    va_status = vaDestroyImage(vaBuf->display, sourceImage.image_id);
    if (va_status!=VA_STATUS_SUCCESS)
    {
        LOGE("Failed to Destroy Image!");
    }
    return va_status;
}

uint8_t *IntelColorConverter::intelInitClip() {
    static const signed kClipMin = -278;
    static const signed kClipMax = 535;

    if (mIntelClip == NULL) {
        mIntelClip = new uint8_t[kClipMax - kClipMin + 1];

        for (signed i = kClipMin; i <= kClipMax; ++i) {
            mIntelClip[i - kClipMin] = (i < 0) ? 0 : (i > 255) ? 255 : (uint8_t)i;
        }
    }

    return &mIntelClip[-kClipMin];
}

}  // namespace android
