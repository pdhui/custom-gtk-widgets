all: bdbliststore

PKGS = gtk+-2.0
FILES = main.c bdb-list-store.c

bdbliststore: $(FILES)
	$(CC) -g -o $@ -Wall $(FILES) `pkg-config --libs --cflags $(PKGS)` -ldb-4.6

clean:
	rm -rf bdbliststore
