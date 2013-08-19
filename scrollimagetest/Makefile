all: scrollimagetest scrollimagetest-clutter

GTK_FILES = \
gb-animation.c \
gb-animation.h \
gb-frame-source.c \
gb-frame-source.h \
img-view.c \
img-view.h \
main.c

CLUTTER_FILES = \
clutter-main.c

scrollimagetest: $(GTK_FILES)
	$(CC) -o $@ -Wall $(GTK_FILES) $(shell pkg-config --cflags --libs gtk+-3.0)

scrollimagetest-clutter: $(CLUTTER_FILES)
	$(CC) -o $@ -Wall $(CLUTTER_FILES) $(shell pkg-config --cflags --libs clutter-1.0)

clean:
	rm -f scrollimagetest scrollimagetest-clutter
