all: animations animbin

PKGS = gtk+-3.0

FILES = \
	gb-animation.c \
	gb-animation.h \
	gb-frame-source.c \
	gb-frame-source.h \
	gb-timeout-interval.c \
	gb-timeout-interval.h

OBJECTS =
OBJECTS += gb-animation.o
OBJECTS += gb-frame-source.o
OBJECTS += gb-timeout-interval.o

animations: $(FILES) main.c
	$(CC) -g -o $@ $(FILES) main.c $(shell pkg-config --cflags --libs $(PKGS))

%.o: %.c %.h
	$(CC) -g -c -o $@ $*.c $(shell pkg-config --cflags $(PKGS))

animbin: $(OBJECTS) gb-anim-bin.o animbin.c
	$(CC) -g -o $@ animbin.c $(OBJECTS) gb-anim-bin.o $(shell pkg-config --cflags --libs $(PKGS))

clean:
	rm -f animations animbin *.o
