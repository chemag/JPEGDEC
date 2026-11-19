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

  const int formats[] = {RGB565_LITTLE_ENDIAN, RGB565_BIG_ENDIAN,
                         RGB8888, EIGHT_BIT_GRAYSCALE};
  const int scales[] = {0, JPEG_SCALE_HALF, JPEG_SCALE_QUARTER,
                        JPEG_SCALE_EIGHTH};
  const int format = formats[jpegdec_fuzz::Option(data, size, 0) % 4];
  const int scale = scales[jpegdec_fuzz::Option(data, size, 1) % 4];
  JPEG_setPixelType(&image.jpeg, format);
  JPEG_setMaxOutputSize(&image.jpeg,
                           1 + jpegdec_fuzz::Option(data, size, 2) % 8);

  if ((jpegdec_fuzz::Option(data, size, 4) & 1) != 0 && scale == 0 &&
      jpegdec_test_get_jpeg_type(&image.jpeg) != JPEG_MODE_PROGRESSIVE) {
    // Crop to a rectangle chosen by four more trailer bytes; the decoder
    // rounds it to whole MCUs. Crop areas are in source pixels and the
    // decoder does not map them to scaled output (progressive images always
    // decode at 1/8 scale), so only unscaled baseline decodes are cropped.
    const int width = image.jpeg.iWidth;
    const int height = image.jpeg.iHeight;
    JPEG_setCropArea(&image.jpeg,
                        jpegdec_fuzz::Option(data, size, 5) * width / 256,
                        jpegdec_fuzz::Option(data, size, 6) * height / 256,
                        1 + jpegdec_fuzz::Option(data, size, 7) * width / 256,
                        1 + jpegdec_fuzz::Option(data, size, 8) * height / 256);
  }

  std::vector<uint8_t> framebuffer;
  if ((jpegdec_fuzz::Option(data, size, 3) & 1) != 0 && scale == 0) {
    // The framebuffer path writes complete MCU rows. Supply MCU-padded
    // storage rather than treating required edge padding as a decoder bug.
    const size_t width =
        (static_cast<size_t>(image.jpeg.iWidth) + 15) & ~size_t(15);
    const size_t height =
        (static_cast<size_t>(image.jpeg.iHeight) + 15) & ~size_t(15);
    const size_t bpp = format == RGB8888               ? 4
                       : format == EIGHT_BIT_GRAYSCALE ? 1
                                                                   : 2;
    framebuffer.resize(width * height * bpp);
    JPEG_setFramebuffer(&image.jpeg, framebuffer.data());
  }
  (void)JPEG_decode(&image.jpeg, 0, 0, scale);
  (void)JPEG_getLastError(&image.jpeg);
  return 0;
}
