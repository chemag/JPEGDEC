CFLAGS=-D__LINUX__ -Wall -O2
PORTABLE_FLAGS=-DJPEGDEC_NO_ARDUINO -DNO_SIMD -Wall -Wextra -Werror -O2
ASAN_FLAGS=-fsanitize=address -fno-omit-frame-pointer -O1 -g -Werror=vla
ALIGN_FLAGS=-DNO_SIMD -fsanitize=alignment -fno-sanitize-recover=alignment -fno-omit-frame-pointer -O1 -g
SHIFT_FLAGS=-fsanitize=shift -fno-sanitize-recover=shift -fno-omit-frame-pointer -O1 -g
OVERFLOW_FLAGS=-fsanitize=signed-integer-overflow -fno-sanitize-recover=signed-integer-overflow -fno-omit-frame-pointer -O1 -g
BOUNDS_FLAGS=-fsanitize=bounds -fno-sanitize-recover=bounds -fno-omit-frame-pointer -O1 -g
LIBS=-lm -lpthread

all: libJPEGDEC.a

libJPEGDEC.a: JPEGDEC.o jpegdec.o
	ar -rc libJPEGDEC.a JPEGDEC.o jpegdec.o

c_api_test: c_api_test.o jpegdec_asan.o
	$(CC) $(ASAN_FLAGS) c_api_test.o jpegdec_asan.o $(LIBS) -o c_api_test

cpp_api_test: cpp_api_test.o JPEGDEC_asan.o jpegdec_asan.o
	$(CXX) $(ASAN_FLAGS) cpp_api_test.o JPEGDEC_asan.o jpegdec_asan.o $(LIBS) -o cpp_api_test

progressive_test: progressive_test.o jpegdec_asan.o
	$(CC) $(ASAN_FLAGS) progressive_test.o jpegdec_asan.o $(LIBS) -o progressive_test

progressive_grayscale_test: progressive_grayscale_test.o jpegdec_asan.o
	$(CC) $(ASAN_FLAGS) progressive_grayscale_test.o jpegdec_asan.o $(LIBS) -o progressive_grayscale_test

yuv420p_test: yuv420p_test.o jpegdec_asan.o
	$(CC) $(ASAN_FLAGS) yuv420p_test.o jpegdec_asan.o $(LIBS) -o yuv420p_test

nv12_test: nv12_test.o jpegdec_asan.o
	$(CC) $(ASAN_FLAGS) nv12_test.o jpegdec_asan.o $(LIBS) -o nv12_test

rgb565_endian_test: rgb565_endian_test.o jpegdec_asan.o
	$(CC) $(ASAN_FLAGS) rgb565_endian_test.o jpegdec_asan.o $(LIBS) -o rgb565_endian_test

unaligned_read_test: unaligned_read_test.o jpegdec_align.o
	$(CC) $(ALIGN_FLAGS) unaligned_read_test.o jpegdec_align.o $(LIBS) -o unaligned_read_test

unaligned_output_test: unaligned_output_test.o jpegdec_align.o
	$(CC) $(ALIGN_FLAGS) unaligned_output_test.o jpegdec_align.o $(LIBS) -o unaligned_output_test

huffman_shift_test: huffman_shift_test.o jpegdec_shift.o
	$(CC) $(SHIFT_FLAGS) huffman_shift_test.o jpegdec_shift.o $(LIBS) -o huffman_shift_test

exif_long_test: exif_long_test.o jpegdec_overflow.o
	$(CC) $(OVERFLOW_FLAGS) exif_long_test.o jpegdec_overflow.o $(LIBS) -o exif_long_test

exif_ifd_test: exif_ifd_test.o jpegdec_asan.o
	$(CC) $(ASAN_FLAGS) exif_ifd_test.o jpegdec_asan.o $(LIBS) -o exif_ifd_test

dqt_length_test: dqt_length_test.o jpegdec_asan.o
	$(CC) $(ASAN_FLAGS) dqt_length_test.o jpegdec_asan.o $(LIBS) -o dqt_length_test

subsampling_test: subsampling_test.o jpegdec_asan.o
	$(CC) $(ASAN_FLAGS) subsampling_test.o jpegdec_asan.o $(LIBS) -o subsampling_test

huffman_oversubscribed_test: huffman_oversubscribed_test.o jpegdec_asan.o
	$(CC) $(ASAN_FLAGS) huffman_oversubscribed_test.o jpegdec_asan.o $(LIBS) -o huffman_oversubscribed_test

progressive_scan_test: progressive_scan_test.o jpegdec_asan.o
	$(CC) $(ASAN_FLAGS) progressive_scan_test.o jpegdec_asan.o $(LIBS) -o progressive_scan_test

vlc_exhausted_test: vlc_exhausted_test.o jpegdec_asan.o
	$(CC) $(ASAN_FLAGS) vlc_exhausted_test.o jpegdec_asan.o $(LIBS) -o vlc_exhausted_test

