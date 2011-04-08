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

#ifndef INTEL_COLOR_CONVERTER_H_

#define INTEL_COLOR_CONVERTER_H_

#include <ColorConverter.h>
#include <va/va_android.h>
#include <vabuffer.h>

namespace android {

static const int OMX_INTEL_COLOR_FormatRawVa = 0x7FA00E00;

struct IntelColorConverter : public ColorConverter {
    IntelColorConverter(OMX_COLOR_FORMATTYPE from, OMX_COLOR_FORMATTYPE to);
    virtual ~IntelColorConverter();

    virtual void convert(
        size_t width, size_t height,
        const void *srcBits, size_t srcSkip,
        void *dstBits, size_t dstSkip);

private:

    void convertIntelYUV420SemiPlanar(
        size_t width, size_t height,
        const void *srcBits, size_t srcSkip,
        void *dstBits, size_t dstSkip);

    VAStatus mapVaData(const void *srcBits, void *&surface_buf,
                       const VABuffer *&vaBuf, VAImage &sourceImage);

    VAStatus unmapVaData(const VABuffer *vaBuf, VAImage &sourceImage);

    uint8_t *intelInitClip();

    OMX_COLOR_FORMATTYPE mIntelSrcFormat, mIntelDstFormat;

    uint8_t *mIntelClip;
};

}  // namespace android

#endif  // INTEL_COLOR_CONVERTER_H_
