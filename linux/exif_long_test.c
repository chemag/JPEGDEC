/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Parse an Exif TIFF LONG value whose most significant byte is 0xff. Build
 * with UBSan signed-integer-overflow checking.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/JPEGDEC.h"
#include "../test_images/tulips.h"

/* APP1 Exif with one big-endian IFD entry: orientation, LONG, 0xff000000. */
static const uint8_t kExif[] = {
    0xff, 0xe1, 0x00, 0x22, 'E',  'x',  'i',  'f',  0x00, 0x00, 'M',  'M',
    0x00, 0x2a, 0x00, 0x00, 0x00, 0x08, 0x00, 0x01, 0x01, 0x12, 0x00, 0x04,
    0x00, 0x00, 0x00, 0x01, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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

  if (JPEG_openRAM(&jpeg, jpeg_data, (int)size, Draw) == 0) {
    fprintf(stderr, "open failed: %d\n", JPEG_getLastError(&jpeg));
  } else if (JPEG_getWidth(&jpeg) != 640 ||
             JPEG_getHeight(&jpeg) != 480) {
    fprintf(stderr, "unexpected dimensions\n");
  } else if (JPEG_decode(&jpeg, 0, 0, 0) == 0) {
    fprintf(stderr, "decode failed: %d\n", JPEG_getLastError(&jpeg));
  } else {
    result = 0;
  }
  JPEG_close(&jpeg);
  free(jpeg_data);
  if (result == 0) printf("Exif LONG with high byte 0xff parsed\n");
  return result;
}
