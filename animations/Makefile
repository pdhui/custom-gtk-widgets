all: animations animbin

PKGS = gtk+-3.0

FILES = \
	gb-animation.c \
	gb-animation.h \
	gb-frame-source.c \
	gb-frame-source.h \
	gb-timeout-interval.c \
	gb-timeout-interval.h

animations: $(FILES) main.c
	$(CC) -g -o $@ -Wall -Werror $(FILES) main.c $(shell pkg-config --cflags --libs $(PKGS))

animbin: $(FILES) animbin.c gb-anim-bin.c gb-anim-bin.h
	$(CC) -g -o $@ -Wall -Werror $(FILES) animbin.c gb-anim-bin.c $(shell pkg-config --cflags --libs $(PKGS))

clean:
	rm -f animations animbin
