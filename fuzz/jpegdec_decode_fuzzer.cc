// SPDX-License-Identifier: Apache-2.0
#include <cstdlib>
#include <vector>

#include "jpegdec_fuzz_common.h"

namespace {

// Inspect emitted pixels, without introducing display or filesystem code.
int Draw(JPEGDRAW* draw) {
  if (draw == nullptr || draw->pPixels == nullptr || draw->iWidth < 0 ||
      draw->iWidthUsed < 0 || draw->iWidthUsed > draw->iWidth ||
      draw->iHeight < 0 || draw->iWidth > 4096 || draw->iHeight > 16 ||
      (draw->iBpp != 8 && draw->iBpp != 16 && draw->iBpp != 32)) {
    std::abort();
  }
  const size_t pitch = static_cast<size_t>(draw->iWidth) * draw->iBpp / 8;
  const size_t row_size =
      static_cast<size_t>(draw->iWidthUsed) * draw->iBpp / 8;
  const uint8_t* pixels = reinterpret_cast<const uint8_t*>(draw->pPixels);
  volatile uint8_t checksum = 0;
  for (int y = 0; y < draw->iHeight; ++y) {
    for (size_t x = 0; x < row_size; ++x) {
      checksum = static_cast<uint8_t>(checksum ^ pixels[y * pitch + x]);
    }
  }
  (void)checksum;
  return 1;
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size > jpegdec_fuzz::kMaxInputSize) return 0;
  jpegdec_fuzz::Image image(data, size);
  if (!image.Open(Draw) || !image.BoundedDimensions()) return 0;

  const int formats[] = {JPEGD_PIXEL_RGB565_LE, JPEGD_PIXEL_RGB565_BE,
                         JPEGD_PIXEL_RGB8888, JPEGD_PIXEL_EIGHT_BIT_GRAYSCALE};
  const int scales[] = {0, JPEG_SCALE_HALF, JPEG_SCALE_QUARTER,
                        JPEG_SCALE_EIGHTH};
  const int format = formats[jpegdec_fuzz::Option(data, size, 0) % 4];
  const int scale = scales[jpegdec_fuzz::Option(data, size, 1) % 4];
  JPEGDEC_setPixelType(&image.jpeg, format);
  JPEGDEC_setMaxOutputSize(&image.jpeg,
                           1 + jpegdec_fuzz::Option(data, size, 2) % 8);

  std::vector<uint8_t> framebuffer;
  if ((jpegdec_fuzz::Option(data, size, 3) & 1) != 0 && scale == 0) {
    // The framebuffer path writes complete MCU rows. Supply MCU-padded
    // storage rather than treating required edge padding as a decoder bug.
    const size_t width =
        (static_cast<size_t>(image.jpeg.iWidth) + 15) & ~size_t(15);
    const size_t height =
        (static_cast<size_t>(image.jpeg.iHeight) + 15) & ~size_t(15);
    const size_t bpp = format == JPEGD_PIXEL_RGB8888               ? 4
                       : format == JPEGD_PIXEL_EIGHT_BIT_GRAYSCALE ? 1
                                                                   : 2;
    framebuffer.resize(width * height * bpp);
    JPEGDEC_setFramebuffer(&image.jpeg, framebuffer.data());
  }
  (void)JPEGDEC_decode(&image.jpeg, 0, 0, scale);
  (void)JPEGDEC_getLastError(&image.jpeg);
  return 0;
}
