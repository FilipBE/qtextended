CC = /opt/toolchains/greenphone/gcc-3.4.5-glibc-2.3.6/arm-iwmmxt-linux-gnu/bin/arm-iwmmxt-linux-gnu-gcc
OBJECTS = mmx32_blur.o mmx16_blur.o mmx16_blend.o plugin.o mmx32_blend.o mmx16_gray.o mmx_memory.o mmx32_scale.o mmx16_color.o

%.o: %.c
	$(CC) -O2 -c -I. -I../.. -o $@ $<

first: libmmx.so

libmmx.so: $(OBJECTS)
	$(CC) -shared -O2 $(OBJECTS) -o libmmx.so

clean:
	rm $(OBJECTS)
	rm libmmx.so
