OBJECTS = dummy_plugin.o

%.o: %.c
	$(CC) -O2 -c -I. -I../.. -o $@ $<

first: libdummy.so

libdummy.so: $(OBJECTS)
	$(CC) -shared -O2 $(OBJECTS) -o libdummy.so

clean:
	rm $(OBJECTS)
	rm libdummy.so
