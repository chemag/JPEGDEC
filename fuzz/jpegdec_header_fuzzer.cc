// SPDX-License-Identifier: Apache-2.0
#include "jpegdec_fuzz_common.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size > jpegdec_fuzz::kMaxInputSize) return 0;
  jpegdec_fuzz::Image image(data, size);
  if (image.Open()) {
    (void)JPEG_getWidth(&image.jpeg);
    (void)JPEG_getHeight(&image.jpeg);
    (void)JPEG_getOrientation(&image.jpeg);
    (void)JPEG_getBpp(&image.jpeg);
    (void)JPEG_getSubSample(&image.jpeg);
    (void)jpegdec_test_get_jpeg_type(&image.jpeg);
    (void)JPEG_hasThumb(&image.jpeg);
    (void)JPEG_getThumbWidth(&image.jpeg);
    (void)JPEG_getThumbHeight(&image.jpeg);
  }
  (void)JPEG_getLastError(&image.jpeg);
  return 0;
}
