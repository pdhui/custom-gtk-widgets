all: test

FILES = *.c

PKGS = `pkg-config --cflags --libs gtk+-2.0`

test: $(FILES)
	gcc -g -o $@ -Wall $(FILES) $(PKGS)

