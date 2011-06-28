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
//#define LOG_NDEBUG 0
#define LOG_TAG "IntelHwRenderer"
#include <utils/Log.h>

#include "IntelHwRenderer.h"

#include <media/stagefright/MediaDebug.h>
#include <surfaceflinger/ISurface.h>
#include <ui/Overlay.h>
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

    if (colorFormat == OMX_COLOR_FormatYUV420Planar){
        LOGV("use hardware render for frame from software decoder!");
        VAStatus va_status = InitSurfaceForRender();
        if( va_status != VA_STATUS_SUCCESS){
            LOGE("initial intel hardware render error!");
            mInitCheck = UNKNOWN_ERROR;
        }
    }else if (colorFormat == OMX_INTEL_COLOR_FormatRawVa){
        // nothind to do here
        return ;
    }else {
        LOGE("not supported format for render");
        mInitCheck = UNKNOWN_ERROR;
    }
}

IntelHwRenderer::~IntelHwRenderer() {
    if (OMX_INTEL_COLOR_FormatRawVa != mColorFormat){
        DeInitSurfaceForRender();
    }
    //LOGV("---- Destructor ---g_framecount=%d, g_start=%lu, g_end=%lu\n", g_framecount, g_start, g_end);
    float duration = (float)(g_end - g_start) / 1000.0f;
    float fps = (float) g_framecount / duration;
    LOGV("duration=%f, fps=%f\n", duration, fps);
}

VAStatus IntelHwRenderer::InitSurfaceForRender(){
    int major_ver, minor_ver;
    VAStatus va_status = VA_STATUS_SUCCESS;
    mDisplay = new Display;
    *mDisplay = 0x18c34078;

    mVADisplay = vaGetDisplay(&mDisplay);

    va_status = vaInitialize(mVADisplay, &major_ver, &minor_ver);
    if(va_status != VA_STATUS_SUCCESS)
    {
       LOGE("Faild to initialize va driver!");
       return va_status;
    }

    va_status = vaCreateSurfaces(mVADisplay,mDecodedWidth, mDecodedHeight,
                                VA_RT_FORMAT_YUV420, MAX_RENDER_SURFACE_COUNT, &mRTSurfaces[0]);
    if(va_status != VA_STATUS_SUCCESS)
    {
       LOGE("Failed to create surface for render");
       return va_status;
    }

    for(int i = 0; i < MAX_RENDER_SURFACE_COUNT; i++)
    {
       va_status = vaDeriveImage(mVADisplay, mRTSurfaces[i], &mSurfaceImages[i]);
       if (va_status != VA_STATUS_SUCCESS)
       {
           LOGE("Failed to derive image!");
           return va_status;
       }

       va_status = vaMapBuffer(mVADisplay, mSurfaceImages[i].buf, &mBufferPointers[i]);
       if (va_status != VA_STATUS_SUCCESS) {
           LOGE("Failed to map buffer!");
           vaDestroyImage(mVADisplay, mSurfaceImages[i].image_id);
           return va_status;
       }

    }

    mCurrentSurfaceIndex = 0;

    return va_status;
}

VAStatus IntelHwRenderer::DeInitSurfaceForRender(){
    VAStatus va_status = VA_STATUS_SUCCESS;
    for(int i = 0; i < MAX_RENDER_SURFACE_COUNT; i++)
    {
       va_status = vaUnmapBuffer(mVADisplay, mSurfaceImages[i].buf);
       if (va_status != VA_STATUS_SUCCESS)
       {
           LOGE("Failed to unmap buffer!");
           vaDestroyImage(mVADisplay, mSurfaceImages[i].image_id);
           return va_status;
       }

       va_status = vaDestroyImage(mVADisplay, mSurfaceImages[i].image_id);
       if (va_status != VA_STATUS_SUCCESS)
       {
           LOGE("Failed to Destroy Image!");
           return va_status;
       }
    }

    va_status = vaDestroySurfaces(mVADisplay,mRTSurfaces,MAX_RENDER_SURFACE_COUNT);
    if (va_status != VA_STATUS_SUCCESS)
    {
       LOGE("Failed to Destroy surfaces!");
       return va_status;
    }

     vaTerminate(mVADisplay);
     mVADisplay = NULL;
     mCurrentSurfaceIndex = 0;

     return OK;
}

