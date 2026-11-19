// SPDX-License-Identifier: Apache-2.0
#ifndef JPEGDEC_FUZZ_COMMON_H_
#define JPEGDEC_FUZZ_COMMON_H_

#include <cstddef>
#include <cstdint>
#include <vector>

#include "jpegdec_test.h"

namespace jpegdec_fuzz {

constexpr size_t kMaxInputSize = 1024 * 1024;
constexpr size_t kMaxPixels = 1024 * 1024;
constexpr int kMaxDimension = 4096;

// Own writable input for the C API without modifying libFuzzer's buffer.
class Image {
 public:
  Image(const uint8_t* data, size_t size) : input_(data, data + size) {}
  ~Image() { JPEG_close(&jpeg); }
  Image(const Image&) = delete;
  Image& operator=(const Image&) = delete;

  bool Open(JPEG_DRAW_CALLBACK* draw = nullptr) {
    return JPEG_openRAM(&jpeg, input_.data(),
                           static_cast<int>(input_.size()), draw) != 0;
  }

  bool BoundedDimensions() const {
    const int width = jpeg.iWidth;
    const int height = jpeg.iHeight;
    return width > 0 && height > 0 && width <= kMaxDimension &&
           height <= kMaxDimension &&
           static_cast<size_t>(width) <= kMaxPixels / height;
  }

  JPEGIMAGE jpeg = {};

 private:
  std::vector<uint8_t> input_;
};

// Keep the JPEG input intact. Trailer bytes also select API configurations,
// so appended bytes after EOI can explore options without corrupting a seed.
inline uint8_t Option(const uint8_t* data, size_t size, size_t offset) {
  return size > offset ? data[size - offset - 1] : 0;
}

}  // namespace jpegdec_fuzz
#endif  // JPEGDEC_FUZZ_COMMON_H_
