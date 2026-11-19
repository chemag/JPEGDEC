/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Ignore Exif IFD offsets that point outside the APP1 segment, both far
 * outside the buffered data and just past the segment, into a following
 * marker that holds a well-formed IFD. Build with AddressSanitizer.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/JPEGDEC.h"
#include "../test_images/tulips.h"

/* APP1 Exif whose big-endian first-IFD offset is 0x40000000. */
static const uint8_t kExifFar[] = {
    0xff, 0xe1, 0x00, 0x16, 'E',  'x',  'i',  'f',  0x00, 0x00, 'M',  'M',
    0x00, 0x2a, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/*
 * APP1 Exif holding only the TIFF header, whose first-IFD offset (12) points
 * past the segment into an APP2 segment holding an IFD with Orientation 6.
 */
static const uint8_t kExifPastSegment[] = {
    0xff, 0xe1, 0x00, 0x10, 'E',  'x',  'i',  'f',  0x00, 0x00, /* APP1 */
    'M',  'M',  0x00, 0x2a, 0x00, 0x00, 0x00, 0x0c,             /* TIFF */
    0xff, 0xe2, 0x00, 0x14,                                     /* APP2 */
    0x00, 0x01, 0x01, 0x12, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, /* IFD */
    0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static int Draw(JPEGDRAW* draw) {
  return draw != NULL && draw->pPixels != NULL;
}

/* Open the fixture with `exif` inserted after SOI; returns 0 on success. */
static int Check(const char* name, const uint8_t* exif, size_t exif_size) {
  const size_t size = 2 + exif_size + sizeof(tulips) - 2;
  uint8_t* jpeg_data = malloc(size);
  JPEGIMAGE jpeg;
  int result = 1;

  if (jpeg_data == NULL) return 1;
  jpeg_data[0] = 0xff; /* SOI, then Exif, then the fixture after its SOI */
  jpeg_data[1] = 0xd8;
  memcpy(jpeg_data + 2, exif, exif_size);
  memcpy(jpeg_data + 2 + exif_size, tulips + 2, sizeof(tulips) - 2);

  if (JPEG_openRAM(&jpeg, jpeg_data, (int)size, Draw) == 0) {
    fprintf(stderr, "%s: open failed: %d\n", name, JPEG_getLastError(&jpeg));
  } else if (JPEG_hasThumb(&jpeg) || JPEG_getOrientation(&jpeg) != 0) {
    fprintf(stderr, "%s: out-of-segment IFD produced Exif data\n", name);
  } else if (JPEG_decode(&jpeg, 0, 0, 0) == 0) {
    fprintf(stderr, "%s: decode failed: %d\n", name,
            JPEG_getLastError(&jpeg));
  } else {
    result = 0;
  }
  JPEG_close(&jpeg);
  free(jpeg_data);
  return result;
}

int main(void) {
  if (Check("far", kExifFar, sizeof(kExifFar)) != 0 ||
      Check("past segment", kExifPastSegment, sizeof(kExifPastSegment)) != 0) {
    return 1;
  }
  printf("Out-of-segment Exif IFDs ignored\n");
  return 0;
}
