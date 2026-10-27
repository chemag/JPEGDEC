/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Ignore Exif IFD offsets that point outside the APP1 segment. Build with
 * AddressSanitizer.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/JPEGDEC.h"
#include "../test_images/tulips.h"

/* APP1 Exif whose big-endian first-IFD offset is 0x40000000. */
static const uint8_t kExif[] = {0xff, 0xe1, 0x00, 0x16, 'E',  'x',  'i',  'f',
                                0x00, 0x00, 'M',  'M',  0x00, 0x2a, 0x40, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static int Draw(JPEGDRAW* draw) {
  return draw != NULL && draw->pPixels != NULL;
}

int main(void) {
  const size_t size = 2 + sizeof(kExif) + sizeof(tulips) - 2;
  uint8_t* jpeg_data = malloc(size);
  JPEGIMAGE jpeg;
  int result = 1;

  if (jpeg_data == NULL) return 1;
  jpeg_data[0] = 0xff; /* SOI, then Exif, then the fixture after its SOI */
  jpeg_data[1] = 0xd8;
  memcpy(jpeg_data + 2, kExif, sizeof(kExif));
  memcpy(jpeg_data + 2 + sizeof(kExif), tulips + 2, sizeof(tulips) - 2);

  if (JPEGDEC_openRAM(&jpeg, jpeg_data, (int)size, Draw) == 0) {
    fprintf(stderr, "open failed: %d\n", JPEGDEC_getLastError(&jpeg));
  } else if (JPEGDEC_hasThumb(&jpeg) || JPEGDEC_getOrientation(&jpeg) != 0) {
    fprintf(stderr, "out-of-range IFD produced Exif data\n");
  } else if (JPEGDEC_decode(&jpeg, 0, 0, 0) == 0) {
    fprintf(stderr, "decode failed: %d\n", JPEGDEC_getLastError(&jpeg));
  } else {
    result = 0;
  }
  JPEGDEC_close(&jpeg);
  free(jpeg_data);
  if (result == 0) printf("Out-of-range Exif IFD ignored\n");
  return result;
}
