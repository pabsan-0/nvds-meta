
all: rtp-one-byte

rtp-one-byte: rtp-one-byte.c
	gcc -Wall -g3 -o $@.o $^ `pkg-config --libs --cflags gstreamer-1.0 gstreamer-app-1.0 gstreamer-rtp-1.0`

clean:
	rm -rf *.o