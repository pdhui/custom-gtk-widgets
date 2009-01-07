all: gtkmodebutton ModeButton.dll

SOURCES = gtkmodebutton.c main.c gtkmodemarshal.c
HEADERS = gtkmodebutton.h
PKGS = glib-2.0 gtk+-2.0

gtkmodebutton: $(SOURCES) $(HEADERS)
	gcc -g -Wall -o $@ $(SOURCES) `pkg-config --libs --cflags $(PKGS)`

ModeButton.dll:
	gmcs ModeButton.cs -out:$@ -target:library -pkg:gtk-sharp-2.0

clean:
	rm -rf gtkmodebutton
