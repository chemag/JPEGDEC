/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Ignore an Exif thumbnail whose JPEGInterchangeFormat offset points outside
 * the APP1 segment. Turning a near-INT_MAX offset into a file offset used to
 * overflow int. Build with UBSan signed-overflow checking.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/JPEGDEC.h"
#include "../test_images/tulips.h"

/*
 * Big-endian APP1 Exif: IFD0 holds Orientation, IFD1 holds one
 * JPEGInterchangeFormat (tag 513) LONG whose value is patched per case.
 */
static const uint8_t kExif[] = {
    0xff, 0xe1, 0x00, 0x34, 'E',  'x',  'i',  'f',  0x00, 0x00, /* header */
    'M',  'M',  0x00, 0x2a, 0x00, 0x00, 0x00, 0x08,             /* TIFF */
    0x00, 0x01, 0x01, 0x12, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, /* IFD0 */
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1a,             /* next */
    0x00, 0x01, 0x02, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, /* IFD1 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
/* Offset of the tag 513 value within kExif. */
enum { kThumbOffsetPos = 46 };

static int Draw(JPEGDRAW* draw) {
  return draw != NULL && draw->pPixels != NULL;
}

/* Returns 1 if the parse matches expect_thumb and the image still decodes. */
static int Check(uint32_t thumb_offset, int expect_thumb) {
  const size_t size = 2 + sizeof(kExif) + sizeof(tulips) - 2;
  uint8_t* jpeg_data = malloc(size);
  JPEGIMAGE jpeg;
  int ok = 0;

  if (jpeg_data == NULL) return 0;
  jpeg_data[0] = 0xff; /* SOI, then Exif, then the fixture after its SOI */
  jpeg_data[1] = 0xd8;
  memcpy(jpeg_data + 2, kExif, sizeof(kExif));
  jpeg_data[2 + kThumbOffsetPos] = (uint8_t)(thumb_offset >> 24);
  jpeg_data[2 + kThumbOffsetPos + 1] = (uint8_t)(thumb_offset >> 16);
  jpeg_data[2 + kThumbOffsetPos + 2] = (uint8_t)(thumb_offset >> 8);
  jpeg_data[2 + kThumbOffsetPos + 3] = (uint8_t)thumb_offset;
  memcpy(jpeg_data + 2 + sizeof(kExif), tulips + 2, sizeof(tulips) - 2);

  if (JPEGDEC_openRAM(&jpeg, jpeg_data, (int)size, Draw) == 0) {
    fprintf(stderr, "open failed: %d\n", JPEGDEC_getLastError(&jpeg));
  } else if (JPEGDEC_hasThumb(&jpeg) != expect_thumb) {
    fprintf(stderr, "offset 0x%08x: hasThumb %d, expected %d\n",
            (unsigned)thumb_offset, JPEGDEC_hasThumb(&jpeg), expect_thumb);
  } else if (JPEGDEC_getOrientation(&jpeg) != 1) {
    fprintf(stderr, "orientation not parsed\n");
  } else if (JPEGDEC_decode(&jpeg, 0, 0, 0) == 0) {
    fprintf(stderr, "decode failed: %d\n", JPEGDEC_getLastError(&jpeg));
  } else {
    ok = 1;
  }
  JPEGDEC_close(&jpeg);
  free(jpeg_data);
  return ok;
}

int main(void) {
  /* Inside the 44-byte TIFF body: kept. Near INT_MAX, or negative: dropped. */
  if (!Check(0x20, 1) || !Check(0x7ffffffe, 0) || !Check(0xfffffff0, 0)) {
    return 1;
  }
  printf("Out-of-segment Exif thumbnail offsets ignored\n");
  return 0;
}