dc_predictor_test: dc_predictor_test.o jpegdec_overflow.o
	$(CC) $(OVERFLOW_FLAGS) dc_predictor_test.o jpegdec_overflow.o $(LIBS) -o dc_predictor_test

quarter_rgb8888_test: quarter_rgb8888_test.o jpegdec_asan.o
	$(CC) $(ASAN_FLAGS) quarter_rgb8888_test.o jpegdec_asan.o $(LIBS) -o quarter_rgb8888_test

huffman_table_id_test: huffman_table_id_test.o jpegdec_shift.o
	$(CC) $(SHIFT_FLAGS) huffman_table_id_test.o jpegdec_shift.o $(LIBS) -o huffman_table_id_test

truncated_segment_test: truncated_segment_test.o jpegdec_overflow.o
	$(CC) $(OVERFLOW_FLAGS) truncated_segment_test.o jpegdec_overflow.o $(LIBS) -o truncated_segment_test

component_count_test: component_count_test.o jpegdec_bounds.o
	$(CC) $(BOUNDS_FLAGS) component_count_test.o jpegdec_bounds.o $(LIBS) -o component_count_test

huffman_table_index_test: huffman_table_index_test.o jpegdec_bounds.o
	$(CC) $(BOUNDS_FLAGS) huffman_table_index_test.o jpegdec_bounds.o $(LIBS) -o huffman_table_index_test

c_api_test.o: c_api_test.c ../src/JPEGDEC.h ../test_images/tulips.h
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c c_api_test.c

cpp_api_test.o: cpp_api_test.cpp ../src/JPEGDEC.h ../test_images/tulips.h
	$(CXX) $(CFLAGS) $(ASAN_FLAGS) -c cpp_api_test.cpp

progressive_test.o: progressive_test.c progressive_test_image.h ../src/JPEGDEC.h
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c progressive_test.c

progressive_grayscale_test.o: progressive_grayscale_test.c progressive_test_image.h ../src/JPEGDEC.h
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c progressive_grayscale_test.c

yuv420p_test.o: yuv420p_test.c ../src/JPEGDEC.h ../test_images/tulips.h
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c yuv420p_test.c

nv12_test.o: nv12_test.c ../src/JPEGDEC.h ../test_images/tulips.h
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c nv12_test.c

rgb565_endian_test.o: rgb565_endian_test.c ../src/JPEGDEC.h ../test_images/tulips.h ../test_images/zebra.h
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c rgb565_endian_test.c

unaligned_read_test.o: unaligned_read_test.c progressive_test_image.h ../src/JPEGDEC.h ../test_images/tulips.h
	$(CC) $(CFLAGS) $(ALIGN_FLAGS) -c unaligned_read_test.c

unaligned_output_test.o: unaligned_output_test.c ../src/JPEGDEC.h ../test_images/tulips.h
	$(CC) $(CFLAGS) $(ALIGN_FLAGS) -c unaligned_output_test.c

huffman_shift_test.o: huffman_shift_test.c progressive_test_image.h ../src/JPEGDEC.h ../test_images/tulips.h ../test_images/zebra.h
	$(CC) $(CFLAGS) $(SHIFT_FLAGS) -c huffman_shift_test.c

exif_long_test.o: exif_long_test.c ../src/JPEGDEC.h ../test_images/tulips.h
	$(CC) $(CFLAGS) $(OVERFLOW_FLAGS) -c exif_long_test.c

exif_ifd_test.o: exif_ifd_test.c ../src/JPEGDEC.h ../test_images/tulips.h
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c exif_ifd_test.c

dqt_length_test.o: dqt_length_test.c ../src/JPEGDEC.h
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c dqt_length_test.c

subsampling_test.o: subsampling_test.c ../src/JPEGDEC.h ../test_images/tulips.h
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c subsampling_test.c

huffman_oversubscribed_test.o: huffman_oversubscribed_test.c ../src/JPEGDEC.h ../test_images/tulips.h
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c huffman_oversubscribed_test.c

progressive_scan_test.o: progressive_scan_test.c progressive_test_image.h ../src/JPEGDEC.h
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c progressive_scan_test.c

vlc_exhausted_test.o: vlc_exhausted_test.c vlc_exhausted_image.h ../src/JPEGDEC.h
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c vlc_exhausted_test.c

dc_predictor_test.o: dc_predictor_test.c dc_predictor_image.h ../src/JPEGDEC.h
	$(CC) $(CFLAGS) $(OVERFLOW_FLAGS) -c dc_predictor_test.c

quarter_rgb8888_test.o: quarter_rgb8888_test.c quarter_rgb8888_image.h ../src/JPEGDEC.h
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c quarter_rgb8888_test.c

