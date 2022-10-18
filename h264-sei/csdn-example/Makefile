

main:
	LD_LIBRARY_PATH=h264bitstream/.libs/:${LD_LIBRARY_PATH} \
	gcc *.c -o main.o \
		-g3 -Wall --pedantic \
		-lm `pkg-config --libs --cflags gstreamer-1.0 gstreamer-app-1.0 gstreamer-rtp-1.0 gstreamer-codecparsers-1.0 glib-2.0` \
		-Ih264bitstream/   -L ./h264bitstream/.libs/ -lh264bitstream