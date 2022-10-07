
all: continuous on-request

continuous: continuous.c
	gcc -Wall -g3 -o $@.o $^ `pkg-config --libs --cflags gstreamer-1.0 gstreamer-app-1.0`

on-request: on-request.c
	gcc -Wall -g3 -o $@.o $^ `pkg-config --libs --cflags gstreamer-1.0 gstreamer-app-1.0`

clean:
	rm -rf *.o