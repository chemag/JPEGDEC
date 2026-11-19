CFLAGS=-c -Wall -O2 -ggdb -D__LINUX__ -I../src
LIBS = -lm -lpthread

all: libJPEGDEC.a

libJPEGDEC.a: JPEGDEC.o
	ar -rc libJPEGDEC.a JPEGDEC.o ;\
	sudo cp libJPEGDEC.a /usr/local/lib ;\
	sudo cp ../src/JPEGDEC.h /usr/local/include

JPEGDEC.o: ../src/JPEGDEC.cpp ../src/JPEGDEC.h ../src/jpeg.inl
	$(CXX) $(CFLAGS) ../src/JPEGDEC.cpp

# Unit tests: "make test". Each test links against the portable C core
# (JPEGDEC_NO_ARDUINO, NO_SIMD) built with AddressSanitizer and
# UndefinedBehaviorSanitizer. The sanitizer flags need Clang.
TEST_CC ?= clang
TEST_CXX ?= clang++
TEST_CFLAGS = -DJPEGDEC_NO_ARDUINO -DNO_SIMD -I../src -Wall -O1 -g \
	-fno-omit-frame-pointer -fsanitize=address,undefined \
	-fno-sanitize-recover=all -fsanitize-ignorelist=ubsan_ignorelist.txt
TESTS =
TESTS += c_api_test
TESTS += cpp_api_test
TESTS += progressive_test
TESTS += yuv420p_test
TESTS += nv12_test
TESTS += rgb565_endian_test
TESTS += unaligned_read_test
TESTS += huffman_shift_test
TESTS += progressive_grayscale_test
TESTS += exif_long_test
TESTS += huffman_table_id_test
TESTS += truncated_segment_test
TESTS += unaligned_output_test
TESTS += exif_ifd_test
TESTS += component_count_test
TESTS += dqt_length_test
TESTS += huffman_table_index_test
TESTS += subsampling_test
TESTS += huffman_oversubscribed_test
TESTS += progressive_scan_test
TESTS += vlc_exhausted_test
TESTS += dc_predictor_test
TESTS += quarter_rgb8888_test
TESTS += exif_thumb_offset_test
TESTS += edge_444_test
TESTS += edge_422_test
TESTS += rgb8888_order_test
TESTS += crop_area_test
TESTS += crop_callback_test
TESTS += crop_scale_test

jpegdec_test_core.o: jpegdec_test_core.c ../src/JPEGDEC.h ../src/jpeg.inl ubsan_ignorelist.txt
	$(TEST_CC) $(TEST_CFLAGS) -c jpegdec_test_core.c -o $@

%_test: %_test.c jpegdec_test_core.o $(wildcard *.h)
	$(TEST_CC) $(TEST_CFLAGS) $< jpegdec_test_core.o -lm -o $@

%_test: %_test.cpp ../src/JPEGDEC.cpp ../src/JPEGDEC.h ../src/jpeg.inl
	$(TEST_CXX) $(TEST_CFLAGS) $< ../src/JPEGDEC.cpp -lm -o $@

test: $(TESTS)
	@for t in $(TESTS); do \
		echo "./$$t"; \
		ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
		UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ./$$t || exit 1; \
	done

.PHONY: all test clean

clean:
	rm -f *.o libJPEGDEC.a $(TESTS)
