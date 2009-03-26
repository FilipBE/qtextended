
linuxrc: linuxrc.o busybox-1.2.1/modutils/insmod.o busybox-1.2.1/libbb/libbb.a
	$(CROSS)-gcc -static -o linuxrc linuxrc.o busybox-1.2.1/modutils/insmod.o busybox-1.2.1/libbb/libbb.a
	$(CROSS)-strip --strip-all linuxrc

linuxrc.o: linuxrc.c
	$(CROSS)-gcc -c -o linuxrc.o linuxrc.c