VAStatus IntelHwRenderer::convertBufferToSurface(const void *data, size_t size, OMX_COLOR_FORMATTYPE srcColorFormat,
                                    int surfaceIndex, OMX_COLOR_FORMATTYPE dstColorFormat)
{
    VAStatus va_status;
    unsigned char *srcY, *srcU, *srcV;
    unsigned char *dstY, *dstUV;
    int imageWidth, imageHeight;

    if(srcColorFormat != OMX_COLOR_FormatYUV420Planar
       || dstColorFormat != OMX_COLOR_FormatYUV420SemiPlanar)
    {
       LOGE(" Unsupported src or dest color format! ");
       return VA_STATUS_ERROR_INVALID_BUFFER;
    }

    if(size != mDecodedWidth * mDecodedHeight * 3/2)
    {
       LOGE("Error decoded frame size!");
       return VA_STATUS_ERROR_INVALID_BUFFER;
    }

    srcY = (unsigned char*)data ;
    srcU = srcY + mDecodedWidth * mDecodedHeight;
    srcV = srcU + mDecodedWidth * mDecodedHeight / 4;

    dstY = (unsigned char*)(mBufferPointers[surfaceIndex] + mSurfaceImages[surfaceIndex].offsets[0]);
    dstUV = (unsigned char*)( mBufferPointers[surfaceIndex] + mSurfaceImages[surfaceIndex].offsets[1]);

    for(int i = 0; i < mDecodedHeight; i ++){
      //copy y planar
      memcpy(dstY + i * mSurfaceImages[surfaceIndex].pitches[0], srcY + i * mDecodedWidth, mDecodedWidth);

      //copy uv planar
      if(i % 2 == 0){
          for(int j = 0; j < mDecodedWidth; j = j + 2)
          {
             *(dstUV + i/2 * mSurfaceImages[surfaceIndex].pitches[1] + j) = *(srcU + i/2 * mDecodedWidth / 2 + j / 2);
             *(dstUV + i/2 * mSurfaceImages[surfaceIndex].pitches[1] + j + 1) = *(srcV + i/2 * mDecodedWidth / 2 + j / 2);
          }
      }

    }

    return  VA_STATUS_SUCCESS;
}

void IntelHwRenderer::render(const void *data,
    size_t size, void *platformPrivate) {

    VAStatus va_status;
    VASurfaceID mSurfaceForRender;
    ++g_framecount;
    if(0 == g_start) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        g_start = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }

    if (OMX_INTEL_COLOR_FormatRawVa == mColorFormat){
         VABuffer *vaBuf = (VABuffer *)(data);
         mVADisplay = vaBuf->display;
         mSurfaceForRender = vaBuf->surface;
    }else{
         mCurrentSurfaceIndex = (mCurrentSurfaceIndex + 1)%MAX_RENDER_SURFACE_COUNT;
         mSurfaceForRender = mRTSurfaces[mCurrentSurfaceIndex];

         va_status = convertBufferToSurface(data, size, mColorFormat,
                                mCurrentSurfaceIndex, OMX_COLOR_FormatYUV420SemiPlanar);
    }

    LOGV("In IntelHwRenderer vadisplay = %x surfaceid = %x aDataLen = %d\n", 
                        mVADisplay, mSurfaceForRender, size);
    va_status = vaSyncSurface(mVADisplay, mSurfaceForRender);
    LOGV("In IntelHwRenderer vaSyncSurface() returns = %x\n", va_status);

    va_status = vaPutSurface(mVADisplay, mSurfaceForRender,
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

