

gstmeta: gstmeta.c include/gstdetectionsmeta.c
	gcc -Wall -g3 -o $@.o $^ -Iinclude/ `pkg-config --libs --cflags gstreamer-1.0`


clean:
	rm -rf *.o