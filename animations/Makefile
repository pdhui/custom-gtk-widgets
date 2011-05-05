all: animations

PKGS = gtk+-3.0

FILES = \
	gb-animation.c \
	gb-animation.h \
	gb-frame-source.c \
	gb-frame-source.h \
	gb-timeout-interval.c \
	gb-timeout-interval.h \
	main.c

animations: $(FILES)
	$(CC) -g -o $@ -Wall -Werror $(FILES) $(shell pkg-config --cflags --libs $(PKGS))

clean:
	rm -f animations
