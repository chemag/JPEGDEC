// SPDX-License-Identifier: Apache-2.0
#include <vector>

#include "jpegdec_fuzz_common.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size > jpegdec_fuzz::kMaxInputSize) return 0;
  jpegdec_fuzz::Image image(data, size);
  if (!image.Open() || !image.BoundedDimensions()) return 0;

  const int height = image.jpeg.iHeight;
  const int chroma_height = (height + 1) / 2;
  const int chroma_width = (image.jpeg.iWidth + 1) / 2;
  const int y_pitch =
      image.jpeg.iWidth + jpegdec_fuzz::Option(data, size, 1) % 8;
  const int chroma_padding = jpegdec_fuzz::Option(data, size, 2) % 8;
  std::vector<uint8_t> y(static_cast<size_t>(y_pitch) * height);

  if ((jpegdec_fuzz::Option(data, size, 0) & 1) == 0) {
    const int chroma_pitch = chroma_width + chroma_padding;
    std::vector<uint8_t> cb(static_cast<size_t>(chroma_pitch) * chroma_height);
    std::vector<uint8_t> cr(static_cast<size_t>(chroma_pitch) * chroma_height);
    (void)JPEGDEC_decodeYUV420P(&image.jpeg, y.data(), y_pitch, cb.data(),
                                chroma_pitch, cr.data(), chroma_pitch);
  } else {
    const int uv_pitch = chroma_width * 2 + chroma_padding;
    std::vector<uint8_t> uv(static_cast<size_t>(uv_pitch) * chroma_height);
    (void)JPEGDEC_decodeNV12(&image.jpeg, y.data(), y_pitch, uv.data(),
                             uv_pitch);
  }
  (void)JPEGDEC_getLastError(&image.jpeg);
  return 0;
}
