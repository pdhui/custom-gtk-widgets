all: gtkmodebutton

SOURCES = gtkmodebutton.c main.c gtkmodemarshal.c
HEADERS = gtkmodebutton.h
PKGS = glib-2.0 gtk+-2.0

gtkmodebutton: $(SOURCES) $(HEADERS)
	gcc -g -Wall -o $@ $(SOURCES) `pkg-config --libs --cflags $(PKGS)`

clean:
	rm -rf gtkmodebutton
