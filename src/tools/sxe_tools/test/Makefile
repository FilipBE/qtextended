all: sandboxed proc_keys suid

sandboxed: sandboxed.c
	$(CC) -Wall -o sandboxed sandboxed.c

proc_keys: proc_keys.c lidsif_usr.h
	$(CC) -Wall -o proc_keys proc_keys.c

suid: suid.c lidsif_usr.h
	$(CC) -Wall -o suid suid.c

.PHONY: all
