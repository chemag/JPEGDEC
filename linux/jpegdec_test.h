/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Accessors the unit tests use that the C API does not provide.
 */
#ifndef JPEGDEC_TEST_H
#define JPEGDEC_TEST_H

#include "../src/JPEGDEC.h"

#ifdef __cplusplus
// JPEGDEC.h declares its C API for C compilers only; the fuzz
// harnesses are C++.
extern "C" {
int JPEG_openRAM(JPEGIMAGE *pJPEG, uint8_t *pData, int iDataSize, JPEG_DRAW_CALLBACK *pfnDraw);
void JPEG_setFramebuffer(JPEGIMAGE *pJPEG, void *pFramebuffer);
void JPEG_setCropArea(JPEGIMAGE *pJPEG, int x, int y, int w, int h);
void JPEG_getCropArea(JPEGIMAGE *pJPEG, int *x, int *y, int *w, int *h);
int JPEG_openFile(JPEGIMAGE *pJPEG, const char *szFilename, JPEG_DRAW_CALLBACK *pfnDraw);
int JPEG_getWidth(JPEGIMAGE *pJPEG);
int JPEG_getHeight(JPEGIMAGE *pJPEG);
int JPEG_decode(JPEGIMAGE *pJPEG, int x, int y, int iOptions);
int JPEG_decodeYUV420P(JPEGIMAGE *pJPEG, uint8_t *pY, int iYPitch, uint8_t *pCb, int iCbPitch, uint8_t *pCr, int iCrPitch);
int JPEG_decodeNV12(JPEGIMAGE *pJPEG, uint8_t *pY, int iYPitch, uint8_t *pUV, int iUVPitch);
int JPEG_decodeDither(JPEGIMAGE *pJPEG, uint8_t *pDither, int iOptions);
void JPEG_close(JPEGIMAGE *pJPEG);
int JPEG_getLastError(JPEGIMAGE *pJPEG);
int JPEG_getOrientation(JPEGIMAGE *pJPEG);
int JPEG_getBpp(JPEGIMAGE *pJPEG);
int JPEG_getSubSample(JPEGIMAGE *pJPEG);
int JPEG_hasThumb(JPEGIMAGE *pJPEG);
int JPEG_getThumbWidth(JPEGIMAGE *pJPEG);
int JPEG_getThumbHeight(JPEGIMAGE *pJPEG);
void JPEG_setPixelType(JPEGIMAGE *pJPEG, int iType);
void JPEG_setMaxOutputSize(JPEGIMAGE *pJPEG, int iMaxMCUs);
}
#endif

static inline int jpegdec_test_get_jpeg_type(JPEGIMAGE *pJPEG)
{
    return (pJPEG->ucMode == 0xc2) ? JPEG_MODE_PROGRESSIVE : JPEG_MODE_BASELINE;
}

static inline int jpegdec_test_get_pixel_type(JPEGIMAGE *pJPEG)
{
    return (int)pJPEG->ucPixelType;
}

#endif // JPEGDEC_TEST_H
