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

#ifndef INTEL_HW_RENDERER_H_

#define INTEL_HW_RENDERER_H_

#include <media/stagefright/VideoRenderer.h>
#include <utils/RefBase.h>
#include <OMX_Component.h>
#include <OMX_IVCommon.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <va/va.h>

#ifdef __cplusplus
}
#endif

#include <va/va_android.h>
#include <vabuffer.h>

static const int OMX_INTEL_COLOR_FormatRawVa = 0x7FA00E00;
static const int MAX_RENDER_SURFACE_COUNT = 3;
typedef unsigned int Display;

namespace android {

class ISurface;
class Overlay;

class IntelHwRenderer : public VideoRenderer {
public:
    IntelHwRenderer(
            const sp<ISurface> &surface,
            size_t displayWidth, size_t displayHeight,
            size_t decodedWidth, size_t decodedHeight,
            OMX_COLOR_FORMATTYPE colorFormat);

    virtual ~IntelHwRenderer();

    status_t initCheck() const { return mInitCheck; }

    virtual void render(
            const void *data, size_t size, void *platformPrivate);

private:
    sp<ISurface> mISurface;
    size_t mDisplayWidth, mDisplayHeight;
    size_t mDecodedWidth, mDecodedHeight;
    OMX_COLOR_FORMATTYPE mColorFormat;
    status_t mInitCheck;

    //for software decoder and hardware render case
    Display *mDisplay;
    VADisplay mVADisplay;

    int mCurrentSurfaceIndex;
    VASurfaceID mRTSurfaces[MAX_RENDER_SURFACE_COUNT];
    VAImage mSurfaceImages[MAX_RENDER_SURFACE_COUNT];
    void * mBufferPointers[MAX_RENDER_SURFACE_COUNT];

    VAStatus InitSurfaceForRender();
    VAStatus DeInitSurfaceForRender();
    VAStatus convertBufferToSurface(const void *data, size_t size, OMX_COLOR_FORMATTYPE srcColorFormat,
                             int surfaceIndex, OMX_COLOR_FORMATTYPE dstColorFormat);

    IntelHwRenderer(const IntelHwRenderer &);
    IntelHwRenderer &operator=(const IntelHwRenderer &);
};

}  // namespace android

#endif  // INTEL_HW_RENDERER_H_

