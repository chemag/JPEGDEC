/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Accessors the unit tests use that the C API does not provide.
 */
#ifndef JPEGDEC_TEST_H
#define JPEGDEC_TEST_H

#include "../src/JPEGDEC.h"

static inline int jpegdec_test_get_jpeg_type(JPEGIMAGE *pJPEG)
{
    return (pJPEG->ucMode == 0xc2) ? JPEG_MODE_PROGRESSIVE : JPEG_MODE_BASELINE;
}

static inline int jpegdec_test_get_pixel_type(JPEGIMAGE *pJPEG)
{
    return (int)pJPEG->ucPixelType;
}

#endif // JPEGDEC_TEST_H
