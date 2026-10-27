// SPDX-License-Identifier: Apache-2.0
#include "jpegdec_fuzz_common.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size > jpegdec_fuzz::kMaxInputSize) return 0;
  jpegdec_fuzz::Image image(data, size);
  if (image.Open()) {
    (void)JPEGDEC_getWidth(&image.jpeg);
    (void)JPEGDEC_getHeight(&image.jpeg);
    (void)JPEGDEC_getOrientation(&image.jpeg);
    (void)JPEGDEC_getBpp(&image.jpeg);
    (void)JPEGDEC_getSubSample(&image.jpeg);
    (void)JPEGDEC_getJPEGType(&image.jpeg);
    (void)JPEGDEC_hasThumb(&image.jpeg);
    (void)JPEGDEC_getThumbWidth(&image.jpeg);
    (void)JPEGDEC_getThumbHeight(&image.jpeg);
  }
  (void)JPEGDEC_getLastError(&image.jpeg);
  return 0;
}