huffman_table_id_test.o: huffman_table_id_test.c ../src/JPEGDEC.h ../test_images/tulips.h
	$(CC) $(CFLAGS) $(SHIFT_FLAGS) -c huffman_table_id_test.c

truncated_segment_test.o: truncated_segment_test.c ../src/JPEGDEC.h
	$(CC) $(CFLAGS) $(OVERFLOW_FLAGS) -c truncated_segment_test.c

component_count_test.o: component_count_test.c ../src/JPEGDEC.h ../test_images/tulips.h
	$(CC) $(CFLAGS) $(BOUNDS_FLAGS) -c component_count_test.c

huffman_table_index_test.o: huffman_table_index_test.c ../src/JPEGDEC.h ../test_images/tulips.h
	$(CC) $(CFLAGS) $(BOUNDS_FLAGS) -c huffman_table_index_test.c

JPEGDEC.o: ../src/JPEGDEC.cpp ../src/JPEGDEC.h
	$(CXX) $(CFLAGS) -c ../src/JPEGDEC.cpp

jpegdec.o: ../src/jpegdec.c ../src/jpeg.inl ../src/JPEGDEC.h
	$(CC) $(CFLAGS) -c ../src/jpegdec.c

JPEGDEC_asan.o: ../src/JPEGDEC.cpp ../src/JPEGDEC.h
	$(CXX) $(CFLAGS) $(ASAN_FLAGS) -c ../src/JPEGDEC.cpp -o JPEGDEC_asan.o

jpegdec_asan.o: ../src/jpegdec.c ../src/jpeg.inl ../src/JPEGDEC.h
	$(CC) $(CFLAGS) $(ASAN_FLAGS) -c ../src/jpegdec.c -o jpegdec_asan.o

jpegdec_align.o: ../src/jpegdec.c ../src/jpeg.inl ../src/JPEGDEC.h
	$(CC) $(CFLAGS) $(ALIGN_FLAGS) -c ../src/jpegdec.c -o jpegdec_align.o

jpegdec_shift.o: ../src/jpegdec.c ../src/jpeg.inl ../src/JPEGDEC.h
	$(CC) $(CFLAGS) $(SHIFT_FLAGS) -c ../src/jpegdec.c -o jpegdec_shift.o

jpegdec_overflow.o: ../src/jpegdec.c ../src/jpeg.inl ../src/JPEGDEC.h
	$(CC) $(CFLAGS) $(OVERFLOW_FLAGS) -c ../src/jpegdec.c -o jpegdec_overflow.o

jpegdec_bounds.o: ../src/jpegdec.c ../src/jpeg.inl ../src/JPEGDEC.h
	$(CC) $(CFLAGS) $(BOUNDS_FLAGS) -c ../src/jpegdec.c -o jpegdec_bounds.o

jpegdec_no_arduino.o: ../src/jpegdec.c ../src/jpeg.inl ../src/JPEGDEC.h
	$(CC) $(PORTABLE_FLAGS) -c ../src/jpegdec.c -o jpegdec_no_arduino.o

JPEGDEC_no_arduino.o: ../src/JPEGDEC.cpp ../src/JPEGDEC.h
	$(CXX) $(PORTABLE_FLAGS) -c ../src/JPEGDEC.cpp -o JPEGDEC_no_arduino.o

test: c_api_test cpp_api_test progressive_test progressive_grayscale_test yuv420p_test nv12_test rgb565_endian_test unaligned_read_test unaligned_output_test huffman_shift_test exif_long_test exif_ifd_test huffman_table_id_test truncated_segment_test component_count_test dqt_length_test huffman_table_index_test subsampling_test huffman_oversubscribed_test progressive_scan_test vlc_exhausted_test dc_predictor_test quarter_rgb8888_test jpegdec_no_arduino.o JPEGDEC_no_arduino.o
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./c_api_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./cpp_api_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./progressive_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./progressive_grayscale_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./yuv420p_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./nv12_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./rgb565_endian_test
	UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ./unaligned_read_test
	UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ./unaligned_output_test
	UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ./huffman_shift_test
	UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ./exif_long_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./exif_ifd_test
	UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ./huffman_table_id_test
	UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ./truncated_segment_test
	UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ./component_count_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./dqt_length_test
	UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ./huffman_table_index_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./subsampling_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./huffman_oversubscribed_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./progressive_scan_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./vlc_exhausted_test
	UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ./dc_predictor_test
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 ./quarter_rgb8888_test

clean:
	rm -rf *.o libJPEGDEC.a c_api_test cpp_api_test progressive_test progressive_grayscale_test yuv420p_test nv12_test rgb565_endian_test unaligned_read_test unaligned_output_test huffman_shift_test exif_long_test exif_ifd_test huffman_table_id_test truncated_segment_test component_count_test dqt_length_test huffman_table_index_test subsampling_test huffman_oversubscribed_test progressive_scan_test vlc_exhausted_test dc_predictor_test quarter_rgb8888_test
